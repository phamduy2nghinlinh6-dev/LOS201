<div align="center">

# Hệ thống Linux Nhúng — LAB 02
### Viết Character Device Driver, Giao tiếp procfs/sysfs, MTD/JFFS2 & BusyBox Init

![Architecture](https://img.shields.io/badge/KIẾN%20TRÚC-ARMv7%20Cortex--A9-007ec6?style=for-the-badge&logo=arm&logoColor=white)
![Linux](https://img.shields.io/badge/LINUX%20KERNEL-5.15%20LTS-fe7d37?style=for-the-badge&logo=linux&logoColor=white)
![QEMU](https://img.shields.io/badge/QEMU-vexpress--a9-97ca00?style=for-the-badge&logo=qemu&logoColor=white)
![Driver](https://img.shields.io/badge/CHAR%20DEVICE-MAJOR%20240-8a2be2?style=for-the-badge)

![procfs/sysfs](https://img.shields.io/badge/PROCFS%20%26%20SYSFS-ĐÃ%20KIỂM%20CHỨNG-e05d44?style=for-the-badge)
![JFFS2/MTD](https://img.shields.io/badge/MTD%20%2F%20JFFS2-BỀN%20VỮNG%20DỮ%20LIỆU-dfb317?style=for-the-badge)
![BusyBox](https://img.shields.io/badge/BUSYBOX%20INIT-TỰ%20ĐỘNG%20NẠP-2ea44f?style=for-the-badge)

</div>

---

## 1. Giới thiệu tổng quan

Thư mục này chứa toàn bộ mã nguồn, cấu hình hệ thống tệp gốc (`rootfs`), kết quả đóng gói (`output`) và nhật ký kiểm thử (`logs`) của bài thực hành **LAB-02: Viết Device Driver & Quản lý File System** trên kiến trúc **ARMv7 Cortex-A9 (`QEMU vexpress-a9`)**.

| Phần thực hành | Nội dung triển khai | Kết quả |
| :--- | :--- | :---: |
| **Phần A — Character Device Driver** | Xây dựng module `lab2_driver.ko` (Major `240`), hỗ trợ `open`, `read`, `write`, `release`, bảo vệ bộ đệm bằng `mutex` và truyền dữ liệu qua `copy_to_user()` / `copy_from_user()`. | Hoàn thành |
| **Phần B — Giao tiếp procfs & sysfs** | Tạo giao diện thống kê `/proc/lab2_info` (dùng `seq_file` API) và các thuộc tính `/sys/class/lab2_class/lab2/{buffer_len,open_count,last_data}`. | Hoàn thành |
| **Phần C — MTD Subsystem & JFFS2** | Giả lập bộ nhớ NAND Flash 32MB qua `nandsim`, xóa phân vùng với `flash_erase`, tạo image `mkfs.jffs2`, nạp bằng `nandwrite` và kiểm tra tính bền vững dữ liệu (persistence). | Hoàn thành |
| **Phần D — Tích hợp BusyBox Init** | Cấu hình `/etc/inittab` và `/etc/init.d/rcS` để tự động nạp `lab2_driver.ko` và tạo thiết bị `/dev/lab2` ngay khi khởi động hệ thống. | Hoàn thành |

---

## 2. Sơ đồ kiến trúc hệ thống

    +--------------------------------------------------------------------------+
    |                         KHÔNG GIAN NGƯỜI DÙNG (USER SPACE)               |
    |                                                                          |
    |   [ BusyBox Shell ]     [ test_driver.sh ]       [ test_procfs.sh ]      |
    |           |                     |                        |               |
    |     open/read/write        cat / echo               cat /proc & /sys     |
    +-----------+---------------------+------------------------+---------------+
                |                     |                        |
    ============v=====================v========================v================
    +--------------------------------------------------------------------------+
    |                           KHÔNG GIAN NHÂN (KERNEL SPACE)                 |
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
    |  | file_ops      |      |  Thống kê chỉ đọc |      |  - open_count    |  |
    |  +-------+-------+      +---------+---------+      |  - last_data     |  |
    |          |                        |                +---------+--------+  |
    |          +------------------------+--------------------------+           |
    |                                   v                                      |
    |                    +-----------------------------+                       |
    |                    |   device_buffer [1024 B]    |                       |
    |                    |  Đồng bộ bởi: lab2_mutex    |                       |
    |                    +-----------------------------+                       |
    |                                                                          |
    |  +--------------------------------------------------------------------+  |
    |  | MTD Subsystem (/dev/mtd0) ---> nandsim (32MB) ---> JFFS2 Filesystem|  |
    |  +--------------------------------------------------------------------+  |
    +--------------------------------------------------------------------------+

---

## 3. Cấu trúc thư mục

    lab2/
    ├── driver/
    │   ├── lab2_driver.c                  # Mã nguồn C của Character Device Driver
    │   ├── Makefile                       # KBuild Makefile biên dịch chéo cho ARMv7
    │   └── lab2_driver.ko                 # Kernel module binary (ELF 32-bit ARM, 5.15.0)
    ├── logs/
    │   ├── boot_log.txt                   # Log khởi động QEMU & tự động nạp driver
    │   ├── driver_test.txt                # Kết quả chạy kịch bản test_driver.sh
    │   ├── procfs_test.txt                # Kết quả kiểm thử giao diện /proc và /sys
    │   └── mtd_jffs2.txt                  # Log thao tác MTD, JFFS2 & kiểm tra persistence
    ├── output/
    │   └── initramfs_lab2.cpio.gz         # Image rootfs đóng gói sẵn sàng boot trên QEMU
    ├── rootfs/
    │   └── initramfs/                     # Cây thư mục gốc BusyBox
    │       ├── etc/inittab                # Cấu hình tiến trình khởi tạo BusyBox init
    │       ├── etc/init.d/rcS             # Script khởi động hệ thống & auto-load module
    │       ├── lib/modules/5.15.0/        # Chứa module lab2_driver.ko
    │       └── usr/bin/                   # Chứa test_driver.sh và test_procfs.sh
    ├── .gitignore                         # Danh sách loại trừ các file rác khi biên dịch
    ├── README.md                          # Tài liệu hướng dẫn và mô tả bài Lab 2 (Tiếng Việt)
    └── license.md                         # Giấy phép mã nguồn mở MIT

---

## 4. Hướng dẫn Biên dịch & Chạy thực nghiệm

### 4.1. Biên dịch Kernel Module
    cd driver
    make clean && make
    file lab2_driver.ko

### 4.2. Đóng gói lại initramfs
    cd rootfs/initramfs
    find . | cpio -H newc -o | gzip > ../../output/initramfs_lab2.cpio.gz

### 4.3. Khởi động hệ thống trên QEMU
    qemu-system-arm \
      -M vexpress-a9 \
      -cpu cortex-a9 \
      -m 512M \
      -smp 2 \
      -nographic \
      -kernel ../lab1/output/zImage \
      -dtb ../lab1/output/vexpress-v2p-ca9.dtb \
      -initrd output/initramfs_lab2.cpio.gz \
      -append 'console=ttyAMA0,115200 rdinit=/sbin/init mem=512M'

### 4.4. Các lệnh kiểm thử trong QEMU Shell
    lsmod
    ls -la /dev/lab2
    test_procfs.sh
    rmmod lab2_driver && test_driver.sh

---

## 5. Điểm nhấn Kỹ thuật & Phân tích

* **An toàn bộ nhớ (`copy_to_user` / `copy_from_user`):** Kiểm tra quyền truy cập vùng nhớ User Space (`access_ok`) và xử lý ngoại lệ trang nhớ (Page Fault), ngăn ngừa lỗi sập nhân (**Kernel Panic**) so với việc dùng `memcpy()` thuần túy.
* **Cơ chế khóa `Mutex` (`DEFINE_MUTEX`):** Ngăn chặn xung đột dữ liệu (**Race Condition**) khi nhiều tiến trình cùng mở và ghi vào `/dev/lab2` đồng thời.
* **So sánh `procfs` và `sysfs`:** `/proc/lab2_info` sử dụng `seq_file` để xuất bảng tổng hợp trạng thái nhiều dòng, trong khi `/sys/class/lab2_class/lab2/` tuân thủ chuẩn mỗi thuộc tính một file độc lập gắn với đối tượng thiết bị `kobject`.
* **Đặc tính Flash NAND & `JFFS2`:** Bộ nhớ NAND chỉ cho phép xóa theo khối (`erase block`) trước khi ghi trang mới. `JFFS2` giải quyết bằng kiến trúc ghi nhật ký tuần tự (*log-structured*) kèm cơ chế cân bằng hao mòn (**Wear Leveling**).

---

## 6. Thông tin tác giả
* **Họ và tên:** Phạm Đặng Khánh Duy
* **Môn học:** Hệ thống Linux Nhúng — Embedded Linux Systems (LOS201)
* **Giấy phép:** MIT License ([license.md](license.md))
