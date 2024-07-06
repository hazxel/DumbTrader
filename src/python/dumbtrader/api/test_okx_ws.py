import asyncio
import os
import uuid

from dumbtrader.api.okxws.client import *
from dumbtrader.api.okxws.constants import *

async def test_subscribe_trades():
    async with websockets.connect(WS_URI.PUBLIC_GENERAL) as ws:
        client = OkxWsClient(ws)
        response = await client.subscribe(WS_SUBSCRIBE_CHANNEL.TRADES, "ETH-USDT-SWAP")
        print(f"subscribe success, connection id: {response}")
        response = await client.recv_data()
        print(f"received data: {response}")

async def test_login_and_order():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    credential_path = os.path.join(script_dir, '../../../../credentials/okx-paper-key.json')

    with open(credential_path, 'r') as file:
        config = json.load(file)
        api_key = config.get('api_key')
        secret_key = config.get('secret_key')
        passphrase = config.get('passphrase')

    async with websockets.connect(WS_URI.PRIVATE_PAPER) as ws:
        client = OkxPrivateWsClient(ws)

        response = await client.login(api_key, passphrase, secret_key)
        print(f"login success, connection id: {response}")

        internal_order_id = str(uuid.uuid4()).replace('-', '')
        await client.submit_order(
            client_order_id=internal_order_id, 
            inst_id="ETH-USDT-SWAP",
            td_mode=TD_MODE.ISOLATED,
            side=SIDE.BUY,
            pos_side=POS_SIDE.LONG,
            order_type=ORD_TYPE.LMT,
            sz=0.1,
            px=2200)
        print(f"submit buy order {internal_order_id} success")

        await client.cancel_order(
            client_order_id=internal_order_id, 
            inst_id="ETH-USDT-SWAP")
        print(f"cancel order {internal_order_id} success")
        
        
if __name__ == '__main__':
    asyncio.run(test_subscribe_trades())
    asyncio.run(test_login_and_order())