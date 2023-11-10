# AWD PWN WAF

[中文版](./README_ZH.md)

CTF AWD  WAF FOR PWN

Author: I0gan

Mail: i0gan@pwnsky.com

## Intro

The WAF is a traffic capture tool for AWD PWN, which is convenient to analyze and write anti attack exp, and is very conducive to the use of `PWN Ctfer`. The tool has been used in AWD competitions for many times. I hope you can `give me a star`. The WAF has `four modes`. 

1. The `CATCH` mode simply capture the attacked interactive traffic, which can be viewed under the  `log path`. 

2. The `I0GAN` mode is a `defense mode`, which can prevent attackers from getting the shell, and can also view the attacker's interactive traffic. This mode is used cautiously, strictly abides by the rules of the ctf competition. If you violates the rules of the competition. The consequences are borne by yourslves. 

3. The `FORWARD` [Recommend] mode simply forwards the attacker's traffic to hit others, and we can capture the traffic in the middle. Of course, if the attacker successfully gets the flag, We can also get the flag in the log file. 

4. The `FORWARD_MUTIL` is based on `FORWARD` mode, it mainly loop get victims's host  information from `hosts.txt` file then forward attacker's traffic to victims.

## Contributor  

```
    i0gan: Code writer
    b0ldfrev: Commit IO blocking bug
    moon: Commit Chinese Document
```

## Fix Log
    2021-05-13: Thancks b0ldfrev commit the IO blocking problem for FORWARD mode 



## CATCH / I0GAN modes waf principle

Execve the target elf file by creating a child process, and then the parent process uses ptrace to monitor the syscall  of the child process. If the standard IO is used, the data  is read and recorded in the log. If the syscall is dangerous, it is also recorded in the log



## FORWARD / FORWARD_MULTI modes waf principle

Capture traffic from standard I / O and forward it to the target server. You can captrue traffic between the attacker and the victim, and you can also get victim's flag.




## Code Tree

```
src
├── logger.c
├── logger.h
├── test_pwn.c
├── waf.c
└── waf.h
```



##  How to use?

### 0x00 Know your server

After ssh to your server, checking your server for monitored program.

```
 cat /etc/xinetd.d/pwn
 
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

So monitored program is `/pwn`.

If the server program is `chroot`, you should notice something, like this

```
service pwn
{
    disable = no
    flags = REUSE
    socket_type = stream
    protocol    = tcp
    wait        = no
    user        = root
    type        = UNLISTED
    port        = 80
    
    bind        = 0.0.0.0
    server      = /usr/sbin/chroot   
    server_args = --userspec=1000:1000 /home/pwn ./pwn
    # safety options
    per_source  = 5 # the maximum instances of this service per source IP address
    rlimit_cpu  = 20 # the maximum number of CPU seconds that the service may use
    rlimit_as  = 100M # the Address Space resource limit for the service
    #access_times = 8:50-17:10
}
```

Like this situation, the monitored program is `/home/pwn/pwn`

Next, we should found a path that have read && write permissions to `pwn` user.

Use `touch` command to test then find this path.

In general, this condition will be satisfied in the `/tmp` directory. But if server program is `chroot` you should find this path in `chroot args1` [/home/pwn].

If not found path, this waf just can be used in `RUN_FORWARD` mode.



replace this program.

You should replace  /home/pwn/pwn to our waf program.



### 0x01 Configure waf

Some configuration information of all modes is in `makefile`

#### CATCH and I0GAN mode configure

In makefile

```
# configure
# log path
LOG_METHOD  := OPEN
LOG_PATH    := /tmp/.waf
ARCH        := 64
```

 `LOG_PATH` is a log file path

`ARCH` Represents whether the program is 32-bit or 64 bit. 

You shold set `LOG_PATH` to your path that have  read && write permissions in `pwn` user.

#### FORWARD mode configure

```
# configure
# log path
LOG_METHOD  := OPEN
LOG_PATH    := /tmp/.waf
ARCH        := 64

# Just used in FORWARD mode
FORWARD_IP   := 127.0.0.1
FORWARD_PORT := 20000
```

 `LOG_PATH` is a log file path

`ARCH` Represents whether the program is 32-bit or 64 bit. 

`FORWARD_IP` is static victim's ip

`FORWARD_PORT` is static victim's port

You shold set `LOG_PATH` to your path that have  read && write permissions in `pwn` user.

If you not have permissions to write anaything, You should close logger. Modify makefile

```
LOG_METHOD  := CLOSE
```

So this mode is not rely on other file. you can use this mode to avoid attacked from attacker.

#### FOWARD_MULTI mode configure

If you use `RUN_FORWARD_MULTI` mode, you must set hosts information in hosts.txt, like

hosts.txt

```
127.0.0.1:20000
127.0.0.1:8081
```

And set log path in `makefile`

```
# configure
# log path
LOG_PATH    := /tmp/.waf
ARCH        := 64
```

 `LOG_PATH` is a log file path

`ARCH` Represents whether the program is 32-bit or 64 bit. 

You shold set `LOG_PATH` to your path that have  read && write permissions in `pwn` user.



### 0x02 Compile

#### Compile all

#### Notice

    Compile Env glibc version must <= target glibc version
    Or waf cannot execute
```
./i0gan: /lib/x86_64-linux-gnu/libc.so.6: version `GLIBC_2.33' not found (required by ./i0gan)
 
```


```
make # or make all
```

or

```
make catch
make i0gan
make forward
make forward_multi
```



You will get `catch`,`i0gan`,`redir` files

The `catch` is `RUN_CATCH` mode waf program

The `i0gan` is `RUN_I0GAN` mode waf program

The `forward` is `RUN_FORWARD` mode waf program

The `forward_mutil` is `RUN_FORWARD_MULTI` mode waf program

#### simple test

After compiling all successfully, you can run `./catch`  or ` ./i0gan`  directly to test
Store the interactive log file in the directory `/tmp/.waf/` , The name of file format is  `time + hex microseconds + .log`

### 0x03 Add waf to server

The `pwn service paths` stored in different events may be different. Take `/pwn/pwn` as an example. If you don't know where it is, `cat /etc/xinetd.d/pwn` service can view its own service path. The `LOG_PATH` you should find by yourself that have `read && write` permissions in `pwn` user. Here take `LOG_PATH` is `/tmp/.waf` as an example.

#### CATCH or I0GAN mode

Upload the compiled `catch` or `i0gan`  program file to the `/tmp` directory.

1. Copy the monitored service program to `LOG_PATH` directory.

2. Replace the service program with WAF program

```
mkdir /tmp/.waf          # Create LOG_PATH directory
chmod 777 /tmp/.waf      # Modify permissions
cp /pwn/pwn /tmp/.waf    # Move service binary program to /tmp/.i0gan dirctory
chown pwn:pwn /tmp/.waf/pwn # Chown as pwn
chmod 777 /tmp/.waf/pwn
mv /tmp/catch /pwn/pwn # Replace your service binary program to our waf
chmod 777 /pwn/pwn
```


Notice:
    You must use nc to test. If server is down, checking your listened pwn program's permission and log permission.
    If your program is orw type, open fd start with 6, not 3

If the attacker attacks, the corresponding attack log file will be generated in the directory `/tmp/.i0gan/`. Each attack will generate a file, which can be directly analyzed after being attacked

#### FOWARD mode

Upload the compiled ` forward` program file to the `/tmp` directory.

 If your server have not permissions to  store log, you should Close log, just set  `LOG_METHOD  := OPEN` in makefile and recompile forward.

1. Replace the service program with WAF program

```
cp /tmp/forward /pwn/pwn # Replace your service binary program to our waf
chmod 777 /pwn/pwn
```



#### FOWARD_MULTI mode

Upload the compiled ` forward_multi`  and `hosts.txt` files to the `/tmp` directory.

1. Copy `hosts.txt` under `LOG_PATH`
2. Replace the service program with WAF program

```
mkdir /tmp/.waf          # Create LOG_PATH directory
chmod 777 /tmp/.waf      # Modify permissions
cp /tmp/hosts.txt /tmp/.waf # Upload hostes.txt under LOG_PATH
cp /tmp/forward_multi /pwn/pwn # Replace your service binary program to our waf
chmod 777 /pwn/pwn
```

Notice:
    You must use nc to test. Check your log permission


### 0x04 Test your server

Note that you need to test your service

```
nc your_server_ip your_server_port
```

The common problem is that the permissions are not enough, which  appear in the `log path` without permissions.



## Test CATCH

server

```
socat TCP-LISTEN:1234,reuseaddr,fork EXEC:./catch 
```

attacker

```
nc 127.0.0.1 1234
Test puts:
Test write�
Test read:
abcasdfasdf
Test gets:

asdfadf
Test get shell:

ls
README.md
catch
forward
forward_multi
hosts.txt
i0gan
makefile
src
test
test_pwn
exit 
```



server log

```
cat /tmp/.waf/14_32_43_6740c.log
```



```
// Date: 2021-06-20 14:32:43
// Mode: CATCH
// CTF AWD PWN WAF
// Deved By I0gan

<-------------------- write ----------------->
Test puts:
Test write�
Test read:

w_0 = "\x54\x65\x73\x74\x20\x70\x75\x74\x73\x3a\x0a\x54\x65\x73\x74\x20\x77\x72\x69\x74\x65\x00\x01\x02\x03\xff\x0a\x54\x65\x73\x74\x20\x72\x65\x61\x64\x3a\x0a"

<-------------------- read ------------------>
abcasdfasdf

r_0 = "\x61\x62\x63\x61\x73\x64\x66\x61\x73\x64\x66\x0a"

<-------------------- write ----------------->
Test gets:


w_1 = "\x54\x65\x73\x74\x20\x67\x65\x74\x73\x3a\x0a\x0a"

<-------------------- read ------------------>
asdfadf

r_1 = "\x61\x73\x64\x66\x61\x64\x66\x0a"

<-------------------- write ----------------->
Test get shell:


<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>
w_2 = "\x54\x65\x73\x74\x20\x67\x65\x74\x20\x73\x68\x65\x6c\x6c\x3a\x0a\x0a"

<-------------------- read ------------------>
ls

<-------------- dangerous syscall------------>
<-------------- dangerous syscall------------>exit

r_2 = "\x6c\x73\x0a\x65\x78\x69\x74\x0a"
```





## Test I0GAN

server

```
socat TCP-LISTEN:1234,reuseaddr,fork EXEC:./i0gan
```



attacker

```
nc 127.0.0.1 1234
Test puts:
Test write�
Test read:
asdfadsf
Test gets:

asdfasdf
Test get shell:

```



server log

```
cat /tmp/.waf/14_37_22_5159c.log
```



```
// Date: 2021-06-20 14:37:22
// Mode: I0GAN
// CTF AWD PWN WAF
// Deved By I0gan

<-------------------- write ----------------->
Test puts:
Test write�
Test read:

w_0 = "\x54\x65\x73\x74\x20\x70\x75\x74\x73\x3a\x0a\x54\x65\x73\x74\x20\x77\x72\x69\x74\x65\x00\x01\x02\x03\xff\x0a\x54\x65\x73\x74\x20\x72\x65\x61\x64\x3a\x0a"

<-------------------- read ------------------>
asdfadsf

r_0 = "\x61\x73\x64\x66\x61\x64\x73\x66\x0a"

<-------------------- write ----------------->
Test gets:


w_1 = "\x54\x65\x73\x74\x20\x67\x65\x74\x73\x3a\x0a\x0a"

<-------------------- read ------------------>
asdfasdf

r_1 = "\x61\x73\x64\x66\x61\x73\x64\x66\x0a"

<-------------------- write ----------------->
Test get shell:


<-------------- dangerous syscall------------>
AVOID

w_2 = "\x54\x65\x73\x74\x20\x67\x65\x74\x20\x73\x68\x65\x6c\x6c\x3a\x0a\x0a"
```



## Test FORWARD

victim's ip is 127.0.0.1, port is 20000

test nc

```
nc 127.0.0.1 20000
  ____ _   _ ___ _____
 / ___| | | |_ _|_   _|
| |   | | | || |  | |  
| |___| |_| || |  | |  
 \____|\___/|___| |_| 

CUIT 2021 Experimental Class Exam [PWN 1 calc error]

Why you wanna enter experimental class?

asdf
Ok, Now you can input two number to calc, Can you make an error?
asdf
Invalid!
```



In makefile 

```
FORWARD_IP   := 127.0.0.1
FORWARD_PORT := 20000
```

Compile forward

Server

```
socat TCP-LISTEN:1234,reuseaddr,fork EXEC:./forward
```

attacker

```
nc 127.0.0.1 1234
  ____ _   _ ___ _____
 / ___| | | |_ _|_   _|
| |   | | | || |  | |  
| |___| |_| || |  | |  
 \____|\___/|___| |_| 

CUIT 2021 Experimental Class Exam [PWN 1 calc error]

Why you wanna enter experimental class?

asfasdfasdf
Ok, Now you can input two number to calc, Can you make an error?
asdf
Invalid!

```



server log

```
cat /tmp/.waf/14_41_28_81a62.log
```



```
// Date: 2021-06-20 14:41:28
// Mode: FORWARD
// CTF AWD PWN WAF
// Deved By I0gan

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
asfasdfasdf

r_0 = "\x61\x73\x66\x61\x73\x64\x66\x61\x73\x64\x66\x0a"

<-------------------- write ----------------->
Ok, Now you can input two number to calc, Can you make an error?

w_1 = "\x4f\x6b\x2c\x20\x4e\x6f\x77\x20\x79\x6f\x75\x20\x63\x61\x6e\x20\x69\x6e\x70\x75\x74\x20\x74\x77\x6f\x20\x6e\x75\x6d\x62\x65\x72\x20\x74\x6f\x20\x63\x61\x6c\x63\x2c\x20\x43\x61\x6e\x20\x79\x6f\x75\x20\x6d\x61\x6b\x65\x20\x61\x6e\x20\x65\x72\x72\x6f\x72\x3f\x0a"

<-------------------- read ------------------>
asdf

r_1 = "\x61\x73\x64\x66\x0a"

<-------------------- write ----------------->
Invalid!

w_2 = "\x49\x6e\x76\x61\x6c\x69\x64\x21\x0a"

```







## Test FORWARD_MULTI

In hosts.txt

```
127.0.0.1:20000
127.0.0.1:8081
```

The 8081 port is not open.

Server 

```
socat TCP-LISTEN:1234,reuseaddr,fork EXEC:./forward_multi 
```



Attacker:

```
[i0gan@arch ~]$ nc 127.0.0.1 1234
  ____ _   _ ___ _____
 / ___| | | |_ _|_   _|
| |   | | | || |  | |  
| |___| |_| || |  | |  
 \____|\___/|___| |_| 

CUIT 2021 Experimental Class Exam [PWN 1 calc error]

Why you wanna enter experimental class?

asdfasdf
Ok, Now you can input two number to calc, Can you make an error?
asdf
Invalid!

[i0gan@arch ~]$ nc 127.0.0.1 1234
127.0.0.1

[i0gan@arch ~]$ nc 127.0.0.1 1234
  ____ _   _ ___ _____
 / ___| | | |_ _|_   _|
| |   | | | || |  | |  
| |___| |_| || |  | |  
 \____|\___/|___| |_| 

CUIT 2021 Experimental Class Exam [PWN 1 calc error]

Why you wanna enter experimental class?

asdfasdf
Ok, Now you can input two number to calc, Can you make an error?
asdf
Invalid!
```

The server listener log

```
socat TCP-LISTEN:1234,reuseaddr,fork EXEC:./forward_multi
connect: Connection refused
2021/06/20 14:49:48 socat[31889] E waitpid(): child 31890 exited with status 111
connect: Connection refused
2021/06/20 14:50:02 socat[31919] E waitpid(): child 31920 exited with status 111
connect: Connection refused
2021/06/20 14:50:07 socat[31928] E waitpid(): child 31929 exited with status 111
```

