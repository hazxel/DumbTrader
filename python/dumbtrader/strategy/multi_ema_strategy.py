from enum import Enum, auto
import uuid

from dumbtrader.strategy.strategy import *

class Position(Enum):
    LONG = auto()
    SHORT = auto()
    EMPTY = auto()


class MultiEmaStrategy(Strategy):
    def __init__(self, inst_id, weight, *periods):
        self.inst_id = inst_id
        self.weight = weight
        self.periods = list(periods)
        self.periods.sort() # fast ema first
        self.alpha = [ 1 / period for period in self.periods]
        self.alphadiff = [ 1 - alpha for alpha in self.alpha]
        self.ema_count = len(self.periods)
        self.emas = None
        
        self.init_ticks = 0
        self.position = Position.EMPTY
        self.continue_counts = 0
        self.continue_side = Position.EMPTY

        self.win = 0
        self.loss = 0
        self.open_px = None

    def sell_signal(self):
        id = str(uuid.uuid4()).replace('-', '')
        return [(Signal.SUBMIT, MktSellOrder(self.inst_id, self.weight, id))]

    def buy_signal(self):
        id = str(uuid.uuid4()).replace('-', '')
        return [(Signal.SUBMIT, MktBuyOrder(self.inst_id, self.weight, id))]

    def on_start(self, px_tuple):
        self.emas = [[px_tuple['px']] for _ in range(self.ema_count)]
        return []
    
    def on_trade(self, trade):
        cur_px = trade['px']
        for i in range(self.ema_count):
            self.emas[i].append(self.alphadiff[i] * self.emas[i][-1] + cur_px * self.alpha[i])

        if self.init_ticks < self.periods[-1]:
            self.init_ticks += 1
            return []
        
        isLong = cur_px > self.emas[0][-1]
        isShort = cur_px < self.emas[0][-1]
        for i in range(self.ema_count - 1):
            if self.emas[i][-1] <= self.emas[i+1][-1]:
                isLong = False
            if self.emas[i][-1] >= self.emas[i+1][-1]:
                isShort = False
            if (not isLong) and (not isShort):
                break
        
        if self.position == Position.EMPTY:
            if isShort:
                if self.continue_side == Position.SHORT:
                    if self.continue_counts > self.periods[0]:
                        self.position = Position.SHORT
                        self.open_px = cur_px
                        return self.sell_signal()
                    else:
                        self.continue_counts += 1
                else:
                    self.continue_counts = 0
                    self.continue_side = Position.SHORT
            elif isLong:
                if self.continue_side == Position.LONG:
                    # no! just self.emas[-2][-1] -5 > self.emas[-1][-1]
                    if self.continue_counts > self.periods[0]:
                        self.position = Position.LONG
                        self.open_px = cur_px
                        return self.buy_signal()
                    else:
                        self.continue_counts += 1
                else:
                    self.continue_counts = 0
                    self.continue_side = Position.LONG
            else:
                self.continue_counts = 0
                self.continue_side = Position.EMPTY
        elif self.position == Position.LONG and (cur_px < self.emas[-1][-1]):
            self.position = Position.EMPTY
            self.continue_side = Position.EMPTY
            self.continue_counts = 0
            if cur_px > self.open_px:
                self.win += 1
            else:
                self.loss += 1
            return self.sell_signal()
        elif self.position == Position.SHORT and (cur_px > self.emas[-1][-1]):
            self.position = Position.EMPTY
            self.continue_side = Position.EMPTY
            self.continue_counts = 0
            if cur_px < self.open_px:
                self.win += 1
            else:
                self.loss += 1
            return self.buy_signal()

        return []
    
    def on_order_filled(self, internal_ord_id):
        return super().on_order_filled(internal_ord_id)
    
    def on_order_submit_success(self, internal_ord_id):
        return super().on_order_submit_success(internal_ord_id)
    
    def on_order_withdraw_success(self, internal_ord_id):
        return super().on_order_withdraw_success(internal_ord_id)
        