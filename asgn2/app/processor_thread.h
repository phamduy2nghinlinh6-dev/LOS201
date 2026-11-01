/*
 * File: processor_thread.h
 * Purpose: Processor Thread interface (10-sample rolling average & threshold check)
 * Author: Pham Dang Khanh Duy
 * Date: 2026-11-01
 */
#ifndef PROCESSOR_THREAD_H
#define PROCESSOR_THREAD_H

#include "circular_buffer.h"

typedef struct {
    CircularBuffer *cb;
    AppConfig *cfg;
    volatile int *running;
} ProcessorContext;

void *processor_thread_func(void *arg);

#endif /* PROCESSOR_THREAD_H */
