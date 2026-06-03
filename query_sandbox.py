import sqlite3
import os

def initialize_sql_sandbox(db_name="pipeline_logs.db"):
    print(f"[INIT] Instantiating relational database framework: {db_name}")
    
    # Establish local relational database file hook
    connection = sqlite3.connect(db_name)
    cursor = connection.cursor()
    
    # 1. Construct target tracking schema
    cursor.execute("""
    CREATE TABLE IF NOT EXISTS pipeline_logs (
        log_id INTEGER PRIMARY KEY AUTOINCREMENT,
        service_name TEXT NOT NULL,
        status_msg TEXT NOT NULL,
        compute_cost_tokens REAL NOT NULL
    );
    """)
    
    # 2. Populate validation dataset parameters
    sample_dataset = [
        ('forge-core', 'PROCESSING_COMPLETE', 0.012),
        ('forge-stream', 'STREAM_INIT', 0.045),
        ('forge-ipc', 'BUFFER_SATURATED', 0.089),
        ('forge-core', 'PROCESSING_COMPLETE', 0.011),
        ('forge-stream', 'SYSTEM_READY', 0.005),
        ('forge-core', 'RESOURCE_ERROR', 0.120),
        ('forge-ipc', 'BUFFER_SATURATED', 0.095),
        ('forge-core', 'PROCESSING_COMPLETE', 0.014),
    ]
    
    cursor.executemany("""
    INSERT INTO pipeline_logs (service_name, status_msg, compute_cost_tokens) 
    VALUES (?, ?, ?);
    """, sample_dataset)
    
    connection.commit()
    print("[SUCCESS] Production mock infrastructure populated cleanly.")
    
    # 3. Execute the Filtering Sequence Test (WHERE vs. HAVING)
    print("\n[QUERY EXECUTION] Isolating high-cost resource bottlenecks:")
    cursor.execute("""
        SELECT service_name, COUNT(*) as incident_count, SUM(compute_cost_tokens) as total_bleed
        FROM pipeline_logs
        WHERE status_msg != 'STREAM_INIT'
        GROUP BY service_name
        HAVING COUNT(*) >= 2;
    """)
    
    results = cursor.fetchall()
    print("-" * 65)
    print(f"{'Service Name':<15} | {'Incident Count':<14} | {'Total Cost Bleed':<15}")
    print("-" * 65)
    for row in results:
        print(f"{row[0]:<15} | {row[1]:<14} | {row[2]:<15}")
    print("-" * 65)
    
    connection.close()

if __name__ == "__main__":
    initialize_sql_sandbox()
