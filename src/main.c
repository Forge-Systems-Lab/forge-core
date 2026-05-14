#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <math.h> // Required for sqrt()
#include "forge.h"

void scan_directory(const char *dir_path, ForgeTaskQueue *queue) {
    DIR *dir = opendir(dir_path);
    if (!dir) { perror("opendir"); exit(1); }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
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

void export_intelligence_json(const char *filename, ForgeSignals *gs, ForgeSchema *schema, size_t valid_rows, double throughput) {
    FILE *f = fopen(filename, "w");
    if (!f) { perror("JSON Export Failed"); return; }

    fprintf(f, "{\n  \"engine\": \"Forge-Core v4.3-STABLE\",\n");
    fprintf(f, "  \"metadata\": {\n    \"valid_rows\": %zu,\n    \"throughput_m_rows_sec\": %.2f\n  },\n", valid_rows, throughput);
    fprintf(f, "  \"intelligence\": [\n");

    for (int c = 0; c < schema->expected_cols; c++) {
        fprintf(f, "    {\n      \"column\": %d,\n", c);
        if ((schema->validation_mask >> c) & 1) {
            double avg = gs->signals[c].count > 0 ? gs->signals[c].sum / gs->signals[c].count : 0;
            double variance = gs->signals[c].count > 0 ? (gs->signals[c].sum_sq / gs->signals[c].count) - (avg * avg) : 0;
            double std_dev = sqrt(variance);
            
            fprintf(f, "      \"type\": \"INT\",\n      \"metrics\": {\n");
            fprintf(f, "        \"sum\": %.0f,\n        \"avg\": %.2f,\n        \"std_dev\": %.2f,\n", gs->signals[c].sum, avg, std_dev);
            fprintf(f, "        \"volatility_index\": %.4f,\n", std_dev / (avg != 0 ? avg : 1));
            fprintf(f, "        \"min\": %ld,\n        \"max\": %ld\n      }\n", gs->signals[c].min, gs->signals[c].max);
        } else {
            fprintf(f, "      \"type\": \"STR\"\n");
        }
        fprintf(f, "    }%s\n", (c == schema->expected_cols - 1) ? "" : ",");
    }

    fprintf(f, "  ]\n}\n");
    fclose(f);
    printf("📄 [v4.3-STABLE] Intelligence Payload Exported: %s\n", filename);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <directory_path>\n", argv[0]);
        return 1;
    }

    ForgeTaskQueue queue = { .head = 0, .tail = 0, .total_tasks = 0 };
    pthread_mutex_init(&queue.lock, NULL);
    ForgeSchema schema = { .delimiter = ',', .expected_cols = 3, .validation_mask = 0x4 }; 

    printf("\n🚀 [FORGE-CORE v4.3 | QUANT-READY ORCHESTRATOR]\n");
    scan_directory(argv[1], &queue);

    int num_threads = 12; 
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    ForgeThreadTask *tasks = malloc(sizeof(ForgeThreadTask) * num_threads);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {
        tasks[i].thread_id = i;
        tasks[i].queue = &queue;
        tasks[i].schema = &schema;
        tasks[i].valid_rows = 0;
        memset(tasks[i].column_errors, 0, sizeof(size_t) * 32);

        for(int c=0; c<32; c++) {
            tasks[i].thread_signals.signals[c].sum = 0;
            tasks[i].thread_signals.signals[c].sum_sq = 0;
            tasks[i].thread_signals.signals[c].count = 0;
            tasks[i].thread_signals.signals[c].min = 9223372036854775807LL;
            tasks[i].thread_signals.signals[c].max = -9223372036854775807LL - 1LL;
        }
        pthread_create(&threads[i], NULL, forge_worker, &tasks[i]);
    }

    size_t global_valid_rows = 0;
    ForgeSignals global_signals;
    for(int c=0; c<32; c++) {
        global_signals.signals[c].sum = 0;
        global_signals.signals[c].sum_sq = 0;
        global_signals.signals[c].count = 0;
        global_signals.signals[c].min = 9223372036854775807LL;
        global_signals.signals[c].max = -9223372036854775807LL - 1LL;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        global_valid_rows += tasks[i].valid_rows;
        for (int c = 0; c < 32; c++) {
            global_signals.signals[c].sum += tasks[i].thread_signals.signals[c].sum;
            global_signals.signals[c].sum_sq += tasks[i].thread_signals.signals[c].sum_sq;
            global_signals.signals[c].count += tasks[i].thread_signals.signals[c].count;
            if (tasks[i].thread_signals.signals[c].min < global_signals.signals[c].min)
                global_signals.signals[c].min = tasks[i].thread_signals.signals[c].min;
            if (tasks[i].thread_signals.signals[c].max > global_signals.signals[c].max)
                global_signals.signals[c].max = tasks[i].thread_signals.signals[c].max;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double throughput = (global_valid_rows / elapsed) / 1e6;

    printf("\n--- GLOBAL INGESTION AUDIT COMPLETE ---\n");
    printf("✅ Total Valid Rows: %zu\n", global_valid_rows);
    printf("📊 Global Throughput: %.2f M rows/sec\n", throughput);

    export_intelligence_json("intelligence.json", &global_signals, &schema, global_valid_rows, throughput);

    pthread_mutex_destroy(&queue.lock);
    free(threads); free(tasks);
    return 0;
}
