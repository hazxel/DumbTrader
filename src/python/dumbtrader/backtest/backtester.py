import heapq

from dumbtrader.strategy.strategy import *
from dumbtrader.utils.file_utils import *

MKT_FEE = 0.00041
LMT_FEE = 0.00026

class backetester:
    def __init__(self, strategy, file_list):
        self.strategy = strategy
        self.file_list = file_list
        self.positions = {'ETH-USDT': .0}
        self.last_pxs = {'ETH-USDT': .0}
        self.limit_buy_orders = []
        self.limit_sell_orders = []
        self.cancelled_orders = set()
        self.balance = 0
        # stats
        self.mkt_ord_amt = 0
        self.lmt_ord_amt = 0
        self.trades = 0;

    def execute_order(self, order):
        self.trades += 1
        traded_amount = 0
        # print(f"px: {order.px} bbo: {heapq.nsmallest(1,self.limit_buy_orders)[0].px} bao: {heapq.nsmallest(1,self.limit_sell_orders)[0].px}")
        # print(f"cur pnl: {self.calc_pnl()}, cur px {self.last_pxs['ETH-USDT']}, order px: {order.px}, {order.side} {order.volume}")
        if order.type == OrderType.MARKET:
            # todo: last px not usually available 
            traded_amount = order.volume * self.last_pxs[order.inst_id]
            self.mkt_ord_amt += traded_amount
        elif order.type == OrderType.LIMIT:
            traded_amount = order.volume * order.px
            self.lmt_ord_amt += traded_amount
        else:
            raise Exception(f"Unsupported order type: {order.type}")

        if order.side == OrderSide.BUY:
            self.balance -= traded_amount
            self.positions[order.inst_id] += order.volume
            print(f"buy at {order.px if order.px != 0 else self.last_pxs[order.inst_id]}, position {self.positions[order.inst_id]}, pnl:{self.calc_pnl()}")
            return self.strategy.on_order_filled(order.internal_id)
        elif order.side == OrderSide.SELL:
            self.balance += traded_amount
            self.positions[order.inst_id] -= order.volume
            print(f"sell at {order.px if order.px != 0 else self.last_pxs[order.inst_id]}, position {self.positions[order.inst_id]}, pnl:{self.calc_pnl()}")
            return self.strategy.on_order_filled(order.internal_id)
        else:
            raise Exception(f"Unsupported order side: {order.side}")
        
        
    # signals: [(sig, order1), ...]
    def handle_signals(self, signals):
        for sig, order in signals:
            if sig == Signal.SUBMIT:
                if order.type == OrderType.LIMIT \
                    and order.side == OrderSide.BUY \
                    and order.px < self.last_pxs[order.inst_id]:
                        heapq.heappush(self.limit_buy_orders, order)
                elif order.type == OrderType.LIMIT \
                    and order.side == OrderSide.SELL \
                    and order.px > self.last_pxs[order.inst_id]:
                        heapq.heappush(self.limit_sell_orders, order)
                else:
                    signals.extend(self.execute_order(order))
            elif sig == Signal.WITHDRAW:
                self.cancelled_orders.add(order)
                signals.extend(self.strategy.on_order_withdraw_success(order))
            else:
                raise Exception(f"Unsupported signal: {sig}")
        signals.clear()

    def run(self):
        # init strategy
        record = next(gen_csv_record(self.file_list))
        signals = self.strategy.on_start(record)
        self.last_pxs["ETH-USDT"] = record['px']
        self.handle_signals(signals)

        pxs = []
        pnls = []

        # begin iteration
        for record in gen_csv_record(self.file_list):
            signals.extend(self.strategy.on_px_change(record))
            while self.limit_buy_orders and self.limit_buy_orders[0].px > record['px']:
                order = heapq.heappop(self.limit_buy_orders)
                if order.internal_id in self.cancelled_orders:
                    self.cancelled_orders.remove(order.internal_id)
                    continue
                signals.extend(self.execute_order(order))

            while self.limit_sell_orders and self.limit_sell_orders[0].px < record['px']:
                order = heapq.heappop(self.limit_sell_orders)
                if order.internal_id in self.cancelled_orders:
                    self.cancelled_orders.remove(order.internal_id)
                    continue
                signals.extend(self.execute_order(order))

            self.last_pxs["ETH-USDT"] = record['px']
               
            # signals.append(strategy.onOrderBookChange(tuple))

            self.handle_signals(signals)
            pxs.append(record['px'])
            pnls.append(self.calc_pnl())
        return (pxs, pnls)

    def calc_pnl(self):
        return self.balance \
            - self.mkt_ord_amt * MKT_FEE \
            - self.lmt_ord_amt * LMT_FEE \
            + sum([ pos * self.last_pxs[inst] * (1-MKT_FEE if pos>0 else 1+MKT_FEE) for inst, pos in self.positions.items()])
    
    def summary(self):
        return "summary:\n"\
            f"* number of trades: {self.trades}\n"\
            f"* total traded amounts: {self.mkt_ord_amt+self.lmt_ord_amt}\n"\
            f"  - market order amounts: {self.mkt_ord_amt}\n"\
            f"  - limit order amounts: {self.lmt_ord_amt}\n"\
            f"* transaction fee: {self.mkt_ord_amt*MKT_FEE+self.lmt_ord_amt*LMT_FEE}\n"\
            f"* final balance: {self.balance}\n"\
            f"* final position: {self.positions}\n"\
            f"* pnl: {self.calc_pnl()}"
    