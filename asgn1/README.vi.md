<div align="center">

# Hệ thống Linux Nhúng — ASSIGNMENT 01
### Thiết kế Character Device Driver nâng cao (ioctl) & Misc Device Framework

[![English](https://img.shields.io/badge/Language-English-007ec6?style=flat-square)](README.md)
[![Vietnamese](https://img.shields.io/badge/Ngôn%20ngữ-Tiếng%20Việt%20(Hiện%20tại)-e05d44?style=flat-square)](README.vi.md)

<br>

![Architecture](https://img.shields.io/badge/KIẾN%20TRÚC-ARMv7%20Cortex--A9-007ec6?style=for-the-badge&logo=arm&logoColor=white)
![Linux](https://img.shields.io/badge/LINUX%20KERNEL-5.15%20LTS-fe7d37?style=for-the-badge&logo=linux&logoColor=white)
![IOCTL](https://img.shields.io/badge/TASK%201-GIAO%20DIỆN%20IOCTL-8a2be2?style=for-the-badge)
![MiscDevice](https://img.shields.io/badge/TASK%202-MISC%20ATOMIC%20COUNTER-2ea44f?style=for-the-badge)

</div>

---

## 1. Giới thiệu tổng quan

Thư mục này chứa toàn bộ mã nguồn, kịch bản kiểm thử và nhật ký thực thi của bài **ASSIGNMENT-01: How to Write a Device Driver** trên kiến trúc **ARMv7 Cortex-A9 (QEMU vexpress-a9)**.

| Hạng mục | Module & Thiết bị | Tính năng cốt lõi | Trạng thái |
| :--- | :--- | :--- | :---: |
| **Task 1 — Giao diện ioctl** | `asgn1_driver.ko`<br>`/dev/asgn1` (Major `241`) | Hỗ trợ 4 lệnh `ioctl`: `ASGN1_RESET_BUFFER`, `ASGN1_GET_STATS`, `ASGN1_SET_MODE` (Mode 0 thêm `[KERNEL] `, Mode 1 Echo) và `ASGN1_GET_VERSION`, đồng bộ bằng `mutex`. | Hoàn thành |
| **Task 2 — Misc Counter** | `counter_driver.ko`<br>`/dev/counter` (Major `10`) | Đăng ký qua `misc_register()`, quản lý bộ đếm `atomic_t` tự tăng khi đọc, kiểm tra đầu vào bằng `kstrtoint()` khi ghi và xuất thống kê qua `/proc/counter_info`. | Hoàn thành |

---

## 2. Cấu trúc thư mục

    asgn1/
    ├── task1_ioctl/
    │   ├── asgn1_driver.c                 # Mã nguồn driver hỗ trợ unlocked_ioctl
    │   ├── asgn1_ioctl.h                  # Header định nghĩa magic 'A', struct & macro
    │   ├── ioctl_helper.c                 # Tiện ích C gọi lệnh ioctl từ userspace
    │   ├── Makefile                       # KBuild Makefile biên dịch chéo ARMv7
    │   ├── asgn1_driver.ko                # Module nhân ARMv7 đã biên dịch
    │   └── test_asgn1.sh                  # Script kiểm thử tự động Task 1
    ├── task2_misc/
    │   ├── counter_driver.c               # Mã nguồn Misc Counter Driver dùng atomic_t
    │   ├── Makefile                       # KBuild Makefile biên dịch chéo ARMv7
    │   ├── counter_driver.ko              # Module nhân ARMv7 đã biên dịch
    │   └── test_counter.sh                # Script kiểm thử tự động Task 2
    ├── logs/
    │   ├── test_asgn1_output.txt          # Kết quả chạy test_asgn1.sh trên QEMU
    │   ├── test_counter_output.txt        # Kết quả chạy test_counter.sh trên QEMU
    │   └── dmesg_full.txt                 # Nhật ký dmesg của cả 2 driver
    ├── output/
    │   └── initramfs_asgn1.cpio.gz        # Image rootfs đóng gói cả 2 module
    ├── README.md                          # Tài liệu hướng dẫn (Tiếng Anh)
    ├── README.vi.md                       # Tài liệu hướng dẫn (Tiếng Việt)
    └── license.md                         # Giấy phép MIT

---

## 3. Tác giả
* **Họ và tên:** Phạm Đặng Khánh Duy
* **Môn học:** Hệ thống Linux Nhúng — Embedded Linux Systems (LOS201)
