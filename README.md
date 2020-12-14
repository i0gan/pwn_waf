# AWD PWN WAF

[中文](./README_ZN.md)

## WAF principle

Open the target elf by creating a child process, and then the parent process uses ptrace to monitor the syscall call call of the child process. If the standard IO is used, the data in the IO is read and recorded in the log. If the syscall is dangerous, it is also recorded in the log



## Code Tree

**src**

```
| -- hex.c    [print file data as hexadecimal string]
| -- i0gan.c [WAF program source code]
└ -- Test.c   [test code]
```



##  How to use

### 0x01

Compile first

```
make
```

The pwn and hex files are compiled, and the test program is `/tmp/.i0gan/pwn`
pwn is a waf program, which is used to grab the standard input and output data of`/tmp/.i0gan/pwn `program
Hex is used to print file data in hexadecimal strings



### 0x02

After compiling successfully, you can run `./pwn` directly to test
Store the interactive log file in the directory `/tmp/.i0gan/` with the format of time + timestamp. i0gan.



### 0x03

Upload the compiled `pwn` file to the `/tmp` directory. The `pwn service paths` stored in different events may be different. Take `/pwn/pwn` as an example. If you don't know where it is, `cat /etc/xinetd.d/pwn` service can view its own service path. First create a `.i0gan` directory in the `/tmp` directory. If you don't want to create it yourself, run the WAF program `(/tmp/pwn)` directly on the server to create it automatically. Copy the monitored service program to `/tmp/.i0gan` directory, and replace the service program with WAF program

```
cp /pwn/pwn /tmp/.i0gan
cp /tmp/pwn /pwn/pwn
```

If the attacker attacks, the corresponding attack log file will be generated in the directory `/tmp/.i0gan/`. Each attack will generate a file, which can be directly analyzed after being attacked



## Test

Here is a test. C program as an example



### Interaction

```
┌[logan☮arch]-(~/share/template/awd/i0gan_waf)
└> ./pwn 
Test puts:
Test write�
Test read:
AAAA
Test gets:

BBBBADFadsjflaj
Test system:

sh-5.0$ exit
exit
┌[logan☮arch]-(~/share/template/awd/i0gan_waf)
└> cat /tmp/.i0gan/20_45_35_5fd75e6f.i0gan
```



### Log

```
// AWD Pwn Waf
// Powered By I0gan

""
-------------------- write -----------------
Test puts:
Test write�
Test read:

"\x54\x65\x73\x74\x20\x70\x75\x74\x73\x3a\x0a\x54\x65\x73\x74\x20\x77\x72\x69\x74\x65\x00\x01\x02\x03\xff\x0a\x54\x65\x73\x74\x20\x72\x65\x61\x64\x3a\x0a"
-------------------- read ------------------
AAAA

"\x41\x41\x41\x41\x0a"
-------------------- write -----------------
Test gets:


"\x54\x65\x73\x74\x20\x67\x65\x74\x73\x3a\x0a\x0a"
-------------------- read ------------------
BBBBADFadsjflaj

"\x42\x42\x42\x42\x41\x44\x46\x61\x64\x73\x6a\x66\x6c\x61\x6a\x0a"
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
