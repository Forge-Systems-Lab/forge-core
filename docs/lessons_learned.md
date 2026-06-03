# Forge Pipeline: Real Lessons & Workspace Realities

## 1. Multi-Language Synchronization & Boundary Issues
- **The Problem**: Encountered data corruption and structural line drops when testing the ingestion of `chaos_data.csv` (277MB) across the pipeline boundary.
- **The Diagnosis**: The python baseline systems (`python_baseline.py` / `consumer.py`) and the native C compiled engine (`forge-core`) were referencing inconsistent data schemas. Tracking down tracking tokens revealed file-handling mismatches when dealing with trailing boundaries on massive, raw text streams.
- **The Resolution**: Standardized the system layout by implementing an explicit validation contract (`schema.json`). Cleaned up duplicate binary execution states between the compiled `forge-core` and `forge_core` targets to establish a single source of true system output.

## 2. Ingestion Overhead: C vs. Python Baselines
- **The Problem**: Initial data transfers using standard interpreted loops in `slow_parser.py` choked under high-volume log inputs.
- **The Diagnosis**: Interpreted runtime processing introduced massive execution bottlenecks when managing multi-megabyte disk inputs.
- **The Resolution**: Moved the entire processing loop to zero-copy memory mapping (`mmap`) inside the C layer, using Python strictly as a high-level coordination and configuration orchestration framework.
