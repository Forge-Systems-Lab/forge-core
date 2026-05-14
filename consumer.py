import json
import time
import os

def analyze_market_signals():
    print("🤖 [FORGE-AGENT] Monitoring for Intelligence Payload...")
    
    # Wait for the C engine to produce the contract
    while not os.path.exists("intelligence.json"):
        time.sleep(0.5)

    with open("intelligence.json", "r") as f:
        data = json.load(f)

    print(f"\n✅ [CONTRACT RECEIVED] Engine: {data['engine']}")
    print(f"📈 Performance: {data['metadata']['throughput_m_rows_sec']} M Rows/Sec")
    
    for col in data['intelligence']:
        if col['type'] == "INT":
            metrics = col['metrics']
            print(f"\n--- Column {col['column']} Strategy ---")
            print(f"📍 Mean: {metrics['avg']:.2f}")
            print(f"📉 StdDev: {metrics['std_dev']:.2f}")
            print(f"📊 Volatility: {metrics['volatility_index']:.4f}")
            
            # Simulated Execution Logic
            if metrics['std_dev'] > 25.0:
                print("🚨 ALERT: High Volatility Detected. Shift to Defensive Strategy.")
            else:
                print("🛡️ STATUS: Market Stable. Normal Operations.")

if __name__ == "__main__":
    analyze_market_signals()
