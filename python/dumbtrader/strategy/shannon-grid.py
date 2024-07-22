import matplotlib.pyplot as plt
import numpy as np

leverage = 5

init_worth = 1000.0
init_bal = 500.0
init_pos = 5.0
init_px = 100.0

min_trade_vol = 0.1
tick = 0.1

unif_grid_interval = 1.0
unif_grid_order_vol = 0.1
unif_grid_down_lmt_px = 90.0
unif_grid_up_lmt_px = 110.0

px_list = [unif_grid_up_lmt_px, init_px, unif_grid_down_lmt_px]
long_gird_worth_list = [init_worth]
mkt_neutral_worth = [0.0]

unif_grid_max_diff_pos = (init_px - unif_grid_down_lmt_px) / unif_grid_interval * unif_grid_order_vol

# down calculation
unif_grid_down_lmx_buy_avg_px = (init_px + unif_grid_down_lmt_px - unif_grid_interval) / 2
unif_grid_down_max_buy_value = unif_grid_down_lmx_buy_avg_px * unif_grid_max_diff_pos

cur_pos = init_pos + unif_grid_max_diff_pos
cur_bal = init_bal - unif_grid_down_max_buy_value

down_ratio = (init_pos * unif_grid_down_lmt_px + unif_grid_down_max_buy_value) / cur_bal

long_gird_worth_list.append(cur_bal + cur_pos * unif_grid_down_lmt_px)
mkt_neutral_worth.append(cur_bal + cur_pos * unif_grid_down_lmt_px  \
                        - init_worth                                  \
                        + init_pos * (init_px - unif_grid_down_lmt_px))

stop = False

for i in range(300):
    cur_pos += min_trade_vol
    trigger_px = down_ratio * cur_bal / (cur_pos + down_ratio * min_trade_vol)
    cur_bal -= trigger_px * min_trade_vol

    px_list.append(trigger_px)
    long_gird_worth_list.append(cur_bal + cur_pos * trigger_px)
    mkt_neutral_worth.append(cur_bal + cur_pos * trigger_px     \
                            - init_worth                        \
                            + init_pos * (init_px - trigger_px))
    
    # 太密集了，不下单了
    if trigger_px - px_list[-1] < tick:
        if not stop:
            cur_pos_ = cur_pos
            cur_bal_ = cur_bal
            stop = True
        long_gird_worth_list[-1] = cur_bal_ + cur_pos_ * trigger_px
        mkt_neutral_worth[-1] = cur_bal_ + cur_pos_ * trigger_px   \
                            - init_worth                           \
                            + init_pos * (init_px - trigger_px)

# up calculation
unif_grid_up_lmx_sell_avg_px = (init_px + unif_grid_up_lmt_px + unif_grid_interval) / 2
unif_grid_up_max_sell_value = unif_grid_up_lmx_sell_avg_px * unif_grid_max_diff_pos

cur_pos = init_pos - unif_grid_max_diff_pos
cur_bal = init_bal + unif_grid_up_max_sell_value

up_ratio = (init_pos * unif_grid_up_lmt_px - unif_grid_up_max_sell_value) / cur_bal

long_gird_worth_list.insert(0, cur_bal + cur_pos * unif_grid_up_lmt_px)
mkt_neutral_worth.insert(0, cur_bal + cur_pos * unif_grid_up_lmt_px \
                        - init_worth                                \
                        - init_pos * (unif_grid_up_lmt_px - init_px))

for i in range(25):
    cur_pos -= min_trade_vol
    if cur_pos < 0:
        continue

    trigger_px = up_ratio * cur_bal / (cur_pos - up_ratio * min_trade_vol)
    cur_bal += trigger_px * min_trade_vol
    
    px_list.insert(0, trigger_px)
    long_gird_worth_list.insert(0, cur_bal + cur_pos * trigger_px)
    mkt_neutral_worth.insert(0, cur_bal + cur_pos * trigger_px  \
                        - init_worth                            \
                        - init_pos * (trigger_px - init_px))
    
    

px_norm = [(px - init_px) / init_px for px in px_list]
naive_long_benefit_norm = [leverage * 2 * (px - init_px) * init_pos / init_worth for px in px_list] # 网格的资金没有用到，也就是有两倍的资金做多
long_grid_benefit_norm = [leverage * (w - init_worth) / init_worth for w in long_gird_worth_list]
netural_grid_benefit_norm = [leverage * 2 * w / init_worth for w in mkt_neutral_worth] # 做多时开仓的资金没有用到，也就是有两倍的资金做中性

# print(naive_long_benefit_norm)
# print(long_grid_benefit_norm)
# print(netural_grid_benefit_norm)

def calc_pure_long_limited_worth(px):
    pos_diff = (init_px - px) / unif_grid_interval * unif_grid_order_vol
    pos_diff = max(pos_diff, -init_pos)
    avg_price = (px + init_px + (unif_grid_interval if px > init_px else -1 * unif_grid_interval)) / 2
    avg_price = avg_price if init_pos + pos_diff > 0 else (2 * init_px + int(init_pos / unif_grid_order_vol) * unif_grid_interval + unif_grid_interval) / 2
    bal = init_bal - pos_diff * avg_price
    worth = bal + (init_pos + pos_diff) * px
    return worth

def calc_pure_long_worth(px):
    pos_diff = (init_px - px) / unif_grid_interval * unif_grid_order_vol
    avg_price = (px + init_px + (unif_grid_interval if px > init_px else -1 * unif_grid_interval)) / 2
    bal = init_bal - pos_diff * avg_price
    worth = bal + (init_pos + pos_diff) * px
    return worth

def calc_pure_neutural_worth(px):
    long_worth = calc_pure_long_worth(px)
    return long_worth - init_worth + init_pos * (init_px - px)

# pure uniform strategy
pure_unif_px_list = np.arange(0, 300, unif_grid_interval)
pure_unif_px_norm = [(px - init_px) / init_px for px in pure_unif_px_list]
pure_unif_long_benefit_norm = [leverage * (calc_pure_long_limited_worth(px) / init_worth - 1) for px in pure_unif_px_list]
pure_unif_neutural_benefit_norm = [leverage * 2 * calc_pure_neutural_worth(px) / init_worth for px in pure_unif_px_list]

# 绘制折线图
plt.plot(px_norm, naive_long_benefit_norm, label='naive-long', color='red')
plt.plot(px_norm, long_grid_benefit_norm, label='mixed-grid-long', color='blue', linestyle='--')
# plt.plot(px_norm, netural_grid_benefit_norm, label='mixed-grid-netural', color='green', linestyle='--')
plt.plot(pure_unif_px_norm, pure_unif_long_benefit_norm, label='unif-grid-long', linestyle=':')
# plt.plot(pure_unif_px_norm, pure_unif_neutural_benefit_norm, label='unif-grid-neutural', linestyle=':')

print(pure_unif_long_benefit_norm)
# 标记每个点
# plt.scatter(px_norm, naive_long_benefit_norm, color='red', s=20, marker='x')
# plt.scatter(px_norm, long_grid_benefit_norm, color='blue', s=20, marker='x')
# plt.scatter(px_norm, netural_grid_benefit_norm, color='green', s=20, marker='x')

# 设置 x 和 y 轴范围
plt.xlim(-1, 1.5)
plt.ylim(-1, 1.5)

# 添加图例
plt.legend()

# 添加标题和标签
plt.title('Channon grid strategy')
plt.xlabel('normalized price diff')
plt.ylabel('normalized benefit')

# 显示图形
plt.show()