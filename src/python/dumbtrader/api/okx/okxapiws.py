import asyncio
import base64
import hmac
import json
import time
import uuid
import websockets

from .okxconstants import *

class OkxWsChannel:
    def __init__(self, websocket):
        self.websocket = websocket
        self.conn_id = None
        # not saving msg id generated by client for now
        # not saving order id generated by exchange for now

    async def send(self, msg):
        await self.websocket.send(msg)

    async def recv(self):
        while True:
            try:
                response = await asyncio.wait_for(self.websocket.recv(), timeout=25)
            except (asyncio.TimeoutError, websockets.exceptions.ConnectionClosed) as e:
                try:
                    print("connection lost, try to ping server...")
                    await self.websocket.send('ping')
                    response = await self.websocket.recv()
                    print(f"ping response: {response}, continue waiting for response...")
                    continue
                except Exception:
                    print("unexpected error occurs, websocket recv aboarting...")
                    raise
            return response
        
    async def recv_data(self, col_filter=[]):
        response = await self.recv()
        data = json.loads(response)
        if "data" not in data or not data["data"]:
            raise Exception("invalid msg received, error code: {}, message: {}".format(data["code"], data["msg"]))
        for col in col_filter:
            del data["data"][0][col]
        return data["data"][0]
    
    async def subscribe(self, channel, inst_id):
        subscribe_msg = build_subscribe_msg(channel, inst_id)
        await self.send(subscribe_msg)
        response = await self.recv()
        data = json.loads(response)
        if "event" not in data or data["event"] != "subscribe" or "connId" not in data:
            raise Exception(f"subscribe failed, response: {response}")
        return data["connId"]

class OkxWsPrivateChannel(OkxWsChannel):
    async def login(self, api_key, passphrase, secret_key):
        login_msg = build_login_msg(api_key, passphrase, secret_key)
        await self.websocket.send(login_msg)
        response = await self.recv()
        data = json.loads(response)
        if "event" not in data or data["event"] != "login" or "connId" not in data:
            raise Exception(f"login failed, response: {response}")
        return data["connId"]
    
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
    return build_json_msg(WS_OP.LOGIN, login_args)

def build_subscribe_msg(channel, inst_id):
    subscribe_args = [{"channel": channel,"instId": inst_id}]
    return build_json_msg(WS_OP.SUBSCRIBE, subscribe_args)

def build_submit_order_msg(client_order_id, inst_id, td_mode, side, pos_side, order_type, sz, px):
    order_args = [{"instId": inst_id,
                   "tdMode": td_mode,
                   "clOrdId": client_order_id,
                   "side": side,
                   "posSide": pos_side,
                   "ordType": order_type,
                   "sz": sz,
                   "px": px}]
    return build_json_msg(WS_OP.SUBMIT_ORD, order_args, str(uuid.uuid4()).replace('-', ''))

def build_cancel_order_msg(inst_id, client_order_id):
    order_args = [{"instId": inst_id, "clOrdId": client_order_id}]
    return build_json_msg(WS_OP.CANCEL_ORD, order_args, str(uuid.uuid4()).replace('-', ''))
