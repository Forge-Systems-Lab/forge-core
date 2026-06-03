import sqlite3

DB_FILE = "pipeline_logs.db"

print("🔍 Reading Historical Diagnostics Ledger...\n")

try:
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    
    # Fetch the most recent entries from the history table
    cursor.execute("SELECT id, timestamp, total_traces, estimated_cost, avg_latency, health_score, incident_tag FROM agent_fleet_history ORDER BY id DESC LIMIT 5")
    rows = cursor.fetchall()
    
    if not rows:
        print("⏳ Ledger is currently empty. Run consumer.py to write a record.")
    else:
        print(f"{'ID':<4} | {'Timestamp':<19} | {'Traces':<8} | {'Cost':<9} | {'Latency':<10} | {'Health':<6} | {'Incident Tag'}")
        print("-" * 85)
        for row in rows:
            print(f"{row[0]:<4} | {row[1]:<19} | {row[2]:<8,} | ${row[3]:<7.2f} | {row[4]:<7.2f}ms | {row[5]:<6}/100 | {row[6]}")
            
    conn.close()
except Exception as e:
    print(f"❌ Database read failure: {e}")
