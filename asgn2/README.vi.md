<div align="center">

# Hệ thống Linux Nhúng — ĐỒ ÁN CUỐI KỲ (ASSIGNMENT 02)
### Hệ thống Giám sát Cảm biến Công nghiệp (Đa luồng, IPC, Kernel Driver & Thời gian thực)

[![English](https://img.shields.io/badge/Language-English-007ec6?style=flat-square)](README.md)
[![Vietnamese](https://img.shields.io/badge/Ngôn%20ngữ-Tiếng%20Việt%20(Hiện%20tại)-e05d44?style=flat-square)](README.vi.md)

<br>

![Architecture](https://img.shields.io/badge/KIẾN%20TRÚC-ARMv7%20Cortex--A9-007ec6?style=for-the-badge&logo=arm&logoColor=white)
![KernelDriver](https://img.shields.io/badge/KERNEL%20DRIVER-sms__sensor.ko-8a2be2?style=for-the-badge&logo=linux&logoColor=white)
![MultiThread](https://img.shields.io/badge/POSIX%20THREADS-4%20LUỒNG%20SONG%20SONG-fe7d37?style=for-the-badge)
![Valgrind](https://img.shields.io/badge/VALGRIND-0%20BYTES%20RÒ%20RỈ-2ea44f?style=for-the-badge)
![RealTime](https://img.shields.io/badge/THỜI%20GIAN%20THỰC-SCHED__FIFO%20JITTER-dfb317?style=for-the-badge)

</div>

---

## 1. Giới thiệu tổng quan

Thư mục này chứa toàn bộ mã nguồn, tệp thực thi và báo cáo phân tích thực nghiệm của đồ án cuối kỳ **ASSIGNMENT-02: Write Embedded Application – Sensor Monitoring System (SMS)** trên nền tảng **ARMv7 Cortex-A9 (QEMU vexpress-a9)** và **Ubuntu Host x86_64**.

| Thành phần | Phân loại | Chức năng & Cơ chế Kỹ thuật | Trạng thái |
| :--- | :--- | :--- | :---: |
| **`sms_sensor_driver.ko`** | Kernel Module | Cung cấp `/dev/sms_sensor` (xuất chuỗi CSV `<ts_ms>,<temp>,<humid>`), thống kê `/proc/sms_stats`, và lệnh `ioctl SMS_SET_INTERVAL`. | Hoàn thành |
| **Sensor Thread** | Luồng `SCHED_FIFO` | Đọc `/dev/sms_sensor` theo chu kỳ chính xác bằng `clock_nanosleep(TIMER_ABSTIME)`, đo độ trễ Jitter, đẩy mẫu vào `shared_buffer`. | Hoàn thành |
| **Processor Thread** | Luồng Xử lý | Lấy mẫu từ `shared_buffer`, tính trung bình trượt 10 mẫu (rolling average), phát cảnh báo (`TEMP_HIGH`, `HUMID_HIGH`, `TEMP_SPIKE`). | Hoàn thành |
| **Alert Thread** | Luồng Cảnh báo | Nhận cảnh báo qua hàng đợi `alarm_queue` (POSIX MQ), in ra `stderr`, thống kê tần suất từng loại lỗi và chuyển tiếp tới Logger Thread. | Hoàn thành |
| **Logger Thread** | Luồng Ghi nhật ký | Ghi log định kỳ ra `/tmp/sms.log` (flush mỗi 5 giây để tối ưu I/O) và tự động xoay vòng file (log rotation) khi vượt quá 1MB. | Hoàn thành |

---

## 2. Cấu trúc thư mục

    asgn2/
    ├── driver/
    │   ├── sms_sensor_driver.c            # Mã nguồn kernel module mô phỏng cảm biến
    │   ├── sms_sensor.h                   # Header định nghĩa ioctl và đường dẫn thiết bị
    │   ├── Makefile                       # KBuild Makefile biên dịch chéo cho ARMv7
    │   └── sms_sensor_driver.ko           # Kernel module binary (ARMv7)
    ├── app/
    │   ├── sms_app.c                      # Hàm main, phân tích tham số CLI & bắt SIGINT
    │   ├── circular_buffer.c/.h           # Bộ đệm vòng 64 phần tử trên Heap (Mutex + Condvar)
    │   ├── sensor_thread.c/.h             # Luồng thu thập dữ liệu thời gian thực & đo Jitter
    │   ├── processor_thread.c/.h          # Luồng tính trung bình trượt & phát hiện bất thường
    │   ├── alert_thread.c/.h              # Luồng nhận cảnh báo qua POSIX Message Queue
    │   ├── logger_thread.c/.h             # Luồng ghi nhật ký bất đồng bộ & xoay vòng 1MB
    │   ├── Makefile                       # Hỗ trợ cả `make arm` và `make host`
    │   ├── sms_app                        # File thực thi ARMv7 chạy trên QEMU
    │   └── sms_app_x86                    # File thực thi x86_64 phục vụ Valgrind/strace/perf
    ├── logs/
    │   ├── sms_sample_run.txt             # Log chạy thực nghiệm trên QEMU (kèm VmRSS)
    │   ├── sms.log                        # File nhật ký sinh ra bởi Logger Thread
    │   ├── valgrind_sms.txt               # Báo cáo Valgrind xác nhận 0 bytes definitely lost
    │   ├── strace_sms.txt                 # Thống kê lời gọi hệ thống (strace -f -c)
    │   ├── perf_sms.txt                   # Báo cáo phân tích điểm nghẽn CPU (perf)
    │   ├── gdb_sms.txt                    # Nhật ký gỡ lỗi đa luồng bằng GDB
    │   ├── proc_sms_stats_*.txt           # Thông số /proc/sms_stats tại 3 thời điểm
    │   └── jitter_report.txt              # Bảng so sánh độ trễ SCHED_FIFO và SCHED_OTHER
    ├── output/
    │   └── initramfs_asgn2.cpio.gz        # Image rootfs đóng gói hoàn chỉnh cho QEMU
    ├── README.md                          # Tài liệu dự án (Tiếng Anh)
    ├── README.vi.md                       # Tài liệu dự án (Tiếng Việt)
    └── license.md                         # Giấy phép MIT

---

## 3. Tác giả
* **Họ và tên:** Phạm Đặng Khánh Duy
* **Môn học:** Hệ thống Linux Nhúng — Embedded Linux Systems (LOS201)
