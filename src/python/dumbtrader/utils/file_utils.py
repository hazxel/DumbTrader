import os
import pandas as pd
import pickle

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

def gen_record_from_csv_files(file_list):
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

def gen_record_from_pickled_dataframes(file_list):
    for file_path in file_list:
        if not os.path.exists(file_path):
            print(f"File {file_path} does not exist. Skipping.")
            continue
        
        try:
            with open(file_path, 'rb') as f:
                df = pickle.load(f)
        except Exception as e:
            print(f"Failed to read {file_path}: {e}")
            continue

        dict_records = df.to_dict('records')
        
        for record in dict_records:
            yield record

def find_files(path, prefix='', suffix=''):
    files = os.listdir(path)
    matching_files = [os.path.join(path, f) for f in files if f.startswith(prefix) and f.endswith(suffix)]
    sorted_files = sorted(matching_files)
    return sorted_files

def find_csv_files_with_prefix(path, prefix):
    return find_files(path, prefix=prefix, suffix='.csv')

def find_pickled_dataframes_with_prefix(path, prefix):
    return find_files(path, prefix=prefix, suffix='.df.pkl')