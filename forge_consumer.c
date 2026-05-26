#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdatomic.h>
#include <stdalign.h>
#include <time.h>
#include <sched.h> 

#define SHM_OBJ_NAME  "/forge_shm"
#define SHM_TOTAL_SIZE 622784
#define DATA_BUF_SIZE  622656
#define STATS_INTERVAL 5000000 // Compute stats every 5 Million rows for tracking scale

typedef struct {
    char payload[256];
    uint64_t sequence_id;
    uint64_t timestamp;
} __attribute__((packed)) log_record_t;

typedef struct {
    alignas(64) _Atomic uint64_t write_head;
    alignas(64) _Atomic uint64_t read_tail;
    uint8_t data_buffer[DATA_BUF_SIZE];
} forge_shm_layout_t;

int main(void) {
    printf("[CONSUMER] Pinning process execution to physical CPU Core 1...\n");
    
    // Explicitly lock this entire process thread to physical CPU Core 1
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) == -1) {
        perror("[WARNING] Could not enforce CPU affinity tracking layout");
    }

    int shm_fd = shm_open(SHM_OBJ_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("[FATAL] Link broken");
        exit(EXIT_FAILURE);
    }

    forge_shm_layout_t *shm_layout = mmap(NULL, SHM_TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_layout == MAP_FAILED) {
        perror("[FATAL] mmap failed");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    close(shm_fd);

    uint64_t buffer_capacity = DATA_BUF_SIZE / sizeof(log_record_t);
    printf("[CONSUMER] Benchmarking engine active on Core 1. Monitoring...\n");

    uint64_t records_processed = 0;
    uint64_t total_latency_ns = 0;
    
    struct timespec current_time;
    struct timespec start_interval_time;
    clock_gettime(CLOCK_MONOTONIC, &start_interval_time);

    while (true) {
        uint64_t current_head = atomic_load_explicit(&shm_layout->write_head, memory_order_relaxed);
        uint64_t current_tail = atomic_load_explicit(&shm_layout->read_tail, memory_order_relaxed);

        if (current_tail == current_head) {
            usleep(1);
            continue;
        }

        uint64_t tail_index = current_tail % buffer_capacity;
        log_record_t *src_ptr = (log_record_t *)(&shm_layout->data_buffer[tail_index * sizeof(log_record_t)]);

        clock_gettime(CLOCK_MONOTONIC, &current_time);
        uint64_t egress_ns = (uint64_t)current_time.tv_sec * 1000000000ULL + (uint64_t)current_time.tv_nsec;
        
        if (egress_ns >= src_ptr->timestamp) {
            total_latency_ns += (egress_ns - src_ptr->timestamp);
        }

        records_processed++;

        if (records_processed % STATS_INTERVAL == 0) {
            struct timespec end_interval_time;
            clock_gettime(CLOCK_MONOTONIC, &end_interval_time);
            
            double elapsed_sec = (end_interval_time.tv_sec - start_interval_time.tv_sec) + 
                                 (end_interval_time.tv_nsec - start_interval_time.tv_nsec) / 1000000000.0;
            
            double throughput_miops = (STATS_INTERVAL / elapsed_sec) / 1000000.0;
            double avg_latency_ns = (double)total_latency_ns / STATS_INTERVAL;

            printf("[HARDWARE PINNED REPORT] Processed: %luM rows | Throughput: %.2f MIOPS | Avg Latency: %.1f ns\n", 
                   records_processed / 1000000, throughput_miops, avg_latency_ns);

            total_latency_ns = 0;
            clock_gettime(CLOCK_MONOTONIC, &start_interval_time);
        }

        atomic_store_explicit(&shm_layout->read_tail, current_tail + 1, memory_order_release);
    }

    munmap(shm_layout, SHM_TOTAL_SIZE);
    return 0;
}
