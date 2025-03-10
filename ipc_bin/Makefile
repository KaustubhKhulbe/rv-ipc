CC = /home/fluxbyt/riscv-compiled/bin/riscv64-unknown-elf-gcc
DUMP = /home/fluxbyt/riscv-compiled/bin/riscv64-unknown-elf-objdump
CFLAGS = -Wall -Werror

all: test

test: test.c
	$(CC) $(CFLAGS) -o test test.c

dump: test
	$(DUMP) -S test > test.dump


clean:
	rm -f test
