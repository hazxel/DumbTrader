from enum import Enum, auto
from abc import ABC, abstractmethod

class Signal(Enum):
    SUBMIT = auto()
    WITHDRAW = auto()

class OrderType(Enum):
    LIMIT = auto()
    MARKET = auto()

class OrderDirection(Enum):
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
    def __init__(self, direction, type, inst_id, volume, px, internal_id):
        if not isinstance(direction, OrderDirection):
            raise ValueError(f"Invalid order direction: {direction}. Must be an instance of OrderStatus.")
        if not isinstance(type, OrderType):
            raise ValueError(f"Invalid order type: {type}. Must be an instance of OrderStatus.")

        self.direction = direction  
        self.type = type  
        self.inst_id = inst_id
        self.volume = volume  
        self.px = px
        self.internal_id = internal_id # creater need to provide unique id
        self.external_id = None
        self.status = None

    def __lt__(self, other) -> bool:
        return self.px < other.px if self.direction == OrderDirection.SELL else self.px > other.px
    
    def __hash__(self) -> int:
        return hash(self.internal_id)
    
    def __eq__(self, __value: object) -> bool:
        return isinstance( __value, Order) and self.internal_id == __value.internal_id
    
class LmtSellOrder(Order):
    def __init__(self, inst_id, volume, px, internal_id):
        super().__init__(OrderDirection.SELL, OrderType.LIMIT, inst_id, volume, px, internal_id)
    
class LmtBuyOrder(Order):
    def __init__(self, inst_id, volume, px, internal_id):
        super().__init__(OrderDirection.BUY, OrderType.LIMIT, inst_id, volume, px, internal_id)

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