#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "engine.h"

#define MAX_FILES 128
#define THREAD_COUNT 12

// Expanded path headroom to 512 bytes to satisfy GCC format-truncation safety
char file_queue[MAX_FILES][512];
int file_count = 0;
int next_file_idx = 0;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t metrics_mutex = PTHREAD_MUTEX_INITIALIZER;

FleetMetrics global_metrics = {0};

extern void parse_csv_line(const char *line, AgentTrace *trace);
extern void accumulate_fleet_telemetry(FleetMetrics *metrics, const AgentTrace *trace);

void *worker_thread_handler(void *arg) {
    (void)arg; // Explicitly cast to silence the unused-parameter warning
    while (1) {
        char target_file[512] = {0};
        
        pthread_mutex_lock(&queue_mutex);
        if (next_file_idx >= file_count) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }
        strncpy(target_file, file_queue[next_file_idx++], sizeof(target_file) - 1);
        pthread_mutex_unlock(&queue_mutex);

        int fd = open(target_file, O_RDONLY);
        if (fd < 0) continue;

        struct stat sb;
        if (fstat(fd, &sb) < 0 || sb.st_size == 0) {
            close(fd);
            continue;
        }

        char *file_mem = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);
        if (file_mem == MAP_FAILED) continue;

        char *line_start = file_mem;
        char *file_end = file_mem + sb.st_size;
        
        while (line_start < file_end && *line_start != '\n') line_start++;
        if (line_start < file_end) line_start++;

        char line_buffer[512];
        while (line_start < file_end) {
            char *line_end_ptr = line_start;
            while (line_end_ptr < file_end && *line_end_ptr != '\n') line_end_ptr++;
            
            size_t line_len = line_end_ptr - line_start;
            if (line_len > 0 && line_len < sizeof(line_buffer)) {
                memcpy(line_buffer, line_start, line_len);
                line_buffer[line_len] = '\0';

                AgentTrace local_trace = {0};
                parse_csv_line(line_buffer, &local_trace);

                if (local_trace.timestamp > 0) {
                    pthread_mutex_lock(&metrics_mutex);
                    accumulate_fleet_telemetry(&global_metrics, &local_trace);
                    pthread_mutex_unlock(&metrics_mutex);
                }
            }
            line_start = line_end_ptr + 1;
        }
        munmap(file_mem, sb.st_size);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    char *target_dir = (argc > 1) ? argv[1] : "agent_telemetry";
    DIR *dir = opendir(target_dir);
    if (!dir) {
        fprintf(stderr, "❌ Error: Could not open telemetry directory: %s\n", target_dir);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        if (strstr(entry->d_name, ".csv")) {
            snprintf(file_queue[file_count++], sizeof(file_queue[0]), "%s/%s", target_dir, entry->d_name);
        }
    }
    closedir(dir);

    printf("⚙️  Spawning %d POSIX Threads over %d telemetry batches...\n", THREAD_COUNT, file_count);
    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, worker_thread_handler, NULL);
    }
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    double avg_latency = global_metrics.total_records > 0 ? 
        global_metrics.cumulative_execution_ms / global_metrics.total_records : 0.0;

    FILE *json_file = fopen("intelligence.json", "w");
    if (json_file) {
        fprintf(json_file, "{\n");
        fprintf(json_file, "  \"total_records\": %lu,\n", global_metrics.total_records);
        fprintf(json_file, "  \"total_input_tokens\": %lu,\n", global_metrics.total_input_tokens);
        fprintf(json_file, "  \"total_output_tokens\": %lu,\n", global_metrics.total_output_tokens);
        fprintf(json_file, "  \"total_failures\": %lu,\n", global_metrics.total_failures);
        fprintf(json_file, "  \"avg_execution_time_ms\": %.2f\n", avg_latency);
        fprintf(json_file, "}\n");
        fclose(json_file);
        printf("📋 Data contract compiled successfully: intelligence.json written.\n");
    }

    return 0;
}
