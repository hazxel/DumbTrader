import os
import heapq
import pandas as pd

from dumbtrader.strategy.strategy import *

def gen_csv_tuple(file_list):
    for file_path in file_list:
        if not os.path.exists(file_path):
            print(f"File {file_path} does not exist. Skipping.")
            continue
        
        try:
            df = pd.read_csv(file_path)
        except Exception as e:
            print(f"Failed to read {file_path}: {e}")
            continue
        
        for tuple in df.itertuples(index=False):
            yield tuple

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
        # print(f"cur pnl: {self.calc_pnl()}, cur px {self.last_pxs['ETH-USDT']}, order px: {order.px}, {order.direction} {order.volume}")
        if order.type == OrderType.MARKET:
            # todo: last px not usually available 
            traded_amount = order.volume * self.last_pxs[order.inst_id]
            self.mkt_ord_amt += traded_amount
        elif order.type == OrderType.LIMIT:
            traded_amount = order.volume * order.px
            self.lmt_ord_amt += traded_amount
        else:
            raise Exception(f"Unsupported order type: {order.type}")

        if order.direction == OrderDirection.BUY:
            self.balance -= traded_amount
            self.positions[order.inst_id] += order.volume
            # print(f"buy at {order.px}, position {self.positions[order.inst_id]}")
            return self.strategy.on_order_filled(order.internal_id)
        elif order.direction == OrderDirection.SELL:
            self.balance += traded_amount
            self.positions[order.inst_id] -= order.volume
            # print(f"sell at {order.px}, position {self.positions[order.inst_id]}")
            return self.strategy.on_order_filled(order.internal_id)
        else:
            raise Exception(f"Unsupported order direction: {order.direction}")
        
        
    # signals: [(sig, order1), ...]
    def handle_signals(self, signals):
        for sig, order in signals:
            if sig == Signal.SUBMIT:
                if order.type == OrderType.LIMIT \
                    and order.direction == OrderDirection.BUY \
                    and order.px < self.last_pxs[order.inst_id]:
                        heapq.heappush(self.limit_buy_orders, order)
                elif order.type == OrderType.LIMIT \
                    and order.direction == OrderDirection.SELL \
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
        tuple = next(gen_csv_tuple(self.file_list))
        signals = self.strategy.on_start(tuple)
        self.last_pxs["ETH-USDT"] = tuple.px
        self.handle_signals(signals)

        pxs = []
        pnls = []

        # begin iteration
        for tuple in gen_csv_tuple(self.file_list):
            signals.extend(self.strategy.on_px_change(tuple))
            while self.limit_buy_orders and self.limit_buy_orders[0].px > tuple.px:
                order = heapq.heappop(self.limit_buy_orders)
                if order.internal_id in self.cancelled_orders:
                    self.cancelled_orders.remove(order.internal_id)
                    continue
                signals.extend(self.execute_order(order))

            while self.limit_sell_orders and self.limit_sell_orders[0].px < tuple.px:
                order = heapq.heappop(self.limit_sell_orders)
                if order.internal_id in self.cancelled_orders:
                    self.cancelled_orders.remove(order.internal_id)
                    continue
                signals.extend(self.execute_order(order))

            self.last_pxs["ETH-USDT"] = tuple.px
               
            # signals.append(strategy.onOrderBookChange(tuple))

            self.handle_signals(signals)
            pxs.append(tuple.px)
            pnls.append(self.calc_pnl())
        return (pxs, pnls)

    def calc_pnl(self):
        return self.balance \
            - self.mkt_ord_amt * MKT_FEE \
            - self.lmt_ord_amt * LMT_FEE \
            + sum([ pos * self.last_pxs[inst] for inst, pos in self.positions.items()]) * (1 - MKT_FEE)
    
    def summary(self):
        return "summary:\n"\
            f"* number of trades: {self.trades}\n"\
            f"* total traded amounts: {self.mkt_ord_amt+self.lmt_ord_amt}\n"\
            f"  - market order amounts: {self.mkt_ord_amt}\n"\
            f"  - limit order amounts: {self.lmt_ord_amt}\n"\
            f"* transaction fee: {self.mkt_ord_amt*MKT_FEE+self.lmt_ord_amt*LMT_FEE}\n"\
            f"* final balance: {self.balance}\n"\
            f"* final position: {self.positions}\n"\
            f"* final prices: {self.last_pxs}\n"\
            f"* pnl: {self.calc_pnl()}"
            

def find_files_with_prefix(path, prefix):
    files = os.listdir(path)
    matching_files = [os.path.join(path, f) for f in files if f.startswith(prefix)]
    sorted_files = sorted(matching_files)
    return sorted_files
