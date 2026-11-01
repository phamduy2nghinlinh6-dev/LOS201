/*
 * File: sms_sensor.h
 * Purpose: Shared header for SMS Sensor Kernel Driver and Userspace Application
 * Author: Pham Dang Khanh Duy
 * Date: 2026-11-01
 */
#ifndef SMS_SENSOR_H
#define SMS_SENSOR_H

#ifdef __KERNEL__
#include <linux/ioctl.h>
#else
#include <sys/ioctl.h>
#endif

#define SMS_DEVICE_NAME  "sms_sensor"
#define SMS_DEVICE_PATH  "/dev/sms_sensor"
#define SMS_PROC_NAME    "sms_stats"
#define SMS_PROC_PATH    "/proc/sms_stats"
#define SMS_MAGIC        'S'

/* ioctl command to configure minimum sampling interval in ms (default: 100ms) */
#define SMS_SET_INTERVAL _IOW(SMS_MAGIC, 1, int)

#endif /* SMS_SENSOR_H */
