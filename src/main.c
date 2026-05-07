#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
    // 1. THE SENTINEL GATE
    if (argc != 4) {
        fprintf(stderr, "🛡️ [SENTINEL] Usage: %s <dataset_path> <delimiter> <expected_cols>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filepath = argv[1];
    char delimiter = argv[2][0];
    int expected_cols = atoi(argv[3]);

    // START GLOBAL CHRONOS
    struct timespec total_start, total_end;
    clock_gettime(CLOCK_MONOTONIC, &total_start);

    // 2. THE METAL LAYER
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) { perror("⛓️ [METAL] I/O Error"); exit(EXIT_FAILURE); }

    struct stat sb;
    fstat(fd, &sb);
    size_t file_size = sb.st_size;

    char *map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) { perror("⛓️ [METAL] mmap Error"); close(fd); exit(EXIT_FAILURE); }

    // 3. THE SCRIBE INITIALIZATION
    FILE *audit_log = fopen("audit_report.txt", "w");
    if (!audit_log) { perror("✍️ [SCRIBE] Error"); munmap(map, file_size); close(fd); exit(EXIT_FAILURE); }

    // 4. THE SHIELD LAYER: Forensic Scan with Timing
    printf("📡 [SYSTEM] Target: %s (%zu bytes)\n", filepath, file_size);
    
    struct timespec scan_start, scan_end;
    clock_gettime(CLOCK_MONOTONIC, &scan_start);

    size_t row_count = 0;
    int current_row_cols = 1;
    size_t malformed_rows = 0;
    size_t row_start = 0;

    for (size_t i = 0; i < file_size; i++) {
        if (map[i] == delimiter) {
            current_row_cols++;
        } else if (map[i] == '\n') {
            row_count++;
            if (current_row_cols != expected_cols) {
                malformed_rows++;
                fprintf(audit_log, "[BREACH] Row %zu: Expected %d, Found %d\n", row_count, expected_cols, current_row_cols);
            }
            current_row_cols = 1; 
            row_start = i + 1;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &scan_end);

    // 5. CALCULATE LATENCY (The Chronos Output)
    double scan_time = (scan_end.tv_sec - scan_start.tv_sec) + 
                       (scan_end.tv_nsec - scan_start.tv_nsec) / 1e9;
    
    printf("📊 [SHIELD] Audit Complete.\n");
    printf("   >> Total Rows: %zu\n", row_count);
    printf("   >> Integrity Breaches: %zu\n", malformed_rows);
    printf("⏱️  [CHRONOS] Core Scan Latency: %.6f seconds\n", scan_time);

    // CLEANUP
    fclose(audit_log);
    munmap(map, file_size);
    close(fd);
    
    clock_gettime(CLOCK_MONOTONIC, &total_end);
    double total_time = (total_end.tv_sec - total_start.tv_sec) + 
                        (total_end.tv_nsec - total_start.tv_nsec) / 1e9;
    printf("🛡️ [SYSTEM] Total Lifecycle: %.6f seconds. Terminated.\n", total_time);

    return 0;
}
