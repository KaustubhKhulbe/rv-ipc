# rv-ipc

## Layout
```
.
├── gem5                   # Modifications to the gem5 part of RVipc
├── ipc_bin                # IPC binaries to test
├── riscv-opcodes          # Macros to build a custom toolchain
├── toolchain              # Modifications to official RISC-V toolchain
├── shm / other testing scripts
└── README.md
```

## Handshake Protocol

Let Hart 0 be the producer, and Hart 1 be the consumer.

1. Hart 0 issues `fcreate` which sends a request to the pool to initialize a FIFO queue
- If the request completes, `status` will show `0`.
- If there are no available FIFOs, `status` will show `1 << ERR_NO_FIFO_AVAIL`
- If a FIFO already exists with the issued core, `status` will show `1 << ERR_FIFO_EXISTS`
- There is no dependence on Hart 1 when *setting up* the FIFO queue
- The FIFO pool automatically closes the FIFO if inactive for `INACTIVE_THRESH` cycles

2. Hart 1 issues `fconn`, which sends a request to the pool to connect to a FIFO queue
- If the request completes, `status` will show `0`. The FIFO is now ready to use.
- If there is no FIFO set up for the core, `status` will show `1 << ERR_NO_FIFO_CREATED`
- If the core has already set up a prior connection with the same core, `status` will show `1 << ERR_ALREADY_INITIALIZED`

3. Hart 0 polls the pool with `fstatus`, which sends back `status`
- `status` will include information if the handshake was successful, FIFO is ready, etc.

4. Hart 1 polls the pool with `fstatus`, which sends back `status`
- `status` will include information if the handshake was successful, FIFO is ready, etc.

5. Hart 0 is now able to send with `fsend`. Hart 1 is now able to recieve with `frecv`.

#### Reasoning/Potential Bottlenecks
- Hart 0/1 have to poll with `fstatus`. In a many-core system, this can cause back pressure on the pool to service these instructions
- The reason for this is to allow the instructions to resolve by the WB stage of the core
    - If the request succeeded/failed, we would know by the WB stage, since we keep polling
    - We also want to avoid trapping into the kernel, and do the entire communication via user space. This will come at the cost of no interrupts, but we expect handshakes to be low latency and not be IO-bound.
