#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

uint64_t fifo_create(uint64_t id, uint64_t key) {
	uint64_t status;
	asm volatile
	(
	"fcreate %[z], %[x], %[y]\n\t"
	: [z] "=r" (status)
	: [x] "r" (id), [y] "r" (key)
	);
    return status;
}

uint64_t fifo_conn(uint64_t id, uint64_t key) {
	uint64_t status;
	asm volatile
	(
	"fconn %[z], %[x], %[y]\n\t"
	: [z] "=r" (status)
	: [x] "r" (id), [y] "r" (key)
	);
    return status;
}

uint64_t fifo_send(uint64_t id, uint64_t key, uint64_t data[4]) {
    uint64_t status;

    asm volatile (
        "ld x28, 0(%[d])\n\t"   // Load data[0] into x28
        "ld x29, 8(%[d])\n\t"   // Load data[1] into x29
        "ld x30, 16(%[d])\n\t"  // Load data[2] into x30
        "ld x31, 24(%[d])\n\t"  // Load data[3] into x31
        "1:\n\t"
        "fsend %[z], %[x], %[y]\n\t"  // Execute fsend
        "li t0, -1\n\t"               // Load -1 into t0
        "beq %[z], t0, 2f\n\t"        // If status == -1, exit
        "beqz %[z], 2f\n\t"           // If status == 0, exit
        "j 1b\n\t"                    // Else, repeat fsend
        "2:\n\t"
        : [z] "=r" (status)
        : [x] "r" (id), [y] "r" (key), [d] "r" (data)
        : "x28", "x29", "x30", "x31", "t0", "memory" // Clobbers
    );

    return status;
}



uint64_t fifo_recv(uint64_t regnum, uint64_t timeout, uint64_t data[4]) {
    uint64_t status;

    asm volatile (
        "1:\n\t"
        "frecv %[z], %[x], %[y]\n\t"  // Perform frecv operation
        "li t0, -1\n\t"               // Load -1 into t0
        "beq %[z], t0, 2f\n\t"        // If status == -1, exit
        "beqz %[z], 2f\n\t"           // If status == 0, exit
        "j 1b\n\t"                    // Else, repeat frecv
        "2:\n\t"
        "sd x28, 0(%[d])\n\t"         // Store x28 into data[0]
        "sd x29, 8(%[d])\n\t"         // Store x29 into data[1]
        "sd x30, 16(%[d])\n\t"        // Store x30 into data[2]
        "sd x31, 24(%[d])\n\t"        // Store x31 into data[3]
        : [z] "=r" (status)
        : [x] "r" (regnum), [y] "r" (timeout), [d] "r" (data)
        : "x28", "x29", "x30", "x31", "t0", "memory" // Clobber list
    );

    return status;
}



uint64_t fifo_close(uint64_t core_num) {
	uint64_t status;
	asm volatile
	(
	"fclose %[z], %[x], %[y]\n\t"
	: [z] "=r" (status)
	: [x] "r" (core_num), [y] "r" (0)
	);
    return status;
}

// clobber x28, x29, x30, x31
void clobber_registers() 
{
	asm volatile (
		"li x28, 0\n\t"
		"li x29, 0\n\t"
		"li x30, 0\n\t"
		"li x31, 0\n\t"
	);
}

void fifo_test_recv(){
    uint8_t* recv_data = (uint8_t*)malloc(DEF_SIZE * 4 * 8);
    struct timespec start_time_ns, end_time_ns;

    while(fifo_conn(2,1234321) != 0) {}
    printf("Fifo connected\n");

    
    clock_gettime(CLOCK_MONOTONIC, &start_time_ns);
    for(int i = 0; i < DEF_SIZE; i++){
        // clock_gettime(CLOCK_MONOTONIC, &start_time_ns);
        while(fifo_recv(2, 1234321, (uint64_t*)(recv_data + i*32)) != 0){}
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time_ns);
    // uint64_t start_ns = start_time_ns.tv_sec * 1000000000 + start_time_ns.tv_nsec;
    uint64_t end_ns = end_time_ns.tv_sec * 1000000000 + end_time_ns.tv_nsec;
    double total_mb = (double)DEF_SIZE * 8 * 4 / 1024 / 1024;
    // double local_delta_s = (double)(end_ns - start_ns)/1000000000;


    uint64_t remote_start_ns = (uint64_t)recv_data[0] | ((uint64_t)recv_data[1] << 8) | ((uint64_t)recv_data[2] << 16) | ((uint64_t)recv_data[3] << 24) | ((uint64_t)recv_data[4] << 32) | ((uint64_t)recv_data[5] << 40) | ((uint64_t)recv_data[6] << 48) | ((uint64_t)recv_data[7] << 56);
    double remote_delta_s = (double)(end_ns - remote_start_ns)/1000000000;
    // printf("Local start: %ld, Local end: %ld, Delta_ns: %ld, Delta_ms: %f, MB: %f, MB/s: %f\n", start_ns, end_ns, end_ns - start_ns, local_delta_s, total_mb, total_mb/local_delta_s);
    printf("FIFO Remote start: %ld, Local end: %ld, Delta_ns: %ld, Delta_ms: %f, MB: %f, MB/s: %f\n", remote_start_ns, end_ns, end_ns - remote_start_ns, remote_delta_s, total_mb, total_mb/remote_delta_s);
    // printf("FIFO Delta_ns: %ld\n", end_ns - remote_start_ns);
}


int main() {
	fifo_test_recv();

    // uint64_t buf[4];
    // while(fifo_recv(2, 1234321, buf) != 0){}
    // printf("FIFO recv done\n");
}
