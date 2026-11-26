#ifndef SYSMON_H
#define SYSMON_H

#include <pthread.h>
#include <stdbool.h>

typedef struct {
    unsigned long mem_total_kb;
    unsigned long mem_free_kb;
    unsigned long mem_available_kb;

    double load1;
    double load5;
    double load15;

    int total_procs;
    int running_procs;

    bool high_load_alert;
    bool running;

    pthread_mutex_t lock;
    pthread_cond_t alert_cond;
} SystemState;

void *mem_sensor_thread(void *arg);
void *cpu_sensor_thread(void *arg);
void *proc_sensor_thread(void *arg);
void *alert_thread(void *arg);

#endif
