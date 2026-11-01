/*
 * File: processor_thread.c
 * Purpose: Consumes SensorData, computes 10-sample rolling average, triggers alerts
 * Author: Pham Dang Khanh Duy
 * Date: 2026-11-01
 */
#include <stdio.h>
#include <math.h>
#include "processor_thread.h"
#include "alert_thread.h"
#include "logger_thread.h"

#define WINDOW_SIZE 10
#define SPIKE_DELTA 5.0f

void *processor_thread_func(void *arg)
{
    ProcessorContext *ctx = (ProcessorContext *)arg;
    float temp_win[WINDOW_SIZE] = {0};
    float hum_win[WINDOW_SIZE]  = {0};
    int win_count = 0, win_idx = 0;
    int has_prev = 0;
    float prev_temp = 0.0f;

    while (*(ctx->running)) {
        SensorData sample;
        if (!cb_pop(ctx->cb, &sample, ctx->running))
            break;

        temp_win[win_idx] = sample.temperature;
        hum_win[win_idx]  = sample.humidity;
        win_idx = (win_idx + 1) % WINDOW_SIZE;
        if (win_count < WINDOW_SIZE)
            win_count++;

        float sum_t = 0.0f, sum_h = 0.0f;
        for (int i = 0; i < win_count; i++) {
            sum_t += temp_win[i];
            sum_h += hum_win[i];
        }
        float avg_t = sum_t / win_count;
        float avg_h = sum_h / win_count;

        if (sample.temperature > ctx->cfg->temp_threshold) {
            AlertMsg al = { sample.timestamp_ms, ALERT_TEMP_HIGH,
                            sample.temperature, ctx->cfg->temp_threshold };
            alert_send(&al);
        }
        if (sample.humidity > ctx->cfg->humid_threshold) {
            AlertMsg al = { sample.timestamp_ms, ALERT_HUMID_HIGH,
                            sample.humidity, ctx->cfg->humid_threshold };
            alert_send(&al);
        }
        if (has_prev && fabsf(sample.temperature - prev_temp) > SPIKE_DELTA) {
            AlertMsg al = { sample.timestamp_ms, ALERT_TEMP_SPIKE,
                            fabsf(sample.temperature - prev_temp), SPIKE_DELTA };
            alert_send(&al);
        }
        prev_temp = sample.temperature;
        has_prev = 1;

        logger_enqueue("[SAMPLE][%llu] Temp=%.1fC (Avg10=%.1fC) Humid=%.1f%% (Avg10=%.1f%%)",
                       (unsigned long long)sample.timestamp_ms,
                       sample.temperature, avg_t,
                       sample.humidity, avg_h);
    }
    return NULL;
}
