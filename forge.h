#ifndef FORGE_H
#define FORGE_H

#include <stdint.h>
#include <pthread.h>
#include <immintrin.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_QUEUE_SIZE 10000 
#define MAX_PATH_LEN 512

typedef struct {
    char delimiter;
    int expected_cols;
    uint32_t validation_mask; 
} ForgeSchema;

// --- NEW: ORCHESTRATION LAYER ---
typedef struct {
    char filepath[MAX_PATH_LEN];
    size_t file_size;
} ForgeTask;

typedef struct {
    ForgeTask tasks[MAX_QUEUE_SIZE];
    int head;             // Next task to pull
    int tail;             // Last task added
    int total_tasks;
    pthread_mutex_t lock; // The synchronization gate
} ForgeTaskQueue;

typedef struct {
    int thread_id;
    ForgeTaskQueue *queue; 
    ForgeSchema *schema;
    size_t valid_rows;
    size_t column_errors[32]; 
} ForgeThreadTask;

void *forge_worker(void *arg);

#endif
