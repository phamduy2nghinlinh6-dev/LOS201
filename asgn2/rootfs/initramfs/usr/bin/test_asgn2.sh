#!/bin/sh
echo "======================================================"
echo "=== ASSIGNMENT-02: SENSOR MONITORING SYSTEM TEST   ==="
echo "======================================================"

rmmod sms_sensor_driver 2>/dev/null
insmod /lib/modules/5.15.0/sms_sensor_driver.ko || insmod /lib/modules/5.15.0/sms_sensor_driver.ko
MINOR=$(awk '$2=="sms_sensor" {print $1}' /proc/misc)
rm -f /dev/sms_sensor
mknod /dev/sms_sensor c 10 ${MINOR:-0}
ls -la /dev/sms_sensor /proc/sms_stats

echo "--- [T=0s] /proc/sms_stats (Before Run) ---"
cat /proc/sms_stats

echo ""
echo "--- Starting sms_app in background (-i 500 -d 15 -v) ---"
sms_app -i 500 -d 15 -v &
APP_PID=$!

sleep 3
echo ""
echo "--- [T=3s] Memory Footprint (/proc/$APP_PID/status) ---"
grep -E "VmSize|VmRSS|Threads" /proc/$APP_PID/status 2>/dev/null

sleep 5
echo ""
echo "--- [T=8s] /proc/sms_stats (Mid Run) ---"
cat /proc/sms_stats
grep -E "VmSize|VmRSS|Threads" /proc/$APP_PID/status 2>/dev/null

wait $APP_PID

echo ""
echo "--- [T=15s] /proc/sms_stats (After Completion) ---"
cat /proc/sms_stats

echo ""
echo "--- Generated /tmp/sms.log ---"
cat /tmp/sms.log
echo "=== ASSIGNMENT-02 TEST COMPLETED SUCCESSFULLY ==="
