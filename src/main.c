#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include "forge.h"

void scan_directory(const char *dir_path, ForgeTaskQueue *queue) {
    DIR *dir = opendir(dir_path);
    if (!dir) { perror("opendir"); exit(1); }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // Skip . and ..
        
        if (strstr(entry->d_name, ".csv")) {
            pthread_mutex_lock(&queue->lock);
            if (queue->tail < MAX_QUEUE_SIZE) {
                snprintf(queue->tasks[queue->tail].filepath, MAX_PATH_LEN, "%s/%s", dir_path, entry->d_name);
                struct stat st;
                stat(queue->tasks[queue->tail].filepath, &st);
                queue->tasks[queue->tail].file_size = st.st_size;
                queue->tail++;
                queue->total_tasks++;
            }
            pthread_mutex_unlock(&queue->lock);
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <directory_path>\n", argv[0]);
        return 1;
    }

    ForgeTaskQueue queue = { .head = 0, .tail = 0, .total_tasks = 0 };
    pthread_mutex_init(&queue.lock, NULL);

    ForgeSchema schema = { .delimiter = ',', .expected_cols = 3, .validation_mask = 0x5 };

    printf("\n🚀 [FORGE-CORE v4.0 | DISTRIBUTED ORCHESTRATOR]\n");
    scan_directory(argv[1], &queue);
    printf("Found %d CSV files in registry.\n", queue.total_tasks);

int num_threads = 12; 
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    ForgeThreadTask *tasks = malloc(sizeof(ForgeThreadTask) * num_threads);

    // --- START TIMER ---
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {
        tasks[i].thread_id = i;
        tasks[i].queue = &queue;
        tasks[i].schema = &schema;
        tasks[i].valid_rows = 0;
        memset(tasks[i].column_errors, 0, sizeof(size_t) * 32);
        pthread_create(&threads[i], NULL, forge_worker, &tasks[i]);
    }

    // --- WAIT FOR ALL WORKERS ---
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // --- AGGREGATE GLOBAL TELEMETRY ---
    size_t global_valid_rows = 0;
    size_t global_col_errors[32] = {0};

    for (int i = 0; i < num_threads; i++) {
        global_valid_rows += tasks[i].valid_rows;
        for (int c = 0; c < 32; c++) {
            global_col_errors[c] += tasks[i].column_errors[c];
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // --- FINAL INFRASTRUCTURE REPORT ---
    printf("\n--- GLOBAL INGESTION AUDIT COMPLETE ---\n");
    printf("📁 Files Processed:  %d\n", queue.total_tasks);
    printf("✅ Total Valid Rows: %zu\n", global_valid_rows);
    printf("📊 Global Throughput: %.2f M rows/sec\n", (global_valid_rows / elapsed) / 1e6);
    printf("⏱️  Execution Time:   %.4fs\n", elapsed);

    printf("\n--- GLOBAL COLUMN INTEGRITY ---\n");
    for (int c = 0; c < schema.expected_cols; c++) {
        const char *type = (schema.validation_mask >> c) & 1 ? "INT" : "STR";
        printf("Col %-2d [%-3s]: %-12zu total errors %s\n", 
               c, type, global_col_errors[c], 
               global_col_errors[c] > 0 ? "⚠️" : "✅");
    }
    printf("--------------------------------------\n\n");

    pthread_mutex_destroy(&queue.lock);
    free(threads);
    free(tasks);
    return 0;
}
