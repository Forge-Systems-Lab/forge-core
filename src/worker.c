#include "forge.h"
#include <stdlib.h>
#include <string.h>

void *forge_worker(void *arg) {
    ForgeThreadTask *task = (ForgeThreadTask *)arg;
    ForgeArena arena;
    arena.tokens = malloc(sizeof(ForgeToken) * TOKEN_CAPACITY);
    
    char *buf = task->output_buffer;
    size_t ptr = 0;
    int in_quotes = 0;
    const char *field_start = task->map_base + task->start_offset;
    size_t current_row_tokens = 0;

    for (size_t i = task->start_offset; i < task->end_offset; i++) {
        char c = task->map_base[i];
        if (c == '"') {
            if (in_quotes && (i + 1 < task->end_offset) && task->map_base[i+1] == '"') { i++; continue; }
            in_quotes = !in_quotes;
        } else if ((c == task->config->delimiter || c == '\n') && !in_quotes) {
            arena.tokens[current_row_tokens].data = (char*)field_start;
            arena.tokens[current_row_tokens].length = (task->map_base + i) - field_start;
            current_row_tokens++;
            field_start = task->map_base + i + 1;

            if (current_row_tokens == (size_t)task->config->expected_cols) {
                // --- SEMANTIC VALIDATION GATE ---
                int row_valid = 1;
                for (int c_idx = 0; c_idx < task->config->expected_cols; c_idx++) {
                    ForgeToken *t = &arena.tokens[c_idx];
                    
                    // 1. Required Constraint
                    if (task->config->required[c_idx] && t->length == 0) {
                        row_valid = 0; break;
                    }
                    // 2. Type Constraint: Fast-Digit Check (Benchmark Friendly)
                    if (task->config->types[c_idx] == FORGE_INT && t->length > 0) {
                        for (size_t k = 0; k < t->length; k++) {
                            if (t->data[k] < '0' || t->data[k] > '9') { row_valid = 0; break; }
                        }
                    }
                }

                if (row_valid) {
                    if (!task->config->benchmark_only) {
                        size_t absolute_id = (task->thread_id * 1000000) + task->valid_rows;
                        serialize_row(buf, &ptr, absolute_id, &arena, task->config);
                        if (i < task->end_offset - 1) { memcpy(buf + ptr, ",\n", 2); ptr += 2; }
                    }
                    task->valid_rows++;
                } else {
                    task->error_count++;
                }
                current_row_tokens = 0;
            }
        }
    }
    task->output_size = ptr;
    free(arena.tokens);
    return NULL;
}
