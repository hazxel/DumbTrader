import sys

from dumbtrader.backtest.backtester import *
from dumbtrader.strategy.grid_strategy import *

if __name__ == '__main__':
    data_files = find_pickled_dataframes_with_prefix(path=sys.argv[1], prefix="ETH-USDT-SWAP-trades-all-17")
    print(f"processing {len(data_files)} files")
    print(data_files)
    for grid_num in range(2,30):
        grid_stra = GridStrategy("ETH-USDT", 3207.31, 3689.72, grid_num, 0.03)
        bt = backetester(grid_stra, data_files)
        pxs, pnls = bt.run()
        print(f"{grid_num} grids, trades: {bt.trades}, pnl: {pnls[-1]:.2f}, margin: {grid_stra.total_margin():.2f}, profit rate: {pnls[-1]/grid_stra.total_margin()*100:.2f}%")

# 2 grids, trades: 1, pnl: 0.49, margin: 413.82, profit rate: 0.12%
# 3 grids, trades: 5, pnl: 5.21, margin: 620.73, profit rate: 0.84%
# 4 grids, trades: 13, pnl: 11.20, margin: 827.64, profit rate: 1.35%
# 5 grids, trades: 23, pnl: 12.21, margin: 1034.55, profit rate: 1.18%
# 6 grids, trades: 25, pnl: 10.11, margin: 1241.47, profit rate: 0.81%
# 7 grids, trades: 43, pnl: 17.12, margin: 1448.38, profit rate: 1.18%
# 8 grids, trades: 48, pnl: 12.38, margin: 1655.29, profit rate: 0.75%
# 9 grids, trades: 77, pnl: 21.28, margin: 1862.20, profit rate: 1.14%
# 10 grids, trades: 99, pnl: 25.62, margin: 2069.11, profit rate: 1.24%
# 11 grids, trades: 103, pnl: 23.30, margin: 2276.02, profit rate: 1.02%
# 12 grids, trades: 130, pnl: 24.22, margin: 2482.93, profit rate: 0.98%
# 13 grids, trades: 145, pnl: 24.89, margin: 2689.84, profit rate: 0.93%
# 14 grids, trades: 185, pnl: 31.43, margin: 2896.75, profit rate: 1.09%
# 15 grids, trades: 202, pnl: 27.92, margin: 3103.66, profit rate: 0.90%
# 16 grids, trades: 238, pnl: 32.25, margin: 3310.57, profit rate: 0.97%
# 17 grids, trades: 263, pnl: 33.70, margin: 3517.49, profit rate: 0.96%
# 18 grids, trades: 297, pnl: 36.51, margin: 3724.40, profit rate: 0.98%
# 19 grids, trades: 328, pnl: 34.59, margin: 3931.31, profit rate: 0.88%
# 20 grids, trades: 348, pnl: 34.58, margin: 4138.22, profit rate: 0.84%
# 21 grids, trades: 409, pnl: 40.55, margin: 4345.13, profit rate: 0.93%
# 22 grids, trades: 431, pnl: 40.30, margin: 4552.04, profit rate: 0.89%
# 23 grids, trades: 460, pnl: 37.16, margin: 4758.95, profit rate: 0.78%
# 24 grids, trades: 516, pnl: 41.24, margin: 4965.86, profit rate: 0.83%
# 25 grids, trades: 531, pnl: 39.76, margin: 5172.77, profit rate: 0.77%
# 26 grids, trades: 598, pnl: 40.56, margin: 5379.68, profit rate: 0.75%
# 27 grids, trades: 644, pnl: 42.46, margin: 5586.59, profit rate: 0.76%
# 28 grids, trades: 676, pnl: 42.71, margin: 5793.51, profit rate: 0.74%
# 29 grids, trades: 727, pnl: 44.67, margin: 6000.42, profit rate: 0.74%