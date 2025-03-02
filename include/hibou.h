#ifndef HIBOU_H
#define HIBOU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <stdbool.h>
#include <sys/statvfs.h>

// ncurses
#define FPS 6
#define COL_WIDTH 11
#define WIN_WIDTH 36
#define QUIT_KEY 'q'

// files
#define PROCMEM "/proc/meminfo"
#define PROCSTAT "/proc/stat"
#define PROCNET "/proc/net/dev"
#define PRESENT "/sys/devices/system/cpu/present"

typedef struct { // structure to hold CPU statistics
     long user, nice, system, idle, iowait, irq, softirq, steal;
} cpu_stats;

typedef struct {

     unsigned long total;
     unsigned long free;
} resource_info;

typedef struct {
     unsigned long rx_bytes; // Total received bytes
     unsigned long tx_bytes; // Total transmitted bytes
} traffic;

traffic get_total_network_traffic();
double resource_usage(resource_info);
resource_info get_storage(const char *);
int get_num_cpus();
int get_cpu_usage(cpu_stats *, int);
double usage_per_core(cpu_stats *, cpu_stats *, int);
resource_info get_ram();
int get_input_non_blocking();

#endif
