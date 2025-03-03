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
#define PROC_STAT "/proc/stat"
#define PROC_MEMINFO "/proc/meminfo"
#define PROC_NET_DEV "/proc/net/dev"
#define SYS_DEVICES_SYSTEM_CPU_PRESENT "/sys/devices/system/cpu/present"

typedef struct { // CPU statistics

     long user, nice, system, idle, iowait, irq, softirq, steal;
} cpu_stats;

typedef struct {

     unsigned long total;
     unsigned long free;
} resource_info;

void get_network_traffic(unsigned long long *, unsigned long long *);
int get_num_cpus();
int get_cpu_usage(cpu_stats *, int);
int get_input_non_blocking();
double resource_usage(resource_info);
double usage_per_core(cpu_stats *, cpu_stats *, int);
resource_info get_storage(const char *);
resource_info get_ram();

#endif
