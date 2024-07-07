import asyncio
import csv
import sys
import time
import websockets

from dumbtrader.api.okxws.client import *
from dumbtrader.api.okxws.constants import *

# one websocker connection for one instrument
async def ws_channel_dump_csv(inst_id, new_file_period_s, dump_path, col_filter=[]):
    while True:
        try:
            async with websockets.connect(OKX_WS_URI.PUBLIC_BUSINESS) as ws:
                client = OkxWsClient(ws)
                response = await client.subscribe(OKX_WS_SUBSCRIBE_CHANNEL.TRADES_ALL, inst_id)
                print((f"subscribe success, connection id: {response}"))
                while True:
                    period_ts_prefix = int(time.time()) // new_file_period_s
                    filename = f"{dump_path}/{inst_id}-{OKX_WS_SUBSCRIBE_CHANNEL.TRADES_ALL}-{period_ts_prefix}.csv"
                    with open(filename, 'a', newline='') as file:
                        # recv first response to get field names
                        row = await client.recv_data(col_filter)
                        writer = csv.DictWriter(f=file, fieldnames=row.keys())
                        if file.tell() == 0:
                            writer.writeheader()
                        writer.writerow(row)
                        # resp ts is in ms, while time.time() is in s
                        stop_ts = (period_ts_prefix + 1) * new_file_period_s * 1000
                        while int(row["ts"]) < stop_ts:
                            row = await client.recv_data(col_filter)
                            writer.writerow(row)
        except Exception as e:
            print(f"Caught a general exception: {e}, setting up new websockt connection...")


async def ws_channel_dump_tradeall_csv(inst_id_list, dump_path="."):
    tasks = [
        asyncio.create_task(ws_channel_dump_csv(
            inst_id = inst_id,
            new_file_period_s = 10000,
            path = dump_path,
            col_filter=['instId', 'tradeId']
        )) 
        for inst_id in inst_id_list
    ]
    await asyncio.gather(*tasks)

if __name__ == "__main__":
    asyncio.run(ws_channel_dump_tradeall_csv(["BTC-USDT-SWAP","ETH-USDT-SWAP"], sys.argv[1]))
