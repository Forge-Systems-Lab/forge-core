#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <immintrin.h>
#include "forge.h"

// --- THE TITAN+ SENTINEL KERNEL ---
// Detects non-digits (0-9) across 32 bytes in 1 cycle using unsigned saturation.
static inline uint32_t validate_chunk(__m256i chunk, __m256i v_zero, __m256i v_nine, uint32_t d_mask, uint32_t n_mask) {
    __m256i v_bias = _mm256_sub_epi8(chunk, v_zero);
    __m256i v_err_bytes = _mm256_subs_epu8(v_bias, v_nine);
    
    // Compare against zero: 0xFF for digits (0x00 in v_err_bytes), 0x00 for non-digits
    __m256i v_is_digit = _mm256_cmpeq_epi8(v_err_bytes, _mm256_setzero_si256());
    
    // Movemask gives 1s for digits. We flip it (~) to get 1s for errors.
    uint32_t digit_mask = _mm256_movemask_epi8(v_is_digit);
    uint32_t error_mask = ~digit_mask;

    // Mask out valid delimiters and newlines so they don't count as "corruption"
    return error_mask & ~d_mask & ~n_mask;
}

void *forge_worker(void *arg) {
    ForgeThreadTask *task = (ForgeThreadTask *)arg;

    // CPU Affinity for zero-latency execution
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(task->thread_id % sysconf(_SC_NPROCESSORS_ONLN), &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    const char *data = task->map_base;
    size_t i = task->start_offset;
    
    const __m256i v_delim   = _mm256_set1_epi8(task->config->delimiter);
    const __m256i v_newline = _mm256_set1_epi8('\n');
    const __m256i v_zero    = _mm256_set1_epi8(0x30);
    const __m256i v_nine    = _mm256_set1_epi8(9);
    
    const int target_cols = task->config->expected_cols;
    size_t local_valid = 0;
    size_t local_errors = 0;

    while (i < task->end_offset) {
        size_t line_len = 0;
        int col_count = 1;

        while (i + line_len < task->end_offset) {
            if (i + line_len + 32 <= task->end_offset) {
                __m256i chunk = _mm256_loadu_si256((const __m256i *)&data[i + line_len]);
                uint32_t n_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_newline));
                uint32_t d_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_delim));

                // 🛡️ SENTINEL AUDIT (Always Active)
                // We check the full chunk before splitting into Fast/Slow paths
                uint32_t violations = validate_chunk(chunk, v_zero, v_nine, d_mask, n_mask);
                
                if (__builtin_expect(n_mask == 0, 1)) {
                    // Fast Path
                    if (__builtin_expect(violations != 0, 0)) local_errors += __builtin_popcount(violations);
                    col_count += __builtin_popcount(d_mask);
                    line_len += 32;
                    continue;
                } else {
                    // Slow Path (Record boundary found)
                    int first_n = __builtin_ctz(n_mask);
                    // Only count violations before the newline
                    uint32_t relevant_violations = violations & ((1U << first_n) - 1);
                    local_errors += __builtin_popcount(relevant_violations);
                    
                    col_count += __builtin_popcount(d_mask & ((1U << first_n) - 1));
                    line_len += first_n;
                    break;
                }
            }
            
            // Scalar Fallback with basic validation
            if (data[i + line_len] == '\n') break;
            char c = data[i + line_len];
            if (c == task->config->delimiter) {
                col_count++;
            } else if (c < '0' || c > '9') {
                local_errors++;
            }
            line_len++;
        }

        local_valid += (col_count == target_cols);
        i += line_len + 1;
    }

    task->valid_rows = local_valid;
    task->corrupt_bytes = local_errors; 
    return NULL;
}
