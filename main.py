import subprocess

subprocess.run('./run.sh 32768 5000 > data.csv', shell=True)

import pandas as pd

df = pd.read_csv('data.csv', header=None)

avg_col1 = df[0].mean()
avg_col2 = df[1].mean()

print(f"Average read latency: {avg_col1}")
print(f"Average write latency: {avg_col2}")