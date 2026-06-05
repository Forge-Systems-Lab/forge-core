#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine.h"

/* * High-performance string tokenization.
 * Directly extracts text slices between commas without copying unnecessary buffers.
 */
void parse_csv_line(const char *line, AgentTrace *trace) {
    char *ptr = (char *)line;
    char *next;

    // 1. Parse Timestamp (uint64_t)
    trace->timestamp = (uint64_t)strtoull(ptr, &next, 10);
    if (ptr == next || *next != ',') return;
    ptr = next + 1;

    // 2. Extract Agent ID (Up to 23 characters)
    char *id_start = ptr;
    while (*ptr && *ptr != ',') ptr++;
    size_t id_len = ptr - id_start;
    if (id_len > 23) id_len = 23; 
    memcpy(trace->agent_id, id_start, id_len);
    trace->agent_id[id_len] = '\0';
    if (*ptr == ',') ptr++;

    // 3. Parse Execution Time (uint32_t)
    trace->execution_time_ms = (uint32_t)strtoul(ptr, &next, 10);
    if (ptr == next || *next != ',') return;
    ptr = next + 1;

    // 4. Parse Input Tokens (uint32_t)
    trace->input_tokens = (uint32_t)strtoul(ptr, &next, 10);
    if (ptr == next || *next != ',') return;
    ptr = next + 1;

    // 5. Parse Output Tokens (uint32_t)
    trace->output_tokens = (uint32_t)strtoul(ptr, &next, 10);
    if (ptr == next || *next != ',') return;
    ptr = next + 1;

    // 6. Parse Retry Count (uint32_t)
    trace->retry_count = (uint32_t)strtoul(ptr, &next, 10);
    if (ptr == next || *next != ',') return;
    ptr = next + 1;

    // 7. Extract Status String (Up to 15 characters)
    char *status_start = ptr;
    while (*ptr && *ptr != '\n' && *ptr != '\r' && *ptr != ',') ptr++;
    size_t status_len = ptr - status_start;
    if (status_len > 15) status_len = 15;
    memcpy(trace->status, status_start, status_len);
    trace->status[status_len] = '\0';
}

/*
 * Thread-safe metric accumulation block.
 * Computes moving averages and variations across all running metrics.
 */
void accumulate_fleet_telemetry(FleetMetrics *metrics, const AgentTrace *trace) {
    metrics->total_records++;
    metrics->total_input_tokens += trace->input_tokens;
    metrics->total_output_tokens += trace->output_tokens;

    if (strcmp(trace->status, "SUCCESS") != 0) {
        metrics->total_failures++;
    }

    // Welford's Algorithm primitive path for variance tracking
    double delta = trace->execution_time_ms - (metrics->cumulative_execution_ms / metrics->total_records);
    metrics->cumulative_execution_ms += trace->execution_time_ms;
    double delta2 = trace->execution_time_ms - (metrics->cumulative_execution_ms / metrics->total_records);
    metrics->latency_variance_accumulator += delta * delta2;
}
