/*
 * File: alert_thread.c
 * Purpose: Alert Thread processing alarms via POSIX Message Queue (or condvar fallback)
 * Author: Pham Dang Khanh Duy
 * Date: 2026-11-01
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include "alert_thread.h"
#include "logger_thread.h"

static mqd_t alarm_mq = (mqd_t)-1;
static int use_posix_mq = 0;

/* Fallback ring buffer if target kernel lacks CONFIG_POSIX_MQUEUE */
#define FB_ALARM_CAP 64
static AlertMsg fb_queue[FB_ALARM_CAP];
static int fb_head = 0, fb_tail = 0, fb_size = 0;
static pthread_mutex_t fb_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  fb_cond  = PTHREAD_COND_INITIALIZER;

static unsigned long count_temp_high  = 0;
static unsigned long count_humid_high = 0;
static unsigned long count_temp_spike = 0;

int alert_queue_init(void)
{
    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = sizeof(AlertMsg);

    mq_unlink(ALARM_MQ_NAME);
    alarm_mq = mq_open(ALARM_MQ_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (alarm_mq != (mqd_t)-1) {
        use_posix_mq = 1;
    } else {
        use_posix_mq = 0;
    }
    return 0;
}

void alert_queue_destroy(void)
{
    if (use_posix_mq && alarm_mq != (mqd_t)-1) {
        mq_close(alarm_mq);
        mq_unlink(ALARM_MQ_NAME);
        alarm_mq = (mqd_t)-1;
    }
}

void alert_send(const AlertMsg *msg)
{
    if (use_posix_mq) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;
        mq_timedsend(alarm_mq, (const char *)msg, sizeof(AlertMsg), 1, &ts);
    } else {
        pthread_mutex_lock(&fb_mutex);
        if (fb_size < FB_ALARM_CAP) {
            fb_queue[fb_head] = *msg;
            fb_head = (fb_head + 1) % FB_ALARM_CAP;
            fb_size++;
            pthread_cond_signal(&fb_cond);
        }
        pthread_mutex_unlock(&fb_mutex);
    }
}

void alert_wake(void)
{
    if (!use_posix_mq) {
        pthread_mutex_lock(&fb_mutex);
        pthread_cond_broadcast(&fb_cond);
        pthread_mutex_unlock(&fb_mutex);
    }
}

void alert_get_counts(unsigned long *temp_hi, unsigned long *hum_hi, unsigned long *spike)
{
    if (temp_hi) *temp_hi = count_temp_high;
    if (hum_hi)  *hum_hi  = count_humid_high;
    if (spike)   *spike   = count_temp_spike;
}

static const char *alert_type_str(AlertType t)
{
    switch (t) {
    case ALERT_TEMP_HIGH:  return "TEMP_HIGH";
    case ALERT_HUMID_HIGH: return "HUMID_HIGH";
    case ALERT_TEMP_SPIKE: return "TEMP_SPIKE";
    default:               return "UNKNOWN";
    }
}

void *alert_thread_func(void *arg)
{
    volatile int *running = (volatile int *)arg;

    while (*running) {
        AlertMsg msg;
        int got_msg = 0;

        if (use_posix_mq) {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 1;
            ssize_t n = mq_timedreceive(alarm_mq, (char *)&msg, sizeof(AlertMsg), NULL, &ts);
            if (n == (ssize_t)sizeof(AlertMsg))
                got_msg = 1;
        } else {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 1;
            pthread_mutex_lock(&fb_mutex);
            if (fb_size == 0 && *running)
                pthread_cond_timedwait(&fb_cond, &fb_mutex, &ts);
            if (fb_size > 0) {
                msg = fb_queue[fb_tail];
                fb_tail = (fb_tail + 1) % FB_ALARM_CAP;
                fb_size--;
                got_msg = 1;
            }
            pthread_mutex_unlock(&fb_mutex);
        }

        if (got_msg) {
            if (msg.type == ALERT_TEMP_HIGH)       count_temp_high++;
            else if (msg.type == ALERT_HUMID_HIGH) count_humid_high++;
            else if (msg.type == ALERT_TEMP_SPIKE) count_temp_spike++;

            fprintf(stderr, "[ALERT][%llu] TYPE: %s VALUE: %.1f THRESHOLD: %.1f\n",
                    (unsigned long long)msg.timestamp_ms,
                    alert_type_str(msg.type),
                    msg.value, msg.threshold);

            logger_enqueue("[ALERT][%llu] TYPE: %s VALUE: %.1f THRESHOLD: %.1f",
                           (unsigned long long)msg.timestamp_ms,
                           alert_type_str(msg.type),
                           msg.value, msg.threshold);
        }
    }
    return NULL;
}
