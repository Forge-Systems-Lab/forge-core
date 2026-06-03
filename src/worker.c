#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine.h"

/* * High-performance string tokenization.
 * Directly extracts text slices between commas without copying unnecessary buffers.
 */
void parse_csv_line(const char *line, AgentTrace *trace) {
    char status_buf[16] = {0};
    char id_buf[24] = {0};
    
    // Scan raw line contents directly into target binary primitive targets
    int parsed = sscanf(line, "%lu,%23[^,],%u,%u,%u,%u,%15s",
                        &trace->timestamp,
                        id_buf,
                        &trace->execution_time_ms,
                        &trace->input_tokens,
                        &trace->output_tokens,
                        &trace->retry_count,
                        status_buf);

    if (parsed == 7) {
        // Enforce strict memory truncation bounds on incoming string arrays
        snprintf(trace->agent_id, sizeof(trace->agent_id), "%s", id_buf);
        snprintf(trace->status, sizeof(trace->status), "%s", status_buf);
    }
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
