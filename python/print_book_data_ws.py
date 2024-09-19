import asyncio

from dumbtrader.api.okxws.client import *
from dumbtrader.api.okxws.constants import *

async def main():
    while True:
        try:
            with open("book.txt", 'a', newline='') as file:
                async with OkxWsListenDepthClient(OKX_WS_URI.PUBLIC_GENERAL, "ETH-USDT-SWAP") as client:
                    while True:
                        row = await client.recv_data()
                        for ar in row['asks']:
                            del ar[2]
                            line = ' '.join(map(str, ar))
                            file.write(line + '\n')
                        for br in row['bids']:
                            del br[2]
                            line = ' '.join(map(str, br))
                            file.write(line + '\n')

        except Exception as e:
            print(f"Caught a general exception: {e}, setting up new okx websocket client...")

if __name__ == "__main__":
    asyncio.run(main())
