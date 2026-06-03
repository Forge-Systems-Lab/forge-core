import os
import random
import time
import sys

TARGET_DIR = "agent_telemetry"
TOTAL_FILES = 10
ROWS_PER_FILE = 10000

os.makedirs(TARGET_DIR, exist_ok=True)
AGENT_IDS = [f"agent_{i:02d}" for i in range(1, 26)]

# Determine the target failure profile based on execution arguments
scenario = sys.argv[1] if len(sys.argv) > 1 else "default"

print(f"🚀 Manufacturing {TOTAL_FILES * ROWS_PER_FILE:,} traces under scenario profile: [{scenario.upper()}]")

for batch_idx in range(TOTAL_FILES):
    filepath = os.path.join(TARGET_DIR, f"agent_trace_batch_{batch_idx+1}.csv")
    
    with open(filepath, "w") as f:
        f.write("timestamp,agent_id,execution_time_ms,input_tokens,output_tokens,retry_count,status\n")
        
        for _ in range(ROWS_PER_FILE):
            timestamp = int(time.time()) - random.randint(0, 86400)
            agent_id = random.choice(AGENT_IDS)
            
            if scenario == "token_explosion":
                # 🟡 Triggering a massive data/cost breach scenario
                exec_time = random.randint(1500, 3000)
                in_tokens = random.randint(180000, 300000)  # Catastrophic context bloat
                out_tokens = random.randint(50000, 95000)
                retries = random.randint(0, 1)
                status = "SUCCESS"
                
            elif scenario == "latency_drain":
                # 🔴 Triggering model degradation/network queue lockups
                exec_time = random.randint(6000, 12000)  # Extreme execution delays
                in_tokens = random.randint(1000, 3000)
                out_tokens = random.randint(200, 800)
                retries = random.randint(1, 3)
                status = "SUCCESS"
                
            else:
                # 🟢 Default blended distribution profile
                distribution_roll = random.random()
                if distribution_roll < 0.80:
                    exec_time = random.randint(150, 650)
                    in_tokens = random.randint(800, 4500)
                    out_tokens = random.randint(150, 1200)
                    retries = 0
                    status = "SUCCESS"
                elif distribution_roll < 0.90:
                    exec_time = random.randint(1200, 3500)
                    in_tokens = random.randint(60000, 250000)
                    out_tokens = random.randint(20000, 90000)
                    retries = random.randint(0, 2)
                    status = "SUCCESS"
                else:
                    exec_time = random.randint(5000, 15000)
                    in_tokens = random.randint(1500, 6000)
                    out_tokens = random.randint(0, 300)
                    retries = random.randint(5, 18)
                    status = random.choice(["FAILED", "TIMEOUT"])
                    
            f.write(f"{timestamp},{agent_id},{exec_time},{in_tokens},{out_tokens},{retries},{status}\n")

print("🎯 Simulation generation complete.")
