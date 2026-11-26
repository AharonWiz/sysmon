#include "sysmon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool read_meminfo_values(unsigned long *total_kb,
                                unsigned long *free_kb,
                                unsigned long *avail_kb) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("fopen /proc/meminfo");
        return false;
    }

    char line[256];
    bool got_total = false, got_free = false, got_avail = false;

    while (fgets(line, sizeof(line), fp) != NULL) {
        unsigned long val = 0;
        if (sscanf(line, "MemTotal: %lu kB", &val) == 1) {
            *total_kb = val;
            got_total = true;
        } else if (sscanf(line, "MemFree: %lu kB", &val) == 1) {
            *free_kb = val;
            got_free = true;
        } else if (sscanf(line, "MemAvailable: %lu kB", &val) == 1) {
            *avail_kb = val;
            got_avail = true;
        }

        if (got_total && got_free && got_avail) {
            break;
        }
    }

    fclose(fp);
    return got_total && got_free && got_avail;
}

void *mem_sensor_thread(void *arg) {
    SystemState *state = (SystemState *)arg;

    while (true) {
        pthread_mutex_lock(&state->lock);
        bool keep_running = state->running;
        pthread_mutex_unlock(&state->lock);

        if (!keep_running) {
            break;
        }

        unsigned long total_kb = 0, free_kb = 0, avail_kb = 0;
        if (read_meminfo_values(&total_kb, &free_kb, &avail_kb)) {
            pthread_mutex_lock(&state->lock);
            state->mem_total_kb = total_kb;
            state->mem_free_kb = free_kb;
            state->mem_available_kb = avail_kb;
            pthread_mutex_unlock(&state->lock);
        }

        sleep(1);
    }

    return NULL;
}
