/*
 * File: circular_buffer.c
 * Purpose: Heap-allocated Thread-safe Circular Buffer with Mutex & Condvar
 * Author: Pham Dang Khanh Duy
 * Date: 2026-11-01
 */
#include <stdlib.h>
#include <string.h>
#include "circular_buffer.h"

int cb_init(CircularBuffer *cb, int capacity)
{
    memset(cb, 0, sizeof(*cb));
    cb->items = (SensorData *)malloc(sizeof(SensorData) * capacity);
    if (!cb->items)
        return -1;
    cb->capacity = capacity;
    pthread_mutex_init(&cb->mutex, NULL);
    pthread_cond_init(&cb->cond_not_empty, NULL);
    return 0;
}

void cb_destroy(CircularBuffer *cb)
{
    if (cb->items) {
        free(cb->items);
        cb->items = NULL;
    }
    pthread_mutex_destroy(&cb->mutex);
    pthread_cond_destroy(&cb->cond_not_empty);
}

bool cb_push(CircularBuffer *cb, const SensorData *data)
{
    pthread_mutex_lock(&cb->mutex);
    if (cb->current_size >= cb->capacity) {
        cb->overflow_count++;
        pthread_mutex_unlock(&cb->mutex);
        return false;
    }
    cb->items[cb->head] = *data;
    cb->head = (cb->head + 1) % cb->capacity;
    cb->current_size++;
    cb->total_produced++;
    pthread_cond_signal(&cb->cond_not_empty);
    pthread_mutex_unlock(&cb->mutex);
    return true;
}

bool cb_pop(CircularBuffer *cb, SensorData *out, volatile int *running)
{
    pthread_mutex_lock(&cb->mutex);
    while (cb->current_size == 0 && *running) {
        pthread_cond_wait(&cb->cond_not_empty, &cb->mutex);
    }
    if (cb->current_size == 0 && !(*running)) {
        pthread_mutex_unlock(&cb->mutex);
        return false;
    }
    *out = cb->items[cb->tail];
    cb->tail = (cb->tail + 1) % cb->capacity;
    cb->current_size--;
    cb->total_consumed++;
    pthread_mutex_unlock(&cb->mutex);
    return true;
}

void cb_wake_all(CircularBuffer *cb)
{
    pthread_mutex_lock(&cb->mutex);
    pthread_cond_broadcast(&cb->cond_not_empty);
    pthread_mutex_unlock(&cb->mutex);
}

void cb_get_stats(CircularBuffer *cb, int *cur_size, unsigned long *prod,
                  unsigned long *cons, unsigned long *ovf)
{
    pthread_mutex_lock(&cb->mutex);
    if (cur_size) *cur_size = cb->current_size;
    if (prod)     *prod     = cb->total_produced;
    if (cons)     *cons     = cb->total_consumed;
    if (ovf)      *ovf      = cb->overflow_count;
    pthread_mutex_unlock(&cb->mutex);
}
