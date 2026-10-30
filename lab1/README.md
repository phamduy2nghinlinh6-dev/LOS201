<div align="center">

# Embedded Linux Systems — LAB 01
### Cross-Toolchain, U-Boot Bootloader, Linux Kernel & BusyBox RootFS

[![English](https://img.shields.io/badge/Language-English%20(Current)-007ec6?style=flat-square)](README.md)
[![Vietnamese](https://img.shields.io/badge/Ngôn%20ngữ-Tiếng%20Việt-e05d44?style=flat-square)](README.vi.md)

<br>

![Architecture](https://img.shields.io/badge/ARCHITECTURE-ARMv7%20Cortex--A9-007ec6?style=for-the-badge&logo=arm&logoColor=white)
![Bootloader](https://img.shields.io/badge/U--BOOT-2022.04-orange?style=for-the-badge)
![Linux](https://img.shields.io/badge/LINUX%20KERNEL-5.15%20LTS-fe7d37?style=for-the-badge&logo=linux&logoColor=white)
![BusyBox](https://img.shields.io/badge/BUSYBOX-1.35.0-2ea44f?style=for-the-badge)
![QEMU](https://img.shields.io/badge/QEMU-vexpress--a9-97ca00?style=for-the-badge&logo=qemu&logoColor=white)

</div>

---

## 1. Overview

This directory contains the complete implementation of **LAB-01: Building an Embedded Linux System from Scratch** targeting the **ARMv7 Cortex-A9 (QEMU vexpress-a9)** platform. The project covers configuring the cross-compilation toolchain, building the U-Boot bootloader, compiling the Linux 5.15 LTS kernel with Device Tree blobs, and constructing a minimal root filesystem (initramfs) powered by BusyBox.

| Component | Version / Target | Status |
| :--- | :--- | :---: |
| **Cross-Compiler Toolchain** | arm-linux-gnueabihf-gcc (ARMv7-A hard-float) | Completed |
| **Bootloader (U-Boot)** | U-Boot 2022.04 (vexpress_ca9x4_defconfig) | Completed |
| **Linux Kernel & DTB** | Linux 5.15 LTS (zImage & vexpress-v2p-ca9.dtb) | Completed |
| **Root Filesystem** | BusyBox 1.35.0 static build (initramfs.cpio.gz) | Completed |

---

## 2. Boot Flow Architecture

    +-----------------------------------------------------------------------+
    |                        QEMU Hardware Emulation                        |
    |                 ARM Versatile Express Cortex-A9 (512MB)               |
    +-----------------------------------+-----------------------------------+
                                        |
                                        v
    +-----------------------------------------------------------------------+
    |                      Stage 1: U-Boot Bootloader                       |
    |             Initializes RAM, console, loads zImage & DTB              |
    +-----------------------------------+-----------------------------------+
                                        |
                                        v
    +-----------------------------------------------------------------------+
    |                      Stage 2: Linux Kernel (5.15)                     |
    |          Parses Device Tree (.dtb), initializes hardware & VFS        |
    +-----------------------------------+-----------------------------------+
                                        |
                                        v
    +-----------------------------------------------------------------------+
    |                 Stage 3: Root Filesystem (initramfs)                  |
    |         Mounts cpio archive -> executes /sbin/init (BusyBox)          |
    +-----------------------------------+-----------------------------------+
                                        |
                                        v
    +-----------------------------------------------------------------------+
    |                       Stage 4: User Space Shell                       |
    |                 Runs /etc/init.d/rcS -> Opens /bin/sh                 |
    +-----------------------------------+-----------------------------------+

---

## 3. Directory Structure

    lab1/
    ├── configs/
    │   ├── busybox.config                 # BusyBox static configuration file
    │   └── kernel.config                  # Linux 5.15 ARMv7 kernel configuration
    ├── output/
    │   ├── u-boot                         # U-Boot ELF binary
    │   ├── u-boot.bin                     # U-Boot raw binary executable
    │   ├── zImage                         # Compressed Linux kernel image (ARM)
    │   ├── vexpress-v2p-ca9.dtb           # Compiled Device Tree Blob
    │   └── initramfs.cpio.gz              # Compressed initial RAM filesystem
    ├── rootfs/
    │   └── initramfs/                     # Root filesystem hierarchy (bin, sbin, etc, proc, sys)
    ├── README.md                          # Documentation (English)
    └── README.vi.md                       # Documentation (Vietnamese)

---

## 4. Build & Execution Instructions

### 4.1. Run U-Boot on QEMU
    qemu-system-arm \
      -M vexpress-a9 \
      -m 512M \
      -nographic \
      -kernel output/u-boot

### 4.2. Boot Linux Kernel with Initramfs on QEMU
    qemu-system-arm \
      -M vexpress-a9 \
      -cpu cortex-a9 \
      -m 512M \
      -smp 2 \
      -nographic \
      -kernel output/zImage \
      -dtb output/vexpress-v2p-ca9.dtb \
      -initrd output/initramfs.cpio.gz \
      -append "console=ttyAMA0,115200 rdinit=/sbin/init mem=512M"

### 4.3. Repackage Initramfs
    cd rootfs/initramfs
    find . | cpio -H newc -o | gzip > ../../output/initramfs.cpio.gz

---

## 5. Author
* **Student Name:** Pham Dang Khanh Duy
* **Course:** Embedded Linux Systems (LOS201)
