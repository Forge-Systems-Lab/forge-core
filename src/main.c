#include "forge.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    ForgeOps ops;
    parse_arguments(argc, argv, &ops);

    struct timespec s_t, e_t;
    clock_gettime(CLOCK_MONOTONIC, &s_t);

    // Default Schema: Col0=INT(Req), Col1=STR, Col2=STR
    ForgeConfig config = {',', 3, {FORGE_INT, FORGE_STRING, FORGE_STRING}, {1, 0, 0}, ops.benchmark_only};
    
    int fd = open(ops.input_path, O_RDONLY);
    if (fd < 0) { perror("❌ FATAL"); return 1; }
    struct stat sb; fstat(fd, &sb);
    char *map = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    pthread_t *threads = malloc(sizeof(pthread_t) * ops.threads);
    ForgeThreadTask *tasks = malloc(sizeof(ForgeThreadTask) * ops.threads);
    size_t chunk_size = (size_t)sb.st_size / ops.threads;

    for (int i = 0; i < ops.threads; i++) {
        tasks[i].thread_id = i;
        tasks[i].map_base = map;
        tasks[i].config = &config;
        tasks[i].output_buffer = malloc(SCRIBE_BUFFER_SIZE);
        tasks[i].valid_rows = 0;
        tasks[i].error_count = 0;
        tasks[i].start_offset = (i == 0) ? 0 : tasks[i-1].end_offset;
        size_t tent_end = (i == ops.threads - 1) ? (size_t)sb.st_size : (size_t)(i + 1) * chunk_size;
        while (tent_end < (size_t)sb.st_size && map[tent_end] != '\n') tent_end++;
        tasks[i].end_offset = (tent_end < (size_t)sb.st_size) ? tent_end + 1 : (size_t)sb.st_size;
        pthread_create(&threads[i], NULL, forge_worker, &tasks[i]);
    }

    for (int i = 0; i < ops.threads; i++) pthread_join(threads[i], NULL);

    size_t total_v = 0, total_e = 0;
    if (!ops.benchmark_only) {
        FILE *f_out = fopen(ops.output_path, "w");
        fprintf(f_out, "[\n");
        for (int i = 0; i < ops.threads; i++) {
            fwrite(tasks[i].output_buffer, 1, tasks[i].output_size, f_out);
            if (i < ops.threads - 1 && tasks[i].output_size > 0) fprintf(f_out, ",\n");
            total_v += tasks[i].valid_rows; total_e += tasks[i].error_count;
        }
        fprintf(f_out, "\n]\n");
        fclose(f_out);
    } else {
        for (int i = 0; i < ops.threads; i++) { 
            total_v += tasks[i].valid_rows; total_e += tasks[i].error_count; 
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &e_t);
    double total_ms = (e_t.tv_sec - s_t.tv_sec) * 1000.0 + (e_t.tv_nsec - s_t.tv_nsec) / 1000000.0;
    
    printf("\n📊 [FORGE-CORE v1.9.6 OPERATIONAL REPORT]\n");
    printf("------------------------------------------\n");
    printf("Threads:         %d\n", ops.threads);
    printf("Valid Rows:      %zu\n", total_v);
    printf("Errors Caught:   %zu\n", total_e);
    printf("Data Integrity:  %.2f%%\n", ((double)total_v / (total_v + total_e)) * 100);
    printf("Total Time:      %.2f ms\n", total_ms);
    printf("Throughput:      %.2f M Rows/Sec\n", ((total_v + total_e) / 1000000.0) / (total_ms / 1000.0));
    printf("------------------------------------------\n\n");

    for(int i = 0; i < ops.threads; i++) free(tasks[i].output_buffer);
    free(threads); free(tasks); munmap(map, sb.st_size); close(fd);
    return 0;
}
