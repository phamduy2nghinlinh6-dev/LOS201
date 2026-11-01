/*
 * File: circular_buffer.h
 * Purpose: Thread-safe Circular Buffer declarations and common SMS data structures
 * Author: Pham Dang Khanh Duy
 * Date: 2026-11-01
 */
#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include "../driver/sms_sensor.h"

#define DEFAULT_BUFFER_CAPACITY 64
#define LOG_FILE_PATH           "/tmp/sms.log"
#define LOG_BACKUP_PATH         "/tmp/sms.log.1"
#define ALARM_MQ_NAME           "/sms_alarm_mq"

typedef struct {
    uint64_t timestamp_ms;
    float temperature;
    float humidity;
} SensorData;

typedef enum {
    ALERT_TEMP_HIGH = 0,
    ALERT_HUMID_HIGH,
    ALERT_TEMP_SPIKE
} AlertType;

typedef struct {
    uint64_t timestamp_ms;
    AlertType type;
    float value;
    float threshold;
} AlertMsg;

typedef struct {
    int interval_ms;
    float temp_threshold;
    float humid_threshold;
    int duration_sec;
    int verbose;
    int use_fifo;
    const char *device_path;
    const char *jitter_out_path;
} AppConfig;

typedef struct {
    SensorData *items;
    int capacity;
    int head;
    int tail;
    int current_size;
    unsigned long total_produced;
    unsigned long total_consumed;
    unsigned long overflow_count;
    pthread_mutex_t mutex;
    pthread_cond_t cond_not_empty;
} CircularBuffer;

int  cb_init(CircularBuffer *cb, int capacity);
void cb_destroy(CircularBuffer *cb);
bool cb_push(CircularBuffer *cb, const SensorData *data);
bool cb_pop(CircularBuffer *cb, SensorData *out, volatile int *running);
void cb_wake_all(CircularBuffer *cb);
void cb_get_stats(CircularBuffer *cb, int *cur_size, unsigned long *prod,
                  unsigned long *cons, unsigned long *ovf);

#endif /* CIRCULAR_BUFFER_H */
