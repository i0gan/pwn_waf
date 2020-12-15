// Author: i0gan
// Waf for pwn of awd mode

#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<error.h>
#include<sys/wait.h>
#include<sys/ptrace.h>
#include<sys/syscall.h>
#include<sys/user.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<time.h>
#include<string.h>

// for hide binary file
#define LISTEN_ELF "/tmp/.i0gan/pwn" // trace elf file
#define LOG_PATH   "/tmp/.i0gan/"    // path to log
#define ARCH 64                      // 64 or 32
#define LOG_BUF_LEN 0x10000          // log buffer length
#define RUN_MODE RUN_I0GAN_          // The mode of RUN_I0GAN_ or RUN_CATCH_

const char logo_str[]       = "// AWD I0GAN WAF\n// Powered By I0gan\n";
const char read_str[]       = "-------------------- read ------------------\n";
const char write_str[]      = "-------------------- write -----------------\n";
const char dangerous_str[] = "\n-------------- dangerous syscall------------";

struct log_buf {
	char buf[LOG_BUF_LEN];
	int  size;
};

struct log_buf log_buf;

enum sys_type {
	SYS_READ_,
	SYS_WRITE_
};

enum run_mode {
	RUN_CATCH_,	
	RUN_I0GAN_
};

// error code avoid zombie parent process
#define ERROR_EXIT(x)  \
	x == 0xb7f
	
// judge is standard io
#define STANDARD_IO(x) \
	x == 0 ||          \
	x == 1

// dangerous syscall
#define TRACE_I0GAN_SYSCALL(x)  \
	x == __NR_rt_sigaction   || \
	x == __NR_rt_sigprocmask || \
	x == __NR_clone  || \
	x == __NR_execve

int state = -1;
int log_fd = -1;
int run_mode = RUN_MODE;

void init();
int run(int argc, char* argv[]);
int open_log();
void close_log();
void append_hex_to_log_buf(const char *buf, int length);
void append_to_log_buf(const char *buf, int length);
void write_log_buf();
void write_log(const char *buf, int len);
void interactive_log(pid_t pid, char* addr, int size, enum sys_type type);


void append_hex_to_log_buf(const char *buf, int length) {
	char buf_[16] = {0};
	unsigned char ch = 0;
	for(int i = 0; i < length; i++) {
		ch = (unsigned char)buf[i];
		sprintf(buf_, "\\x%02x", ch);
		append_to_log_buf(buf_, strlen(buf_));
	}
}

void append_to_log_buf(const char *buf, int length) {
	if(length > (LOG_BUF_LEN - log_buf.size)) {
		puts("no memory to store data");
		return ;
	}
	memcpy(log_buf.buf + log_buf.size, buf, length);
	log_buf.size += length;
}

void write_log_buf() {
	if(log_buf.size == 0) return;
	write(log_fd, "\n\"", 2);
	write(log_fd, log_buf.buf, log_buf.size);
	write(log_fd, "\"\n", 2);
	log_buf.size = 0;
}

void write_log(const char *buf, int len) {
	write(log_fd, buf, len);
}

void write_logo() {
	char time_str[128] = {0};
	struct timeval tv;
	time_t time;
	gettimeofday(&tv, NULL);
	time = tv.tv_sec;
	struct tm *p_time = localtime(&time);
	strftime(time_str, 128, "// Date: %Y-%m-%d %H:%M:%S\n", p_time);
	#define mode_i0gan_str  "// Mode: RUN_I0GAN_\n"
	#define mode_catch_str  "// Mode: RUN_CATCH_\n"
	write_log(time_str, strlen(time_str));
	if(run_mode == RUN_I0GAN_) {
		write_log(mode_i0gan_str, strlen(mode_i0gan_str));
	}else {
		write_log(mode_catch_str, strlen(mode_catch_str));
	}
	write_log(logo_str, sizeof(logo_str) - 1);
}

// Write interactive log
void interactive_log(pid_t pid, char* addr, int size, enum sys_type type){
	int i = 0,j = 0;
	char data;
	char* buf = (char*)malloc(size + 1);
	for(i = 0; i < size; i++){
		data = ptrace(PTRACE_PEEKDATA, pid, addr + i, NULL);
		buf[i] = data;
	}
	if(state != type) {
		// when state changed, then write data to log
		write_log_buf(); // write_last_log_buf
		if(type == SYS_READ_) {
			write_log(read_str, sizeof(read_str) - 1);
		}
		else {
			write_log(write_str, sizeof(write_str) - 1);
		}
		state = type;
	}
	append_hex_to_log_buf(buf, size);
	write_log(buf, size);
	free(buf);
}

int run(int argc, char* argv[]){
	pid_t pid;
	struct user_regs_struct regs;
	int status;
	int is_in_syscall = 0;
	int first_time = 1;
	int dangerous_syscall_times = 0;
	pid = fork();
	int sys_num;
	enum sys_type sys_status;
	// we use child process to exec 
	if(pid == 0){
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		argv[1] = LISTEN_ELF;
		status = execvp(LISTEN_ELF, argv + 1);
		if(status<0){
			perror("exec:");
			return -1;
		}
	}
	// parent to get child syscall
	else if (pid > 0){
		while(1) {
			wait(&status);
			if(WIFEXITED(status) || ERROR_EXIT(status))
				break;
			// get rax to ensure witch syscall
			ptrace(PTRACE_GETREGS, pid, NULL, &regs);
#if ARCH == 64
			sys_num = regs.orig_rax;
#elif ARCH == 32
			sys_num = regs.orig_eax;
#endif
			//printf("syscall %d\n", sys_num);
			if(TRACE_I0GAN_SYSCALL(sys_num)) {
				dangerous_syscall_times += 1;
				if(dangerous_syscall_times > 1) {
					write_log(dangerous_str, sizeof(dangerous_str) - 1);
					if(run_mode == RUN_I0GAN_) {
						return 0;
					}
				}
			}
			if (sys_num != SYS_read && sys_num != SYS_write) {
				ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
				continue;
			}
			if (is_in_syscall == 0) {
				is_in_syscall = 1;
				ptrace(PTRACE_SYSCALL, pid, 0, 0);
			} else {
				// we should ignor the first time
				// checl it is standard pipe or not
				int is_standard_io = 0;
#if ARCH == 64
				is_standard_io = STANDARD_IO(regs.rdi);
#elif ARCH == 32
				is_standard_io = STANDARD_IO(regs.ebx);
#endif 
				if(!is_standard_io) {
					first_time = 0;
					ptrace(PTRACE_SYSCALL, pid, NULL ,NULL);
					is_in_syscall ^= 1;
					continue;
				}
				int size = 0;
				char* addr = NULL;
				int fd = -1;
#if ARCH == 64
				addr = (char*)regs.rsi;
				fd = (int)regs.rdi;
				size = regs.rax; // readed length
#elif ARCH == 32
				fd = regs.edx;
				addr = (char*)regs.ecx;
				size = regs.eax; // readed length
#endif
				if(sys_num == SYS_read) {
					sys_status = SYS_READ_;
				}
				else if (sys_num == SYS_write) {
					sys_status = SYS_WRITE_;
				}
				interactive_log(pid, addr, size, sys_status);
				is_in_syscall = 0;
				ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
			}
		}
		return status;
	}
	else{
		perror("fork:");
		return -1;
	}
}

void init() {
	setvbuf(stdin,0,2,0);
	setvbuf(stdout,0,2,0);
	// create log dir
	mkdir(LOG_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

int open_log() {
	char time_str[0x20] = {0};
	char file_name[0x100] = {0};
	struct timeval tv;
	time_t time_;
	gettimeofday(&tv, NULL);
	time_ = tv.tv_sec;
	struct tm *p_time = localtime(&time_);
	strftime(time_str, 128, "%H_%M_%S", p_time);
	time(&time_);
	snprintf(file_name, 0x100, "%s%s_%x%s",LOG_PATH, time_str, time_, ".i0gan");
	log_fd = open(file_name, O_CREAT|O_APPEND|O_WRONLY, 0666);
	if(log_fd == -1) {
		perror("open:");
		return 0;
	}
	return 1;
}

void close_log() {
	close(log_fd);
}

int main(int argc, char *argv[]) {
	int ret = 0;
	init();
	open_log();
	write_logo();
	ret = run(argc, argv);
	write_log_buf(); // write last data to log buf
	close_log();
	return ret;
}
