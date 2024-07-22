from abc import ABC, abstractmethod
from concurrent.futures import ThreadPoolExecutor
from enum import Enum
import json
import os
import threading
import time
import datetime

from dumbtrader.monitoring.telegram_adapter import *

class LogLevel(Enum):
    TRACE = "TRACE"
    DEBUG = "DEBUG"
    INFO = "INFO"
    WARNING = "WARNING"
    ERROR = "ERROR"

class Logger(ABC):
    @abstractmethod
    def log(self, message, log_level):
        pass

class VersatileLogger(Logger):
    def __init__(self, *loggers):
        self.loggers = list(loggers)

    def log(self, message, log_level=LogLevel.INFO):
        for logger in self.loggers:
            logger.log(message, log_level)


class FileLogger(Logger):
    def __init__(self, log_file_name=None):
        if log_file_name is None:
            current_date = datetime.datetime.now().strftime("%Y-%m-%d")
            self.filename = os.path.join(os.getcwd(), f"log-{current_date}.txt")
        else:
            self.filename = log_file_name
        self.log_file = open(self.filename, 'a')
    
    def log(self, message, log_level=LogLevel.INFO):
        date_time = datetime.datetime.now().replace(microsecond=0)
        timestamp = int(date_time.timestamp())
        self.log_file.write(f"[{log_level.value}]<{date_time}>-<{timestamp}>: {message}\n")
        self.log_file.flush()
    
    def __del__(self):
        if self.log_file:
            self.log_file.close()

class TelegramBotLogger(Logger):
    def __init__(self):
        script_dir = os.path.dirname(os.path.abspath(__file__))
        config_path = os.path.join(script_dir, '../../../credentials/telegram-bot-config.json')
        with open(config_path, 'r') as file:
            config = json.load(file)
            token = config.get('token')
            self.channel_id = config.get('channel-id')
        self.bot = TelegramBot(token)
        self.executor = ThreadPoolExecutor(max_workers=10)  # 限制最大线程数量
        self.broken_connection = False

        try:
            self.bot.send(self.channel_id, "test connection")
        except Exception:
            self.broken_connection = True

    def log(self, message, log_level=LogLevel.INFO):
        if log_level in [LogLevel.TRACE, LogLevel.DEBUG] or self.broken_connection:
            return
        timestamp = int(time.time())
        print("going to send msg")
        self.executor.submit(self.bot.send, self.channel_id, f"[{timestamp}]: {message}")

_TG_LOGGER_INSTANCE = TelegramBotLogger()
_FILE_LOGGER_INSTANCE = FileLogger()
_VERSATILE_LOGGER_INSTANCE = VersatileLogger(_TG_LOGGER_INSTANCE, _FILE_LOGGER_INSTANCE)
_LOGGER = None

def init_logger(logger_type):
    global _LOGGER
    if logger_type == TelegramBotLogger:
        _LOGGER = _TG_LOGGER_INSTANCE
    elif logger_type == FileLogger:
        _LOGGER = _FILE_LOGGER_INSTANCE
    elif logger_type == VersatileLogger:
        _LOGGER = _VERSATILE_LOGGER_INSTANCE
    else:
        raise Exception(f"invalid logger type: {logger_type}")

def log(message, log_level=LogLevel.INFO):
    if not _LOGGER:
        raise Exception("logger not initialized")
    _LOGGER.log(message, log_level)
