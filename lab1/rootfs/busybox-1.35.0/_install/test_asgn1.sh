#!/bin/sh
echo "=== ASGN1 AUTO TEST ==="

# 1. Load module và tạo device node
insmod /asgn1_driver.ko
mknod /dev/asgn1_dev c 240 0
chmod 666 /dev/asgn1_dev

# 2. Chạy ứng dụng C
./test_app

# 3. Kích xuất log của Kernel
echo "=== KERNEL LOG ==="
dmesg | tail -n 5

# 4. Dọn dẹp
rm /dev/asgn1_dev
rmmod asgn1_driver
echo "=== TEST COMPLETE ==="
