# Current Marketplace Defenses & Weaknesses

### 1. Framework Step Counters (`recursion_limit` / `max_iterations`)
* **How it works:** Hard stops the execution thread after a fixed number of steps.
* **Weakness:** Acts as a reactive, destructive crash mechanism that kills valid, complex user sessions.

### 2. Provider Spending Alerts
* **How it works:** Sends emails or flags webhooks when budget thresholds cross a threshold.
* **Weakness:** Downstream data processing introduces a multi-hour logging latency window where thousands of tokens run unchecked.

### 3. Passive Telemetry Environments (LangSmith / Arize Phoenix)
* **How it works:** Generates rich visual trace graphs for developers to audit system history.
* **Weakness:** Designed for passive structural observation, not real-time programmatic containment.
