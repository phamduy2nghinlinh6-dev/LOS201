/*
 * File: logger_thread.c
 * Purpose: Logger Thread implementation writing to /tmp/sms.log
 * Author: Pham Dang Khanh Duy
 * Date: 2026-11-01
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <pthread.h>
#include "circular_buffer.h"
#include "logger_thread.h"

#define LOG_QUEUE_CAP 128
#define LOG_LINE_MAX  256
#define MAX_LOG_BYTES (1024 * 1024) /* 1 MB rotation limit */

static char (*log_queue)[LOG_LINE_MAX] = NULL;
static int lq_head = 0, lq_tail = 0, lq_size = 0;
static pthread_mutex_t lq_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  lq_cond  = PTHREAD_COND_INITIALIZER;

int logger_init(void)
{
    log_queue = malloc(sizeof(char[LOG_LINE_MAX]) * LOG_QUEUE_CAP);
    if (!log_queue)
        return -1;
    lq_head = lq_tail = lq_size = 0;
    return 0;
}

void logger_destroy(void)
{
    if (log_queue) {
        free(log_queue);
        log_queue = NULL;
    }
}

void logger_enqueue(const char *fmt, ...)
{
    char msg[LOG_LINE_MAX];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    pthread_mutex_lock(&lq_mutex);
    if (log_queue && lq_size < LOG_QUEUE_CAP) {
        strncpy(log_queue[lq_head], msg, LOG_LINE_MAX - 1);
        log_queue[lq_head][LOG_LINE_MAX - 1] = '\0';
        lq_head = (lq_head + 1) % LOG_QUEUE_CAP;
        lq_size++;
        pthread_cond_signal(&lq_cond);
    }
    pthread_mutex_unlock(&lq_mutex);
}

void logger_wake(void)
{
    pthread_mutex_lock(&lq_mutex);
    pthread_cond_broadcast(&lq_cond);
    pthread_mutex_unlock(&lq_mutex);
}

void *logger_thread_func(void *arg)
{
    volatile int *running = (volatile int *)arg;
    FILE *fp = fopen(LOG_FILE_PATH, "a");
    time_t last_flush = time(NULL);

    while (*running || lq_size > 0) {
        char line[LOG_LINE_MAX];
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        pthread_mutex_lock(&lq_mutex);
        while (lq_size == 0 && *running) {
            pthread_cond_timedwait(&lq_cond, &lq_mutex, &ts);
            break;
        }
        if (lq_size == 0) {
            pthread_mutex_unlock(&lq_mutex);
            if (fp && (time(NULL) - last_flush >= 5)) {
                fflush(fp);
                last_flush = time(NULL);
            }
            continue;
        }
        strncpy(line, log_queue[lq_tail], sizeof(line));
        lq_tail = (lq_tail + 1) % LOG_QUEUE_CAP;
        lq_size--;
        pthread_mutex_unlock(&lq_mutex);

        if (fp) {
            time_t now = time(NULL);
            struct tm tm_now;
            localtime_r(&now, &tm_now);
            fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
                    tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
                    tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec, line);

            if (time(NULL) - last_flush >= 5) {
                fflush(fp);
                last_flush = time(NULL);
            }

            struct stat st;
            if (stat(LOG_FILE_PATH, &st) == 0 && st.st_size > MAX_LOG_BYTES) {
                fclose(fp);
                rename(LOG_FILE_PATH, LOG_BACKUP_PATH);
                fp = fopen(LOG_FILE_PATH, "w");
            }
        }
    }

    if (fp) {
        fflush(fp);
        fclose(fp);
    }
    return NULL;
}
