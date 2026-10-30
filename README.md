<div align="center">

# Embedded Linux Systems — LAB 02
### Character Device Driver, Virtual Filesystems (procfs/sysfs), MTD/JFFS2 & BusyBox Init

![Architecture](https://img.shields.io/badge/ARCHITECTURE-ARMv7%20Cortex--A9-007ec6?style=for-the-badge&logo=arm&logoColor=white)
![Linux](https://img.shields.io/badge/LINUX%20KERNEL-5.15%20LTS-fe7d37?style=for-the-badge&logo=linux&logoColor=white)
![QEMU](https://img.shields.io/badge/QEMU-vexpress--a9-97ca00?style=for-the-badge&logo=qemu&logoColor=white)
![Driver](https://img.shields.io/badge/CHAR%20DEVICE-MAJOR%20240-8a2be2?style=for-the-badge)

![procfs/sysfs](https://img.shields.io/badge/PROCFS%20%26%20SYSFS-VALIDATED-e05d44?style=for-the-badge)
![JFFS2/MTD](https://img.shields.io/badge/MTD%20%2F%20JFFS2-PERSISTENCE%20TESTED-dfb317?style=for-the-badge)
![BusyBox](https://img.shields.io/badge/BUSYBOX%20INIT-AUTO--LOADED-2ea44f?style=for-the-badge)

</div>

---

## 1. Giới thiệu tổng quan (Overview)

Dự án này hiện thực hóa toàn bộ nội dung **LAB-02: Viết Device Driver & Quản lý File System** trên nền tảng nhúng **ARMv7 Cortex-A9 (QEMU vexpress-a9)**. Hệ thống bao gồm một Character Device Driver hoàn chỉnh trong không gian nhân (Kernel Space), giao tiếp hai chiều an toàn với không gian người dùng (User Space), xuất thông tin chẩn đoán thời gian thực qua procfs và sysfs, đồng thời mô phỏng bộ nhớ Flash NAND với hệ thống tệp JFFS2.

| Phân hệ | Nội dung triển khai | Trạng thái |
| :--- | :--- | :---: |
| **Phần A — Character Device Driver** | Xây dựng lab2_driver.ko (Major 240), hỗ trợ open, read, write, release, đồng bộ hóa bằng mutex và truyền dữ liệu qua copy_to_user() / copy_from_user(). | Đạt |
| **Phần B — Giao tiếp procfs & sysfs** | Tạo /proc/lab2_info (dùng seq_file API) và nhóm thuộc tính /sys/class/lab2_class/lab2/{buffer_len,open_count,last_data}. | Đạt |
| **Phần C — MTD Subsystem & JFFS2** | Giả lập NAND Flash 32MB qua nandsim, xóa block bằng flash_erase, đóng gói mkfs.jffs2, ghi nandwrite và kiểm chứng tính bền vững dữ liệu (persistence). | Đạt |
| **Phần D — Tích hợp BusyBox Init** | Cấu hình /etc/inittab và /etc/init.d/rcS để tự động nạp module lab2_driver.ko và khởi tạo node /dev/lab2 ngay khi hệ thống boot. | Đạt |

---

## 2. Kiến trúc hệ thống (System Architecture)

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

## 3. Cấu trúc thư mục (Repository Structure)

    embedded-linux-lab2/
    ├── driver/
    │   ├── lab2_driver.c                  # Mã nguồn C của Character Device Driver
    │   ├── Makefile                       # KBuild Makefile biên dịch chéo cho ARMv7
    │   └── lab2_driver.ko                 # Kernel module binary (ELF 32-bit ARM, 5.15.0)
    ├── logs/
    │   ├── boot_log.txt                   # Log khởi động QEMU & tự động nạp driver
    │   ├── driver_test.txt                # Kết quả thực thi kịch bản test_driver.sh
    │   ├── procfs_test.txt                # Kết quả kiểm thử giao diện /proc và /sys
    │   └── mtd_jffs2.txt                  # Log thao tác MTD, JFFS2 & kiểm tra persistence
    ├── output/
    │   └── initramfs_lab2.cpio.gz         # Image rootfs đã đóng gói sẵn sàng boot trên QEMU
    ├── rootfs/
    │   └── initramfs/                     # Cây thư mục gốc BusyBox
    │       ├── etc/inittab                # Cấu hình tiến trình khởi tạo BusyBox init
    │       ├── etc/init.d/rcS             # Script khởi động hệ thống & auto-load module
    │       ├── lib/modules/5.15.0/        # Chứa module lab2_driver.ko
    │       └── usr/bin/                   # Chứa test_driver.sh và test_procfs.sh
    ├── .gitignore                         # Danh sách loại trừ các file rác khi biên dịch
    ├── README.md                          # Tài liệu hướng dẫn và mô tả dự án
    └── license.md                         # Giấy phép mã nguồn mở MIT

---

## 4. Hướng dẫn Biên dịch & Chạy thử nghiệm (Build & Execution)

### 4.1. Biên dịch Kernel Module
    cd driver
    make clean && make
    file lab2_driver.ko

### 4.2. Đóng gói lại initramfs
    cd rootfs/initramfs
    find . | cpio -H newc -o | gzip > ../../output/initramfs_lab2.cpio.gz

### 4.3. Khởi chạy hệ thống trên QEMU
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

### 4.4. Chạy các kịch bản kiểm thử trong QEMU Shell
    lsmod
    ls -la /dev/lab2
    test_procfs.sh
    rmmod lab2_driver && test_driver.sh

---

## 5. Tóm tắt Phân tích Kỹ thuật (Technical Insights)

| Chủ đề | Điểm cốt lõi |
| :--- | :--- |
| **copy_to/from_user vs memcpy** | Không gian địa chỉ User Space và Kernel Space tách biệt nhau. copy_to/from_user() kiểm tra tính hợp lệ của con trỏ người dùng (access_ok) và xử lý an toàn lỗi trang bộ nhớ (Page Fault) mà không gây Kernel Oops/Panic như memcpy(). |
| **Đồng bộ hóa với Mutex** | Bảo vệ vùng nhớ dùng chung device_buffer và buffer_len khi nhiều tiến trình gọi write() đồng thời, ngăn chặn tình trạng Race Condition làm hỏng hoặc trộn lẫn dữ liệu. |
| **procfs vs sysfs** | /proc phù hợp xuất báo cáo tổng hợp dạng văn bản (qua seq_file), trong khi /sys tuân thủ nghiêm ngặt quy tắc một giá trị trên mỗi tệp (one-value-per-file) gắn liền với mô hình phân cấp thiết bị kobject. |
| **NAND Flash & JFFS2** | Flash NAND không cho phép ghi đè bit 0 thành 1 trực tiếp mà phải xóa theo từng khối (erase block). JFFS2 sử dụng cấu trúc ghi nhật ký tuần tự (log-structured) để gom dữ liệu mới vào vùng trống, kết hợp Wear Leveling bảo vệ tuổi thọ Flash. |

---

## 6. Tác giả (Author)
* **Họ và tên:** Phạm Đặng Khánh Duy
* **Môn học:** Embedded Linux Systems (LOS201)
* **Giấy phép:** MIT License ([license.md](license.md))
