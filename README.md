<div align="center">

# Embedded Linux Systems (LOS201) — Complete Lab Repository
### ARMv7 Cortex-A9 (QEMU vexpress-a9) | Linux Kernel 5.15 LTS | U-Boot | BusyBox

![Architecture](https://img.shields.io/badge/ARCHITECTURE-ARMv7%20Cortex--A9-007ec6?style=for-the-badge&logo=arm&logoColor=white)
![Kernel](https://img.shields.io/badge/LINUX%20KERNEL-5.15%20LTS-fe7d37?style=for-the-badge&logo=linux&logoColor=white)
![QEMU](https://img.shields.io/badge/EMULATOR-QEMU%20vexpress--a9-97ca00?style=for-the-badge&logo=qemu&logoColor=white)
![Status](https://img.shields.io/badge/LABS%2001%20--%2003-COMPLETED-2ea44f?style=for-the-badge)

</div>

---

## 📌 Course Lab Directory Index

| Lab Folder | Title & Core Topics | Documentation | Status |
| :--- | :--- | :---: | :---: |
| 📂 **[`lab1/`](lab1/)** | **LAB 01 — Embedded Linux System Construction**<br>Cross-Toolchain, U-Boot Bootloader, Linux 5.15 Kernel, Device Tree & BusyBox `initramfs`. | [English](lab1/README.md) \| [Tiếng Việt](lab1/README.vi.md) | ✅ Completed |
| 📂 **[`lab2/`](lab2/)** | **LAB 02 — Device Driver & File System Management**<br>Character Device Driver (`/dev/lab2`), `procfs`/`sysfs`, `MTD`/`JFFS2` Flash & BusyBox `init`. | [English](lab2/README.md) \| [Tiếng Việt](lab2/README.vi.md) | ✅ Completed |
| 📂 **[`lab3/`](lab3/)** | **LAB 03 — Kernel Debugging & Performance Profiling**<br>Remote `GDB`/`gdbserver`, `Valgrind` Memcheck, `strace`/`perf` Profiling & `cyclictest` RT Latency. | [English](lab3/README.md) \| [Tiếng Việt](lab3/README.vi.md) | ✅ Completed |

---

## 🗂️ Repository Hierarchy

    LOS201/
    ├── lab1/                              # LAB 01: Bootloader, Kernel & RootFS
    │   ├── configs/
    │   ├── output/
    │   ├── rootfs/
    │   ├── README.md                      # English documentation
    │   └── README.vi.md                   # Tài liệu Tiếng Việt
    ├── lab2/                              # LAB 02: Character Device Driver & JFFS2
    │   ├── driver/
    │   ├── logs/
    │   ├── output/
    │   ├── rootfs/
    │   ├── README.md                      # English documentation
    │   └── README.vi.md                   # Tài liệu Tiếng Việt
    ├── lab3/                              # LAB 03: GDB, Valgrind, perf & cyclictest
    │   ├── apps/
    │   ├── logs/
    │   ├── output/
    │   ├── rootfs/
    │   ├── README.md                      # English documentation
    │   └── README.vi.md                   # Tài liệu Tiếng Việt
    └── README.md                          # Master Repository Index

---

## 👤 Author
* **Student Name:** Phạm Đặng Khánh Duy
* **Course:** Embedded Linux Systems (`LOS201`)
