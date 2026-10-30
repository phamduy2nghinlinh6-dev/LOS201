<div align="center">

# Hệ thống Linux Nhúng — LAB 01
### Chuỗi công cụ Biên dịch chéo, U-Boot Bootloader, Linux Kernel & BusyBox RootFS

[![English](https://img.shields.io/badge/Language-English-007ec6?style=flat-square)](README.md)
[![Vietnamese](https://img.shields.io/badge/Ngôn%20ngữ-Tiếng%20Việt%20(Hiện%20tại)-e05d44?style=flat-square)](README.vi.md)

<br>

![Architecture](https://img.shields.io/badge/KIẾN%20TRÚC-ARMv7%20Cortex--A9-007ec6?style=for-the-badge&logo=arm&logoColor=white)
![Bootloader](https://img.shields.io/badge/U--BOOT-2022.04-orange?style=for-the-badge)
![Linux](https://img.shields.io/badge/LINUX%20KERNEL-5.15%20LTS-fe7d37?style=for-the-badge&logo=linux&logoColor=white)
![BusyBox](https://img.shields.io/badge/BUSYBOX-1.35.0-2ea44f?style=for-the-badge)
![QEMU](https://img.shields.io/badge/QEMU-vexpress--a9-97ca00?style=for-the-badge&logo=qemu&logoColor=white)

</div>

---

## 1. Giới thiệu tổng quan

Thư mục này chứa toàn bộ kết quả thực hiện bài **LAB-01: Xây dựng Hệ thống Linux Nhúng từ đầu** trên nền tảng phần cứng mục tiêu **ARMv7 Cortex-A9 (QEMU vexpress-a9)**. Nội dung bao gồm thiết lập trình biên dịch chéo (cross-toolchain), biên dịch bộ nạp khởi động U-Boot, biên dịch nhân Linux 5.15 LTS kèm Device Tree, và đóng gói hệ thống tệp gốc tối giản (initramfs) với BusyBox.

| Thành phần | Phiên bản / Cấu hình | Trạng thái |
| :--- | :--- | :---: |
| **Trình biên dịch chéo (Toolchain)** | arm-linux-gnueabihf-gcc (ARMv7-A hard-float) | Hoàn thành |
| **Bộ nạp khởi động (U-Boot)** | U-Boot 2022.04 (vexpress_ca9x4_defconfig) | Hoàn thành |
| **Nhân Linux & Device Tree** | Linux 5.15 LTS (zImage & vexpress-v2p-ca9.dtb) | Hoàn thành |
| **Hệ thống tệp gốc (RootFS)** | BusyBox 1.35.0 liên kết tĩnh (initramfs.cpio.gz) | Hoàn thành |

---

## 2. Sơ đồ chuỗi khởi động (Boot Flow)

    +-----------------------------------------------------------------------+
    |                      Mô phỏng phần cứng trên QEMU                     |
    |                 ARM Versatile Express Cortex-A9 (512MB)               |
    +-----------------------------------+-----------------------------------+
                                        |
                                        v
    +-----------------------------------------------------------------------+
    |                    Giai đoạn 1: U-Boot Bootloader                     |
    |           Khởi tạo RAM, cổng nối tiếp, nạp zImage & DTB vào RAM       |
    +-----------------------------------+-----------------------------------+
                                        |
                                        v
    +-----------------------------------------------------------------------+
    |                   Giai đoạn 2: Nhân Linux (5.15 LTS)                  |
    |        Đọc cấu hình Device Tree (.dtb), khởi tạo phần cứng & VFS      |
    +-----------------------------------+-----------------------------------+
                                        |
                                        v
    +-----------------------------------------------------------------------+
    |                Giai đoạn 3: Hệ thống tệp gốc (initramfs)              |
    |           Giải nén cpio archive -> thực thi /sbin/init (BusyBox)      |
    +-----------------------------------+-----------------------------------+
                                        |
                                        v
    +-----------------------------------------------------------------------+
    |                 Giai đoạn 4: Không gian người dùng (Shell)            |
    |               Chạy kịch bản /etc/init.d/rcS -> Mở /bin/sh             |
    +-----------------------------------+-----------------------------------+

---

## 3. Cấu trúc thư mục

    lab1/
    ├── configs/
    │   ├── busybox.config                 # File cấu hình biên dịch tĩnh BusyBox
    │   └── kernel.config                  # File cấu hình nhân Linux 5.15 cho ARMv7
    ├── output/
    │   ├── u-boot                         # File thực thi U-Boot định dạng ELF
    │   ├── u-boot.bin                     # File nhị phân nguyên bản U-Boot
    │   ├── zImage                         # Image nhân Linux đã nén (ARM)
    │   ├── vexpress-v2p-ca9.dtb           # File mô tả phần cứng Device Tree Blob
    │   └── initramfs.cpio.gz              # Hệ thống tệp gốc ban đầu đã nén
    ├── rootfs/
    │   └── initramfs/                     # Cây thư mục gốc (bin, sbin, etc, proc, sys)
    ├── README.md                          # Tài liệu hướng dẫn (Tiếng Anh)
    └── README.vi.md                       # Tài liệu hướng dẫn (Tiếng Việt)

---

## 4. Hướng dẫn Khởi chạy & Đóng gói

### 4.1. Khởi chạy U-Boot trên QEMU
    qemu-system-arm \
      -M vexpress-a9 \
      -m 512M \
      -nographic \
      -kernel output/u-boot

### 4.2. Khởi chạy Linux Kernel cùng Initramfs trên QEMU
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

### 4.3. Đóng gói lại Initramfs
    cd rootfs/initramfs
    find . | cpio -H newc -o | gzip > ../../output/initramfs.cpio.gz

---

## 5. Tác giả
* **Họ và tên:** Phạm Đặng Khánh Duy
* **Môn học:** Hệ thống Linux Nhúng — Embedded Linux Systems (LOS201)
