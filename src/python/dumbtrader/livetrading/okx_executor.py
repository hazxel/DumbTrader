import threading
import queue
import time
from functools import wraps

from dumbtrader.strategy.strategy import *
from dumbtrader.api.okxws.client import *
from dumbtrader.api.okxws.constants import *
from dumbtrader.monitoring.telegram_bot import *

def run_in_thread(async_func):
    @wraps(async_func)
    def wrapper(*args, **kwargs):
        def runner():
            asyncio.run(async_func(*args, **kwargs))
        thread = threading.Thread(target=runner)
        # thread.start()
        return thread
    return wrapper

class OkxExecutor:
    def __init__(self, strategy, inst_id, inst_type, api_key, passphrase, secret_key):
        self.strategy = strategy
        self.inst_id = inst_id
        self.inst_type = inst_type
        self.api_key = api_key
        self.passphrase = passphrase
        self.secret_key = secret_key

        self.strategy_mutex = threading.Lock()

        self.listen_trades_all_client = None
        self.listen_order_client = None
        self.place_order_client = None

        self.signal_queue = queue.Queue()

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
        self.logger.log(f"canceling order: {order}")
        await self.place_order_client.cancel_order(
            client_order_id=order, 
            inst_id=self.listen_trades_all_client.inst_id) # problem here, may need to record cid-instid mapping
        

    # signals: [(sig, order1), ...]
    def handle_signals(self, signals):
        for sig in signals:
            self.signal_queue.put(sig)
        signals.clear()

    @run_in_thread
    async def listen_trades(self):
        async with OkxWsListenTradesAllClient(OKX_WS_URI.PUBLIC_BUSINESS, self.inst_id) as cli:
            self.listen_trades_all_client = cli

            # init strategy
            data = await self.listen_trades_all_client.recv_data()
            data['px'] = float(data['px'])
            data['sz'] = float(data['sz'])
            signals = self.strategy.on_start(data)

            self.handle_signals(signals)

            while True:
                data = await self.listen_trades_all_client.recv_data()
                data['px'] = float(data['px'])
                data['sz'] = float(data['sz'])

                self.strategy_mutex.acquire()
                try:
                    signals = self.strategy.on_px_change(data)
                finally:
                    self.strategy_mutex.release()

                self.handle_signals(signals)

    @run_in_thread
    async def listen_orders(self):
        async with OkxWsListenOrdersClient(OKX_WS_URI.PRIVATE_PAPER, self.api_key, self.passphrase, self.secret_key, self.inst_type, self.inst_id) as cli:
            self.listen_order_client = cli
            while True:
                data = await self.listen_order_client.recv_data()
                print(f"order update - px: {data['px']}, status: {data['state']}, ordId: {data['ordId']}")
                self.logger.log(f"order update - px: {data['px']}, status: {data['state']}, ordId: {data['ordId']}")

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
                
                self.handle_signals(signals)

    @run_in_thread
    async def listen_queue(self):
        async with OkxWsPlaceOrderClient(OKX_WS_URI.PRIVATE_PAPER, self.api_key, self.passphrase, self.secret_key) as cli:
            self.place_order_client = cli
            while True:
                if not self.signal_queue.empty():
                    print("take signal from queue")
                    self.logger.log("take signal from queue")
                    sig, order = self.signal_queue.get()
                    if sig == Signal.SUBMIT:
                        await self.submit_order(order)
                    elif sig == Signal.WITHDRAW:
                        await self.cancel_order(order)
                    else:
                        raise Exception(f"Unsupported signal: {sig}")
                time.sleep(0.1)

    def run(self):
        threads = []
        thread = self.listen_orders()
        threads.append(thread)
        thread.start()
        print("Waiting for OkxWsListenOrdersClient to be set...")
        while not self.listen_order_client:
            time.sleep(1)
        print("done.")

        thread = self.listen_queue()
        threads.append(thread)
        thread.start()
        print("Waiting for OkxWsPlaceOrderClient to be set...")
        while not self.place_order_client:
            time.sleep(1)
        print("done.")
    
        thread = self.listen_trades()
        threads.append(thread)
        thread.start()

        for t in threads:
            t.join()
