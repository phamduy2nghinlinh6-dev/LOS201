/*
 * File: sms_app.c
 * Purpose: Main entry point for Sensor Monitoring System (SMS) Capstone Application
 * Author: Pham Dang Khanh Duy
 * Date: 2026-11-01
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <pthread.h>
#include "circular_buffer.h"
#include "sensor_thread.h"
#include "processor_thread.h"
#include "alert_thread.h"
#include "logger_thread.h"

static volatile int g_running = 1;

static void sigint_handler(int signo)
{
    (void)signo;
    g_running = 0;
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [OPTIONS]\n", prog);
    printf("  -i <ms>    Sensor sampling interval (default: 500ms)\n");
    printf("  -t <C>     Temperature alert threshold (default: 40.0)\n");
    printf("  -h <%%>     Humidity alert threshold (default: 80.0)\n");
    printf("  -d <sec>   Run duration in seconds, 0 = infinite (default: 60)\n");
    printf("  -v         Verbose mode: print each sample to stdout\n");
    printf("  -o         Use SCHED_OTHER instead of SCHED_FIFO (for jitter comparison)\n");
    printf("  -j <file>  Append jitter summary report to file\n");
}

int main(int argc, char *argv[])
{
    AppConfig cfg = {
        .interval_ms     = 500,
        .temp_threshold  = 40.0f,
        .humid_threshold = 80.0f,
        .duration_sec    = 60,
        .verbose         = 0,
        .use_fifo        = 1,
        .device_path     = SMS_DEVICE_PATH,
        .jitter_out_path = NULL
    };

    int opt;
    while ((opt = getopt(argc, argv, "i:t:h:d:voj:")) != -1) {
        switch (opt) {
        case 'i': cfg.interval_ms     = atoi(optarg); break;
        case 't': cfg.temp_threshold  = strtof(optarg, NULL); break;
        case 'h': cfg.humid_threshold = strtof(optarg, NULL); break;
        case 'd': cfg.duration_sec    = atoi(optarg); break;
        case 'v': cfg.verbose         = 1; break;
        case 'o': cfg.use_fifo        = 0; break;
        case 'j': cfg.jitter_out_path = optarg; break;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    if (cfg.interval_ms <= 0 || cfg.duration_sec < 0) {
        fprintf(stderr, "Error: invalid interval or duration parameter.\n");
        return 1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Lock memory to prevent page faults in real-time thread */
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        /* Non-fatal if running without root or under Valgrind */
    }

    CircularBuffer cb;
    if (cb_init(&cb, DEFAULT_BUFFER_CAPACITY) != 0) {
        fprintf(stderr, "Error: failed to allocate circular buffer.\n");
        return 1;
    }
    if (logger_init() != 0) {
        cb_destroy(&cb);
        return 1;
    }
    alert_queue_init();

    printf("=== Sensor Monitoring System (SMS) Starting ===\n");
    printf("Interval: %d ms | Temp Thresh: %.1f C | Humid Thresh: %.1f %% | Duration: %d s\n",
           cfg.interval_ms, cfg.temp_threshold, cfg.humid_threshold, cfg.duration_sec);

    time_t start_time = time(NULL);

    pthread_t th_sensor, th_proc, th_alert, th_logger;
    SensorContext s_ctx    = { &cb, &cfg, &g_running };
    ProcessorContext p_ctx = { &cb, &cfg, &g_running };

    pthread_create(&th_logger, NULL, logger_thread_func, (void *)&g_running);
    pthread_create(&th_alert,  NULL, alert_thread_func,  (void *)&g_running);
    pthread_create(&th_proc,   NULL, processor_thread_func, &p_ctx);
    pthread_create(&th_sensor, NULL, sensor_thread_func, &s_ctx);

    int elapsed = 0;
    while (g_running) {
        sleep(1);
        elapsed++;
        if (cfg.duration_sec > 0 && elapsed >= cfg.duration_sec) {
            g_running = 0;
        }
    }

    printf("\n[SMS] Graceful shutdown initiated... joining threads.\n");
    cb_wake_all(&cb);
    alert_wake();
    logger_wake();

    pthread_join(th_sensor, NULL);
    pthread_join(th_proc,   NULL);
    pthread_join(th_alert,  NULL);
    pthread_join(th_logger, NULL);

    int cur_sz = 0;
    unsigned long prod = 0, cons = 0, ovf = 0;
    unsigned long al_t = 0, al_h = 0, al_s = 0;
    cb_get_stats(&cb, &cur_sz, &prod, &cons, &ovf);
    alert_get_counts(&al_t, &al_h, &al_s);

    printf("=== SMS Final Summary ===\n");
    printf("  Total Runtime      : %ld seconds\n", (long)(time(NULL) - start_time));
    printf("  Samples Produced   : %lu\n", prod);
    printf("  Samples Consumed   : %lu\n", cons);
    printf("  Buffer Overflows   : %lu\n", ovf);
    printf("  Alerts (TEMP_HIGH) : %lu\n", al_t);
    printf("  Alerts (HUMID_HIGH): %lu\n", al_h);
    printf("  Alerts (TEMP_SPIKE): %lu\n", al_s);
    sensor_print_jitter_summary(stdout, cfg.use_fifo);

    if (cfg.jitter_out_path) {
        FILE *jf = fopen(cfg.jitter_out_path, "a");
        if (jf) {
            sensor_print_jitter_summary(jf, cfg.use_fifo);
            fclose(jf);
        }
    }

    alert_queue_destroy();
    logger_destroy();
    cb_destroy(&cb);
    munlockall();
    return 0;
}
