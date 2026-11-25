#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define INTERVAL_SEC 2
#define OUT_PATH "/tmp/sysmon/cpu.dat"

int main(void) {
    char line[256];

    while (1) {
        double load1 = 0.0, load5 = 0.0, load15 = 0.0;

        FILE *fp = fopen("/proc/loadavg", "r");
        if (fp != NULL) {
            if (fgets(line, sizeof(line), fp) != NULL) {
                sscanf(line, "%lf %lf %lf", &load1, &load5, &load15);
            }
            fclose(fp);
        }

        FILE *out = fopen(OUT_PATH, "w");
        if (out != NULL) {
            fprintf(out, "load_1m: %.2f\n", load1);
            fprintf(out, "load_5m: %.2f\n", load5);
            fprintf(out, "load_15m: %.2f\n", load15);
            fclose(out);
        }

        sleep(INTERVAL_SEC);
    }

    return 0;
}
