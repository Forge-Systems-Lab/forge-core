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
    printf("[PRODUCER] Pinning process execution to physical CPU Core 0...\n");
    
    // Explicitly lock this entire process thread to physical CPU Core 0
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) == -1) {
        perror("[WARNING] Could not enforce CPU affinity tracking layout");
    }

    int shm_fd = shm_open(SHM_OBJ_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("[FATAL] shm_open failed");
        exit(EXIT_FAILURE);
    }

    forge_shm_layout_t *shm_layout = mmap(NULL, SHM_TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_layout == MAP_FAILED) {
        perror("[FATAL] mmap failed");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    close(shm_fd);

    uint64_t local_sequence = 0;
    uint64_t buffer_capacity = DATA_BUF_SIZE / sizeof(log_record_t);
    struct timespec frame_time;

    printf("[PRODUCER] Hot-path streaming active on Core 0. Emitting...\n");

    while (true) {
        log_record_t record;
        record.sequence_id = ++local_sequence;
        
        clock_gettime(CLOCK_MONOTONIC, &frame_time);
        record.timestamp = (uint64_t)frame_time.tv_sec * 1000000000ULL + (uint64_t)frame_time.tv_nsec;

        snprintf(record.payload, sizeof(record.payload), "TELEMETRY_STREAM_DATA_PACKET_%lu", local_sequence);

        uint64_t current_head = atomic_load_explicit(&shm_layout->write_head, memory_order_relaxed);
        uint64_t current_tail = atomic_load_explicit(&shm_layout->read_tail, memory_order_relaxed);

        if ((current_head - current_tail) >= buffer_capacity) {
            usleep(1); 
            continue;
        }

        uint64_t head_index = current_head % buffer_capacity;
        log_record_t *dest_ptr = (log_record_t *)(&shm_layout->data_buffer[head_index * sizeof(log_record_t)]);
        
        memcpy(dest_ptr, &record, sizeof(log_record_t));
        atomic_store_explicit(&shm_layout->write_head, current_head + 1, memory_order_release);
    }

    munmap(shm_layout, SHM_TOTAL_SIZE);
    return 0;
}
