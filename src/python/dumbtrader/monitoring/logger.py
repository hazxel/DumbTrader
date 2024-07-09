from abc import ABC, abstractmethod

class Logger(ABC):
    @abstractmethod
    def log(self):
        pass