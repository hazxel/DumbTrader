import asyncio
import base64
import hmac
import json
import time
import uuid
import websockets

from dumbtrader.api.okxws.constants import *

class OkxWsClient:
    def __init__(self, uri):
        self.uri = uri
        self.websocket = None
        # not saving connection id generated by exchange for now
        # not saving msg id generated by client for now
        # not saving order id generated by exchange for now
    
    async def __aenter__(self):
        self.websocket = await websockets.connect(self.uri)
        return self

    async def __aexit__(self, exc_type, exc, tb):
        if self.websocket:
            await self.websocket.close()
            self.websocket = None

    async def send(self, msg):
        await self.websocket.send(msg)

    async def recv(self):
        while True:
            try:
                try:
                    response = await asyncio.wait_for(self.websocket.recv(), timeout=25)
                    if response == 'pong':
                        print(f"expecting a normal message push but recieved pong, ignoring...")
                    else:
                        return response
                except (asyncio.TimeoutError, websockets.exceptions.ConnectionClosed) as e:
                    print("to keep connection alive, ping server...")
                    await self.websocket.send('ping')
                    response = await asyncio.wait_for(self.websocket.recv(), timeout=25)
                    if response == 'pong':
                        print(f"pong received, continue waiting...")
                    else:
                        print(f"expecting pong but recieved a normal message push, return it anyway")
                        return response
            except Exception:
                print("connection lost, create new one...")
                await self.renew_websocket()

    async def renew_websocket(self):
        if self.websocket:
            await self.websocket.close()
            self.websocket = None
        self.websocket = await websockets.connect(self.uri)
        print("websocket reconnected")

    async def recv_data(self, col_filter=[]):
        response = await self.recv()
        data = json.loads(response)
        if "data" not in data or not data["data"]:
            print(f"data: {data}")
            raise Exception("invalid msg received, error code: {}, message: {}".format(data["code"], data["msg"]))
        for col in col_filter:
            del data["data"][0][col]
        return data["data"][0]
    
    async def subscribe(self, subscribe_msg):
        await self.send(subscribe_msg)
        response = await self.recv()
        data = json.loads(response)
        if "event" not in data or data["event"] != "subscribe" or "connId" not in data:
            raise Exception(f"subscribe failed, response: {response}")
        print("subscribed")
        return data["connId"]

class OkxWsPrivateClient(OkxWsClient):
    def __init__(self, uri, api_key, passphrase, secret_key):
        super().__init__(uri)
        self.api_key = api_key
        self.passphrase = passphrase
        self.secret_key = secret_key

    async def __aenter__(self):
        await super().__aenter__()
        await self.login()
        return self
    
    async def renew_websocket(self):
        await super().renew_websocket()
        await self.login()

    async def login(self):
        login_msg = build_login_msg(self.api_key, self.passphrase, self.secret_key)
        await self.websocket.send(login_msg)
        response = await self.recv()
        data = json.loads(response)
        if "event" not in data or data["event"] != "login" or "connId" not in data:
            raise Exception(f"login failed, response: {response}")
        print("logged in")
        return data["connId"]

class OkxWsListenTradesAllClient(OkxWsClient):
    def __init__(self, uri, inst_id):
        super().__init__(uri)
        self.inst_id = inst_id
    
    async def __aenter__(self):
        await super().__aenter__()
        await self.subscribe()
        return self
    
    async def renew_websocket(self):
        await super().renew_websocket()
        await self.subscribe()

    async def subscribe(self):
        subscribe_msg = build_subscribe_trades_all_msg(OKX_WS_SUBSCRIBE_CHANNEL.TRADES_ALL, self.inst_id)
        connection_id = await super().subscribe(subscribe_msg)
        return connection_id

class OkxWsListenOrdersClient(OkxWsPrivateClient):
    def __init__(self, uri, api_key, passphrase, secret_key, inst_type, inst_id):
        super().__init__(uri, api_key, passphrase, secret_key)
        self.inst_type = inst_type
        self.inst_id = inst_id

    async def __aenter__(self):
        await super().__aenter__()
        await self.subscribe()
        return self
    
    async def renew_websocket(self):
        await super().renew_websocket()
        await self.subscribe()

    async def subscribe(self):
        subscribe_msg = build_subscribe_orders_msg(OKX_WS_SUBSCRIBE_CHANNEL.ORDERS, self.inst_type, self.inst_id)
        connection_id = await super().subscribe(subscribe_msg)
        # on new private channel subscription, server will send channel-conn-count message to update connection count
        print("waiting for channel-conn-count...")
        response = await self.recv()
        data = json.loads(response)
        if "event" not in data or data["event"] != "channel-conn-count":
            raise Exception(f"recieve channel-conn-count failed, response: {response}")
        print("channel-conn-count recieved")
        return connection_id

class OkxWsPlaceOrderClient(OkxWsPrivateClient):
    async def submit_order(self, client_order_id, inst_id, td_mode, side, pos_side, order_type, sz, px):
        submit_order_msg = build_submit_order_msg(client_order_id, inst_id, td_mode, side, pos_side, order_type, sz, px)
        await self.send(submit_order_msg)
        response = await self.recv()
        data = json.loads(response)
        if ("code" not in data or data["code"] != "0" or
            "op" not in data or data["op"] != "order" or
            "data" not in data or len(data["data"])==0 or
            "sCode" not in data["data"][0] or data["data"][0]["sCode"] != "0"
        ):
            raise Exception(f"submit order failed, response: {response}")
        # not using order id generated by exchange for now
        # return data["data"][0]["ordId"]
    
    async def cancel_order(self, inst_id, client_order_id):
        cancel_order_msg = build_cancel_order_msg(inst_id, client_order_id)
        await self.send(cancel_order_msg)
        response = await self.recv()
        data = json.loads(response)
        if ("code" not in data or data["code"] != "0" or
            "op" not in data or data["op"] != "cancel-order" or
            "data" not in data or len(data["data"])==0 or
            "sCode" not in data["data"][0] or data["data"][0]["sCode"] != "0"
        ):
            raise Exception(f"cancel order failed, response: {response}")
        # not using order id generated by exchange for now
        # return data["data"][0]["ordId"]


def build_json_msg(op, args, msg_id=None):
    info = {"op": op, "args": args}
    if msg_id is not None:
        info["id"] = msg_id
    return json.dumps(info)

def build_login_msg(api_key, passphrase, secret_key):
    ts_str = str(int(time.time()))
    message = ts_str + 'GET' + '/users/self/verify'
    mac = hmac.new(bytes(secret_key, encoding='utf8'), bytes(message, encoding='utf-8'), digestmod='sha256')
    d = mac.digest()
    sign = base64.b64encode(d)
    login_args = [{ "apiKey": api_key,
                    "passphrase": passphrase,
                    "timestamp": ts_str,
                    "sign": sign.decode("utf-8")}]
    return build_json_msg(OKX_WS_OP.LOGIN, login_args)

def build_subscribe_trades_all_msg(channel, inst_id):
    subscribe_args = [{"channel": channel,"instId": inst_id}]
    return build_json_msg(OKX_WS_OP.SUBSCRIBE, subscribe_args)

def build_subscribe_orders_msg(channel, inst_type, inst_id):
    subscribe_args = [{"channel": channel, "instType": inst_type, "instId": inst_id}]
    return build_json_msg(OKX_WS_OP.SUBSCRIBE, subscribe_args)

def build_submit_order_msg(client_order_id, inst_id, td_mode, side, pos_side, order_type, sz, px):
    order_args = [{"instId": inst_id,
                   "tdMode": td_mode,
                   "clOrdId": client_order_id,
                   "side": side,
                   "posSide": pos_side,
                   "ordType": order_type,
                   "sz": sz,
                   "px": px}]
    return build_json_msg(OKX_WS_OP.SUBMIT_ORD, order_args, str(uuid.uuid4()).replace('-', ''))

def build_cancel_order_msg(inst_id, client_order_id):
    order_args = [{"instId": inst_id, "clOrdId": client_order_id}]
    return build_json_msg(OKX_WS_OP.CANCEL_ORD, order_args, str(uuid.uuid4()).replace('-', ''))
