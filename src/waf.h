// Author: i0gan
// Github: https://github.com/i0gan/pwn_waf
// Pwn Waf for AWD CTF

# pragma once
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include "logger.h"

#define RUN_CATCH   0x01
#define RUN_I0GAN   0x02
#define RUN_FORWARD 0x03

//#define ARCH 64                          // 64 or 32
//#define LOG_PATH   "/tmp/.i0gan"         // path to log
//#define RUN_MODE RUN_CATCH               // The mode of RUN_CATCH / RUN_I0GAN / RUN_FORWARD
#define LISTEN_ELF   LOG_PATH "/pwn"       // trace elf file
#define HOSTS_FILE   LOG_PATH "/hosts.txt" // hosts file for forward
#define HOSTS_ATTACK_INDEX_FILE   LOG_PATH "/.hosts_attack_index" // hosts attack index file for forward

#define MODE_CATCH_STR    "// Mode: RUN_CATCH\n"
#define MODE_I0GAN_STR    "// Mode: RUN_I0GAN\n"
#define MODE_FORWARD_STR  "// Mode: RUN_FORWARD\n"

#define SEND_BUF_SIZE 0x2000
#define RECV_BUF_SIZE 0x2000

enum log_state {
    LOG_NONE_,
    LOG_READ_,
    LOG_WRITE_
};

// error code avoid zombie parent process
#define ERROR_EXIT(x)  \
    x == 0xb7f

// judge is standard io
#define STANDARD_IO(x) \
    x == 0 ||          \
    x == 1 || \
    x == 2

// dangerous syscall
#define TRACE_I0GAN_SYSCALL(x)  \
    x == __NR_open  || \
    x == __NR_execve

int readn(int fd, char *buf, int length);
int writen(int fd, void *buffer, size_t length);
void waf_write_logo();
void waf_write_hex_log();
void waf_log_open();
void waf_interactive_log(pid_t pid, char* addr, int size, enum log_state state);
void bin_waf_run(int argc, char* argv[]);
int connect_server(char* ip, ushort port);
void redir_waf_run();
void waf_init();
int get_host_from_file(char **ip, ushort *port);
