#ifndef FORGE_H
#define FORGE_H

#include <stdint.h>
#include <pthread.h>
#include <immintrin.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_QUEUE_SIZE 10000 
#define MAX_PATH_LEN 512

// --- INTELLIGENCE & STATISTICAL PRIMITIVES ---
typedef struct {
    double sum;
    double sum_sq;    // Sum of Squares for Variance/Volatility
    uint64_t count;
    int64_t min;
    int64_t max;
} ColumnSignal;

typedef struct {
    ColumnSignal signals[32]; 
} ForgeSignals;

typedef struct {
    char delimiter;
    int expected_cols;
    uint32_t validation_mask; 
} ForgeSchema;

typedef struct {
    char filepath[MAX_PATH_LEN];
    size_t file_size;
} ForgeTask;

typedef struct {
    ForgeTask tasks[MAX_QUEUE_SIZE];
    int head;
    int tail;
    int total_tasks;
    pthread_mutex_t lock;
} ForgeTaskQueue;

typedef struct {
    int thread_id;
    ForgeTaskQueue *queue; 
    ForgeSchema *schema;
    size_t valid_rows;
    size_t column_errors[32]; 
    ForgeSignals thread_signals; 
} ForgeThreadTask;

void *forge_worker(void *arg);

#endif
