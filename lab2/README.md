<div align="center">

# Embedded Linux Systems — LAB 02
### Character Device Driver, Virtual Filesystems (procfs/sysfs), MTD/JFFS2 & BusyBox Init

[![English](https://img.shields.io/badge/Language-English%20(Current)-007ec6?style=flat-square)](README.md)
[![Vietnamese](https://img.shields.io/badge/Ngôn%20ngữ-Tiếng%20Việt-e05d44?style=flat-square)](README.vi.md)

<br>

![Architecture](https://img.shields.io/badge/ARCHITECTURE-ARMv7%20Cortex--A9-007ec6?style=for-the-badge&logo=arm&logoColor=white)
![Linux](https://img.shields.io/badge/LINUX%20KERNEL-5.15%20LTS-fe7d37?style=for-the-badge&logo=linux&logoColor=white)
![QEMU](https://img.shields.io/badge/QEMU-vexpress--a9-97ca00?style=for-the-badge&logo=qemu&logoColor=white)
![Driver](https://img.shields.io/badge/CHAR%20DEVICE-MAJOR%20240-8a2be2?style=for-the-badge)

![procfs/sysfs](https://img.shields.io/badge/PROCFS%20%26%20SYSFS-VALIDATED-e05d44?style=for-the-badge)
![JFFS2/MTD](https://img.shields.io/badge/MTD%20%2F%20JFFS2-PERSISTENCE%20TESTED-dfb317?style=for-the-badge)
![BusyBox](https://img.shields.io/badge/BUSYBOX%20INIT-AUTO--LOADED-2ea44f?style=for-the-badge)

</div>

---

## 1. Overview

This project implements **LAB-02: Device Driver Development & File System Management** targeting the **ARMv7 Cortex-A9 (QEMU vexpress-a9)** architecture. It features a complete Kernel Space Character Device Driver with safe User Space data transfer, real-time diagnostic interfaces via procfs and sysfs, NAND Flash emulation using the MTD subsystem with JFFS2 persistence verification, and automated module initialization via BusyBox init.

| Module / Section | Implementation Details | Status |
| :--- | :--- | :---: |
| **Part A — Character Device Driver** | Built lab2_driver.ko (Major 240), supporting open, read, write, release, mutex synchronization, and copy_to_user() / copy_from_user(). | Passed |
| **Part B — procfs & sysfs Interfaces** | Created /proc/lab2_info (using seq_file API) and attribute group /sys/class/lab2_class/lab2/{buffer_len,open_count,last_data}. | Passed |
| **Part C — MTD Subsystem & JFFS2** | Simulated 32MB NAND Flash via nandsim, erased blocks with flash_erase, formatted mkfs.jffs2, flashed via nandwrite, and verified persistence across remounts. | Passed |
| **Part D — BusyBox Init Integration** | Configured /etc/inittab and /etc/init.d/rcS to automatically insert lab2_driver.ko and create /dev/lab2 at boot time. | Passed |

---

## 2. System Architecture

    +--------------------------------------------------------------------------+
    |                               USER SPACE                                 |
    |                                                                          |
    |   [ BusyBox Shell ]     [ test_driver.sh ]       [ test_procfs.sh ]      |
    |           |                     |                        |               |
    |     open/read/write        cat / echo               cat /proc & /sys     |
    +-----------+---------------------+------------------------+---------------+
                |                     |                        |
    ============v=====================v========================v================
    +--------------------------------------------------------------------------+
    |                              KERNEL SPACE                                |
    |                                                                          |
    |                    +-----------------------------+                       |
    |                    |  VFS (Virtual File System)  |                       |
    |                    +--------------+--------------+                       |
    |                                   |                                      |
    |        +--------------------------+--------------------------+           |
    |        v                          v                          v           |
    |  +---------------+      +-------------------+      +------------------+  |
    |  |  /dev/lab2    |      |  /proc/lab2_info  |      | /sys/class/...   |  |
    |  | (Major: 240)  |      |  (seq_file API)   |      |  - buffer_len    |  |
    |  | file_ops      |      |  Read-only stats  |      |  - open_count    |  |
    |  +-------+-------+      +---------+---------+      |  - last_data     |  |
    |          |                        |                +---------+--------+  |
    |          +------------------------+--------------------------+           |
    |                                   v                                      |
    |                    +-----------------------------+                       |
    |                    |   device_buffer [1024 B]    |                       |
    |                    |  Protected by: lab2_mutex   |                       |
    |                    +-----------------------------+                       |
    |                                                                          |
    |  +--------------------------------------------------------------------+  |
    |  | MTD Subsystem (/dev/mtd0) ---> nandsim (32MB) ---> JFFS2 Filesystem|  |
    |  +--------------------------------------------------------------------+  |
    +--------------------------------------------------------------------------+

---

## 3. Repository Structure

    lab2/
    ├── driver/
    │   ├── lab2_driver.c                  # Character Device Driver C source code
    │   ├── Makefile                       # KBuild cross-compilation Makefile for ARMv7
    │   └── lab2_driver.ko                 # Compiled ARM kernel module (ELF 32-bit, 5.15.0)
    ├── logs/
    │   ├── boot_log.txt                   # QEMU boot & automatic module loading log
    │   ├── driver_test.txt                # Output of test_driver.sh execution
    │   ├── procfs_test.txt                # Output of /proc and /sys interface verification
    │   └── mtd_jffs2.txt                  # Host MTD operations & JFFS2 persistence log
    ├── output/
    │   └── initramfs_lab2.cpio.gz         # Packaged rootfs image ready for QEMU boot
    ├── rootfs/
    │   └── initramfs/                     # BusyBox root filesystem tree
    │       ├── etc/inittab                # BusyBox init process configuration
    │       ├── etc/init.d/rcS             # System startup script & auto-load module logic
    │       ├── lib/modules/5.15.0/        # Contains lab2_driver.ko
    │       └── usr/bin/                   # Contains test_driver.sh & test_procfs.sh
    ├── .gitignore                         # Ignores intermediate KBuild artifacts
    ├── README.md                          # Project documentation (English)
    ├── README.vi.md                       # Project documentation (Vietnamese)
    └── license.md                         # MIT Open Source License

---

## 4. Build & Execution Guide

### 4.1. Compile the Kernel Module
    cd driver
    make clean && make
    file lab2_driver.ko

### 4.2. Repackage Initramfs
    cd rootfs/initramfs
    find . | cpio -H newc -o | gzip > ../../output/initramfs_lab2.cpio.gz

### 4.3. Boot System on QEMU
    qemu-system-arm \
      -M vexpress-a9 \
      -cpu cortex-a9 \
      -m 512M \
      -smp 2 \
      -nographic \
      -kernel ~/embedded_lab1/lab1/output/zImage \
      -dtb ~/embedded_lab1/lab1/output/vexpress-v2p-ca9.dtb \
      -initrd output/initramfs_lab2.cpio.gz \
      -append 'console=ttyAMA0,115200 rdinit=/sbin/init mem=512M'

### 4.4. Run Verification Scripts Inside QEMU Shell
    lsmod
    ls -la /dev/lab2
    test_procfs.sh
    rmmod lab2_driver && test_driver.sh

---

## 5. Key Technical Insights

| Topic | Core Principle |
| :--- | :--- |
| **copy_to/from_user vs memcpy** | User Space and Kernel Space use separate virtual address ranges. copy_to/from_user() validates user pointers (access_ok) and safely handles page faults via exception tables without triggering a Kernel Panic like raw memcpy(). |
| **Mutex Synchronization** | Protects shared device_buffer and buffer_len during concurrent write() calls from multiple processes, preventing race conditions and data corruption. |
| **procfs vs sysfs** | /proc is suited for formatted multi-line status summaries (via seq_file), whereas /sys enforces a strict one-value-per-file rule mapped directly to the kernel kobject device hierarchy. |
| **NAND Flash & JFFS2** | Raw NAND Flash cannot overwrite 0-bits to 1-bits directly and requires block-level erasure (erase block). JFFS2 uses a log-structured design to append new nodes sequentially while providing built-in wear leveling. |

---

## 6. Author
* **Student Name:** Pham Dang Khanh Duy
* **Course:** Embedded Linux Systems (LOS201)
* **License:** MIT License ([license.md](license.md))
