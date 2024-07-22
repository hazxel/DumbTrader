import asyncio
from contextlib import AsyncExitStack
import os

from dumbtrader.livetrading.okx_executor import *
from dumbtrader.strategy.grid_strategy import *
from dumbtrader.api.okxws.client import *
from dumbtrader.api.okxws.constants import *

async def main():
    inst_id = "ETH-USDT-SWAP"
    inst_type = "SWAP"

    script_dir = os.path.dirname(os.path.abspath(__file__))
    credential_path = os.path.join(script_dir, '../credentials/okx-paper-key.json')
    with open(credential_path, 'r') as file:
        config = json.load(file)
        api_key = config.get('api_key')
        secret_key = config.get('secret_key')
        passphrase = config.get('passphrase')

    async with AsyncExitStack() as stack:
        listen_order_client = await stack.enter_async_context(OkxWsListenOrdersClient(OKX_WS_URI.PRIVATE_PAPER, api_key, passphrase, secret_key, inst_type, inst_id))
        while True:
            data = await listen_order_client.recv_data()
            print(f"order update - px: {data['px']}, status: {data['state']}, ordId: {data['ordId']}")        


if __name__ == '__main__':
    asyncio.run(main())