import sys

from dumbtrader.utils.file_utils import *
from dumbtrader.utils.pickle_csv import *

if __name__ == '__main__':
    file_list = find_csv_files_with_prefix(path=sys.argv[1], prefix="ETH-USDT-SWAP-trades-all-172")
    for file_path in file_list:
        convert_csv_to_pickled_df(file_path)