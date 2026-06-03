# Forge-Core: High-Performance AI Agent Telemetry Ingestion Engine

Forge-Core is an experimental high-performance telemetry ingestion and analysis engine built with native C11 and Python. The project explores how large-scale, multi-turn AI-agent execution traces can be processed efficiently at scale while systematically investigating operational failure patterns such as recursive routing loops, retry storms, context growth, and execution anomalies.

---

## 🎯 Why This Project Exists

While exploring modern multi-agent topologies and stateful execution frameworks, recurring, non-deterministic system failure patterns frequently appear in production environments:

* **Recursive Routing Loops:** States getting locked bouncing between graph nodes indefinitely.
* **Retry Storms:** Automated exceptions triggering high-frequency, cascading model re-calls.
* **Context Stuffing / Bloat:** The $O(N^2)$ quadratic accumulation of historical text passing through active windows.
* **Destructive Framework Crashes:** Default safeguards acting as reactive termination boundaries that destructively crash user sessions when an iteration ceiling is breached.

Forge-Core began as an isolated experiment to understand whether these operational failure patterns could be actively detected and evaluated early using low-overhead, user-space telemetry analysis rather than relying exclusively on local, framework-level constraints.

---

## 🔬 Project Status & Strategic Scope

Forge-Core is currently an active **Research & Development project**. It serves as a testing ground to evaluate out-of-band tracing concepts before any high-level operational infrastructure is committed to code.

### 🟢 What Has Been Technically Established (Proven)
* **High-Velocity Ingestion:** Direct memory-mapped structures can parse and process raw serialized JSON agent state footprints with minimal CPU execution overhead.
* **Safeguard Imperfection:** Built-in scheduler limits (like static graph step counters) purely track high-level ticks—allowing dense retry storms and token expansion to occur within a single execution step.
* **Visibility vs. Containment:** Standard observability suites function primarily as historical post-incident analysis platforms rather than real-time programmatic containment engines.

### 🔴 Critical Systemic Unknowns (Research Hypotheses)
* **Financial Blast Radius Distribution:** Validating the explicit frequency and true operational cost of runaway agent failures across scaling early-stage companies.
* **Telemetry Gap:** Investigating whether there is measurable architectural value in complementing existing industry observability tools with lower-overhead, out-of-band analysis focused purely on execution anomalies.
* **Safeguard Sufficiency:** Determining if simple, local programmatic boundaries (such as strict timeout intervals and baseline spending caps) are already "good enough" for the majority of production use cases.

---

## 🚀 Telemetry Ingestion Benchmarks

These internal metrics capture the performance scaling of raw trace log parsing across isolated development iterations:

| Build Phase | Ingestion Methodology | Workload / Dataset Vector | Throughput / Velocity | Latency Delta |
| :--- | :--- | :--- | :--- | :--- |
| **v1.0** | Parallel `mmap` Parsing | Single File, ~10M Rows | ~11.0M Rows / Sec | Baseline |
| **v2.0** | SIMD Vector Bursting | Single File, ~10M Rows | ~40.2M Rows / Sec | -91.3% |
| **v3.3** | Typed Sentinel Buffering | Multi-Threaded Queue | ~248.1M Rows / Sec | -98.9% |
| **v4.3** | Orchestrated Ring Buffer | Multi-File Concurrent Pipeline | ~274.5M Rows / Sec | System Ceiling |

### 📊 Benchmark Reproducibility

To ensure transparency and independent verification, the performance data above was logged under the following hardware and environment constraints:

* **Hardware Topology:** x86_64 Processor architecture | 16 GB RAM System Memory
* **Operating Environment:** Linux Kernel via virtualized environment (WSL2 Ubuntu Distribution)
* **Compilation Toolchain:** GCC Compiler | Optimization Flags: `-O3 -mavx2 -pthread`
* **Test Dataset Asset:** Serialized multi-turn JSON telemetry payloads | 10,000,000 baseline event entries

To execute local validation checks and run the internal performance profiles directly on your machine, review the automation guidelines inside `BENCHMARKS.md`.

---

## 🏗️ Architectural Core & Data Flow

The engine implements an asynchronous processing pipeline to guarantee zero latency interference with the parent agent application thread:

```text
[ Agent Event Traces ] ──> Concurrent telemetry segments committed cleanly to disk.
           │
           ▼ (Memory-Mapped Multi-Threaded Worker Pool)
 [ forge-core (Native C) ] ──> High-speed SIMD structure indexing and trace token schema parsing.
           │
           ▼ (Automated Execution Contract Artifact)
    intelligence.json    ──> Standardized trace metadata highlighting state traversal and cost arrays.
           │
           ▼ (Asynchronous File System Watcher Gate)
 [ consumer.py (Python) ] ──> Aggregates transaction metrics and cross-references active rule tables.
           │
           ▼
   pipeline_logs.db      ──> [ Persistent Relational Analytics Ledger ]

⚙️ Build and Compilation
Prerequisites
A compiler supporting the C11 standard (GCC or Clang).
Host environment featuring POSIX threads (pthread) and an AVX2-compliant CPU instruction set.

Python 3.10+ installation.

Execution
To clean the local build space and compile the native core parsing utility from source coordinates, execute the build sequence:

make clean
make forge-core
