import os
import pandas as pd
import pickle

def convert_csv_to_pickled_df(file_path):
    if not os.path.exists(file_path):
        print(f"File {file_path} does not exist. Skipping.")
    df = pd.read_csv(file_path)
    with open(file_path+".df.pkl", 'wb') as f:
        pickle.dump(df, f)