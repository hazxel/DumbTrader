import threading
import time
from functools import wraps

from dumbtrader.strategy.strategy import *
from dumbtrader.api.okxws.client import *
from dumbtrader.api.okxws.constants import *
from dumbtrader.monitoring.logger import *

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
        
        self.listen_trades_all_client = None
        self.listen_order_client = None
        self.place_order_client = None

        self.strategy_mutex = threading.Lock()
        self.signal_queue = None
        self.positions = {}
        # self.last_pxs = {'ETH-USDT': .0}

        # TODO: no other statistics for now
        self.logger = VersatileLogger(FileLogger(), TelegramBotLogger())
        self.log("okx executor starts...")

    def log(self, message, log_level=LogLevel.INFO):
        self.logger.log(message, log_level)

    async def submit_order(self, order):
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
        
        self.log(f"submit {order.side} {pos_side} order at: {order.px}")
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
        self.log(f"canceling order: {order}")
        await self.place_order_client.cancel_order(
            client_order_id=order, 
            inst_id=self.listen_trades_all_client.inst_id) # problem here, may need to record cid-instid mapping
        
    # signals: [(sig, order1), ...] TODO: change this tuple it to a class
    async def handle_signals(self, signals):
        for sig in signals:
            await self.signal_queue.put(sig)
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
            await self.handle_signals(signals)

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

    @run_in_thread
    async def listen_orders(self):
        async with OkxWsListenOrdersClient(OKX_WS_URI.PRIVATE_PAPER, self.api_key, self.passphrase, self.secret_key, self.inst_type, self.inst_id) as cli:
            self.listen_order_client = cli
            while True:
                data = await self.listen_order_client.recv_data()
                if data['instId'] not in self.positions:
                    self.positions[data['instId']] = .0
                if data['side'] == OKX_SIDE.BUY:
                    self.positions[data['instId']] += float(data['fillSz'])
                elif data['side'] == OKX_SIDE.SELL:
                    self.positions[data['instId']] -= float(data['fillSz'])

                if data['state'] == OKX_ORD_STATE.FILLED:
                    self.strategy_mutex.acquire()
                    try:
                        signals = self.strategy.on_order_filled(data['clOrdId'])
                    finally:
                        self.strategy_mutex.release()
                elif data['state'] == OKX_ORD_STATE.CANCELED:
                    self.strategy_mutex.acquire()
                    try:
                        signals = self.strategy.on_order_withdraw_success(data['clOrdId'])
                    finally:
                        self.strategy_mutex.release()
                else:
                    continue

                self.log(f"{data['state']} order - px: {data['px']}, ordId: {data['ordId']}", LogLevel.DEBUG)
                await self.handle_signals(signals)

    @run_in_thread
    async def listen_queue(self):
        async with OkxWsPlaceOrderClient(OKX_WS_URI.PRIVATE_PAPER, self.api_key, self.passphrase, self.secret_key) as cli:
            self.place_order_client = cli
            self.signal_queue = asyncio.Queue()
            
            get_signal_task = asyncio.create_task(self.signal_queue.get())
            recv_confirm_task = asyncio.create_task(self.place_order_client.confirm_order())
            while True:
                # self.signal_queue.task_done()  # 标记任务已完成 todo: do we need this?
                done, _ = await asyncio.wait(
                    [get_signal_task, recv_confirm_task],
                    return_when=asyncio.FIRST_COMPLETED,
                    timeout=25
                )

                if get_signal_task in done:
                    sig, order = get_signal_task.result()
                    if sig == Signal.SUBMIT:
                        await self.submit_order(order)
                    elif sig == Signal.WITHDRAW:
                        await self.cancel_order(order)
                    else:
                        raise Exception(f"Unsupported signal: {sig}")
                    await asyncio.sleep(0.01) # order speed limit: 60 times/2s
                    get_signal_task = asyncio.create_task(self.signal_queue.get())
                elif recv_confirm_task in done:
                    recv_confirm_task = asyncio.create_task(self.place_order_client.confirm_order())
                else:
                    # timeout, need to ping server, but cannot await on send/recv again
                    get_signal_task.cancel()
                    recv_confirm_task.cancel()
                    try:
                        await get_signal_task
                    except asyncio.CancelledError:
                        pass
                    try:
                        await recv_confirm_task
                    except asyncio.CancelledError:
                        pass
                    await self.place_order_client.ping_pong()
                    get_signal_task = asyncio.create_task(self.signal_queue.get())
                    recv_confirm_task = asyncio.create_task(self.place_order_client.confirm_order())

    def run(self):
        listen_orders_thread = self.listen_orders()
        listen_orders_thread.start()
        while not self.listen_order_client:
            time.sleep(0.1)

        listen_queue_thread = self.listen_queue()
        listen_queue_thread.start()
        while not self.place_order_client:
            time.sleep(0.1)
    
        listen_trades_thread = self.listen_trades()
        listen_trades_thread.start()
        
        listen_orders_thread.join()
        listen_queue_thread.join()
        listen_trades_thread.join()
