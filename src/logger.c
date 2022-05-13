// Author: i0gan
// Github: https://github.com/i0gan/pwn_waf
// Pwn Waf for AWD CTF

#include "logger.h"

struct log_buf logger = {NULL, 0, 0};
char logger_path[0x100] = {0};
int  logger_fd = -1;

void logger_init(const char *path) {
    // Create log dir
    //mkdir(LOG_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (access(path, R_OK | W_OK) != 0) {
		mkdir(path, 0777);
	}
    strncpy(logger_path, path, 0x100);
}

int logger_open(const char *file_name) {
    logger_fd = open(file_name, O_CREAT | O_APPEND | O_WRONLY, 0666);
    if(logger_fd == -1) {
        perror("open:");
        return 0;
    }
    return 1;
}

void logger_close() {
    close(logger_fd);
    free(logger.buf);
    logger.buf = NULL;
}

void logger_append_hex(const char *buf, int length) {
    char buf_[16] = {0};
    unsigned char ch = 0;
    for(int i = 0; i < length; i++) {
        ch = (unsigned char)buf[i];
        sprintf(buf_, "\\x%02x", ch);
        logger_append(buf_, strlen(buf_));
    }
}

void logger_append(const char *buf, int length) {
    if(logger.cap == 0) {
        logger.buf = malloc(0x1000);
        logger.cap = 0x1000;
    }
    if(length >= (logger.cap - logger.size)) {
        int append_size = (length - (logger.cap - logger.size)) / 0x1000 + 0x1000;
        logger.buf = realloc(logger.buf, logger.cap + append_size);
        logger.cap += append_size;
    }
    memcpy(logger.buf + logger.size, buf, length);
    logger.size += length;
}

void logger_write_buf() {
    write(logger_fd, logger.buf, logger.size);
    logger.size = 0;
}

void logger_write(const char *buf, int len) {
    write(logger_fd, buf, len);
}

int logger_size() {
    return logger.size;
}

int logger_cap() {
    return logger.cap;
}
