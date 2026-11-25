#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INTERVAL_SEC 2
#define OUT_PATH "/tmp/sysmon/mem.dat"

int main(void) {
    char line[256];

    while (1) {
        long mem_total = 0, mem_free = 0, mem_avail = 0;
        long swap_total = 0, swap_free = 0;

        FILE *fp = fopen("/proc/meminfo", "r");
        if (fp != NULL) {
            while (fgets(line, sizeof(line), fp) != NULL) {
                if (strncmp(line, "MemTotal:", 9) == 0) {
                    sscanf(line + 9, "%ld", &mem_total);
                } else if (strncmp(line, "MemFree:", 8) == 0) {
                    sscanf(line + 8, "%ld", &mem_free);
                } else if (strncmp(line, "MemAvailable:", 13) == 0) {
                    sscanf(line + 13, "%ld", &mem_avail);
                } else if (strncmp(line, "SwapTotal:", 10) == 0) {
                    sscanf(line + 10, "%ld", &swap_total);
                } else if (strncmp(line, "SwapFree:", 9) == 0) {
                    sscanf(line + 9, "%ld", &swap_free);
                }
            }
            fclose(fp);
        }

        FILE *out = fopen(OUT_PATH, "w");
        if (out != NULL) {
            fprintf(out, "MemTotal: %ld kB\n", mem_total);
            fprintf(out, "MemFree: %ld kB\n", mem_free);
            fprintf(out, "MemAvailable: %ld kB\n", mem_avail);
            fprintf(out, "SwapTotal: %ld kB\n", swap_total);
            fprintf(out, "SwapFree: %ld kB\n", swap_free);
            fclose(out);
        }

        sleep(INTERVAL_SEC);
    }

    return 0;
}
