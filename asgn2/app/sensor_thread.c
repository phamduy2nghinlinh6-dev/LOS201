/*
 * File: sensor_thread.c
 * Purpose: Periodic Sensor Thread using clock_nanosleep(TIMER_ABSTIME) and SCHED_FIFO
 * Author: Pham Dang Khanh Duy
 * Date: 2026-11-01
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sched.h>
#include <sys/ioctl.h>
#include "sensor_thread.h"
#include "logger_thread.h"

#define MAX_JITTER_SAMPLES 5000

static long jitter_samples[MAX_JITTER_SAMPLES];
static int  jitter_count = 0;

static void timespec_add_ms(struct timespec *ts, int ms)
{
    ts->tv_sec  += ms / 1000;
    ts->tv_nsec += (long)(ms % 1000) * 1000000L;
    if (ts->tv_nsec >= 1000000000L) {
        ts->tv_sec  += 1;
        ts->tv_nsec -= 1000000000L;
    }
}

static long timespec_diff_us(const struct timespec *actual, const struct timespec *expected)
{
    long diff_sec  = (long)(actual->tv_sec - expected->tv_sec);
    long diff_nsec = actual->tv_nsec - expected->tv_nsec;
    long us = diff_sec * 1000000L + diff_nsec / 1000L;
    return (us < 0) ? 0 : us;
}

static int cmp_long(const void *a, const void *b)
{
    long la = *(const long *)a;
    long lb = *(const long *)b;
    return (la > lb) - (la < lb);
}

void sensor_print_jitter_summary(FILE *out, int is_fifo)
{
    if (jitter_count == 0) {
        fprintf(out, "No jitter samples collected.\n");
        return;
    }

    qsort(jitter_samples, jitter_count, sizeof(long), cmp_long);

    long min_us = jitter_samples[0];
    long max_us = jitter_samples[jitter_count - 1];
    long p99_us = jitter_samples[(int)(jitter_count * 0.99)];
    long long sum = 0;
    int b_100 = 0, b_500 = 0, b_1000 = 0, b_over = 0;

    for (int i = 0; i < jitter_count; i++) {
        long v = jitter_samples[i];
        sum += v;
        if (v <= 100)       b_100++;
        else if (v <= 500)  b_500++;
        else if (v <= 1000) b_1000++;
        else                b_over++;
    }
    double avg_us = (double)sum / jitter_count;

    fprintf(out, "=== Jitter Analysis (%s, samples=%d) ===\n",
            is_fifo ? "SCHED_FIFO (Priority 50)" : "SCHED_OTHER (Standard)",
            jitter_count);
    fprintf(out, "  Min Latency : %ld us\n", min_us);
    fprintf(out, "  Avg Latency : %.2f us\n", avg_us);
    fprintf(out, "  P99 Latency : %ld us\n", p99_us);
    fprintf(out, "  Max Latency : %ld us\n", max_us);
    fprintf(out, "  Text Histogram:\n");
    fprintf(out, "    0 - 100 us    : %d samples\n", b_100);
    fprintf(out, "    100 - 500 us  : %d samples\n", b_500);
    fprintf(out, "    500 - 1000 us : %d samples\n", b_1000);
    fprintf(out, "    > 1000 us     : %d samples\n\n", b_over);
}

void *sensor_thread_func(void *arg)
{
    SensorContext *ctx = (SensorContext *)arg;
    int fd = open(ctx->cfg->device_path, O_RDONLY);
    int host_sim_mode = 0;

    if (fd < 0) {
        /* If running on x86_64 host for Valgrind/perf where /dev/sms_sensor is absent */
        host_sim_mode = 1;
        logger_enqueue("[WARN] Cannot open %s (%s) -> falling back to user-space simulation",
                       ctx->cfg->device_path, strerror(errno));
    } else {
        int interval = ctx->cfg->interval_ms;
        if (ioctl(fd, SMS_SET_INTERVAL, &interval) < 0) {
            logger_enqueue("[WARN] ioctl SMS_SET_INTERVAL failed: %s", strerror(errno));
        }
    }

    if (ctx->cfg->use_fifo) {
        struct sched_param sp;
        sp.sched_priority = 50;
        if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp) == 0) {
            logger_enqueue("[RT] Sensor Thread configured with SCHED_FIFO priority 50");
        } else {
            logger_enqueue("[RT] SCHED_FIFO not permitted (%s), running SCHED_OTHER", strerror(errno));
        }
    }

    struct timespec expected, actual;
    clock_gettime(CLOCK_MONOTONIC, &expected);

    while (*(ctx->running)) {
        timespec_add_ms(&expected, ctx->cfg->interval_ms);
        while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &expected, NULL) == EINTR) {
            if (!(*(ctx->running)))
                break;
        }
        if (!(*(ctx->running)))
            break;

        clock_gettime(CLOCK_MONOTONIC, &actual);
        if (jitter_count < MAX_JITTER_SAMPLES) {
            jitter_samples[jitter_count++] = timespec_diff_us(&actual, &expected);
        }

        SensorData data;
        if (!host_sim_mode) {
            char buf[64];
            lseek(fd, 0, SEEK_SET);
            ssize_t n = read(fd, buf, sizeof(buf) - 1);
            if (n < 0) {
                if (errno == EAGAIN || errno == EINTR)
                    continue;
                logger_enqueue("[ERROR] Sensor read error: %s", strerror(errno));
                continue;
            }
            if (n == 0)
                continue;
            buf[n] = '\0';
            unsigned long long ts = 0;
            if (sscanf(buf, "%llu,%f,%f", &ts, &data.temperature, &data.humidity) != 3)
                continue;
            data.timestamp_ms = (uint64_t)ts;
        } else {
            data.timestamp_ms = (uint64_t)(actual.tv_sec * 1000ULL + actual.tv_nsec / 1000000ULL);
            data.temperature  = 15.0f + (float)(rand() % 301) / 10.0f;
            data.humidity     = 30.0f + (float)(rand() % 601) / 10.0f;
        }

        if (ctx->cfg->verbose) {
            printf("[SENSOR] ts=%llu ms | temp=%.1f C | humid=%.1f %%\n",
                   (unsigned long long)data.timestamp_ms,
                   data.temperature, data.humidity);
            fflush(stdout);
        }

        if (!cb_push(ctx->cb, &data)) {
            logger_enqueue("[WARN] Shared circular buffer full! Dropping sample ts=%llu",
                           (unsigned long long)data.timestamp_ms);
        }
    }

    if (fd >= 0)
        close(fd);

    return NULL;
}
