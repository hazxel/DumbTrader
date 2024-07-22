import matplotlib.pyplot as plt
import sys
import time

from dumbtrader.backtest.backtester import *
from dumbtrader.strategy.multi_ema_strategy import *

if __name__ == '__main__':
    start_time = time.time()

    data_files = find_pickled_dataframes_with_prefix(path=sys.argv[1], prefix="ETH-USDT-SWAP-trades-all-172")
    stra = MultiEmaStrategy("ETH-USDT", 1, 50000, 100000, 200000, 800000)

    bt = backetester(stra, data_files)
    pxs, pnls = bt.run()

    print(bt.summary())
    print(f"win: {stra.win}, loss: {stra.loss}") # win rate: {100*stra.win/(stra.win+stra.loss):.2f}%")

    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"[backtest took: {elapsed_time:.4f} seconds]")

    # start = 40000
    # end = -1
    # step = 100

    # pnls = pnls[start:end:step]
    # pxs = pxs[start:end:step]
    # x_values = range(len(pnls))

    # fig, ax1 = plt.subplots()
    # ax1.plot(x_values, pnls, marker='s', markersize=0.5, linewidth=0.3, linestyle='-', color='r', label='pnl')
    # ax1.set_ylabel('pnl', color='r')
    # ax1.tick_params(axis='y', labelcolor='r')

    # ax2 = ax1.twinx()
    # ax2.plot(x_values, pxs, marker='o', markersize=0.5, linewidth=0.3, linestyle=':', color='b', label='price')
    # ax2.set_ylabel('instrument price', color='b')
    # ax2.tick_params(axis='y', labelcolor='b')

    # ax2.plot(x_values, stra.emas[0][start:end:step], marker='o', markersize=0.2, linewidth=0.1, linestyle='-.', label='ema')
    # ax2.plot(x_values, stra.emas[1][start:end:step], marker='o', markersize=0.2, linewidth=0.1, linestyle='-.', label='ema')
    # ax2.plot(x_values, stra.emas[2][start:end:step], marker='o', markersize=0.2, linewidth=0.1, linestyle='-.', label='ema')
    # ax2.plot(x_values, stra.emas[3][start:end:step], marker='o', markersize=0.2, linewidth=0.1, linestyle='-.', label='ema')
    
    # plt.title('Ema strategy')
    # plt.xlabel('time')
    # # plt.ylabel('pnl')
    # plt.show()