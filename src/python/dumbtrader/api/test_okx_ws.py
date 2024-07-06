import asyncio
import os
import uuid

from okx.okxapiws import *

async def test_subscribe():
    uri = "wss://ws.okx.com:8443/ws/v5/business"
    async with websockets.connect(uri) as ws:
        channel = OkxWsChannel(ws)
        response = await channel.subscribe("trades-all", "ETH-USDT-SWAP")
        print(f"subscribe success, connection id: {response}")
        response = await channel.recv_data()
        print(f"received data: {response}")

async def test_login_and_order():
    demo_trading_private_uri = "wss://wspap.okx.com:8443/ws/v5/private?brokerId=9999"
    script_dir = os.path.dirname(os.path.abspath(__file__))
    credential_path = os.path.join(script_dir, '../../../../credentials/okx-paper-key.json')

    with open(credential_path, 'r') as file:
        config = json.load(file)
        api_key = config.get('api_key')
        secret_key = config.get('secret_key')
        passphrase = config.get('passphrase')

    async with websockets.connect(demo_trading_private_uri) as ws:
        channel = OkxWsPrivateChannel(ws)

        response = await channel.login(api_key, passphrase, secret_key)
        print(f"login success, connection id: {response}")

        internal_order_id = str(uuid.uuid4()).replace('-', '')
        await channel.submit_order(
            client_order_id=internal_order_id, 
            inst_id="ETH-USDT-SWAP",
            td_mode="isolated",
            side="buy",
            pos_side="long",
            order_type="limit",
            sz=0.1,
            px=2200)
        print(f"submit buy order {internal_order_id} success")

        await channel.cancel_order(
            client_order_id=internal_order_id, 
            inst_id="ETH-USDT-SWAP")
        print(f"cancel order {internal_order_id} success")
        
        
if __name__ == '__main__':
    asyncio.run(test_subscribe())
    asyncio.run(test_login_and_order())