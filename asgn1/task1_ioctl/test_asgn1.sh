#!/bin/sh
echo "=================================================="
echo "=== ASSIGNMENT-01 TASK 1: IOCTL DRIVER TEST    ==="
echo "=================================================="

echo "[1] Loading asgn1_driver.ko..."
rmmod asgn1_driver 2>/dev/null
insmod /lib/modules/5.15.0/asgn1_driver.ko
if [ $? -ne 0 ]; then
    echo "ERROR: insmod asgn1_driver.ko failed!"
    exit 1
fi
[ -c /dev/asgn1 ] || mknod /dev/asgn1 c 241 0
ls -la /dev/asgn1

echo ""
echo "[2] Testing IOCTL 4: ASGN1_GET_VERSION..."
ioctl_helper version

echo ""
echo "[3] Testing Default Mode 1 (Echo Mode)..."
echo -n "Hello Echo Mode" > /dev/asgn1
echo "Read back: [$(cat /dev/asgn1)]"

echo ""
echo "[4] Testing IOCTL 3: ASGN1_SET_MODE -> Mode 0 (Kernel Mode)..."
ioctl_helper mode0
echo -n "Hello Kernel Mode" > /dev/asgn1
echo "Read back: [$(cat /dev/asgn1)]"

echo ""
echo "[5] Testing IOCTL 2: ASGN1_GET_STATS..."
ioctl_helper stats

echo ""
echo "[6] Testing IOCTL 1: ASGN1_RESET_BUFFER..."
ioctl_helper reset
echo "Read back after reset (should be empty): [$(cat /dev/asgn1)]"
ioctl_helper stats

echo ""
echo "[7] Testing Invalid IOCTL Command (-ENOTTY)..."
ioctl_helper invalid

echo ""
echo "[8] Unloading asgn1_driver..."
rmmod asgn1_driver
dmesg | grep asgn1_driver | tail -n 15
echo "=== TASK 1 TEST COMPLETED SUCCESSFULLY ==="
