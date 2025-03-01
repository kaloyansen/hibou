#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <stdbool.h>

#define FPS 4
#define WIN_WIDTH 20
#define QUIT_KEY 'q'
#define MEMINFO "/proc/meminfo"
#define PROCSTAT "/proc/stat"
#define PRESENT "/sys/devices/system/cpu/present"

// structure to hold CPU statistics
typedef struct {
     long user, nice, system, idle, iowait, irq, softirq, steal;
} cpu_stats;

// Function to estimate the number of CPU cores from /sys/devices/system/cpu
int get_num_cpus() {
     int num_cpus = 0;
     FILE *fp = fopen(PRESENT, "r");
     if (fp == NULL) {
          perror("cannot open " PRESENT);
          return -1;
     }

     char line[256];
     if (fgets(line, sizeof(line), fp)) {
          // This will read a line like "0-3" for 4 cores (cpu0 to cpu3)
          sscanf(line, "%d-%d", &num_cpus, &num_cpus);
          num_cpus += 1;  // Ensure we count the number of cores (inclusive range)
     }

     fclose(fp);
     return num_cpus;
}

int get_cpu_usage(cpu_stats *stats, int num_cpus) {

     FILE *fp = fopen(PROCSTAT, "r");
     if (fp == NULL) {
          perror("Unable to open " PROCSTAT);
          return -1;
     }

     char line[256];
     int cpu_count = 0;

     // skip the first line which is the total CPU stats
     fgets(line, sizeof(line), fp);

     // read stats for each core
     while (cpu_count < num_cpus && fgets(line, sizeof(line), fp)) {

          if (sscanf(line, "cpu%d %ld %ld %ld %ld %ld %ld %ld %ld",
                     &cpu_count,
                     &stats[cpu_count].user,
                     &stats[cpu_count].nice,
                     &stats[cpu_count].system,
                     &stats[cpu_count].idle,
                     &stats[cpu_count].iowait,
                     &stats[cpu_count].irq,
                     &stats[cpu_count].softirq,
                     &stats[cpu_count].steal) >= 9) {

               cpu_count++;
          }
     }

     fclose(fp);
     return cpu_count;
}


double calculate_cpu_usage_for_core(cpu_stats *old_stats, cpu_stats *new_stats, int core_num) {

     if (core_num < 0) {

          return -1;  // Invalid core number
     }

     long total_old = old_stats[core_num].user + old_stats[core_num].nice + old_stats[core_num].system + old_stats[core_num].idle +
          old_stats[core_num].iowait + old_stats[core_num].irq + old_stats[core_num].softirq + old_stats[core_num].steal;
     long total_new = new_stats[core_num].user + new_stats[core_num].nice + new_stats[core_num].system + new_stats[core_num].idle +
          new_stats[core_num].iowait + new_stats[core_num].irq + new_stats[core_num].softirq + new_stats[core_num].steal;
     long total_diff = total_new - total_old;
     long idle_diff = new_stats[core_num].idle - old_stats[core_num].idle;
     return 100.0 * (total_diff - idle_diff) / total_diff;  // CPU usage percentage
}


double get_ram_usage() {

     FILE *fp = fopen(MEMINFO, "r");
     if (fp == NULL) {

          perror("cannot open " MEMINFO);
          return -1;
     }

     char line[256];
     long total_memory = 0;
     long free_memory = 0;

     while (fgets(line, sizeof(line), fp)) {

          if (sscanf(line, "MemTotal: %ld kB", &total_memory) == 1) {

          } else if (sscanf(line, "MemFree: %ld kB", &free_memory) == 1) {

               break;
          }
     }

     fclose(fp);

     if (total_memory == 0) {
          return -1;  // Error if total memory was not found
     }

     return 100.0 * (1 - (double)free_memory / total_memory);  // Return RAM usage percentage
}


int get_input_non_blocking() {

     int ch = ERR;
     struct timeval timeout = { 0, 100000 };  // Timeout of 100ms
     fd_set readfds;
     FD_ZERO(&readfds);
     FD_SET(STDIN_FILENO, &readfds);

     // Select returns 1 if input is available
     if (select(1, &readfds, NULL, NULL, &timeout) > 0) {

          ch = getchar();
     }
     return ch;
}


int main() {

     cpu_stats *old_stats = NULL, *new_stats = NULL;
     int num_cpus = get_num_cpus();
     if (num_cpus < 0) {

          return -1;
     }

     old_stats = malloc(num_cpus * sizeof(cpu_stats));
     new_stats = malloc(num_cpus * sizeof(cpu_stats));

     if (!old_stats || !new_stats) {

          perror("Memory allocation failed");
          return -1;
     }

     if (get_cpu_usage(old_stats, num_cpus) < 0) {

          free(old_stats);
          free(new_stats);
          return -1;
     }

     // ncurses
     initscr();
     cbreak();
     noecho();
     curs_set(0);
     struct timespec ts;
     ts.tv_sec = 0;
     ts.tv_nsec = 1e9 / FPS; 
     WINDOW * win = newwin(num_cpus + 3, WIN_WIDTH, 3, 3);
     box(win, 0, 0);
     bool quit = false;
     while (!quit) {

          if (get_input_non_blocking() == QUIT_KEY) {

               quit = true;
          }

          if (get_cpu_usage(new_stats, num_cpus) < 0) {

               break;
          }

          // wclear(win);

          for (int i = 0; i < num_cpus; i++) {

               double usage = calculate_cpu_usage_for_core(old_stats, new_stats, i);
               mvwprintw(win, i + 1, 2, "CPU%d %6.2f%%", i, usage);
          }

          double ram_usage = get_ram_usage();
          mvwprintw(win, num_cpus + 1, 2, "RAM  %6.2f%%", ram_usage);

          memcpy(old_stats, new_stats, num_cpus * sizeof(cpu_stats));

          wrefresh(win);
          nanosleep(&ts, NULL);
     }

     free(old_stats);
     free(new_stats);
     endwin();
     return 0;
}

