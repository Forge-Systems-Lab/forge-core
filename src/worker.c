#include "forge.h"

__attribute__((always_inline))
static inline void process_chunk(const __m256i chunk, const __m256i v_newline, 
                               const __m256i v_delim, int *col_count, size_t *line_len) {
    uint32_t n_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_newline));
    uint32_t d_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_delim));

    if (__builtin_expect(n_mask == 0, 1)) {
        *col_count += __builtin_popcount(d_mask);
        *line_len += 32;
    } else {
        int first_n = __builtin_ctz(n_mask);
        *col_count += __builtin_popcount(d_mask & ((1U << first_n) - 1));
        *line_len += first_n;
    }
}

void *forge_worker(void *arg) {
    ForgeThreadTask *task = (ForgeThreadTask *)arg;
    
    // Core Affinity: Lock thread to physical silicon
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(task->thread_id % sysconf(_SC_NPROCESSORS_ONLN), &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    const char *data = task->map_base;
    size_t i = task->start_offset;
    const __m256i v_delim = _mm256_set1_epi8(task->config->delimiter);
    const __m256i v_newline = _mm256_set1_epi8('\n');
    const int target_cols = task->config->expected_cols;
    size_t local_valid = 0;

    while (i < task->end_offset) {
        size_t line_len = 0;
        int col_count = 1;

        while (i + line_len < task->end_offset) {
            if (i + line_len + 32 <= task->end_offset) {
                __m256i chunk = _mm256_loadu_si256((const __m256i*)&data[i + line_len]);
                uint32_t n_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_newline));
                uint32_t d_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, v_delim));

                if (__builtin_expect(n_mask == 0, 1)) {
                    col_count += __builtin_popcount(d_mask);
                    line_len += 32;
                    continue;
                } else {
                    int first_n = __builtin_ctz(n_mask);
                    col_count += __builtin_popcount(d_mask & ((1U << first_n) - 1));
                    line_len += first_n;
                    break;
                }
            }
            if (data[i + line_len] == '\n') break;
            col_count += (data[i + line_len] == task->config->delimiter);
            line_len++;
        }
        local_valid += (col_count == target_cols);
        i += line_len + 1;
    }
    task->valid_rows = local_valid;
    return NULL;
}
