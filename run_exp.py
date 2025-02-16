import subprocess
import re
import matplotlib.pyplot as plt



def parse_and_plot_results_sizes(filename):
    with open(filename, 'r') as f:
        content = f.read()

    # Split by each experiment block
    blocks = content.strip().split('----------------------------------------')

    sizes = []
    fifo_mbps = []
    shm_mbps = []

    for block in blocks:
        if not block.strip():
            continue

        # Extract TOTAL_DATA_TO_SEND
        size_match = re.search(r'TOTAL_DATA_TO_SEND=(\d+)', block)
        fifo_match = re.search(r'FIFO .*MB/s: ([\d.]+)', block)
        shm_match = re.search(r'SHM .*MB/s: ([\d.]+)', block)

        if size_match and fifo_match and shm_match:
            size_bytes = int(size_match.group(1))
            size_mb = size_bytes / (1024 * 1024)
            fifo_speed = float(fifo_match.group(1))
            shm_speed = float(shm_match.group(1))

            sizes.append(size_mb)
            fifo_mbps.append(fifo_speed)
            shm_mbps.append(shm_speed)

    print(sizes)

    # Plotting
    plt.figure(figsize=(10, 6))
    plt.plot(sizes, fifo_mbps, marker='o', label='FIFO MB/s')
    plt.plot(sizes, shm_mbps, marker='o', label='SHM MB/s')

    plt.xscale('log', base=2)
    plt.xlabel('Data Size (MB)')
    plt.ylabel('Throughput (MB/s)')
    plt.title('Throughput vs Data Size')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

def parse_and_plot_results_cache_sizes(filename):
    with open(filename, 'r') as f:
        content = f.read()

    # Split by each experiment block
    blocks = content.strip().split('----------------------------------------')

    sizes = []
    fifo_mbps = []
    shm_mbps = []

    for block in blocks:
        if not block.strip():
            continue

        # Extract TOTAL_DATA_TO_SEND
        size_match = re.search(r'L1D_SIZE=(\d+)', block)
        fifo_match = re.search(r'FIFO .*MB/s: ([\d.]+)', block)
        shm_match = re.search(r'SHM .*MB/s: ([\d.]+)', block)

        if size_match and fifo_match and shm_match:
            size_bytes = int(size_match.group(1))
            size_mb = size_bytes / (1024 * 1024)
            fifo_speed = float(fifo_match.group(1))
            shm_speed = float(shm_match.group(1))

            sizes.append(size_mb)
            fifo_mbps.append(fifo_speed)
            shm_mbps.append(shm_speed)

    print(sizes)

    # Plotting
    plt.figure(figsize=(10, 6))
    plt.plot(sizes, fifo_mbps, marker='o', label='FIFO MB/s')
    plt.plot(sizes, shm_mbps, marker='o', label='SHM MB/s')

    plt.xscale('log', base=2)
    plt.xlabel('Cacheline Size (B)')
    plt.ylabel('Throughput (MB/s)')
    plt.title('Throughput vs Cacheline Size')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

def parse_and_plot_results_sizes_latency(filename):
    with open(filename, 'r') as f:
        content = f.read()

    # Split by each experiment block
    blocks = content.strip().split('----------------------------------------')

    sizes = []
    fifo_mbps = []
    shm_mbps = []
    fifo_delta_ns = []
    shm_delta_ns = []

    for block in blocks:
        if not block.strip():
            continue


        # Extract TOTAL_DATA_TO_SEND
        size_match = re.search(r'TOTAL_DATA_TO_SEND=(\d+)', block)
        fifo_ns_match = re.search(r'FIFO .*Delta_ns: (\d+)', block)
        shm_ns_match = re.search(r'SHM .*Delta_ns: (\d+)', block)


        if size_match and fifo_ns_match and shm_ns_match:
            size_bytes = int(size_match.group(1))
            size_mb = size_bytes / (1024 * 1024)
            fifo_ns = int(fifo_ns_match.group(1))
            shm_ns = int(shm_ns_match.group(1))
            

            sizes.append(size_mb)
            fifo_delta_ns.append(fifo_ns)
            shm_delta_ns.append(shm_ns)


    # Plotting Throughput (MB/s)
    fig, ax1 = plt.subplots(figsize=(10, 6))

    ax1.plot(sizes, fifo_delta_ns, marker='o', label='FIFO Delta_ns', color='blue')
    ax1.plot(sizes, shm_delta_ns, marker='o', label='SHM Delta_ns', color='green')

    ax1.set_xlabel('Data Size (MB)')
    ax1.set_ylabel('First message latency (ns)')
    ax1.set_xscale('log', base=2)
    ax1.set_title('First message latency (ns) vs Data Size')
    ax1.legend(loc='upper left')
    ax1.grid(True)

    plt.tight_layout()
    plt.show()

def run_exp_sizes():
    # List of TOTAL_DATA_TO_SEND values (in bytes)
    data_sizes = [32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768]

    # File paths
    makefile_path = 'ipc_bin/Makefile'
    output_file = 'results.txt'

    with open(output_file, 'w') as f:
        f.write('')

    # Read the Makefile
    with open(makefile_path, 'r') as f:
        makefile_contents = f.readlines()

    for size in data_sizes:
        print(f"Running experiment for TOTAL_DATA_TO_SEND={size}")

        # Modify the Makefile line
        with open(makefile_path, 'w') as f:
            for line in makefile_contents:
                if line.startswith("TOTAL_DATA_TO_SEND="):
                    f.write(f"TOTAL_DATA_TO_SEND={size}\n")
                else:
                    f.write(line)

        gem5_cmd = [
            "build/RISCV/gem5.fast",
            "--outdir=m5out/",
            "configs/deprecated/example/se.py",
            "--cpu-type=RiscvTimingSimpleCPU",
            "--num-cpus=2",
            "--redirects", "/lib=/home/fluxbyt/riscv-compiled/lib",
            "--cpu-clock=4GHz",
            "--cacheline_size=64",
            "--caches",
            "--l1i_size=8kB",
            "--l1d_size=8kB",
            "--l2cache",
            "--l2_size=4MB",
            "--l2_assoc=16",
            "--cmd=ipc_bin/test_send;ipc_bin/test_recv"
        ]

        # Run make clean && make
        subprocess.run(["make", "clean"], cwd="ipc_bin", check=True)
        subprocess.run(["make"], cwd="ipc_bin", check=True)

        # Run ./run.sh
        result = subprocess.run(gem5_cmd, capture_output=True, text=True)
        output = result.stdout + result.stderr

        # Extract the two lines
        fifo_line = None
        shm_line = None

        for line in output.splitlines():
            if "FIFO Remote start" in line:
                fifo_line = line
            elif "SHM Send done" in line:
                shm_line = line

        # Append to results file
        with open(output_file, 'a') as f:
            f.write(f"TOTAL_DATA_TO_SEND={size}\n")
            if fifo_line:
                f.write(f"{fifo_line}\n")
            if shm_line:
                f.write(f"{shm_line}\n")
            f.write("-" * 40 + "\n")

    print(f"All experiments complete. Results saved in {output_file}")

def run_exp_cache_sizes():
    # List of TOTAL_DATA_TO_SEND values (in bytes)
    cache_sizes = [16, 32, 64, 128, 256, 512]

    # File paths
    makefile_path = 'ipc_bin/Makefile'
    output_file = 'results.txt'
    size_of_data = 32

    with open(output_file, 'w') as f:
        f.write('')

    # Read the Makefile
    with open(makefile_path, 'r') as f:
        makefile_contents = f.readlines()

    for size in cache_sizes:

        # Modify the Makefile line
        with open(makefile_path, 'w') as f:
            for line in makefile_contents:
                if line.startswith("TOTAL_DATA_TO_SEND="):
                    f.write(f"TOTAL_DATA_TO_SEND={size_of_data}\n")
                else:
                    f.write(line)

        gem5_cmd = [
            "build/RISCV/gem5.fast",
            "--outdir=m5out/",
            "configs/deprecated/example/se.py",
            "--cpu-type=RiscvTimingSimpleCPU",
            "--num-cpus=2",
            "--redirects", "/lib=/home/fluxbyt/riscv-compiled/lib",
            "--cpu-clock=4GHz",
            "--cacheline_size={}".format(size),
            "--caches",
            "--l1i_size=8kB",
            "--l1d_size=8kB",
            "--l2cache",
            "--l2_size=4MB",
            "--l2_assoc=16",
            "--cmd=ipc_bin/test_send;ipc_bin/test_recv"
        ]

        # Run make clean && make
        subprocess.run(["make", "clean"], cwd="ipc_bin", check=True)
        subprocess.run(["make"], cwd="ipc_bin", check=True)

        # Run ./run.sh
        result = subprocess.run(gem5_cmd, capture_output=True, text=True)
        output = result.stdout + result.stderr

        # Extract the two lines
        fifo_line = None
        shm_line = None

        for line in output.splitlines():
            if "FIFO Remote start" in line:
                fifo_line = line
            elif "SHM Send done" in line:
                shm_line = line

        # Append to results file
        with open(output_file, 'a') as f:
            # f.write(f"L1D_SIZE={size}\n")
            if fifo_line:
                f.write(f"{fifo_line}\n")
            # if shm_line:
                # f.write(f"{shm_line}\n")
            # f.write("-" * 40 + "\n")

    print(f"All experiments complete. Results saved in {output_file}")   

def run_exp_sizes_latency():
    # List of TOTAL_DATA_TO_SEND values (in bytes)
    data_sizes = [2**i for i in range(5,16)]

    # File paths
    makefile_path = 'ipc_bin/Makefile'
    output_file = 'results.txt'

    # Clear previous results
    with open(output_file, 'w') as f:
        f.write('')

    # Read the Makefile template once
    with open(makefile_path, 'r') as f:
        makefile_contents = f.readlines()


    print(data_sizes)
    for size in data_sizes:
        print(f"Running experiment for TOTAL_DATA_TO_SEND={size}")

        # Modify the Makefile line
        with open(makefile_path, 'w') as f:
            for line in makefile_contents:
                if line.startswith("TOTAL_DATA_TO_SEND="):
                    f.write(f"TOTAL_DATA_TO_SEND={size}\n")
                else:
                    f.write(line)

        # Rebuild the project
        subprocess.run(["make", "clean"], cwd="ipc_bin", check=True)
        subprocess.run(["make"], cwd="ipc_bin", check=True)

        # Run gem5 with given arguments
        gem5_cmd = [
            "build/RISCV/gem5.fast",
            "--outdir=m5out/",
            "configs/deprecated/example/se.py",
            "--cpu-type=RiscvTimingSimpleCPU",
            "--num-cpus=2",
            "--redirects", "/lib=/home/fluxbyt/riscv-compiled/lib",
            "--cpu-clock=4GHz",
            "--cacheline_size=64",
            "--caches",
            "--l1i_size=8kB",
            "--l1d_size=8kB",
            "--l2cache",
            "--l2_size=4MB",
            "--l2_assoc=16",
            "--cmd=ipc_bin/test_send;ipc_bin/test_recv"
        ]

        result = subprocess.run(gem5_cmd, capture_output=True, text=True)
        output = result.stdout + result.stderr

        # Extract relevant lines and Delta_ns
        fifo_line = None
        shm_line = None
        fifo_ns = None
        shm_ns = None

        for line in output.splitlines():
            if "Local start" in line or "FIFO Remote start" in line:
                fifo_line = line
                match = re.search(r'Delta_ns:\s*(\d+)', line)
                if match:
                    fifo_ns = int(match.group(1))
            elif "SHM Send done" in line:
                shm_line = line
                match = re.search(r'Delta_ns:\s*(\d+)', line)
                if match:
                    shm_ns = int(match.group(1))

        # Append to results file
        with open(output_file, 'a') as f:
            # f.write(f"TOTAL_DATA_TO_SEND={size}\n")
            if fifo_line:
                f.write(f"{fifo_line}\n")
                # f.write(f"FIFO Delta_ns: {fifo_ns}\n")
            if shm_line:
                f.write(f"{shm_line}\n")
                f.write(f"SHM Delta_ns: {shm_ns}\n")
            # f.write("-" * 40 + "\n")

    print(f"All experiments complete. Results saved in {output_file}")

# run_exp_sizes_latency()
run_exp_cache_sizes()
# parse_and_plot_results_sizes_latency('results.txt')
# parse_and_plot_results_cache_sizes('results.txt')