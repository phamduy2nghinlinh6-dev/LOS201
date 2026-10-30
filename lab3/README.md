<div align="center">

# Embedded Linux Systems — LAB 03
### Kernel Debugging, Memory Analysis, Profiling & Real-Time Latency

[![English](https://img.shields.io/badge/Language-English%20(Current)-007ec6?style=flat-square)](README.md)
[![Vietnamese](https://img.shields.io/badge/Ngôn%20ngữ-Tiếng%20Việt-e05d44?style=flat-square)](README.vi.md)

<br>

![Architecture](https://img.shields.io/badge/ARCHITECTURE-ARMv7%20Cortex--A9-007ec6?style=for-the-badge&logo=arm&logoColor=white)
![GDB](https://img.shields.io/badge/GDB%20REMOTE-DEBUGGING-8a2be2?style=for-the-badge&logo=gnu&logoColor=white)
![Valgrind](https://img.shields.io/badge/VALGRIND-0%20LEAKS%20VERIFIED-2ea44f?style=for-the-badge)
![Profiling](https://img.shields.io/badge/STRACE%20%26%20PERF-PROFILED-fe7d37?style=for-the-badge&logo=linux&logoColor=white)
![RealTime](https://img.shields.io/badge/CYCLICTEST-RT%20LATENCY-dfb317?style=for-the-badge)

</div>

---

## 1. Overview

This directory contains the complete implementation and analysis reports for **LAB-03: Kernel Debugging & Embedded Application Performance Analysis** targeting **ARMv7 Cortex-A9 (QEMU vexpress-a9)** and **x86_64 Host**. It covers remote cross-architecture debugging with `gdb-multiarch` and `gdbserver`, dynamic heap leak detection using `Valgrind`, system call and CPU hotspot profiling with `strace` and `perf`, and real-time scheduling latency benchmarking using `cyclictest` and `gnuplot`.

| Section | Focus Area & Tools | Status |
| :--- | :--- | :---: |
| **Part A — Remote Debugging** | Cross-debugging `buggy_app` on ARM target via TCP (`:1234`) using `gdbserver` and `gdb-multiarch` to isolate out-of-bounds, use-after-free, and integer overflow bugs. | Completed |
| **Part B — Memory Leak Analysis** | Detecting definite and indirect heap leaks in `leaky_app` with `Valgrind (memcheck)` and verifying zero leaks in `leaky_app_fixed`. | Completed |
| **Part C — Syscall & CPU Profiling** | Tracing I/O bottlenecks with `strace -c` and identifying CPU hotspots (`heavy_compute`) using hardware performance counters via `perf`. | Completed |
| **Part D — Real-Time Latency** | Measuring worst-case wake-up latency under stress load with `cyclictest` (`SCHED_FIFO`) and plotting latency distribution histograms with `gnuplot`. | Completed |

---

## 2. Debugging & Profiling Workflow Architecture

    +--------------------------------------------------------------------------+
    |                           HOST SYSTEM (x86_64)                           |
    |                                                                          |
    |  [ gdb-multiarch ]   [ Valgrind Memcheck ]   [ strace / perf ]           |
    |          |                     |                     |                   |
    |          | TCP :1234           v                     v                   |
    |          |              ./leaky_app_fixed      ./perf_target             |
    |          |                                                               |
    |  [ cyclictest (SCHED_FIFO) ] ---> hist_standard.txt ---> [ gnuplot PNG ] |
    +----------+---------------------------------------------------------------+
               |
               | -net user,hostfwd=tcp::1234-:1234
               v
    +--------------------------------------------------------------------------+
    |                    TARGET SYSTEM (ARMv7 QEMU vexpress-a9)                |
    |                                                                          |
    |  [ gdbserver :1234 /usr/bin/buggy_app ]                                  |
    |  [ leaky_app_arm -> /proc/PID/status & /proc/PID/maps ]                  |
    |  [ strace -c /usr/bin/perf_target_arm ]                                  |
    +--------------------------------------------------------------------------+

---

## 3. Directory Structure

    lab3/
    ├── apps/
    │   ├── buggy_app.c                    # Source code with intentional bugs (Part A)
    │   ├── buggy_app                      # ARMv7 ELF binary compiled with -g -O0
    │   ├── leaky_app.c                    # Memory leak source code (Part B)
    │   ├── leaky_app                      # Unfixed x86_64 binary (with memory leaks)
    │   ├── leaky_app_fixed                # Fixed x86_64 binary (0 leaks verified)
    │   └── perf_target.c                  # CPU & I/O intensive profiling target (Part C)
    ├── logs/
    │   ├── gdb_session.txt                # Remote GDB debugging session transcript
    │   ├── valgrind_report.txt            # Valgrind memcheck report (before fix)
    │   ├── valgrind_fixed.txt             # Valgrind report confirming 0 errors/leaks
    │   ├── strace_summary.txt             # System call time & frequency breakdown
    │   ├── perf_report.txt                # CPU hotspot call-graph analysis
    │   ├── perf_stat.txt                  # Hardware performance counter statistics
    │   ├── hist_standard.txt              # Cyclictest latency histogram data
    │   └── latency_histogram.png          # Gnuplot visualization of wake-up latency
    ├── output/
    │   └── initramfs_lab3.cpio.gz         # Packaged ARM rootfs with debug binaries
    ├── rootfs/
    │   └── initramfs/                     # Root filesystem hierarchy for LAB-03
    ├── .gitignore                         # Excludes raw perf.data and temporary files
    ├── README.md                          # Documentation (English)
    ├── README.vi.md                       # Documentation (Vietnamese)
    └── license.md                         # MIT Open Source License

---

## 4. Quick Reproduction Commands

### 4.1. Remote GDB Debugging (Part A)
    # In QEMU Target:
    gdbserver :1234 /usr/bin/buggy_app

    # On Host:
    gdb-multiarch apps/buggy_app -ex "target remote localhost:1234"

### 4.2. Valgrind Memory Check (Part B)
    valgrind --leak-check=full --show-leak-kinds=all ./apps/leaky_app_fixed

### 4.3. System Call & CPU Profiling (Part C)
    strace -c ./apps/perf_target
    sudo perf stat ./apps/perf_target

### 4.4. Real-Time Latency Test (Part D)
    sudo cyclictest --mlockall --threads=4 --priority=80 --interval=1000 --loops=10000 --histogram=200

---

## 5. Author
* **Student Name:** Pham Dang Khanh Duy
* **Course:** Embedded Linux Systems (LOS201)
* **License:** MIT License ([license.md](license.md))
