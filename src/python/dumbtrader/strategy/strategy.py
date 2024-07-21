from enum import Enum, auto
from abc import ABC, abstractmethod

class Signal(Enum):
    SUBMIT = auto()
    WITHDRAW = auto()

class OrderType(Enum):
    LIMIT = auto()
    MARKET = auto()

class OrderSide(Enum):
    BUY = auto()
    SELL = auto()

class OrderStatus(Enum):
    SUBMITTED = auto()
    REJECTED = auto()
    OPEN = auto()
    PARTIAL_FILLED = auto()
    FILLED = auto()
    CANCELED = auto()

class Order:
    def __init__(self, side, type, inst_id, volume, px, internal_id):
        if not isinstance(side, OrderSide):
            raise ValueError(f"Invalid order side: {side}. Must be an instance of OrderStatus.")
        if not isinstance(type, OrderType):
            raise ValueError(f"Invalid order type: {type}. Must be an instance of OrderStatus.")

        self.side = side  
        self.type = type  
        self.inst_id = inst_id
        self.volume = volume  
        self.px = px
        self.internal_id = internal_id # creater need to provide unique id
        self.external_id = None
        self.status = None

    def __lt__(self, other) -> bool:
        return self.px < other.px if self.side == OrderSide.SELL else self.px > other.px
    
    def __hash__(self) -> int:
        return hash(self.internal_id)
    
    def __eq__(self, __value: object) -> bool:
        return isinstance( __value, Order) and self.internal_id == __value.internal_id
    
    def __str__(self) -> str:
        return f"{self.type} {self.side} {self.volume} {self.inst_id}"
    
class LmtSellOrder(Order):
    def __init__(self, inst_id, volume, px, internal_id):
        super().__init__(OrderSide.SELL, OrderType.LIMIT, inst_id, volume, px, internal_id)
    
class LmtBuyOrder(Order):
    def __init__(self, inst_id, volume, px, internal_id):
        super().__init__(OrderSide.BUY, OrderType.LIMIT, inst_id, volume, px, internal_id)

class MktSellOrder(Order):
    def __init__(self, inst_id, volume, internal_id):
        super().__init__(OrderSide.SELL, OrderType.MARKET, inst_id, volume, 0, internal_id)
    
class MktBuyOrder(Order):
    def __init__(self, inst_id, volume, internal_id):
        super().__init__(OrderSide.BUY, OrderType.MARKET, inst_id, volume, 0, internal_id)

class Strategy(ABC):
    def __init__(self):
        pass

    @abstractmethod
    def on_start(self, px_tuple):
        return []
    
    @abstractmethod
    def on_order_filled(self, internal_ord_id):
        return []
    
    @abstractmethod
    def on_px_change(self, px_tuple):
        return []