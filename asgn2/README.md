<div align="center">

# Embedded Linux Systems — CAPSTONE ASSIGNMENT 02
### Industrial Sensor Monitoring System (Multi-thread, IPC, Kernel Driver & Real-Time)

[![English](https://img.shields.io/badge/Language-English%20(Current)-007ec6?style=flat-square)](README.md)
[![Vietnamese](https://img.shields.io/badge/Ngôn%20ngữ-Tiếng%20Việt-e05d44?style=flat-square)](README.vi.md)

<br>

![Architecture](https://img.shields.io/badge/ARCHITECTURE-ARMv7%20Cortex--A9-007ec6?style=for-the-badge&logo=arm&logoColor=white)
![KernelDriver](https://img.shields.io/badge/KERNEL%20DRIVER-sms__sensor.ko-8a2be2?style=for-the-badge&logo=linux&logoColor=white)
![MultiThread](https://img.shields.io/badge/POSIX%20THREADS-4%20CONCURRENT%20THREADS-fe7d37?style=for-the-badge)
![Valgrind](https://img.shields.io/badge/VALGRIND-0%20BYTES%20LEAKED-2ea44f?style=for-the-badge)
![RealTime](https://img.shields.io/badge/REAL--TIME-SCHED__FIFO%20JITTER-dfb317?style=for-the-badge)

</div>

---

## 1. Overview

This directory contains the capstone implementation for **ASSIGNMENT-02: Write Embedded Application – Sensor Monitoring System (SMS)** targeting **ARMv7 Cortex-A9 (QEMU vexpress-a9)** and **x86_64 Linux Host**. It integrates a custom kernel sensor driver (`sms_sensor_driver.ko`) with a 4-thread real-time userspace monitoring application (`sms_app`), synchronized via a heap-allocated circular buffer (`pthread_mutex_t` + `pthread_cond_t`) and POSIX Message Queues (`mq_open`/`mq_send`/`mq_receive`).

| Component | Type | Responsibility & Key Mechanisms | Status |
| :--- | :--- | :--- | :---: |
| **`sms_sensor_driver.ko`** | Kernel Module | Exposes `/dev/sms_sensor` (CSV stream: `<ts_ms>,<temp>,<humid>`), `/proc/sms_stats`, and `SMS_SET_INTERVAL` ioctl. | Verified |
| **Sensor Thread** | `SCHED_FIFO` Thread | Periodically samples `/dev/sms_sensor` using `clock_nanosleep(TIMER_ABSTIME)`, measures wake-up jitter, pushes to `shared_buffer`. | Verified |
| **Processor Thread** | Worker Thread | Pops samples from `shared_buffer`, computes 10-sample rolling average, detects threshold breaches (`TEMP_HIGH`, `HUMID_HIGH`, `TEMP_SPIKE`). | Verified |
| **Alert Thread** | IPC Consumer | Receives alarms from `alarm_queue` (POSIX MQ), logs to `stderr`, tracks alert counters, and forwards records to Logger Thread. | Verified |
| **Logger Thread** | I/O Daemon | Writes timestamped logs to `/tmp/sms.log` with 5-second periodic `fflush()` and automatic 1MB file rotation (`sms.log.1`). | Verified |

---

## 2. System Architecture

    +-----------------------------------------------------------------------------+
    |                       USER SPACE APPLICATION (sms_app)                      |
    |                                                                             |
    |  +-------------------+     shared_buffer      +--------------------------+  |
    |  |   Sensor Thread   | ---------------------> |     Processor Thread     |  |
    |  | (SCHED_FIFO, 50)  |  (Mutex + Condvar, 64) | (10-sample Rolling Avg)  |  |
    |  +---------+---------+                        +------------+-------------+  |
    |            ^                                               |                |
    |            | read() & ioctl()                              | alarm_queue    |
    |            |                                               v (POSIX MQ)     |
    |            |                                  +--------------------------+  |
    |            |                                  |       Alert Thread       |  |
    |            |                                  | (TEMP/HUMID/SPIKE Alert) |  |
    |            |                                  +------------+-------------+  |
    |            |                                               | log_queue      |
    |            |                                               v                |
    |            |                                  +--------------------------+  |
    |            |                                  |      Logger Thread       |  |
    |            |                                  | (/tmp/sms.log, 1MB rot.) |  |
    |            |                                  +--------------------------+  |
    +------------+----------------------------------------------------------------+
                 |
    =============v=================================================================
    +-----------------------------------------------------------------------------+
    |                         KERNEL SPACE (sms_sensor_driver.ko)                 |
    |                                                                             |
    |   [ /dev/sms_sensor (Misc Major 10) ]          [ /proc/sms_stats ]          |
    |   - get_random_u32() temp & humidity           - read_count, last values    |
    |   - jiffies_to_msecs() timestamp               - driver_uptime (seconds)    |
    +-----------------------------------------------------------------------------+

---

## 3. Directory Structure

    asgn2/
    ├── driver/
    │   ├── sms_sensor_driver.c            # Simulated sensor kernel driver source
    │   ├── sms_sensor.h                   # Shared header (ioctl & device paths)
    │   ├── Makefile                       # ARMv7 KBuild Makefile
    │   └── sms_sensor_driver.ko           # Compiled ARMv7 kernel module
    ├── app/
    │   ├── sms_app.c                      # Main application, CLI parser & SIGINT handler
    │   ├── circular_buffer.c/.h           # Thread-safe 64-item heap circular buffer
    │   ├── sensor_thread.c/.h             # SCHED_FIFO periodic sampler & jitter tracker
    │   ├── processor_thread.c/.h          # Rolling window analytics & anomaly detector
    │   ├── alert_thread.c/.h              # POSIX Message Queue alarm consumer
    │   ├── logger_thread.c/.h             # Buffered file logger with 1MB rotation
    │   ├── Makefile                       # Supports `make arm` & `make host`
    │   ├── sms_app                        # Static ARMv7 ELF binary for QEMU
    │   └── sms_app_x86                    # x86_64 ELF binary for Valgrind/strace/perf
    ├── logs/
    │   ├── sms_sample_run.txt             # QEMU execution log (verbose mode & footprint)
    │   ├── sms.log                        # Output log generated by Logger Thread
    │   ├── valgrind_sms.txt               # Valgrind full memcheck report (0 bytes lost)
    │   ├── strace_sms.txt                 # System call breakdown (strace -f -c)
    │   ├── perf_sms.txt                   # CPU hotspot & performance counter analysis
    │   ├── gdb_sms.txt                    # Multi-thread GDB debugging transcript
    │   ├── proc_sms_stats_*.txt           # /proc/sms_stats snapshots (init, mid, final)
    │   └── jitter_report.txt              # SCHED_FIFO vs SCHED_OTHER latency comparison
    ├── output/
    │   └── initramfs_asgn2.cpio.gz        # Bootable ARMv7 rootfs image with SMS suite
    ├── README.md                          # Documentation (English)
    ├── README.vi.md                       # Documentation (Vietnamese)
    └── license.md                         # MIT License

---

## 4. Author
* **Student Name:** Pham Dang Khanh Duy
* **Course:** Embedded Linux Systems (LOS201)
