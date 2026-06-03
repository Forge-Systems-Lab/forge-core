# Forge Killer Objections & Falsification Checklist

* **Objection:** "We already use recursion limits, so our application layer is naturally capped."
  * *Must Prove:* Legitimate enterprise workflows frequently exceed generic static caps, forcing builders to choose between application stability and cost risk.
* **Objection:** "We just use hard provider credit limits to freeze the API keys."
  * *Must Prove:* Freezing an entire organization's API key downstreams kills the user experience for all active, high-value paying customers to catch one broken background agent.
* **Objection:** "This execution loop failure mode isn't a high-frequency risk in production."
  * *Must Prove:* Complex multi-agent topologies and dynamic tool integrations create unpredictable state-space combinations that cannot be caught via standard local integration tests.
