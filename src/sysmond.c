#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define INTERVAL_SEC 2
#define DATA_DIR "/tmp/sysmon"
#define MEM_FILE "/tmp/sysmon/mem.dat"
#define CPU_FILE "/tmp/sysmon/cpu.dat"
#define PROC_FILE "/tmp/sysmon/proc.dat"

static void print_file_block(const char *title, const char *path) {
    printf("--- %s ---\n", title);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("(not available yet)\n");
        return;
    }

    char line[256];
    int has_content = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        fputs(line, stdout);
        has_content = 1;
    }
    if (!has_content) {
        printf("(empty)\n");
    }
    fclose(fp);
}

int main(void) {
    if (mkdir(DATA_DIR, 0755) != 0 && errno != EEXIST) {
        perror("mkdir /tmp/sysmon failed");
        return 1;
    }

    while (1) {
        printf("\n==============================\n");
        printf("      SYSMON STATUS REPORT\n");
        printf("==============================\n");
        print_file_block("MEMORY", MEM_FILE);
        print_file_block("CPU LOAD", CPU_FILE);
        print_file_block("PROCESSES", PROC_FILE);
        fflush(stdout);

        sleep(INTERVAL_SEC);
    }

    return 0;
}
