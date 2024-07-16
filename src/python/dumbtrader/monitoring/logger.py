from abc import ABC, abstractmethod
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
        config_path = os.path.join(script_dir, '../../../../credentials/telegram-bot-config.json')
        with open(config_path, 'r') as file:
            config = json.load(file)
            token = config.get('token')
            self.channel_id = config.get('channel-id')
        self.url = f'https://api.telegram.org/bot{token}/'
        self.broken_connection = False

        try:
            send_telegram_message(self.url, "test connection")
        except Exception:
            self.broken_connection = True

    def log(self, message, log_level=LogLevel.INFO):
        if log_level in [LogLevel.TRACE, LogLevel.DEBUG] or self.broken_connection:
            return
        timestamp = int(time.time())
        thread = threading.Thread(target=send_telegram_message, args=(self.url, self.channel_id, f"[{timestamp}]: {message}"))
        thread.daemon = True
        thread.start()