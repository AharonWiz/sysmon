#include "sysmon.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t got_sigint = 0;

static void handle_sigint(int sig) {
    (void)sig;
    got_sigint = 1;
}

void *alert_thread(void *arg) {
    SystemState *state = (SystemState *)arg;

    while (true) {
        pthread_mutex_lock(&state->lock);
        while (!state->high_load_alert && state->running) {
            pthread_cond_wait(&state->alert_cond, &state->lock);
        }

        bool keep_running = state->running;
        bool alert_active = state->high_load_alert;
        double load1 = state->load1;

        if (alert_active) {
            state->high_load_alert = false;
        }
        pthread_mutex_unlock(&state->lock);

        if (!keep_running && !alert_active) {
            break;
        }

        if (alert_active) {
            fprintf(stderr,
                    "[ALERT] High CPU load detected: 1m loadavg = %.2f\n",
                    load1);
        }
    }

    return NULL;
}

static void print_snapshot(SystemState *state) {
    pthread_mutex_lock(&state->lock);
    unsigned long mem_total = state->mem_total_kb;
    unsigned long mem_free = state->mem_free_kb;
    unsigned long mem_avail = state->mem_available_kb;
    double l1 = state->load1;
    double l5 = state->load5;
    double l15 = state->load15;
    int total_p = state->total_procs;
    int running_p = state->running_procs;
    pthread_mutex_unlock(&state->lock);

    printf("--- sysmon snapshot ---\n");
    printf("Memory: total=%lu kB, free=%lu kB, available=%lu kB\n",
           mem_total, mem_free, mem_avail);
    printf("Loadavg: 1m=%.2f, 5m=%.2f, 15m=%.2f\n", l1, l5, l15);
    printf("Processes: total=%d, running=%d\n", total_p, running_p);
}

int main(void) {
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    SystemState state = {0};
    state.running = true;
    pthread_mutex_init(&state.lock, NULL);
    pthread_cond_init(&state.alert_cond, NULL);

    pthread_t mem_tid, cpu_tid, proc_tid, alert_tid;
    if (pthread_create(&mem_tid, NULL, mem_sensor_thread, &state) != 0 ||
        pthread_create(&cpu_tid, NULL, cpu_sensor_thread, &state) != 0 ||
        pthread_create(&proc_tid, NULL, proc_sensor_thread, &state) != 0 ||
        pthread_create(&alert_tid, NULL, alert_thread, &state) != 0) {
        perror("pthread_create");
        pthread_mutex_lock(&state.lock);
        state.running = false;
        pthread_cond_broadcast(&state.alert_cond);
        pthread_mutex_unlock(&state.lock);
        return 1;
    }

    while (!got_sigint) {
        sleep(2);
        if (got_sigint) {
            break;
        }
        print_snapshot(&state);
    }

    pthread_mutex_lock(&state.lock);
    state.running = false;
    pthread_cond_broadcast(&state.alert_cond);
    pthread_mutex_unlock(&state.lock);

    pthread_join(mem_tid, NULL);
    pthread_join(cpu_tid, NULL);
    pthread_join(proc_tid, NULL);
    pthread_join(alert_tid, NULL);

    pthread_cond_destroy(&state.alert_cond);
    pthread_mutex_destroy(&state.lock);

    printf("sysmon stopped cleanly.\n");
    return 0;
}
