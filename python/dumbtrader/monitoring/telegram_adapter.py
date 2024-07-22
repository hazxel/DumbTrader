import requests

BOT_API_BASE_URL = 'https://api.telegram.org/bot{}/'

class TelegramBot:
    def __init__(self, token):
        self.url = BOT_API_BASE_URL.format(token)

    def get_updates(self, offset=None):
        url = self.url + 'getUpdates'
        params = {'timeout': 100, 'offset': offset}
        response = requests.get(url, params=params)
        return response.json()

    def send(self, chat_id, text):
        url = self.url + 'sendMessage'
        params = {'chat_id': chat_id, 'text': text}
        try:
            requests.get(url, params=params)
        except Exception as e:
            print(f"Error sending http request: {e}")