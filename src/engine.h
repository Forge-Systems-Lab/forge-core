#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>

/* * Fixed-width structural footprint for ultra-fast cache line alignment.
 * Packing string identifiers into fixed-length arrays guarantees predictable 
 * byte offsets across memory-mapped SIMD ingestion blocks.
 */
typedef struct {
    uint64_t timestamp;
    char agent_id[16];
    uint32_t execution_time_ms;
    uint32_t input_tokens;
    uint32_t output_tokens;
    uint32_t retry_count;
    char status[12];
} AgentTrace;

/*
 * Fleet Analytics Metrics Aggregator
 * Tracks accumulated pipeline metadata to compute real-time averages,
 * standard deviations, and systemic threat indices across concurrent workers.
 */
typedef struct {
    uint64_t total_records;
    uint64_t total_input_tokens;
    uint64_t total_output_tokens;
    uint64_t total_failures;
    double cumulative_execution_ms;
    double latency_variance_accumulator;
} FleetMetrics;

#endif /* ENGINE_H */
