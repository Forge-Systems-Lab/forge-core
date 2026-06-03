# Forge Killer Objections & Falsification Matrix

### Objection #1: "We already use recursion_limit to cap our agent execution graphs."
* **The Reality:** `recursion_limit` only counts framework supersteps (scheduler ticks), not model requests, tool calls, or token expenditures. Up to 77 distinct messages can regularly execute beneath a step limit of 50.
* **The Product Wedge:** When the limit is reached, it raises a hard, unhandled language exception that tears down the user session and drops the transient graph state context, rather than providing a soft, graceful safe-mode fallback.

### Objection #2: "We can just write custom step counters into our local Graph State definition."
* **The Reality:** Forcing developers to manually inject state tracking variables, increment logic, and router conditionals into every individual node schema adds significant code maintenance overhead and fails to inspect cross-agent loop patterns.
