import os
import pandas as pd

def gen_csv_tuple(file_list):
    for file_path in file_list:
        if not os.path.exists(file_path):
            print(f"File {file_path} does not exist. Skipping.")
            continue
        
        try:
            df = pd.read_csv(file_path)
        except Exception as e:
            print(f"Failed to read {file_path}: {e}")
            continue
        
        for tuple in df.itertuples(index=False):
            yield tuple

def gen_csv_record(file_list):
    for file_path in file_list:
        if not os.path.exists(file_path):
            print(f"File {file_path} does not exist. Skipping.")
            continue
        
        try:
            df = pd.read_csv(file_path)
        except Exception as e:
            print(f"Failed to read {file_path}: {e}")
            continue

        dict_records = df.to_dict('records')
        
        for record in dict_records:
            yield record

def find_files_with_prefix(path, prefix):
    files = os.listdir(path)
    matching_files = [os.path.join(path, f) for f in files if f.startswith(prefix)]
    sorted_files = sorted(matching_files)
    return sorted_files