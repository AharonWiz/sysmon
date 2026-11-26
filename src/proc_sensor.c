#include "sysmon.h"
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool is_numeric_name(const char *name) {
    if (!name || *name == '\0') {
        return false;
    }
    for (const char *p = name; *p != '\0'; ++p) {
        if (!isdigit((unsigned char)*p)) {
            return false;
        }
    }
    return true;
}

static bool read_proc_counts(int *total_out, int *running_out) {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("opendir /proc");
        return false;
    }

    int total = 0;
    int running = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (!is_numeric_name(entry->d_name)) {
            continue;
        }

        char status_path[512];
        int written = snprintf(status_path, sizeof(status_path),
                               "/proc/%s/status", entry->d_name);
        if (written < 0 || (size_t)written >= sizeof(status_path)) {
            continue;
        }

        FILE *fp = fopen(status_path, "r");
        if (!fp) {
            continue;
        }

        char line[256];
        while (fgets(line, sizeof(line), fp) != NULL) {
            char state_code = '\0';
            if (sscanf(line, "State: %c", &state_code) == 1) {
                total++;
                if (state_code == 'R') {
                    running++;
                }
                break;
            }
        }

        fclose(fp);
    }

    closedir(dir);
    *total_out = total;
    *running_out = running;
    return true;
}

void *proc_sensor_thread(void *arg) {
    SystemState *state = (SystemState *)arg;

    while (true) {
        pthread_mutex_lock(&state->lock);
        bool keep_running = state->running;
        pthread_mutex_unlock(&state->lock);

        if (!keep_running) {
            break;
        }

        int total = 0;
        int running = 0;
        if (read_proc_counts(&total, &running)) {
            pthread_mutex_lock(&state->lock);
            state->total_procs = total;
            state->running_procs = running;
            pthread_mutex_unlock(&state->lock);
        }

        sleep(1);
    }

    return NULL;
}
