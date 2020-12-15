# AWD PWN WAF



## WAF Principle

Execve the target elf file by creating a child process, and then the parent process uses ptrace to monitor the syscall  of the child process. If the standard IO is used, the data  is read and recorded in the log. If the syscall is dangerous, it is also recorded in the log



## Code Tree

```
src
├── hex.c       [print file data as hexadecimal string]
├── i0gan_waf.c [WAF program source code]
└── Test.c      [test source code]
```



##  How to use

### 0x01

Compile first

```
make
```

The`i0gan_waf` and `hex` files are compiled, and the test program is `/tmp/.i0gan/pwn`
`i0gan_waf` is a waf program, which is used to grab the standard input and output data of`/tmp/.i0gan/pwn `program
`hex`  program is used to print file data in hexadecimal strings



### 0x02

After compiling successfully, you can run `./i0gan_waf`  directly to test
Store the interactive log file in the directory `/tmp/.i0gan/` , The name of file format is  `time + timestamp + .i0gan`



### 0x03

Upload the compiled `i0gan_waf`  file to the `/tmp` directory. The `pwn service paths` stored in different events may be different. Take `/pwn/pwn` as an example. If you don't know where it is, `cat /etc/xinetd.d/pwn` service can view its own service path. First create a `.i0gan` directory in the `/tmp` directory. If you don't want to create it yourself, run the WAF program `(/tmp/i0gan_waf)`  directly on the server to create it automatically. Copy the monitored service program to `/tmp/.i0gan` directory, and replace the service program with WAF program

```
mkdir /tmp/.i0gan          # Create a directory .i0gan in /tmp
cp /pwn/pwn /tmp/.i0gan    # Copy service binary program to /tmp/.i0gan dirctory
cp /tmp/i0gan_waf /pwn/pwn # Replace your service binary program
```

If the attacker attacks, the corresponding attack log file will be generated in the directory `/tmp/.i0gan/`. Each attack will generate a file, which can be directly analyzed after being attacked



## Other use

### Run mode

There are two operating modes, `RUN_I0GAN_` mode and catch mode. After the `RUN_I0GAN_` mode is running, it will `monitor` all `dangerous  syscall` and `capture` the traffic. If it occurs, it will `exit` the program, while the `RUN_CATCH_` mode will only `monitor` and `capture` the traffic, and will `not exit` the program.

In i0gan_waf.c

```
#define RUN_MODE RUN_I0GAN_          // The mode of RUN_I0GAN_ or RUN_CATCH_
```

Default as `RUN_I0GAN_`  mode



## Test

Here is a test.c program as an example

### Interaction

```
┌[logan☮arch]-(~/disk2/github/i0gan_waf)-[git://main ✗]-
└> ./i0gan_waf 
Test puts:
Test write�
Test read:
i0gan AAAAABBBB
Test gets:

WoW! Cool!
Test system:

sh-5.0$ exit
exit
┌[logan☮arch]-(~/disk2/github/i0gan_waf)-[git://main ✗]-
└> cat /tmp/.i0gan/21_26_37_5fd7680d.i0gan
```



### Log

```
// AWD Pwn Waf
// Powered By I0gan

-------------------- write -----------------
Test puts:
Test write�
Test read:

"\x54\x65\x73\x74\x20\x70\x75\x74\x73\x3a\x0a\x54\x65\x73\x74\x20\x77\x72\x69\x74\x65\x00\x01\x02\x03\xff\x0a\x54\x65\x73\x74\x20\x72\x65\x61\x64\x3a\x0a"
-------------------- read ------------------
i0gan AAAAABBBB

"\x69\x30\x67\x61\x6e\x20\x41\x41\x41\x41\x41\x42\x42\x42\x42\x0a"
-------------------- write -----------------
Test gets:


"\x54\x65\x73\x74\x20\x67\x65\x74\x73\x3a\x0a\x0a"
-------------------- read ------------------
WoW! Cool!

"\x57\x6f\x57\x21\x20\x43\x6f\x6f\x6c\x21\x0a"
-------------------- write -----------------
Test system:


-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
-------------- dangerous syscall------------
"\x54\x65\x73\x74\x20\x73\x79\x73\x74\x65\x6d\x3a\x0a\x0a"
```

If there is a string of words "-------------- dangerous syscall------------", basically your server has been hacked!
