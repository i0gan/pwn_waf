// Author: i0gan
// Github: https://github.com/i0gan/pwn_waf
// Pwn Waf for AWD CTF

# pragma once
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
struct log_buf {
    char *buf;
    int size;
    int cap;
};

extern struct log_buf logger;
extern char logger_path[0x100];
extern int logger_fd;

extern void logger_init(const char *path);
extern int logger_open(const char *file);
extern void logger_close();
extern void logger_append_hex(const char *buf, int length);
extern void logger_append(const char *buf, int length);
extern void logger_write_buf();
extern void logger_append(const char *buf, int length);
extern void logger_write_buf();
extern void logger_write(const char *buf, int len);
extern int logger_size();
extern int logger_cap();
