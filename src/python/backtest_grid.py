import matplotlib.pyplot as plt
import sys

from dumbtrader.backtest.backtester import *
from dumbtrader.strategy.grid_strategy import *

def make_ema_generator(init_px, N=5000000):
    prev_px = init_px
    alpha = 1 / N
    alphadiff = 1 - alpha

    def ema_generator(px):
        nonlocal prev_px
        prev_px = prev_px * alphadiff + px * alpha
        return prev_px
    
    return ema_generator
    

if __name__ == '__main__':
    data_files = find_files_with_prefix(sys.argv[1], "ETH-USDT-SWAP-trades-all-1720")
    N = 5000000
    grid_stra = EmaGridStrategy("ETH-USDT", 3207.31, 3689.72, 13, 0.03, N)
    # grid_stra = GridStrategy("ETH-USDT", 3207.31, 3689.72, 13, 0.03)
    bt = backetester(grid_stra, data_files)
    pxs, pnls = bt.run()

    ema_generator = make_ema_generator(pxs[0], N)
    emas = [ ema_generator(px) for px in pxs]

    print(f"start px: {pxs[0]}, end px: {pxs[-1]}, pnl: {pnls[-1]}")
    print(bt.summary())
    print(f"total margin: {grid_stra.total_margin()}")
    print(f"profit rate (pnl/ttl_margin): {pnls[-1] * 100 / grid_stra.total_margin()}%")


    x_values = range(1, len(pnls) + 1)

    fig, ax1 = plt.subplots()
    ax1.plot(x_values, pnls, marker='s', markersize=1, linewidth=0.3, linestyle='-', color='r', label='pnl')
    ax1.set_ylabel('pnl', color='r')
    ax1.tick_params(axis='y', labelcolor='r')

    ax2 = ax1.twinx()
    ax2.plot(x_values, pxs, marker='o', markersize=1, linewidth=0.3, linestyle=':', color='b', label='price')
    ax2.set_ylabel('instrument price', color='b')
    ax2.tick_params(axis='y', labelcolor='b')
    
    ax2.plot(x_values, emas, marker='o', markersize=1, linewidth=0.3, linestyle='-.', color='g', label='ema')

    plt.title('Grid strategy')
    plt.xlabel('time')
    # plt.ylabel('pnl')
    plt.show()