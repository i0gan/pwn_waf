// Author: i0gan
// Github: https://github.com/i0gan/pwn_waf
// Pwn Waf for AWD CTF

#include "waf.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

const char logo_str[]      = "// CTF AWD PWN WAF\n// Deved By I0gan\n";
const char read_str[]      = "\n<-------------------- read ------------------>\n";
const char write_str[]     = "\n<-------------------- write ----------------->\n";
const char dangerous_str[] = "\n<-------------- dangerous syscall------------>";
char *hosts_str1_buf = NULL;
char *hosts_str2_buf = NULL;
enum log_state waf_log_state = LOG_NONE_;

int  waf_run_mode = RUN_MODE;
int  waf_write_times = 0;
int  waf_read_times  = 0;
char send_buf[SEND_BUF_SIZE];
char recv_buf[SEND_BUF_SIZE];

void set_fd_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL,flags);
}

int readn(int fd, char *buf, int length) {
    int read_sum = 0;
    while(read_sum < length) {
        int read_size = read(fd, buf + read_sum, length - read_sum);
        if(read_size <= 0)
        break;
        read_sum += read_size;
    }
    return read_sum;
}

int writen(int fd, void *buffer, size_t length) {
    int write_left = length;
    int write_len = 0;
    int write_sum = 0;
    char *write_ptr = (char *)buffer;
    while(write_left > 0) {
        if((write_len = write(fd, write_ptr, write_left)) < 0) {
            if(errno == EINTR)
                continue;
            else if(errno == EAGAIN) {
                return write_sum;
            }else {
                return -1;
            }
        }
        write_sum += write_len;
        write_left -= write_len;
        write_ptr += write_len;
    }
    return write_sum;
}

void waf_write_logo() {
#if LOG_METHOD == CLOSE
    return;
#endif

    char time_str[128] = {0};
    struct timeval tv;
    time_t time;
    gettimeofday(&tv, NULL);
    time = tv.tv_sec;
    struct tm *p_time = localtime(&time);
    strftime(time_str, 128, "// Date: %Y-%m-%d %H:%M:%S\n", p_time);
    logger_write(time_str, strlen(time_str));
    if(waf_run_mode == CATCH) {
        logger_write(MODE_CATCH_STR, strlen(MODE_CATCH_STR));
    }else if(waf_run_mode == I0GAN){
        logger_write(MODE_I0GAN_STR, strlen(MODE_I0GAN_STR));
    }else if(waf_run_mode == FORWARD) {
        logger_write(MODE_FORWARD_STR, strlen(MODE_FORWARD_STR));
    }else if(waf_run_mode == FORWARD_MULTI) {
        logger_write(MODE_FORWARD_MULTI_STR, strlen(MODE_FORWARD_MULTI_STR));
    }else {}
    logger_write(logo_str, sizeof(logo_str) - 1);
}

void waf_write_hex_log() {
#if LOG_METHOD == CLOSE
    return;
#endif
    if(logger_size() == 0) return;
    char str[0x60] = {0};
    if(waf_log_state == LOG_WRITE_) {
        snprintf(str, 0x60, "\nw_%d = \"", waf_write_times);
        logger_write(str, strlen(str));
        waf_write_times ++;
    }else {
        snprintf(str, 0x60, "\nr_%d = \"", waf_read_times);
        logger_write(str, strlen(str));
        waf_read_times ++;
    }
    logger_write_buf();
    logger_write("\"\n", 2);
}

void waf_log_open() {
#if LOG_METHOD == CLOSE
    return;
#endif
    char time_str[0x20] = {0};
    char file_name[0x100] = {0};
    struct timeval tv;
    time_t time_;
    gettimeofday(&tv, NULL);
    time_ = tv.tv_sec;
    struct tm *p_time = localtime(&time_);
    strftime(time_str, 128, "%H_%M_%S", p_time);
    snprintf(file_name, 0x100, "%s/%s_%lx%s", LOG_PATH, time_str, tv.tv_usec, ".log");
    if(logger_open(file_name) == 0) {
        printf("Open log [%s] file failed!\n", file_name);
        exit(-1);
    }
}

// Write interactive log
void waf_interactive_log(pid_t pid, char* addr, int size, enum log_state state) {
    int i = 0;
    char data;
    char* buf = (char*)malloc(size + 1);
    for(i = 0; i < size; i++){
        data = ptrace(PTRACE_PEEKDATA, pid, addr + i, NULL);
        buf[i] = data;
    }
    if(waf_log_state != state) {
        // when state changed, then write data to log
        waf_write_hex_log(); // write_last_log_buf
        if(state == LOG_READ_) {
            logger_write(read_str, sizeof(read_str) - 1);
        }
        else {
            logger_write(write_str, sizeof(write_str) - 1);
        }
        waf_log_state = state;
    }
    logger_append_hex(buf, size);
    logger_write(buf, size);
    free(buf);
}

void bin_waf_run(int argc, char* argv[]) {
    pid_t pid;
    struct user_regs_struct regs;
    int status;
    int is_in_syscall = 0;
    int first_time = 1;
    int dangerous_syscall_times = 0;
    pid = fork();
    int sys_num;
    enum log_state log_state_;
    // Use child process to exec 
    if(pid == 0){
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        prctl(PR_SET_PDEATHSIG, SIGHUP); // when parent process exit, the child process also exit.
        argv[1] = LISTEN_ELF;
        status = execvp(LISTEN_ELF, argv + 1);
        //dup2(STDOUT_FILENO, STDERR_FILENO);
        if(status<0){
            perror("exec");
            return ;
        }
    }
    // parent to ptrace child syscall
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
            if(DANGEROUS_SYSCALL(sys_num)) {
                dangerous_syscall_times += 1;
                if(dangerous_syscall_times > 1) {
                    // dangerous syscall
                    logger_write(dangerous_str, sizeof(dangerous_str) - 1);
                    
                    if(waf_run_mode == I0GAN) {
                        // Write last data to log buf
                        // Avoid input cmd from attacker
                        //printf("syscall: %d\n", sys_num);
                        logger_write("\nAVOID\n", sizeof("\nAVOID\n") - 1);
                        //exit(0);
                        return ;
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
                // Ignore the first time
                // Check it is standard pipe or not
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
                    log_state_ = LOG_READ_;
                }
                else if (sys_num == SYS_write) {
                    log_state_ = LOG_WRITE_;
                }
                waf_interactive_log(pid, addr, size, log_state_);
                is_in_syscall = 0;
                ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            }
        }
    }
    else{
        perror("fork");
    }

}

int connect_server(char* ip, ushort port) {    
    struct sockaddr_in server_addr;
    int server_fd = -1;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1) {
        perror("socket");
        return -1;
    }
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if(inet_aton(ip, (struct in_addr*)&server_addr.sin_addr.s_addr) == 0){
        perror("ip error");
        return -1;
    }
    if(connect(server_fd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr)) == -1){
        perror("connect");
        exit(errno);
    }
    return server_fd;
}

void forward_waf_run() {
    fd_set read_fds, test_fds;
    int client_read_fd = 0;
    int client_write_fd = 1;
    int client_error_fd = 2;
    char *server_ip = NULL;
    ushort server_port = 0;

#if RUN_MODE == FORWARD
    server_ip = FORWARD_IP;
    server_port = FORWARD_PORT;
#elif RUN_MODE == FORWARD_MULTI
    int ret = get_host_from_file(&server_ip, &server_port);
    if(ret == -1) {
        return;
    }
#endif
    int server_fd = connect_server(server_ip, server_port);

    FD_ZERO(&read_fds);
    FD_ZERO(&test_fds);

    FD_SET(server_fd, &read_fds);
    FD_SET(client_read_fd, &read_fds);  // standard input fd
    FD_SET(client_write_fd, &read_fds); // standard write fd
    FD_SET(client_error_fd, &read_fds); // standard error fd

    set_fd_nonblock(server_fd);
    set_fd_nonblock(client_read_fd);
    set_fd_nonblock(client_write_fd);
    set_fd_nonblock(client_error_fd);

    while(1) {
        enum log_state log_state_ = LOG_NONE_;
        test_fds = read_fds;
        int result = select(FD_SETSIZE, &test_fds, (fd_set *)0, (fd_set *)0, (struct timeval *) 0);
        if(result < 1) {
            perror("select");
            exit(errno);
        }
        for(int fd = 0; fd < FD_SETSIZE; fd ++) {
            if(FD_ISSET(fd,&test_fds)) {
                int write_size = -1;
                int read_size = -1;
                if(fd == server_fd) {
                    write_size = read(server_fd, recv_buf, RECV_BUF_SIZE);
                    writen(client_write_fd, recv_buf, write_size);
                }else if(fd == client_read_fd) {
                    read_size = read(client_read_fd, send_buf, SEND_BUF_SIZE);
                    writen(server_fd, send_buf, read_size);
                }else if(fd == client_write_fd) {
                    read_size = read(client_write_fd, send_buf, SEND_BUF_SIZE);
                    writen(server_fd, send_buf, read_size);
                }else if(fd == client_error_fd) {
                    read_size = read(client_error_fd, send_buf, SEND_BUF_SIZE);
                    writen(server_fd, send_buf, read_size);
				}

                // handle disconnect
                if(read_size == 0 || write_size == 0) {
                    close(fd);
                    free(hosts_str1_buf);
                    free(hosts_str2_buf);
                    return;
                }

                if(read_size > 0 || write_size > 0) {
                    if(read_size > 0)
                        log_state_ = LOG_READ_;
                    else
                        log_state_ = LOG_WRITE_;
                    if(waf_log_state != log_state_) {
                        // when state changed, then write data to log
                        waf_write_hex_log(); // write_last_log_buf
                        if(log_state_ == LOG_READ_) {
                            logger_write(read_str, sizeof(read_str) - 1);
                        } else {
                            logger_write(write_str, sizeof(write_str) - 1);
                        }
                        waf_log_state = log_state_;
                    }
                    if(read_size > 0) {
                        logger_append_hex(send_buf, read_size);
                        logger_write(send_buf, read_size);
                    } else {
                        logger_append_hex(recv_buf, write_size);
                        logger_write(recv_buf, write_size);
                    }
                }
            }
        }
    }
}

void waf_init() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
#if LOG_METHOD == OPEN
	logger_init(LOG_PATH);
    waf_log_open();
    waf_write_logo();
#endif

#if RUN_MODE == I0GAN
    prctl(PR_SET_PDEATHSIG, SIGHUP);
#endif
}

int main(int argc, char *argv[]) {
    waf_init();
#if RUN_MODE == FORWARD
    forward_waf_run();
#elif RUN_MODE == FORWARD_MULTI
    forward_waf_run();
#elif RUN_MODE == I0GAN
    bin_waf_run(argc, argv);
#elif RUN_MODE == CATCH
    bin_waf_run(argc, argv);
#endif
    waf_write_hex_log();
	logger_close();
    return 0;
}


int get_host_from_file(char **ip, ushort *port) {
    int attack_index = 0;
    char attack_index_str[8] = {0};
    int fd = open(HOSTS_FILE, O_RDONLY);
    int ai_fd = open(HOSTS_ATTACK_INDEX_FILE, O_RDWR | O_CREAT, 0666);

    read(ai_fd, attack_index_str, sizeof(attack_index_str));
    attack_index = atoi(attack_index_str);
    if(attack_index == -1) attack_index = 0;
    //printf("now attack_index: %d, file %s ", attack_index, attack_index_str);

    if(fd == -1) {
        perror("open:");
        return -1;
    }

    struct stat stat;
    int ret = fstat(fd, &stat);
    if(ret == -1) {
        perror("fstat:");
        return -1;
    }
    char *hosts_str = malloc(stat.st_size + 0x10);
    char *hosts_str2 = malloc(stat.st_size + 0x10);
    read(fd, hosts_str, stat.st_size);
    memcpy(hosts_str2, hosts_str, stat.st_size);

    char *p = NULL;
    char *line = hosts_str;
    int line_nums = 0;

    p = strsep(&line, "\n");
    while(p != NULL) {
        char *info = p;
        char *f_ip = strsep(&info, ":");
        char *f_port = strsep(&info, ":");
        if(f_ip != NULL && f_port != NULL)
            line_nums ++;
        p = strsep(&line, "\n");
    }

    //printf("line_nums  : %d\n", line_nums);
    line = hosts_str2;
    p = strsep(&line, "\n");
    int now_index = 0;
    while(p != NULL) {
        char *info = p;
        char *f_ip = strsep(&info, ":");
        char *f_port = strsep(&info, ":");
        if(f_ip != NULL && f_port != NULL) {
            if(now_index == attack_index) {
                *ip = f_ip;
                *port = atoi(f_port);
                //printf("host: %s:%d\n", *ip, *port);
                break;
            }
            now_index ++;
        }
        p = strsep(&line, "\n");
        //puts(hosts_str);
    }
    attack_index = (attack_index + 1) % line_nums;
    snprintf(attack_index_str, sizeof(attack_index_str), "%d", attack_index);
    lseek(ai_fd, 0, SEEK_SET);
    write(ai_fd, attack_index_str, sizeof(attack_index_str));
    close(ai_fd);
    close(fd);
    return 0;
}
