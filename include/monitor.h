#ifndef __MONITOR_H__
#define __MONITOR_H__

#define MAX_OPTION_LENGTH 10
#define MAX_IMG_PATH_LENGTH 1000
extern char opt[MAX_OPTION_LENGTH];
extern char img_path[MAX_IMG_PATH_LENGTH];

extern void cmd_c();

extern void cmd_h();

extern void cmd_r();

extern void cmd_s();

extern void cmd_re();
extern void read_file(const char *file_path);
#endif