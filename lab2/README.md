# Embedded Linux Lab 2

<div align="center">

![Architecture](https://img.shields.io/badge/ARCHITECTURE-ARMV7-007ec6?style=for-the-badge)
![Linux](https://img.shields.io/badge/LINUX-5.15-fe7d37?style=for-the-badge)
![QEMU](https://img.shields.io/badge/QEMU-VEXPRESS--A9-97ca00?style=for-the-badge)
![Driver](https://img.shields.io/badge/CHARACTER--DEVICE-DRIVER-8a2be2?style=for-the-badge)

![procfs/sysfs](https://img.shields.io/badge/PROCFS%20%2F%20SYSFS-VALIDATED-e05d44?style=for-the-badge)
![JFFS2/MTD](https://img.shields.io/badge/JFFS2%20%2F%20MTD-TESTED-dfb317?style=for-the-badge)

</div>

---

## 📌 Overview
Repository for **LAB-02: Character Device Driver & File System Management** in Embedded Linux Systems (`ARMv7 Cortex-A9` / `QEMU vexpress-a9`).

- **Part A:** Character Device Driver (`lab2_driver.ko`, major `240`, `/dev/lab2` with `open`, `read`, `write`, `release` & `mutex` synchronization).
- **Part B:** Kernel-Userspace communication via `procfs` (`/proc/lab2_info`) and `sysfs` (`/sys/class/lab2_class/lab2/`).
- **Part C:** NAND Flash simulation (`nandsim`), MTD partition tools, and `JFFS2` persistence verification.
- **Part D:** BusyBox `init` (`/etc/inittab` & `/etc/init.d/rcS`) automatic module loading on system boot.

---

## 📂 Repository Structure
```text
├── driver/
│   ├── lab2_driver.c            # Kernel module source code
│   ├── Makefile                 # KBuild cross-compilation Makefile
│   └── lab2_driver.ko           # Compiled ARMv7 kernel module
├── logs/
│   ├── boot_log.txt             # QEMU boot & auto-load log
│   ├── driver_test.txt          # Character device test output
│   ├── procfs_test.txt          # procfs & sysfs verification output
│   └── mtd_jffs2.txt            # Host MTD & JFFS2 test output
├── output/
│   └── initramfs_lab2.cpio.gz   # Packaged rootfs image for QEMU
├── rootfs/initramfs/            # Root filesystem tree (rcS, inittab, test scripts)
├── .gitignore
├── README.md
└── license.md

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
cat << 'EOF' > license.md
MIT License

Copyright (c) 2026 Phạm Đặng Khánh Duy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
