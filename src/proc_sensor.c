#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define INTERVAL_SEC 2
#define OUT_PATH "/tmp/sysmon/proc.dat"

static int is_pid_dir(const char *name) {
    if (name == NULL || *name == '\0') {
        return 0;
    }
    for (const char *p = name; *p != '\0'; p++) {
        if (!isdigit((unsigned char)*p)) {
            return 0;
        }
    }
    return 1;
}

int main(void) {
    while (1) {
        int active_procs = 0;
        DIR *dir = opendir("/proc");
        if (dir != NULL) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (is_pid_dir(entry->d_name)) {
                    active_procs++;
                }
            }
            closedir(dir);
        }

        FILE *out = fopen(OUT_PATH, "w");
        if (out != NULL) {
            fprintf(out, "ActiveProcesses: %d\n", active_procs);
            fclose(out);
        }

        sleep(INTERVAL_SEC);
    }

    return 0;
}
