<div align="center">

# Hệ thống Linux Nhúng — LAB 03
### Gỡ lỗi Nhân, Phân tích Bộ nhớ, Profiling Hiệu năng & Độ trễ Thời gian thực

[![English](https://img.shields.io/badge/Language-English-007ec6?style=flat-square)](README.md)
[![Vietnamese](https://img.shields.io/badge/Ngôn%20ngữ-Tiếng%20Việt%20(Hiện%20tại)-e05d44?style=flat-square)](README.vi.md)

<br>

![Architecture](https://img.shields.io/badge/KIẾN%20TRÚC-ARMv7%20Cortex--A9-007ec6?style=for-the-badge&logo=arm&logoColor=white)
![GDB](https://img.shields.io/badge/GDB%20REMOTE-GỠ%20LỖI%20TỪ%20XA-8a2be2?style=for-the-badge&logo=gnu&logoColor=white)
![Valgrind](https://img.shields.io/badge/VALGRIND-ĐÃ%20XỬ%20LÝ%200%20LEAK-2ea44f?style=for-the-badge)
![Profiling](https://img.shields.io/badge/STRACE%20%26%20PERF-PHÂN%20TÍCH%20CPU-fe7d37?style=for-the-badge&logo=linux&logoColor=white)
![RealTime](https://img.shields.io/badge/CYCLICTEST-ĐỘ%20TRỄ%20THỜI%20GIAN%20THỰC-dfb317?style=for-the-badge)

</div>

---

## 1. Giới thiệu tổng quan

Thư mục này chứa toàn bộ mã nguồn, tệp thực thi, nhật ký phân tích và biểu đồ của bài thực hành **LAB-03: Debugging Kernel & Phân tích Hiệu năng Ứng dụng Nhúng** trên nền tảng **ARMv7 Cortex-A9 (QEMU vexpress-a9)** và máy **Host Ubuntu x86_64**.

| Phần thực hành | Nội dung & Công cụ triển khai | Trạng thái |
| :--- | :--- | :---: |
| **Phần A — Gỡ lỗi từ xa với GDB** | Kết nối `gdb-multiarch` (Host) với `gdbserver` (QEMU ARM) qua cổng TCP `1234` để phát hiện 3 lỗi trong `buggy_app`: tràn mảng, use-after-free và tràn số nguyên. | Hoàn thành |
| **Phần B — Phân tích Rò rỉ Bộ nhớ** | Sử dụng `Valgrind (memcheck)` truy vết rò rỉ bộ nhớ Heap (`definitely lost` & `indirectly lost`) trong `leaky_app` và xác nhận `0 errors` sau khi sửa ở `leaky_app_fixed`. | Hoàn thành |
| **Phần C — Profiling với strace & perf** | Thống kê lời gọi hệ thống bằng `strace -c` và xác định điểm nghẽn CPU (`heavy_compute`) bằng bộ đếm phần cứng thông qua `perf record` / `perf stat`. | Hoàn thành |
| **Phần D — Phân tích Độ trễ Real-Time** | Đo độ trễ đánh thức tối đa (worst-case latency) bằng `cyclictest` (`SCHED_FIFO`) dưới tải nặng và trực quan hóa phân phối độ trễ bằng biểu đồ `gnuplot`. | Hoàn thành |

---

## 2. Kiến trúc Luồng Gỡ lỗi & Phân tích Hiệu năng

    +--------------------------------------------------------------------------+
    |                         MÁY TRẠM HOST (x86_64 Ubuntu)                    |
    |                                                                          |
    |  [ gdb-multiarch ]   [ Valgrind Memcheck ]   [ strace / perf ]           |
    |          |                     |                     |                   |
    |          | TCP :1234           v                     v                   |
    |          |              ./leaky_app_fixed      ./perf_target             |
    |          |                                                               |
    |  [ cyclictest (SCHED_FIFO) ] ---> hist_standard.txt ---> [ gnuplot PNG ] |
    +----------+---------------------------------------------------------------+
               |
               | -net user,hostfwd=tcp::1234-:1234
               v
    +--------------------------------------------------------------------------+
    |                  HỆ THỐNG MỤC TIÊU (ARMv7 QEMU vexpress-a9)              |
    |                                                                          |
    |  [ gdbserver :1234 /usr/bin/buggy_app ]                                  |
    |  [ leaky_app_arm -> /proc/PID/status & /proc/PID/maps ]                  |
    |  [ strace -c /usr/bin/perf_target_arm ]                                  |
    +--------------------------------------------------------------------------+

---

## 3. Cấu trúc thư mục

    lab3/
    ├── apps/
    │   ├── buggy_app.c                    # Mã nguồn chứa 3 lỗi cố ý (Phần A)
    │   ├── buggy_app                      # Binary ARMv7 biên dịch với cờ -g -O0
    │   ├── leaky_app.c                    # Mã nguồn phân tích rò rỉ bộ nhớ (Phần B)
    │   ├── leaky_app                      # Binary x86_64 khi chưa sửa lỗi leak
    │   ├── leaky_app_fixed                # Binary x86_64 sau khi giải phóng đủ bộ nhớ
    │   └── perf_target.c                  # Mã nguồn kiểm thử hiệu năng CPU & I/O (Phần C)
    ├── logs/
    │   ├── gdb_session.txt                # Nhật ký phiên gỡ lỗi từ xa qua GDB
    │   ├── valgrind_report.txt            # Báo cáo Valgrind chi tiết trước khi sửa lỗi
    │   ├── valgrind_fixed.txt             # Báo cáo Valgrind xác nhận 0 lỗi sau khi sửa
    │   ├── strace_summary.txt             # Bảng thống kê tần suất & thời gian System Call
    │   ├── perf_report.txt                # Báo cáo phân tích hàm tiêu tốn CPU (Hotspot)
    │   ├── perf_stat.txt                  # Thống kê bộ đếm hiệu năng phần cứng CPU
    │   ├── hist_standard.txt              # Dữ liệu phân phối độ trễ từ cyclictest
    │   └── latency_histogram.png          # Biểu đồ Histogram độ trễ vẽ bằng gnuplot
    ├── output/
    │   └── initramfs_lab3.cpio.gz         # Image rootfs tích hợp công cụ debug cho QEMU
    ├── rootfs/
    │   └── initramfs/                     # Cây thư mục gốc BusyBox của LAB-03
    ├── .gitignore                         # Loại trừ các file dữ liệu thô dung lượng lớn
    ├── README.md                          # Tài liệu dự án (Tiếng Anh)
    ├── README.vi.md                       # Tài liệu dự án (Tiếng Việt)
    └── license.md                         # Giấy phép mã nguồn mở MIT

---

## 4. Tóm tắt Phân tích Kỹ thuật

* **Cơ chế GDB Remote Debugging (RSP):** Sử dụng giao thức *GDB Remote Serial Protocol* qua TCP. `gdbserver` trên target ARM điều khiển tiến trình con qua `ptrace()`, trong khi `gdb-multiarch` trên host giải mã bảng ký hiệu DWARF (`-g`) và khung ngăn xếp ARMv7.
* **Phân biệt `definitely lost` và `indirectly lost`:** Trong `delete_record()`, khi giải phóng cấu trúc cha `free(r)` mà quên giải phóng hai con trỏ thành viên `r->name` và `r->scores`, con trỏ gốc trỏ trực tiếp tới chúng bị mất nên Valgrind xếp vào nhóm **definitely lost** (cần giải phóng từ trong ra ngoài trước khi `free(r)`).
* **Tối ưu hóa điểm nghẽn CPU (`heavy_compute`):** Có thể tăng tốc bằng tập lệnh vector **ARM NEON SIMD** (`-mfpu=neon-vfpv3`), kết hợp **Loop Unrolling** và tiền tính toán bảng tra (Lookup Table) để giảm số chu kỳ gọi hàm toán học `sqrt()` và `log()`.
* **Độ trễ Thời gian thực (`PREEMPT_RT` vs Standard Kernel):** Nhân tiêu chuẩn có các vùng không thể bị chiếm quyền (Hard IRQ handlers, `spinlock_t`). Bản vá `PREEMPT_RT` chuyển các trình xử lý ngắt thành luồng nhân ưu tiên (`irq/XX-*`) và thay `spinlock` bằng `rt_mutex` có kế thừa độ ưu tiên (*Priority Inheritance*), giúp triệt tiêu các đỉnh trễ đột biến (Max Latency spikes).

---

## 5. Tác giả
* **Họ và tên:** Phạm Đặng Khánh Duy
* **Môn học:** Hệ thống Linux Nhúng — Embedded Linux Systems (LOS201)
* **Giấy phép:** MIT License ([license.md](license.md))
