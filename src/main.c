#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include "forge.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // 1. Open and Map File
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    struct stat sb;
    fstat(fd, &sb);
    char *map = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) { perror("mmap"); return 1; }

    // 2. Configure System (Titan+ Kernel Settings)
    ForgeConfig config = { .delimiter = ',', .expected_cols = 3 };
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    ForgeThreadTask *tasks = malloc(sizeof(ForgeThreadTask) * num_threads);
    size_t chunk_size = sb.st_size / num_threads;

    printf("\n🚀 [FORGE-CORE v3.1 TITAN+ KERNEL]\n");
    printf("Mapping: %zu bytes | Threads: %d\n", sb.st_size, num_threads);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // 3. Launch Sentinel Threads
    for (int i = 0; i < num_threads; i++) {
        tasks[i].thread_id = i;
        tasks[i].map_base = map;
        tasks[i].start_offset = i * chunk_size;
        tasks[i].end_offset = (i == num_threads - 1) ? sb.st_size : (i + 1) * chunk_size;
        tasks[i].config = &config;
        tasks[i].valid_rows = 0;
        tasks[i].corrupt_bytes = 0;

        pthread_create(&threads[i], NULL, forge_worker, &tasks[i]);
    }

    // 4. Collect Telemetry
    size_t total_v = 0;
    size_t total_c = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_v += tasks[i].valid_rows;
        total_c += tasks[i].corrupt_bytes;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // 5. Final Report
    printf("\n--- SENTINEL AUDIT COMPLETE ---\n");
    printf("✅ Valid Rows:    %zu\n", total_v);
    printf("⚠️  Corrupt Bytes: %zu\n", total_c);
    printf("📊 Throughput:    %.2f M rows/sec\n", (total_v / elapsed) / 1e6);
    printf("⏱️  Total Time:    %.4fs\n\n", elapsed);

    munmap(map, sb.st_size);
    close(fd);
    free(threads);
    free(tasks);

    return 0;
}
