# Level 1 Evidence: Verifiable Infrastructure Failures

### Case #1: LangGraph Routing Deadlock (Issue #3570)
* **Source:** https://github.com/langchain-ai/langgraph/issues/3570
* **Failure Mode:** State machine fails to escape child graph routing logic.
* **Verified Metric:** 1,777+ continuous execution steps recorded out-of-band.
* **Financial Impact:** Unknown.

### Case #2: Destructive Threshold Crash (Issue #6731)
* **Source:** https://github.com/langchain-ai/langgraph/issues/6731
* **Failure Mode:** Agent ignores explicit prompt stop conditions, looping until it hits the framework ceiling.
* **Verified Metric:** Hard application termination via `GraphRecursionError`.
* **Financial Impact:** Unknown.

### Case #3: Tool Self-Correction Loop (Issue #7138)
* **Source:** https://github.com/langchain-ai/langgraph/issues/7138
* **Failure Mode:** Tool node error responses cause the agent to continuously re-fire identical failing arguments.
* **Verified Metric:** Multi-turn duplicate tool invocation chains.
* **Financial Impact:** Unknown.
