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
    strategy = EmaGridStrategy(inst_id, 3086.31, 3203.72, 4, 0.1, N)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    credential_path = os.path.join(script_dir, '../../credentials/okx-paper-key.json')
    with open(credential_path, 'r') as file:
        config = json.load(file)
        api_key = config.get('api_key')
        secret_key = config.get('secret_key')
        passphrase = config.get('passphrase')

    executor = OkxExecutor(strategy, inst_id, inst_type, api_key, passphrase, secret_key)
    executor.run()


if __name__ == '__main__':
    asyncio.run(main())