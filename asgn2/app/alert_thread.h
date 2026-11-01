/*
 * File: alert_thread.h
 * Purpose: Alert Thread & POSIX Message Queue alarm_queue interface
 * Author: Pham Dang Khanh Duy
 * Date: 2026-11-01
 */
#ifndef ALERT_THREAD_H
#define ALERT_THREAD_H

#include "circular_buffer.h"

int   alert_queue_init(void);
void  alert_queue_destroy(void);
void  alert_send(const AlertMsg *msg);
void  alert_wake(void);
void  alert_get_counts(unsigned long *temp_hi, unsigned long *hum_hi, unsigned long *spike);
void *alert_thread_func(void *arg);

#endif /* ALERT_THREAD_H */
