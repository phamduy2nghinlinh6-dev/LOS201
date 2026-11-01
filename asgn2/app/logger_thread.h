/*
 * File: logger_thread.h
 * Purpose: Asynchronous Logger Thread interface with 1MB rotation & 5s periodic flush
 * Author: Pham Dang Khanh Duy
 * Date: 2026-11-01
 */
#ifndef LOGGER_THREAD_H
#define LOGGER_THREAD_H

int   logger_init(void);
void  logger_destroy(void);
void  logger_enqueue(const char *fmt, ...);
void  logger_wake(void);
void *logger_thread_func(void *arg);

#endif /* LOGGER_THREAD_H */
