import requests

def get_telegram_updates(url, offset=None):
    url = url + 'getUpdates'
    params = {'timeout': 100, 'offset': offset}
    response = requests.get(url, params=params)
    return response.json()

def send_telegram_message(url, chat_id, text):
    url = url + 'sendMessage'
    params = {'chat_id': chat_id, 'text': text}
    requests.get(url, params=params)