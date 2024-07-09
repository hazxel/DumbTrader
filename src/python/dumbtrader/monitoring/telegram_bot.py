import json
import os
import requests
import threading

from dumbtrader.monitoring.logger import *

def get_updates(url, offset=None):
    url = url + 'getUpdates'
    params = {'timeout': 100, 'offset': offset}
    response = requests.get(url, params=params)
    return response.json()

def send_message(url, chat_id, text):
    url = url + 'sendMessage'
    params = {'chat_id': chat_id, 'text': text}
    requests.get(url, params=params)

class TelegramBotLogger(Logger):
    def __init__(self):
        script_dir = os.path.dirname(os.path.abspath(__file__))
        config_path = os.path.join(script_dir, '../../../../credentials/telegram-bot-config.json')
        with open(config_path, 'r') as file:
            config = json.load(file)
            token = config.get('token')
            self.channel_id = config.get('channel-id')
        self.url = f'https://api.telegram.org/bot{token}/'

    def log(self, msg):
        thread = threading.Thread(target=send_message, args=(self.url, self.channel_id, msg))
        thread.daemon = True
        thread.start()
        