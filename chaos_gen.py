import random

def generate_chaos(filename, rows=1000):
    with open(filename, 'w') as f:
        for i in range(rows):
            choice = random.random()
            
            if choice < 0.7: # 70% Valid Data
                f.write(f"{i},Valid_Data,Active\n")
            
            elif choice < 0.8: # Extra Columns (The "Overflow" Attack)
                f.write(f"{i},Broken,Data,Extra,Fields,That,Should,Not,Be,Here\n")
            
            elif choice < 0.9: # Missing Columns (The "Underflow" Attack)
                f.write(f"{i},Missing_Fields\n")
            
            else: # The "Nuclear" Option (Random Nulls and Long Strings)
                garbage = "A" * 5000 # Massive row length
                f.write(f"{i},Chaos_{garbage},Err\n")

if __name__ == "__main__":
    generate_chaos("chaos_test.csv", rows=5000)
    print("🔥 Chaos file 'chaos_test.csv' generated with 5,000 hostile rows.")
