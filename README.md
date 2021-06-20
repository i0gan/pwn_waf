# AWD PWN WAF

## Intro

CTF AWD  WAF FOR PWN

Author: I0gan

QQ: 418894113



The WAF is a traffic capture tool for AWD PWN, which is convenient to analyze and write anti attack exp, and is very conducive to the use of `PWN Ctfer`. The tool has been used in AWD competitions for many times. I hope you can `give me a star`. The WAF has `three modes`. The `RUN_CATCH` mode simply capture the attacked interactive traffic, which can be viewed under the  `log path`. The `RUN_I0GAN` mode is a `defense mode`, which can prevent attackers from getting the shell, and can also view the attacker's interactive traffic. This mode is used cautiously, strictly abides by the rules of the ctf competition. If you violates the rules of the competition. The consequences are borne by yourslves. The `RUN_FORWARD` mode simply forwards the attacker's traffic to hit others, and we can capture the traffic in the middle. Of course, if the attacker successfully gets the flag, We can also get the flag in the log file.



## Fix Log
    2021-05-13: Thancks b0ldfrev commit the IO blocking problem for FORWARD mode 





## RUN_CATCH / RUN_I0GAN MODE WAF Principle

Execve the target elf file by creating a child process, and then the parent process uses ptrace to monitor the syscall  of the child process. If the standard IO is used, the data  is read and recorded in the log. If the syscall is dangerous, it is also recorded in the log



## RUN_FORWARD MODE WAF Principle

Capture traffic from standard I / O and forward it to the target server. You can captrue traffic between the attacker and the victim, and you can also get victim's flag


## Code Tree

```
src
├── logger.c
├── logger.h
├── test_pwn.c
├── waf.c
└── waf.h
```



##  How to use

### 0x01 config

In makefile

```makefile
# configure
LOG_PATH    := /tmp/.waf # Set your own log path
ARCH        := 64
SERVER_IP   := 127.0.0.1 # RUN_FORWARD used
SERVER_PORT := 9090      # RUN_FORWARD used
```

Some configuration information of this mode is in `makefile`, `LOG_PATH` is a log file path, `ARCH` Represents whether the program is 32-bit or 64 bit. The `SERVER_IP` and `SERVER_PORT` only used in `FORWARD` mode.  So You must have to set your own base configure information. 



### 0x02 Compile

#### Compile all

```
make # or make all
```

You will get `catch`,`i0gan`,`redir` files

The `catch` is RUN_CATCH MODE waf program

The `i0gan` is RUN_I0GAN MODE waf program

The `forward` is RUN_FORWARD MODE waf program

#### Compile RUN_CATCH mode

```
make catch
```

#### Compile RUN_I0GAN mode

```
make i0gan
```

Compile RUN_FORWARD mode

```
make forward
```



### 0x03

After compiling all successfully, you can run `./catch`  or ` ./i0gan`  directly to test
Store the interactive log file in the directory `/tmp/.i0gan/` , The name of file format is  `time + hex microseconds + .i0gan`



### 0x04

Upload the compiled `catch/i0gan/forward`  program file to the `/tmp` directory. The `pwn service paths` stored in different events may be different. Take `/pwn/pwn` as an example. If you don't know where it is, `cat /etc/xinetd.d/pwn` service can view its own service path.  Copy the monitored service program to `LOG_PATH` directory, and replace the service program with WAF program

```
mkdir /tmp/.i0gan          # Create a directory .i0gan in /tmp
chmod 777 /tmp/.i0gan      # Modify permissions
cp /pwn/pwn /tmp/.i0gan    # Copy service binary program to /tmp/.i0gan dirctory
cp /tmp/catch /pwn/pwn # Replace your service binary program
```

If the attacker attacks, the corresponding attack log file will be generated in the directory `/tmp/.i0gan/`. Each attack will generate a file, which can be directly analyzed after being attacked


## How to use FORWARD MODE

Some configuration information of this mode is in makefile

```
# for forward mode
SERVER_IP   := 127.0.0.1
# for forward mode
SERVER_PORT := 8080

```


## RUN_CATCH MODE Test

### Interaction

```
┌[logan☮arch]-(~/disk2/github/i0gan_waf)-[git://main ✗]-
└> ./catch 
Test puts:
Test write�
Test read:
AAAABBBB
Test gets:

CCCCDDDD
Test system:

sh-5.0$ ls
catch  forward  i0gan  makefile  README.md  src  test_pwn  waf.pro  waf.pro.user
sh-5.0$ exit
exit
```



### Log

```
// Date: 2020-12-22 00:24:08
// Mode: RUN_CATCH
// CTF AWD I0GAN WAF
// Powered By I0gan

<-------------------- write ----------------->
Test puts:
Test write�
Test read:

w_0 = "\x54\x65\x73\x74\x20\x70\x75\x74\x73\x3a\x0a\x54\x65\x73\x74\x20\x77\x72\x69\x74\x65\x00\x01\x02\x03\xff\x0a\x54\x65\x73\x74\x20\x72\x65\x61\x64\x3a\x0a"

<-------------------- read ------------------>
AAAABBBB

r_0 = "\x41\x41\x41\x41\x42\x42\x42\x42\x0a"

<-------------------- write ----------------->
Test gets:


w_1 = "\x54\x65\x73\x74\x20\x67\x65\x74\x73\x3a\x0a\x0a"

<-------------------- read ------------------>
CCCCDDDD

r_1 = "\x43\x43\x43\x43\x44\x44\x44\x44\x0a"

<-------------------- write ----------------->
Test system:


<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
w_2 = "\x54\x65\x73\x74\x20\x73\x79\x73\x74\x65\x6d\x3a\x0a\x0a"
```

If there is a string of words "-------------- dangerous syscall------------", basically your server has been hacked!





## RUN_I0GAN MODE Test

### Interaction

```
┌[logan☮arch]-(~/disk2/github/i0gan_waf)-[git://main ✗]-
└> ./i0gan 
Test puts:
Test write�
Test read:
aaajaksdjf   
Test gets:

asjdfjadsfasd
Test system:

ls
jasjjd
^C
sh: initialize_job_control: no job control in background: Bad file descriptor 
```

### Log

```
// Date: 2020-12-22 00:25:55
// Mode: RUN_I0GAN
// CTF AWD I0GAN WAF
// Powered By I0gan

<-------------------- write ----------------->
Test puts:
Test write�
Test read:

w_0 = "\x54\x65\x73\x74\x20\x70\x75\x74\x73\x3a\x0a\x54\x65\x73\x74\x20\x77\x72\x69\x74\x65\x00\x01\x02\x03\xff\x0a\x54\x65\x73\x74\x20\x72\x65\x61\x64\x3a\x0a"

<-------------------- read ------------------>
aaajaksdjf

r_0 = "\x61\x61\x61\x6a\x61\x6b\x73\x64\x6a\x66\x0a"

<-------------------- write ----------------->
Test gets:


w_1 = "\x54\x65\x73\x74\x20\x67\x65\x74\x73\x3a\x0a\x0a"

<-------------------- read ------------------>
asjdfjadsfasd

r_1 = "\x61\x73\x6a\x64\x66\x6a\x61\x64\x73\x66\x61\x73\x64\x0a"

<-------------------- write ----------------->
Test system:


<-------------- dangerous syscall------------>
w_2 = "\x54\x65\x73\x74\x20\x73\x79\x73\x74\x65\x6d\x3a\x0a\x0a"
```



### RUN_FORWARD MODE Test

### Interaction

```
┌[logan☮arch]-(~/disk2/github/i0gan_waf)-[git://main ✗]-
└> ./forward 
asdfjadsf
HTTP/1.1 400 Bad Request
Content-Type: text/plain; charset=utf-8
Connection: close

400 Bad Request
```

### Log

```
// Date: 2020-12-22 00:27:18
// Mode: RUN_FORWARD
// CTF AWD I0GAN WAF
// Powered By I0gan

<-------------------- read ------------------>
asdfjadsf

r_0 = "\x61\x73\x64\x66\x6a\x61\x64\x73\x66\x0a"

<-------------------- write ----------------->
HTTP/1.1 400 Bad Request
Content-Type: text/plain; charset=utf-8
Connection: close

400 Bad Request
w_0 = "\x48\x54\x54\x50\x2f\x31\x2e\x31\x20\x34\x30\x30\x20\x42\x61\x64\x20\x52\x65\x71\x75\x65\x73\x74\x0d\x0a\x43\x6f\x6e\x74\x65\x6e\x74\x2d\x54\x79\x70\x65\x3a\x20\x74\x65\x78\x74\x2f\x70\x6c\x61\x69\x6e\x3b\x20\x63\x68\x61\x72\x73\x65\x74\x3d\x75\x74\x66\x2d\x38\x0d\x0a\x43\x6f\x6e\x6e\x65\x63\x74\x69\x6f\x6e\x3a\x20\x63\x6c\x6f\x73\x65\x0d\x0a\x0d\x0a\x34\x30\x30\x20\x42\x61\x64\x20\x52\x65\x71\x75\x65\x73\x74"
```







## Example RUN_CATCH

env: 

our server [ chall 127.0.0.1 8080, ssh 127.0.0.1 8022]

attacker



### Step1 

Connect your server use ssh.



### Step2

Check your server program path

```
root@efff8f877f79:/# cat /etc/xinetd.d/pwn
```

output

```
service pwn
{
    disable = no
    socket_type = stream
    protocol    = tcp
    wait        = no
    user        = pwn
    type        = UNLISTED
    port        = 80
    bind        = 0.0.0.0
    server      = /pwn
    server_args = none
    # safety options
    per_source  = 10 # the maximum instances of this service per source IP address
    rlimit_cpu  = 20 # the maximum number of CPU seconds that the service may use
    rlimit_as  = 256M # the Address Space resource limit for the service
    #access_times = 8:50-17:10
}

```

So we find listened program is `/pwn`,  so we just replace this program as our waf.

```
mkdir /tmp/.i0gan
chmod 777 /tmp/.i0gan
cp /pwn /tmp/.i0gan/pwn
```

upload catch file to server and replace /pwn to our waf

```
cp catch /pwn
```



Now attacker connect to this server

```
  ____ _   _ ___ _____
 / ___| | | |_ _|_   _|
| |   | | | || |  | |  
| |___| |_| || |  | |  
 \____|\___/|___| |_| 

CUIT 2021 Experimental Class Exam [PWN 1 calc error]

Why you wanna enter experimental class?

asdfasdfasdfasdfasdf
Ok, Now you can input two number to calc, Can you make an error?
asdfasdf
Invalid!
```



Check our server log

```
root@efff8f877f79:/# cat /tmp/.i0gan/15_04_25_b51a7.i0gan 
// Date: 2021-06-19 15:04:25
// Mode: RUN_CATCH
// CTF AWD PWN WAF
// Powered By I0gan

<-------------------- write ----------------->
  ____ _   _ ___ _____
 / ___| | | |_ _|_   _|
| |   | | | || |  | |  
| |___| |_| || |  | |  
 \____|\___/|___| |_| 

CUIT 2021 Experimental Class Exam [PWN 1 calc error]

Why you wanna enter experimental class?


w_0 = "\x20\x20\x5f\x5f\x5f\x5f\x20\x5f\x20\x20\x20\x5f\x20\x5f\x5f\x5f\x20\x5f\x5f\x5f\x5f\x5f\x0a\x20\x2f\x20\x5f\x5f\x5f\x7c\x20\x7c\x20\x7c\x20\x7c\x5f\x20\x5f\x7c\x5f\x20\x20\x20\x5f\x7c\x0a\x7c\x20\x7c\x20\x20\x20\x7c\x20\x7c\x20\x7c\x20\x7c\x7c\x20\x7c\x20\x20\x7c\x20\x7c\x20\x20\x0a\x7c\x20\x7c\x5f\x5f\x5f\x7c\x20\x7c\x5f\x7c\x20\x7c\x7c\x20\x7c\x20\x20\x7c\x20\x7c\x20\x20\x0a\x20\x5c\x5f\x5f\x5f\x5f\x7c\x5c\x5f\x5f\x5f\x2f\x7c\x5f\x5f\x5f\x7c\x20\x7c\x5f\x7c\x20\x0a\x0a\x43\x55\x49\x54\x20\x32\x30\x32\x31\x20\x45\x78\x70\x65\x72\x69\x6d\x65\x6e\x74\x61\x6c\x20\x43\x6c\x61\x73\x73\x20\x45\x78\x61\x6d\x20\x5b\x50\x57\x4e\x20\x31\x20\x63\x61\x6c\x63\x20\x65\x72\x72\x6f\x72\x5d\x0a\x0a\x57\x68\x79\x20\x79\x6f\x75\x20\x77\x61\x6e\x6e\x61\x20\x65\x6e\x74\x65\x72\x20\x65\x78\x70\x65\x72\x69\x6d\x65\x6e\x74\x61\x6c\x20\x63\x6c\x61\x73\x73\x3f\x0a\x0a"

<-------------------- read ------------------>
asdfasdfasdfasdfasdf

r_0 = "\x61\x73\x64\x66\x61\x73\x64\x66\x61\x73\x64\x66\x61\x73\x64\x66\x61\x73\x64\x66\x0a"

<-------------------- write ----------------->
Ok, Now you can input two number to calc, Can you make an error?

w_1 = "\x4f\x6b\x2c\x20\x4e\x6f\x77\x20\x79\x6f\x75\x20\x63\x61\x6e\x20\x69\x6e\x70\x75\x74\x20\x74\x77\x6f\x20\x6e\x75\x6d\x62\x65\x72\x20\x74\x6f\x20\x63\x61\x6c\x63\x2c\x20\x43\x61\x6e\x20\x79\x6f\x75\x20\x6d\x61\x6b\x65\x20\x61\x6e\x20\x65\x72\x72\x6f\x72\x3f\x0a"

<-------------------- read ------------------>

<-------------------- write ----------------->
```



## Example RUN_I0GAN







## Example RUN_FORWARD







