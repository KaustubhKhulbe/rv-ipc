#include <stdio.h>
#include <stdint.h>
#include <time.h>

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

int main() {
	int status, i = 0, max_i = 1000;
	clock_t start_time, end_time;
	uint64_t send_data[4];
	uint64_t recv_data[4];


	status = fifo_create(2,1234321);
	printf("fcreate Status: %d\n", status);
	status = fifo_conn(2,1234321);
	printf("fconn Status: %d\n", status);


	max_i = 10;

	start_time = clock();
	while (i < max_i) {
		send_data[0] = i;
		send_data[1] = i+1;
		send_data[2] = i+2;
		send_data[3] = i+3;
		
		status = fifo_send(2, 1234321, send_data);
		if (status != 0) {
			printf("fsend Status: %d\n", status);
			break;
		}

		clobber_registers();

		status = fifo_recv(2, 1234321, recv_data);
		if (status != 0) {
			printf("frecv Status: %d\n", status);
			break;
		}

		// if (recv_data[0] != send_data[0] || recv_data[1] != send_data[1] || recv_data[2] != send_data[2] || recv_data[3] != send_data[3]) {
		// 	printf("Data mismatch at iteration %d\n", i);
		// 	break;
		// }
		i++;
	}
	end_time = clock();

	int data_transferred = max_i * 8 * 4; // 8 bytes per register, 4 registers
	double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	printf("Time taken: %f seconds\n", time_taken);
	printf("Data transferred: %d bytes\n", data_transferred);
	printf("Bandwidth: %f MB/s, %f GB/s\n", (double)data_transferred / time_taken, (double)data_transferred / time_taken / 1024 / 1024);
}
