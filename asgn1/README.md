<div align="center">

# Embedded Linux Systems — ASSIGNMENT 01
### Advanced Character Device Driver (ioctl Interface) & Misc Device Framework

[![English](https://img.shields.io/badge/Language-English%20(Current)-007ec6?style=flat-square)](README.md)
[![Vietnamese](https://img.shields.io/badge/Ngôn%20ngữ-Tiếng%20Việt-e05d44?style=flat-square)](README.vi.md)

<br>

![Architecture](https://img.shields.io/badge/ARCHITECTURE-ARMv7%20Cortex--A9-007ec6?style=for-the-badge&logo=arm&logoColor=white)
![Linux](https://img.shields.io/badge/LINUX%20KERNEL-5.15%20LTS-fe7d37?style=for-the-badge&logo=linux&logoColor=white)
![IOCTL](https://img.shields.io/badge/TASK%201-IOCTL%20INTERFACE-8a2be2?style=for-the-badge)
![MiscDevice](https://img.shields.io/badge/TASK%202-MISC%20ATOMIC%20COUNTER-2ea44f?style=for-the-badge)

</div>

---

## 1. Overview

This directory contains the complete implementation of **ASSIGNMENT-01: How to Write a Device Driver** targeting **ARMv7 Cortex-A9 (QEMU vexpress-a9)**. The project designs and validates two independent kernel modules demonstrating multi-command `unlocked_ioctl` control and the Linux `miscdevice` subsystem with SMP-safe `atomic_t` operations.

| Task | Module & Device Node | Key Features | Status |
| :--- | :--- | :--- | :---: |
| **Task 1 — ioctl Driver** | `asgn1_driver.ko`<br>`/dev/asgn1` (Major `241`) | Supports `ASGN1_RESET_BUFFER`, `ASGN1_GET_STATS`, `ASGN1_SET_MODE` (Kernel prefix vs. Echo mode), and `ASGN1_GET_VERSION` with `mutex` protection. | Verified |
| **Task 2 — Misc Counter** | `counter_driver.ko`<br>`/dev/counter` (Major `10`) | Uses `misc_register()`, `atomic_t` auto-increment on read, `kstrtoint()` validation on write, and `/proc/counter_info` diagnostics. | Verified |

---

## 2. System Architecture

    +--------------------------------------------------------------------------+
    |                               USER SPACE                                 |
    |                                                                          |
    |    [ test_asgn1.sh / ioctl_helper ]          [ test_counter.sh ]         |
    |                   |                                   |                  |
    |     open/read/write/ioctl('A', 1..4)        open/read/write & cat /proc  |
    +-------------------+-----------------------------------+------------------+
                        |                                   |
    ====================v===================================v===================
    +--------------------------------------------------------------------------+
    |                              KERNEL SPACE                                |
    |                                                                          |
    |  +---------------------------------+  +-------------------------------+  |
    |  | Task 1: asgn1_driver (Maj 241)  |  | Task 2: counter_driver (Maj10)|  |
    |  | - unlocked_ioctl (4 commands)   |  | - struct miscdevice           |  |
    |  | - Mode 0: "[KERNEL] " prefix    |  | - atomic_t counter_val        |  |
    |  | - Mode 1: Raw Echo mode         |  | - kstrtoint() input parser    |  |
    |  | - Protected by: asgn1_mutex     |  | - /proc/counter_info (seq)    |  |
    |  +---------------------------------+  +-------------------------------+  |
    +--------------------------------------------------------------------------+

---

## 3. Directory Structure

    asgn1/
    ├── task1_ioctl/
    │   ├── asgn1_driver.c                 # Character driver with unlocked_ioctl
    │   ├── asgn1_ioctl.h                  # Shared header: magic 'A', struct & macros
    │   ├── ioctl_helper.c                 # Userspace C helper for ioctl invocations
    │   ├── Makefile                       # ARMv7 KBuild Makefile
    │   ├── asgn1_driver.ko                # Compiled ARMv7 kernel module
    │   └── test_asgn1.sh                  # Automated test script for Task 1
    ├── task2_misc/
    │   ├── counter_driver.c               # Misc device driver using atomic_t
    │   ├── Makefile                       # ARMv7 KBuild Makefile
    │   ├── counter_driver.ko              # Compiled ARMv7 kernel module
    │   └── test_counter.sh                # Automated test script for Task 2
    ├── logs/
    │   ├── test_asgn1_output.txt          # Execution output of test_asgn1.sh
    │   ├── test_counter_output.txt        # Execution output of test_counter.sh
    │   └── dmesg_full.txt                 # Kernel log messages for both drivers
    ├── output/
    │   └── initramfs_asgn1.cpio.gz        # Bootable ARM initramfs with both modules
    ├── README.md                          # Documentation (English)
    ├── README.vi.md                       # Documentation (Vietnamese)
    └── license.md                         # MIT License

---

## 4. Author
* **Student Name:** Pham Dang Khanh Duy
* **Course:** Embedded Linux Systems (LOS201)
