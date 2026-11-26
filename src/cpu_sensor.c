#include "sysmon.h"
#include <stdio.h>
#include <unistd.h>

static bool read_loadavg_values(double *l1, double *l5, double *l15) {
    FILE *fp = fopen("/proc/loadavg", "r");
    if (!fp) {
        perror("fopen /proc/loadavg");
        return false;
    }

    int parsed = fscanf(fp, "%lf %lf %lf", l1, l5, l15);
    fclose(fp);
    return parsed == 3;
}

void *cpu_sensor_thread(void *arg) {
    SystemState *state = (SystemState *)arg;

    while (true) {
        pthread_mutex_lock(&state->lock);
        bool keep_running = state->running;
        pthread_mutex_unlock(&state->lock);

        if (!keep_running) {
            break;
        }

        double l1 = 0.0, l5 = 0.0, l15 = 0.0;
        if (read_loadavg_values(&l1, &l5, &l15)) {
            pthread_mutex_lock(&state->lock);
            state->load1 = l1;
            state->load5 = l5;
            state->load15 = l15;

            if (l1 > 1.50) {
                state->high_load_alert = true;
                pthread_cond_signal(&state->alert_cond);
            }
            pthread_mutex_unlock(&state->lock);
        }

        sleep(1);
    }

    return NULL;
}
