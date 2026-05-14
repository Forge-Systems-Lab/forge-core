#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>
#include <immintrin.h>
#include <string.h>
#include "forge.h"

static inline int64_t fast_parse_int(const char *start, size_t len) {
    int64_t val = 0;
    for (size_t i = 0; i < len; ++i) {
        if (start[i] >= '0' && start[i] <= '9')
            val = val * 10 + (start[i] - '0');
    }
    return val;
}

void *forge_worker(void *arg) {
    ForgeThreadTask *task = (ForgeThreadTask *)arg;

    while (1) {
        ForgeTask t;
        int has_work = 0;
        pthread_mutex_lock(&task->queue->lock);
        if (task->queue->head < task->queue->tail) {
            t = task->queue->tasks[task->queue->head++];
            has_work = 1;
        }
        pthread_unlock:
        pthread_mutex_unlock(&task->queue->lock);
        if (!has_work) break;

        int fd = open(t.filepath, O_RDONLY);
        if (fd < 0) continue;
        char *data = mmap(NULL, t.file_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data == MAP_FAILED) { close(fd); continue; }

        size_t i = 0;
        while (i < t.file_size) {
            int cur_col = 0;
            size_t col_start = i;
            while (i < t.file_size && data[i] != '\n') {
                if (data[i] == task->schema->delimiter) {
                    if ((task->schema->validation_mask >> cur_col) & 1) {
                        int64_t val = fast_parse_int(&data[col_start], i - col_start);
                        ColumnSignal *sig = &task->thread_signals.signals[cur_col];
                        sig->sum += (double)val;
                        sig->sum_sq += (double)val * val; 
                        sig->count++;
                        if (val < sig->min) sig->min = val;
                        if (val > sig->max) sig->max = val;
                    }
                    cur_col++;
                    col_start = i + 1;
                }
                i++;
            }
            // Last column check
            if ((task->schema->validation_mask >> cur_col) & 1) {
                int64_t val = fast_parse_int(&data[col_start], i - col_start);
                ColumnSignal *sig = &task->thread_signals.signals[cur_col];
                sig->sum += (double)val;
                sig->sum_sq += (double)val * val;
                sig->count++;
                if (val < sig->min) sig->min = val;
                if (val > sig->max) sig->max = val;
            }
            task->valid_rows += (cur_col + 1 == task->schema->expected_cols);
            i++; 
        }
        munmap(data, t.file_size);
        close(fd);
    }
    return NULL;
}
