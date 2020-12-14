# AWD PWN WAF



## WAF原理

使用创建子进程打开目标elf, 然后父进程使用ptrace监测子进程的syscall调用,  若是标准io, 那么读取io中的数据, 记录在log里, 若是危险的syscall, 也记录在log里.



## Code Tree

**src** 
├── hex.c    [将文件数据打印为16进制字符串]
├── i0gan.c [waf程序源代码]
└── test.c    [测试代码]



##  How to use

### 0x01

先编译

```
make
```

编译得到 pwn与hex文件还有测试程序，测试程序为/tmp/.i0gan/pwn

pwn是一个waf程序，用于抓取/tmp/.i0gan/pwn程序标准输入输出的数据

hex用于以16进制字符串打印文件数据



### 0x02

编译成功之后，可以直接运行./pwn来进行测试

在/tmp/.i0gan/目录下储存交互日志文件，文件名称格式为 时间+时间戳.i0gan。



### 0x03

上传编译好的pwn文件到/tmp目录下

不同的举办可能所存放的pwn服务路径不一样，这里以/pwn/pwn为例，若不知道在哪里，cat /etc/xinetd.d/pwn 服务来查看自己的服务路径。

先在/tmp目录下创建一个.i0gan目录，若不想自己创建，直接在服务器上运行该waf程序即可自动创建。

复制自己所被监控的服务程序到/tmp/.i0gan目录下，且将服务程序替换为waf程序

```
cp /pwn/pwn /tmp/.i0gan
cp /tmp/pwn /pwn/pwn
```

若攻击方打入之后，会在tmp/.i0gan/目录下生成相应的攻击日志文件。每次攻击都会生成一个文件，被攻击之后就直接分析日志即可





## Test

这里一test.c程序为例子

### 交互

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



### 日志

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

出现一串-------------- dangerous syscall------------字样的话,  基本上你的服务器已经被打了...
