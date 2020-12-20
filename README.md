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
Store the interactive log file in the directory `/tmp/.i0gan/` , The name of file format is  `time + hex microseconds + .i0gan`



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

Default as `RUN_CATCH_`  mode



## Test

Here is a test.c program as an example

### Interaction

```
┌[logan☮arch]-(~/disk2/github/i0gan_waf)-[git://main ✗]-
└> ./i0gan_waf 
Test puts:
Test write�
Test read:
AAABBB
Test gets:

I0gan'ssss waf
Test system:

ls
whoami 
exit
dddd
^C
sh: initialize_job_control: no job control in background: Bad file descriptor
┌[logan☮arch]-(~/disk2/github/i0gan_waf)-[git://main ✗]-
└> cat /tmp/.i0gan/18_31_43_a55a4.i0gan
```



### Log

```
// Date: 2020-12-15 18:31:43
// Mode: RUN_I0GAN_
// AWD I0GAN WAF
// Powered By I0gan
-------------------- write -----------------
Test puts:
Test write�
Test read:

hex="\x54\x65\x73\x74\x20\x70\x75\x74\x73\x3a\x0a\x54\x65\x73\x74\x20\x77\x72\x69\x74\x65\x00\x01\x02\x03\xff\x0a\x54\x65\x73\x74\x20\x72\x65\x61\x64\x3a\x0a"
-------------------- read ------------------
AAABBB

hex="\x41\x41\x41\x42\x42\x42\x0a"
-------------------- write -----------------
Test gets:


hex="\x54\x65\x73\x74\x20\x67\x65\x74\x73\x3a\x0a\x0a"
-------------------- read ------------------
I0gan'ssss waf

hex="\x49\x30\x67\x61\x6e\x27\x73\x73\x73\x73\x20\x77\x61\x66\x0a"
-------------------- write -----------------
Test system:


-------------- dangerous syscall------------
hex="\x54\x65\x73\x74\x20\x73\x79\x73\x74\x65\x6d\x3a\x0a\x0a"
```

If there is a string of words "-------------- dangerous syscall------------", basically your server has been hacked!
