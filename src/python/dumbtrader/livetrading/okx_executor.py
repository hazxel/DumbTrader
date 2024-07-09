import threading
import queue
import time

from dumbtrader.strategy.strategy import *
from dumbtrader.api.okxws.client import *
from dumbtrader.api.okxws.constants import *
from dumbtrader.monitoring.telegram_bot import *

class OkxExecutor:
    def __init__(self, strategy, listen_trades_all_client, listen_order_client, place_order_client):
        self.strategy = strategy
        self.strategy_mutex = threading.Lock()

        self.listen_trades_all_client = listen_trades_all_client
        self.listen_order_client = listen_order_client
        self.place_order_client = place_order_client

        self.signal_queue = queue.Queue()
        # self.order_queue = queue.Queue()
        # self.cancel_queue = queue.Queue()

        self.positions = {}
        # self.last_pxs = {'ETH-USDT': .0}

        # no statistics for now
        self.logger = TelegramBotLogger()

    async def submit_order(self, order):
        print(f"submit {order.side} order of sz: {order.volume}, px: {order.px}")
        self.logger.log(f"submit {order.side} order of sz: {order.volume}, px: {order.px}")
        if order.side == OrderSide.BUY:
            order_side = OKX_SIDE.BUY
            pos_side = OKX_POS_SIDE.LONG
        elif order.side == OrderSide.SELL:
            order_side = OKX_SIDE.SELL
            pos_side = OKX_POS_SIDE.SHORT
        else:
            raise Exception(f"unsupported order side: {order.side}")
        
        if order.inst_id not in self.positions or self.positions[order.inst_id] == 0:
            pass # already set when checking sides
        elif self.positions[order.inst_id] > 0:
            pos_side = OKX_POS_SIDE.LONG
        elif self.positions[order.inst_id] < 0:
            pos_side = OKX_POS_SIDE.SHORT

        if order.type == OrderType.MARKET:
            order_type = OKX_ORD_TYPE.MKT
        elif order.type == OrderType.LIMIT:
            order_type = OKX_ORD_TYPE.LMT
        else:
            raise Exception(f"unsupported order type {order.type}")
        
        await self.place_order_client.submit_order(
            client_order_id=order.internal_id, 
            inst_id=order.inst_id,
            td_mode=OKX_TD_MODE.ISOLATED,
            side=order_side,
            pos_side=pos_side,
            order_type=order_type,
            sz=order.volume,
            px=order.px)

    async def cancel_order(self, order):
        print(f"canceling order: {order}")
        await self.place_order_client.cancel_order(
            client_order_id=order, 
            inst_id=self.listen_trades_all_client.inst_id) # problem here, may need to record cid-instid mapping
        

    # signals: [(sig, order1), ...]
    async def handle_signals(self, signals):
        for sig in signals:
            self.signal_queue.put(sig)
        # for sig, order in signals:
        #     if sig == Signal.SUBMIT:
        #         # await self.submit_order(order)
        #         self.order_queue.put(order)
        #     elif sig == Signal.WITHDRAW:
        #         # await self.cancel_order(order)
        #         self.cancel_queue.put(order) # actually id
        #     else:
        #         raise Exception(f"Unsupported signal: {sig}")
        signals.clear()

    async def listen_trades(self):
        while True:
            data = await self.listen_trades_all_client.recv_data()
            data['px'] = float(data['px'])
            data['sz'] = float(data['sz'])

            self.strategy_mutex.acquire()
            try:
                signals = self.strategy.on_px_change(data)
            finally:
                self.strategy_mutex.release()
            
            await self.handle_signals(signals)

    async def listen_orders(self):
        while True:
            data = await self.listen_order_client.recv_data()
            print(f"order at px: {data['px']}, status: {data['state']}, ordId: {data['ordId']}")
            self.logger.log(f"order at px: {data['px']}, status: {data['state']}, ordId: {data['ordId']}")

            if data['instId'] not in self.positions:
                self.positions[data['instId']] = .0
            if data['side'] == OKX_SIDE.BUY:
                self.positions[data['instId']] += float(data['fillSz'])
            elif data['side'] == OKX_SIDE.SELL:
                self.positions[data['instId']] -= float(data['fillSz'])

            if data['state'] != OKX_ORD_STATE.FILLED:
                continue

            self.strategy_mutex.acquire()
            try:
                signals = self.strategy.on_order_filled(data['clOrdId'])
            finally:
                self.strategy_mutex.release()
            
            print(f"triggered signal: {signals}")
            self.logger.log(f"triggered signal: {signals}")
            
            await self.handle_signals(signals)

    async def listen_queue(self):
        while True:
            if not self.signal_queue.empty():
                sig, order = self.signal_queue.get()
                if sig == Signal.SUBMIT:
                    await self.submit_order(order)
                elif sig == Signal.WITHDRAW:
                    await self.cancel_order(order)
                else:
                    raise Exception(f"Unsupported signal: {sig}")
            time.sleep(0.1)
            # if not self.order_queue.empty():
            #     order = self.order_queue.get()
            #     await self.submit_order(order)
            #     time.sleep(0.5)
            # if not self.cancel_queue.empty():
            #     order = self.cancel_queue.get()
            #     await self.cancel_order(order)
            #     time.sleep(0.5)

    async def run(self):
        # init strategy
        data = await self.listen_trades_all_client.recv_data()
        data['px'] = float(data['px'])
        data['sz'] = float(data['sz'])
        signals = self.strategy.on_start(data)
        await self.handle_signals(signals)

        tasks = [
            asyncio.create_task(self.listen_orders()),
            asyncio.create_task(self.listen_trades()),
            asyncio.create_task(self.listen_queue())
        ]
        await asyncio.gather(*tasks)
