# import pandas as pd
import uuid
import bisect

from dumbtrader.strategy.strategy import *

class GridStrategy(Strategy):
    def __init__(self, inst_id, low_lmt, high_lmt, grid_num, weight):
        self.inst_id = inst_id
        self.grid_num = grid_num
        self.weight = weight

        self.grid_interval = (high_lmt - low_lmt) / (2 * grid_num - 1)
        self.grid = []
        for i in range(2 * grid_num):
            self.grid.append(round(low_lmt + i * self.grid_interval, 2))

        self.id2px = {}
        self.px2id = {}
        self.piviot_px = None
        self.latest_trade_px = None
        self.latest_trade_idx = None

    def total_margin(self):
        return sum([px for px in self.grid]) * self.weight

    def gen_lmt_sell_signal(self, px):
        id = str(uuid.uuid4()).replace('-', '')
        self.id2px[id] = px
        self.px2id[px] = id
        return (Signal.SUBMIT, LmtSellOrder(self.inst_id, self.weight, px, id))

    def gen_lmt_buy_signal(self, px):
        id = str(uuid.uuid4()).replace('-', '')
        self.id2px[id] = px
        self.px2id[px] = id
        return (Signal.SUBMIT, LmtBuyOrder(self.inst_id, self.weight, px, id))
    
    # helper functions
    def closest_grid_idx(self, px):
        idx = round((px - self.grid[0]) / self.grid_interval)
        return max(0, min(len(self.grid)-1, idx))
    
    def closest_grid_px(self, px):
        return self.grid[self.closest_grid_idx(px)]

    def on_start(self, record):
        cur_px = record['px']
        self.latest_trade_idx = self.closest_grid_idx(cur_px)
        self.latest_trade_px = self.grid[self.latest_trade_idx]
        self.piviot_px = self.latest_trade_px
        signals = []
        for px in self.grid:
            if px > self.piviot_px:
                signals.append(self.gen_lmt_sell_signal(px))
            elif px < self.piviot_px:
                signals.append(self.gen_lmt_buy_signal(px))
        return signals
    
    def on_order_filled(self, internal_ord_id):
        cur_trade_px = self.id2px.pop(internal_ord_id)
        self.px2id.pop(cur_trade_px)
        signals = []
        if cur_trade_px > self.latest_trade_px:
            signals.append(self.gen_lmt_buy_signal(self.latest_trade_px))
        else:
            signals.append(self.gen_lmt_sell_signal(self.latest_trade_px))
        self.latest_trade_px = cur_trade_px
        self.latest_trade_idx = bisect.bisect_left(self.grid, cur_trade_px)
        return signals
    
    def on_trade(self, trade):
        return super().on_trade(trade)
    
    def on_order_filled(self, internal_ord_id):
        return super().on_order_filled(internal_ord_id)
    
    def on_order_submit_success(self, internal_ord_id):
        return super().on_order_submit_success(internal_ord_id)
    
    def on_order_withdraw_success(self, internal_ord_id):
        return super().on_order_withdraw_success(internal_ord_id)

class EmaGridStrategy(GridStrategy):
    def __init__(self, inst_id, low_lmt, high_lmt, grid_num, weight, N=5000000):
        super().__init__(inst_id, low_lmt, high_lmt, grid_num, weight)
        self.ema = None
        self.alpha = 1 / N
        self.alphadiff = 1 - 1 / N
        self.delta = 0
        self.ignore_on_filled = set()

    def on_start(self, record):
        sigs = super().on_start(record)
        self.ema = record['px']
        return sigs
    
    # helper functions
    def grid_new_down_px(self):
        return round(2 * self.grid[0] - self.grid[1],2)
    def grid_new_up_px(self):
        return round(2 * self.grid[-1] - self.grid[-2],2)
    
    def on_order_filled(self, internal_ord_id):
        # print(f"filled order at px {self.id2px[internal_ord_id]}, last idx: {self.latest_trade_idx}, last px: {self.latest_trade_px}")
        if internal_ord_id in self.ignore_on_filled: # filled event arrive in wrong order
            cur_trade_px = self.id2px.pop(internal_ord_id)
            self.ignore_on_filled.remove(internal_ord_id)
            print(f"  ****  ignored order {internal_ord_id} of px {cur_trade_px}.")
            return []
        
        cur_trade_px = self.id2px.pop(internal_ord_id)
        cur_trade_idx = self.closest_grid_idx(cur_trade_px)
        cur_trade_internal_id = self.px2id.pop(cur_trade_px)
        
        signals = []
        if cur_trade_idx > self.latest_trade_idx:
            while True:
                should_triggered_idx = self.latest_trade_idx + max(0, self.delta) + 1
                should_triggered_px = self.grid[should_triggered_idx]
                if self.delta >= 0:
                    signals.append(self.gen_lmt_buy_signal(self.latest_trade_px))
                elif self.latest_trade_idx + self.delta >= 0:
                    signals.append(self.gen_lmt_buy_signal(self.grid[self.latest_trade_idx + self.delta]))
                self.latest_trade_idx += 1
                self.latest_trade_px = self.grid[self.latest_trade_idx]
                if should_triggered_idx == cur_trade_idx:
                    break
                else:
                    print(f"  ****  will ignore {should_triggered_px} of {self.px2id[should_triggered_px]} once")
                    self.ignore_on_filled.add(self.px2id[should_triggered_px])
                    self.px2id.pop(should_triggered_px)
        elif cur_trade_idx < self.latest_trade_idx:
            while True:
                should_triggered_idx = self.latest_trade_idx + min(0, self.delta) - 1
                should_triggered_px = self.grid[should_triggered_idx]
                if self.delta <= 0:
                    signals.append(self.gen_lmt_sell_signal(self.latest_trade_px))
                elif self.latest_trade_idx + self.delta < len(self.grid):
                    signals.append(self.gen_lmt_sell_signal(self.grid[self.latest_trade_idx + self.delta]))
                self.latest_trade_idx -= 1
                self.latest_trade_px = self.grid[self.latest_trade_idx]
                if should_triggered_idx == cur_trade_idx:
                    break
                else:
                    print(f"  ****  will ignore {should_triggered_px} of {self.px2id[should_triggered_px]} once")
                    self.ignore_on_filled.add(self.px2id[should_triggered_px])
                    self.px2id.pop(should_triggered_px)
        else:
            raise Exception(f"order {internal_ord_id} with px {cur_trade_px} shouldn't be filled")
        return signals
    
    def gen_withdraw_signal(self, px):
        return (Signal.WITHDRAW, self.px2id[px])
    
    def on_order_withdraw_success(self, internal_ord_id):
        px = self.id2px.pop(internal_ord_id)
        self.px2id.pop(px)
        self.ignore_on_filled.discard(internal_ord_id) # need this?
        return []
    
    # on fake order filled, grid is shifted
    def check_dummy_order(self, px):
        signals = []
        new_up_px = self.grid_new_up_px()
        new_down_px = self.grid_new_down_px()
        if self.delta > 0:
            if self.latest_trade_idx + 1 == len(self.grid):
                return []
            while px > self.grid[self.latest_trade_idx + 1]:
                # shift grid up
                signals.append(self.gen_lmt_buy_signal(self.latest_trade_px))
                self.delta -= 1
                signals.append(self.gen_withdraw_signal(self.grid[0]))
                self.grid.pop(0)
                self.grid.append(new_up_px)
                if self.delta + self.latest_trade_idx < len(self.grid)-1:
                    signals.append(self.gen_lmt_sell_signal(new_up_px))
                self.latest_trade_px = self.grid[self.latest_trade_idx]
                self.piviot_px = self.grid[self.closest_grid_idx(self.piviot_px)+1]
                # print(f"fake order at {self.grid[self.latest_trade_idx + 1]} triggered, grid shifted up: [{self.grid[0]},{self.grid[-1]}]")
        elif self.delta < 0:
            if self.latest_trade_idx == 0:
                return []
            while px < self.grid[self.latest_trade_idx - 1]:
                # shift grid down
                signals.append(self.gen_lmt_sell_signal(self.latest_trade_px))
                self.delta += 1
                signals.append(self.gen_withdraw_signal(self.grid[-1]))
                self.grid.pop()
                self.grid.insert(0, new_down_px)
                if self.delta + self.latest_trade_idx > 0:
                    signals.append(self.gen_lmt_buy_signal(new_down_px))
                self.latest_trade_px = self.grid[self.latest_trade_idx]
                self.piviot_px = self.grid[self.closest_grid_idx(self.piviot_px)-1]
                # print(f"fake order at {self.grid[self.latest_trade_idx - 1]} triggered, grid shifted down [{self.grid[0]},{self.grid[-1]}]")
        return signals
    
    def on_trade(self, record):
        signals = self.check_dummy_order(record['px'])

        self.ema = self.ema * self.alphadiff + record['px'] * self.alpha
        ema_grid_px = self.closest_grid_px(self.ema)
        # print(f"ema: {ema_grid_px}, piviot: {self.piviot_px}")
        new_delta = round((ema_grid_px - self.piviot_px) / self.grid_interval)

        if new_delta > self.delta:
            for d in range(self.delta, new_delta):
                if d >= 0:
                    to_withdraw = self.latest_trade_idx + d + 1
                    if (to_withdraw < len(self.grid)):
                        px = self.grid[to_withdraw]
                        signals.append(self.gen_withdraw_signal(px))
                        # print(f"delta: {self.delta} -> {new_delta}, withdraw {px}")
                else:
                    px = self.grid[self.latest_trade_idx + d]
                    signals.append(self.gen_lmt_buy_signal(px))
                    # print(f"put back buy {px}, delta: {new_delta}")
        elif new_delta < self.delta:
            for d in range(new_delta, self.delta):
                if d < 0:
                    to_withdraw = self.latest_trade_idx + d
                    if (to_withdraw >= 0):
                        px = self.grid[to_withdraw]
                        signals.append(self.gen_withdraw_signal(px))
                        # print(f"delta: {self.delta} -> {new_delta}, withdraw {px}")
                else:
                    px = self.grid[self.latest_trade_idx + d + 1]
                    signals.append(self.gen_lmt_sell_signal(px))
                    # print(f"put back sell {px}, delta: {new_delta}")
        self.delta = new_delta
        return signals

