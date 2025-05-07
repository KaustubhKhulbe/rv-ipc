#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#define SHM_NAME "/shm_benchmark"

int main(int argc, char *argv[]) {
    __sync_synchronize(); // prevent caching
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <size_in_bytes>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse the size parameter
    size_t shm_size = atoi(argv[1]);
    if (shm_size <= 0) {
        fprintf(stderr, "Error: Size must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }

    int shm_fd;
    char *shm_base;
    struct timespec start, end;

    // Create the shared memory object
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Set the size of the shared memory object
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    // Map the shared memory into the address space
    shm_base = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Pin this process to CPU 0
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    if (sched_setaffinity(getpid(), sizeof(mask), &mask) == -1) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }

    // Write data to shared memory
    clock_gettime(CLOCK_MONOTONIC, &start);
    memset(shm_base, 'A', shm_size); // Fill with data
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time
    long elapsed_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    printf("%ld, ", elapsed_ns);

    // Clean up
    // munmap(shm_base, shm_size);
    close(shm_fd);
    // shm_unlink(SHM_NAME); // Remove the shared memory object

    return 0;
}