#include "forge.h"
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    ForgeOps ops = {0};
    parse_arguments(argc, argv, &ops);

    struct timespec s_t, e_t;
    clock_gettime(CLOCK_MONOTONIC, &s_t);

    ForgeConfig config = {',', 3, ops.benchmark_only};
    int fd = open(ops.input_path, O_RDONLY);
    struct stat sb; fstat(fd, &sb);
    char *map = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    madvise(map, sb.st_size, MADV_SEQUENTIAL | MADV_WILLNEED);

    pthread_t *threads = malloc(sizeof(pthread_t) * ops.threads);
    ForgeThreadTask *tasks = malloc(sizeof(ForgeThreadTask) * ops.threads);
    size_t chunk_size = (size_t)sb.st_size / ops.threads;

    for (int i = 0; i < ops.threads; i++) {
        tasks[i].thread_id = i;
        tasks[i].map_base = map;
        tasks[i].config = &config;
        tasks[i].valid_rows = 0; 
        tasks[i].start_offset = (i == 0) ? 0 : tasks[i-1].end_offset;
        size_t tent_end = (i == ops.threads - 1) ? (size_t)sb.st_size : (size_t)(i + 1) * chunk_size;
        while (tent_end < (size_t)sb.st_size && map[tent_end] != '\n') tent_end++;
        tasks[i].end_offset = (tent_end < (size_t)sb.st_size) ? tent_end + 1 : (size_t)sb.st_size;
        pthread_create(&threads[i], NULL, forge_worker, &tasks[i]);
    }

    for (int i = 0; i < ops.threads; i++) pthread_join(threads[i], NULL);

    size_t total_v = 0;
    for (int i = 0; i < ops.threads; i++) total_v += tasks[i].valid_rows;

    clock_gettime(CLOCK_MONOTONIC, &e_t);
    double total_ms = (e_t.tv_sec - s_t.tv_sec) * 1000.0 + (e_t.tv_nsec - s_t.tv_nsec) / 1000000.0;
    
    printf("\n🚀 [FORGE-CORE v3.1 TITAN+ KERNEL]\n");
    printf("Throughput:   %.2f M Rows/Sec\n", (total_v / 1000000.0) / (total_ms / 1000.0));
    printf("------------------------------------\n\n");

    munmap(map, sb.st_size); close(fd);
    return 0;
}
