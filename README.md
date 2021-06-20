# AWD PWN WAF

## Intro

CTF AWD  WAF FOR PWN

Author: I0gan

QQ: 418894113



The WAF is a traffic capture tool for AWD PWN, which is convenient to analyze and write anti attack exp, and is very conducive to the use of `PWN Ctfer`. The tool has been used in AWD competitions for many times. I hope you can `give me a star`. The WAF has `four modes`. 

1. The `CATCH` mode simply capture the attacked interactive traffic, which can be viewed under the  `log path`. 

2. The `I0GAN` mode is a `defense mode`, which can prevent attackers from getting the shell, and can also view the attacker's interactive traffic. This mode is used cautiously, strictly abides by the rules of the ctf competition. If you violates the rules of the competition. The consequences are borne by yourslves. 

3. The `FORWARD` mode simply forwards the attacker's traffic to hit others, and we can capture the traffic in the middle. Of course, if the attacker successfully gets the flag, We can also get the flag in the log file. 

4. The `FORWARD_MUTIL` is based on `FORWARD` mode, it mainly loop get victims's host  information from `hosts.txt` file then forward attacker's traffic to victims.

   

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
cp /pwn/pwn /tmp/.waf    # Copy service binary program to /tmp/.i0gan dirctory
cp /tmp/catch /pwn/pwn # Replace your service binary program to our waf
```

If the attacker attacks, the corresponding attack log file will be generated in the directory `/tmp/.i0gan/`. Each attack will generate a file, which can be directly analyzed after being attacked

#### FOWARD mode

Upload the compiled ` forward` program file to the `/tmp` directory.

 If your server have not permissions to  store log, you should Close log, just set  `LOG_METHOD  := OPEN` in makefile and recompile forward.

1. Replace the service program with WAF program

```
cp /tmp/forward /pwn/pwn # Replace your service binary program to our waf
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
```



### 0x04 Test your server

Note that you need to test your service

```
nc your_server_ip your_server_port
```

The common problem is that the permissions are not enough, which  appear in the `log path` without permissions.



## Test CATCH

```
[i0gan@arch pwn_waf]$ ./catch 
Test puts:
Test write�
Test read:
aabb
Test gets:

dddddaf
Test system:

sh-5.1$ ls
catch  forward  forward_multi  hosts.txt  i0gan  makefile  README.md  src  test_pwn
sh-5.1$ exit
exit
```

log

```
```





## Test I0GAN







## Test FORWARD





## Test FORWARD_MULTI







