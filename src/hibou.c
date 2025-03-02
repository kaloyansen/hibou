#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <stdbool.h>
#include <sys/statvfs.h>

#define FPS 7
#define WIN_WIDTH 40
#define QUIT_KEY 'q'
#define MEMINFO "/proc/meminfo"
#define PROCSTAT "/proc/stat"
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
} NetTraffic;

NetTraffic get_total_network_traffic() {
    FILE *fp = fopen("/proc/net/dev", "r");
    if (!fp) {
        perror("Failed to open /proc/net/dev");
        exit(1);
    }

    NetTraffic total_traffic = {0, 0};
    char line[256], iface[32];

    // Skip first two lines (headers)
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    // Read network interface data
    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%31s %lu %*d %*d %*d %*d %*d %*d %*d %lu", iface, &total_traffic.rx_bytes, &total_traffic.tx_bytes);
    }

    fclose(fp);
    return total_traffic;
}


double resource_usage(resource_info info) {

     return 100.0 * (1 - (double)info.free / info.total);
}


resource_info get_storage(const char *path) {

     struct statvfs fs;
     resource_info info = {0, 0};

     if (statvfs(path, &fs) != 0) {
          perror("statvfs failed");
          return info;
     }

     info.total = fs.f_blocks * fs.f_frsize;
     info.free = fs.f_bfree * fs.f_frsize;
     return info;
}


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


resource_info get_ram() {

     resource_info info = {0, 0};
     FILE *fp = fopen(MEMINFO, "r");
     if (fp == NULL) {

          perror("cannot open " MEMINFO);
          return info;
     }

     char line[256];

     while (fgets(line, sizeof(line), fp)) {

          if (sscanf(line, "MemTotal: %ld kB", &info.total) == 1) {

          } else if (sscanf(line, "MemFree: %ld kB", &info.free) == 1) {

               break;
          }
     }

     fclose(fp);

     if (info.total == 0) {
          return info;
     }

     return info;
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
     WINDOW * win = newwin(num_cpus + 10, WIN_WIDTH, 3, 3);
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
          int c1 = 2;
          int c2 = 20;
          int c3 = 31;

          mvwprintw(win, 1, c1, "resource");
          mvwprintw(win, 1, c2, " size");
          mvwprintw(win, 1, c3, "  usage");

          for (int i = 0; i < num_cpus; i++) {

               double usage = calculate_cpu_usage_for_core(old_stats, new_stats, i);
               mvwprintw(win, i + 3, c1, "CPU%d", i);
               mvwprintw(win, i + 3, c3, "%6.2f%%", usage);
          }

          resource_info ram = get_ram();
          resource_info storage_root = get_storage("/");
          //resource_info storage_home = get_storage("/home");
          
          NetTraffic traffic = get_total_network_traffic();
          
          mvwprintw(win, num_cpus + 3, c1, "memory");
          mvwprintw(win, num_cpus + 3, c2, "%4.1fG", (double)ram.total / 1024 / 1024);
          mvwprintw(win, num_cpus + 3, c3, "%6.2f%%", resource_usage(ram));

          mvwprintw(win, num_cpus + 4, c1, "/");
          mvwprintw(win, num_cpus + 4, c2, "%4.1fG", (double)storage_root.total / 1e9);
          mvwprintw(win, num_cpus + 4, c3, "%6.2f%%", resource_usage(storage_root));

          mvwprintw(win, num_cpus + 5, c1, "traffic");
          mvwprintw(win, num_cpus + 5, c2, "in");
          mvwprintw(win, num_cpus + 5, c3, "%.2fMB", traffic.rx_bytes  / (1024.0 * 1024.0));

          mvwprintw(win, num_cpus + 6, c1, "traffic");
          mvwprintw(win, num_cpus + 6, c2, "out");
          mvwprintw(win, num_cpus + 6, c3, "%.2fMB", traffic.tx_bytes  / (1024.0 * 1024.0));

          memcpy(old_stats, new_stats, num_cpus * sizeof(cpu_stats));

          wrefresh(win);
          nanosleep(&ts, NULL);
     }

     free(old_stats);
     free(new_stats);
     endwin();
     return 0;
}

