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
    N = 5000000
    strategy = EmaGridStrategy(inst_id, 3066.31, 3073.72, 2, 0.1, N)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    credential_path = os.path.join(script_dir, '../../credentials/okx-paper-key.json')
    with open(credential_path, 'r') as file:
        config = json.load(file)
        api_key = config.get('api_key')
        secret_key = config.get('secret_key')
        passphrase = config.get('passphrase')

    async with AsyncExitStack() as stack:
        listen_trades_all_client = await stack.enter_async_context(OkxWsListenTradesAllClient(OKX_WS_URI.PUBLIC_BUSINESS, inst_id))
        listen_order_client = await stack.enter_async_context(OkxWsListenOrdersClient(OKX_WS_URI.PRIVATE_PAPER, api_key, passphrase, secret_key, inst_type, inst_id))
        place_order_client = await stack.enter_async_context(OkxWsPlaceOrderClient(OKX_WS_URI.PRIVATE_PAPER, api_key, passphrase, secret_key))

        executor = OkxExecutor(strategy, listen_trades_all_client, listen_order_client, place_order_client)
        await executor.run()


if __name__ == '__main__':
    asyncio.run(main())