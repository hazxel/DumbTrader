#### SIDE & POS_SIDE
#### 开平仓模式下，side 和 posSide 需要进行组合; 组合保证金模式下，交割和永续仅支持买卖模式
#
# 开多：买入开多（side=buy； posSide=long ）
# 开空：卖出开空（side=sell； posSide=short ）
# 平多：卖出平多（side=sell；posSide=long ）
# 平空：买入平空（side=buy； posSide=short ）

class OKX_SIDE:
    BUY = "buy"
    SELL = "sell"

class OKX_POS_SIDE:
    LONG = "long"
    SHORT = "short"


#### TD_MODE 
#### 交易模式，下单时需要指定
#
# 简单交易模式：
# * 币币和期权买方：cash
# 单币种保证金模式：
# - 逐仓杠杆：isolated
# - 全仓杠杆：cross
# - 币币：cash
# - 全仓交割/永续/期权：cross
# - 逐仓交割/永续/期权：isolated
# 跨币种保证金模式：
# - 逐仓杠杆：isolated
# - 全仓币币：cross
# - 全仓交割/永续/期权：cross
# - 逐仓交割/永续/期权：isolated
# 组合保证金模式：
# - 逐仓杠杆：isolated
# - 全仓币币：cross
# - 全仓交割/永续/期权：cross
# - 逐仓交割/永续/期权：isolated

class OKX_TD_MODE:
    ISOLATED = "isolated"   # 保证金逐仓
    CROSS = "cross"         # 保证金全仓
    CASH = "cash"           # 非保证金现货


class OKX_ORD_TYPE:
    LMT = "limit"
    MKT = "market"

class OKX_ORD_STATE:
    CANCELED = "canceled"                   # 撤单成功
    LIVE = "live"                           # 等待成交
    PARTIALLY_FILLED = "partially_filled"   # 部分成交
    FILLED = "filled"                       # 完全成交
    MMP_CANCELED = "mmp_canceled"           # 做市商保护机制导致的自动撤单


#### URI & CHANNEL
# These public subscription channels are switched to BUSINESS URL from 2023-06-20:
# - TRADES_ALL
# - ...

class OKX_WS_URI:
    PUBLIC_BUSINESS = "wss://ws.okx.com:8443/ws/v5/business"
    PUBLIC_GENERAL = "wss://ws.okx.com:8443/ws/v5/public"
    PRIVATE_PAPER = "wss://wspap.okx.com:8443/ws/v5/private?brokerId=9999"
    PRIVATE_LIVE = ""

class OKX_WS_SUBSCRIBE_CHANNEL:
    ORDERS = "orders"            # 订单频道，当下单、撤单、成交等事件触发时推送数据
    TICKERS = "tickers"         # 行情频道，推送最新成交价、买一卖一价、交易量等
    TRADES = "trades"           # 交易推送，可能聚合多条成交
    TRADES_ALL = "trades-all"   # 全部交易，一次仅推送一条成交
    BOOK_1_10MS = "bbo-tbt"         # 深度频道，1   档行情，每 10ms  全量推送
    BOOK_5_100MS = "books5"         # 深度频道，5   档行情，每 100ms 全量推送
    BOOK_400_100MS = "books"        # 深度频道，400 档行情，每 100ms 增量推送
    BOOK_50_10MS = "books50-l2-tbt" # 深度频道，50  档行情，每 10ms  增量推送 (VIP4)
    BOOK_400_10MS = "books-l2-tbt"  # 深度频道，400 档行情，每 10ms  增量推送 (VIP5)
 

class OKX_WS_OP:
    LOGIN = "login"
    SUBSCRIBE = "subscribe"
    UNSUBSCRIBE = "unsubscribe"
    SUBMIT_ORD = "order"
    CANCEL_ORD = "cancel-order"


# class INST_ID:
#     BTC_USDT = "BTC-USDT"
#     BTC_USDT_SWAP = "BTC-USDT-SWAP"
#     ETH_USDT = "ETH-USDT"
#     ETH_USDT_SWAP = "ETH-USDT-SWAP"


# class INST_TYPE:
#     SPOT = "SPOT"
#     SWAP = "SWAP"



