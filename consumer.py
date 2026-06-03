import json
import os
import sqlite3
from datetime import datetime

# Performance Operational Thresholds
INPUT_TOKEN_RATE_PER_K = 0.0015   # $0.0015 per 1K tokens
OUTPUT_TOKEN_RATE_PER_K = 0.0020  # $0.0020 per 1K tokens
EXPECTED_LATENCY_MS = 800.0       # Standard operating baseline latency
CRITICAL_LATENCY_MS = 2500.0      # Boundary for systemic degradation

CONTRACT_FILE = "intelligence.json"
DB_FILE = "pipeline_logs.db"

print("🤖 [FORGE-INTELLIGENCE] Processing Data Contract Analytics Matrix...")

if os.path.exists(CONTRACT_FILE):
    try:
        with open(CONTRACT_FILE, "r") as f:
            data = json.load(f)
        
        # 1. Harvest Hardware Ingestion Primitives
        total_records = data["total_records"]
        total_in_tokens = data["total_input_tokens"]
        total_out_tokens = data["total_output_tokens"]
        total_failures = data["total_failures"]
        avg_latency = data["avg_execution_time_ms"]
        
        # 2. Compute Core Financial Metrics
        estimated_cost_usd = ((total_in_tokens / 1000) * INPUT_TOKEN_RATE_PER_K) + \
                             ((total_out_tokens / 1000) * OUTPUT_TOKEN_RATE_PER_K)
        failure_rate = total_failures / total_records if total_records > 0 else 0.0
        avg_tokens_per_run = (total_in_tokens + total_out_tokens) / total_records if total_records > 0 else 0
        
        # 3. Dynamic Health Scoring Engine (Starts at 100 Perfect Baseline)
        health_score = 100.0
        root_causes = []
        recommendations = []
        classification = "HEALTHY"
        
        # Penalty Vector A: System Failure Rates
        if failure_rate > 0.0:
            failure_penalty = failure_rate * 400  
            health_score -= failure_penalty
            
        # Penalty Vector B: Latency Inefficiencies
        if avg_latency > EXPECTED_LATENCY_MS:
            latency_delta = avg_latency - EXPECTED_LATENCY_MS
            latency_penalty = (latency_delta / EXPECTED_LATENCY_MS) * 15
            health_score -= latency_penalty
            
        # Penalty Vector C: Massive Token Bloat
        if avg_tokens_per_run > 15000:
            health_score -= 15.0
            
        health_score = max(0.0, min(100.0, health_score))
        
        # 4. Asymmetric Failure Classification & Prescriptive Action Vectors
        if failure_rate >= 0.08:
            classification = "RETRY_STORM"
            root_causes.append("Systemic API/Tool dropouts triggering cascading worker retry logs.")
            recommendations.extend([
                "Activate immediate exponential backoff or dynamic circuit-breaker gates.",
                "Rotate runtime endpoint paths to fallback secondary API providers.",
                "Throttle concurrent pipeline worker allocation sizes to mitigate rate-limiting."
            ])
        elif avg_tokens_per_run > 20000 and estimated_cost_usd > 1000:
            classification = "TOKEN_EXPLOSION"
            root_causes.append("Context injection overflow. Fleet consuming massive input vector boundaries.")
            recommendations.extend([
                "Enforce immediate upper boundaries on runtime context window allocation parameters.",
                "Inject an intermediate text-summarizer abstraction loop before model ingestion.",
                "Set strict max_tokens constraints on multi-turn conversational agent models."
            ])
        elif avg_latency > CRITICAL_LATENCY_MS:
            classification = "HIGH_LATENCY_DRAIN"
            root_causes.append("Severe network/model queuing overhead blocking asynchronous execution contexts.")
            recommendations.extend([
                "Decouple downstream network/tool requests into an asynchronous message queue (e.g., Celery/Redis).",
                "Audit vector database clustering profiles and append strategic index lookups.",
                "Deploy local prompt-caching policies to catch identical recurrent semantic queries."
            ])
        elif failure_rate > 0.02:
            classification = "DEGRADED_PERFORMANCE"
            root_causes.append("Minor structural agent drift detected across localized operations.")
            recommendations.extend([
                "Flag and isolate drifting agent IDs for dynamic parameter re-tuning.",
                "Audit prompt version adjustments against established validation testbeds."
            ])
            
        if health_score < 50.0 and classification == "HEALTHY":
            classification = "RUNAWAY_LOOP"
            root_causes.append("Indeterminate execution cycles draining operational runway buffers.")
            recommendations.extend([
                "Apply rigid graph max-depth recursion parameters inside multi-agent frameworks.",
                "Deploy an out-of-band semantic guardrail thread to intercept recursive loop dialogs."
            ])

        if not root_causes:
            root_causes.append("All tracking parameters operating within verified baseline tolerances.")
            recommendations.append("Maintain active fleet tracking monitoring policies against current baselines.")

        # 5. Render Upgraded Decision-Engine Manifest
        print("\n==================================================")
        print("🛡️  FORGE AI AGENT OBSERVABILITY DIAGNOSTICS")
        print("==================================================")
        print(f"📈 Fleet Health Score     : {int(health_score)}/100")
        print(f"🚨 Incident Tag           : {classification}")
        print(f"💰 Logged Fleet Cost       : ${estimated_cost_usd:.2f}")
        print(f"📦 Active Audited Traces  : {total_records:,}")
        print(f"⏱️  Mean Run Latency        : {avg_latency:.2f} ms")
        print(f"❌ Failed System Cycles   : {total_failures:,} ({failure_rate*100:.1f}%)")
        print("--------------------------------------------------")
        print("🔍 ROOT CAUSE DIAGNOSIS:")
        for cause in root_causes:
            print(f"   • {cause}")
        print("--------------------------------------------------")
        print("💡 RECOMMENDED OPERATIONAL ACTIONS:")
        for idx, action in enumerate(recommendations, 1):
            print(f"   {idx}. {action}")
        print("==================================================\n")
        
        # 6. Relational Persistence Ledger Integration & Safe Migration
        conn = sqlite3.connect(DB_FILE)
        cursor = conn.cursor()
        
        # Construct the core infrastructure tracking table if missing
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS agent_fleet_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT,
                total_traces INTEGER,
                estimated_cost REAL,
                avg_latency REAL,
                total_failures INTEGER,
                health_score INTEGER,
                incident_tag TEXT,
                root_cause TEXT
            )
        ''')
        
        # Safe SQL Migration: Inject recommended_actions column if it does not exist
        try:
            cursor.execute("ALTER TABLE agent_fleet_history ADD COLUMN recommended_actions TEXT")
        except sqlite3.OperationalError:
            pass # Column already exists, swallow the exception safely
        
        # Commit the operational intelligence package to the relational archive
        cursor.execute('''
            INSERT INTO agent_fleet_history 
            (timestamp, total_traces, estimated_cost, avg_latency, total_failures, health_score, incident_tag, root_cause, recommended_actions)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (
            datetime.now().strftime("%Y-%m-%d %H:%M:%S"), 
            total_records, 
            estimated_cost_usd, 
            avg_latency, 
            total_failures, 
            int(health_score), 
            classification, 
            root_causes[0],
            json.dumps(recommendations)
        ))
        
        conn.commit()
        conn.close()
        print("💾 [DATABASE] Prescriptive diagnostic manifest permanently archived to ledger.")
        
    except Exception as e:
        print(f"❌ Structural intelligence parsing exception: {e}")
else:
    print("⏳ Missing hardware payload. Execute native compilation pipelines first.")
