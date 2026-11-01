#!/bin/sh
echo "=================================================="
echo "=== ASSIGNMENT-01 TASK 2: MISC COUNTER TEST    ==="
echo "=================================================="

echo "[1] Loading counter_driver.ko..."
rmmod counter_driver 2>/dev/null
insmod /lib/modules/5.15.0/counter_driver.ko
if [ $? -ne 0 ]; then
    echo "ERROR: insmod counter_driver.ko failed!"
    exit 1
fi

MINOR=$(awk '$2=="counter" {print $1}' /proc/misc)
[ -c /dev/counter ] || mknod /dev/counter c 10 ${MINOR:-0}
ls -la /dev/counter

echo ""
echo "[2] /proc/counter_info (Snapshot 1 - Initial State):"
cat /proc/counter_info

echo ""
echo "[3] Reading /dev/counter 3 times (auto-increment test):"
echo "  Read 1: $(cat /dev/counter)"
echo "  Read 2: $(cat /dev/counter)"
echo "  Read 3: $(cat /dev/counter)"

echo ""
echo "[4] Writing '100' to /dev/counter..."
echo "100" > /dev/counter
echo "  Read after set 100: $(cat /dev/counter)"
echo "  Next read (101)   : $(cat /dev/counter)"

echo ""
echo "[5] /proc/counter_info (Snapshot 2 - After Reads & Write 100):"
cat /proc/counter_info

echo ""
echo "[6] Resetting counter to '0'..."
echo "0" > /dev/counter
echo "  Read after reset  : $(cat /dev/counter)"

echo ""
echo "[7] /proc/counter_info (Snapshot 3 - After Reset):"
cat /proc/counter_info

echo ""
echo "[8] Testing Invalid Writes (negative & non-numeric -> expect -EINVAL)..."
echo "-50" > /dev/counter 2>&1 || echo "  -> Negative number correctly rejected (-EINVAL)"
echo "abc" > /dev/counter 2>&1 || echo "  -> Non-numeric string correctly rejected (-EINVAL)"

echo ""
echo "[9] Unloading counter_driver..."
rmmod counter_driver
dmesg | grep counter_driver | tail -n 15
echo "=== TASK 2 TEST COMPLETED SUCCESSFULLY ==="
