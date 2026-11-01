/*
 * File: sensor_thread.h
 * Purpose: Real-Time Sensor Thread reading /dev/sms_sensor and measuring wake-up jitter
 * Author: Pham Dang Khanh Duy
 * Date: 2026-11-01
 */
#ifndef SENSOR_THREAD_H
#define SENSOR_THREAD_H

#include "circular_buffer.h"

typedef struct {
    CircularBuffer *cb;
    AppConfig *cfg;
    volatile int *running;
} SensorContext;

void *sensor_thread_func(void *arg);
void  sensor_print_jitter_summary(FILE *out, int is_fifo);

#endif /* SENSOR_THREAD_H */
