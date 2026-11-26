CC = gcc
CFLAGS = -Wall -Wextra -g -Iinc -pthread

all: bin/sysmond

bin/sysmond: src/sysmond.c src/mem_sensor.c src/cpu_sensor.c src/proc_sensor.c inc/sysmon.h
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ src/sysmond.c src/mem_sensor.c src/cpu_sensor.c src/proc_sensor.c

clean:
	rm -rf bin/*.o bin/sysmond
