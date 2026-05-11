import random

FILENAME = "chaos_data.csv"
TOTAL_ROWS = 10_000_000 # Adjust based on your RAM/Disk
POISON_COUNT = 1000
POISON_INDICES = set(random.sample(range(TOTAL_ROWS), POISON_COUNT))

print(f"Generating {TOTAL_ROWS} rows with {POISON_COUNT} poisoned records...")

with open(FILENAME, "w") as f:
    for i in range(TOTAL_ROWS):
        # Column 1: ID (Integer)
        # Column 2: Value (Integer, but sometimes poisoned)
        if i in POISON_INDICES:
            # Inject a non-digit 'A' into the numeric field
            val = f"{random.randint(100, 888)}A{random.randint(10, 99)}"
        else:
            val = random.randint(10000, 99999)
            
        f.write(f"{i},{val},DATA_BLOCK_{i % 100}\n")

print(f"Done. File '{FILENAME}' is ready for the Sentinel.")
