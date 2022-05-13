
## CTF AWD WAF FOR PWN 

github仓库地址：https://github.com/I0gan/pwn_waf

作者：I0gan

翻译：moon、i0gan

邮箱：i0gan@mail.pwnsky.com 

### 简介

PWN WAF是AWD PWN的流量抓包工具，方便分析和编写反攻击exp，非常有利于PWN Ctfer的使用。该工具已在 AWD 比赛中多次使用。给我一颗星是最大对我的支持。 WAF 有四种模式。 CATCH模式只是简单的捕获被攻击的交互流量，可以在日志路径下查看。 I0GAN模式是一种防御模式，可以防止攻击者拿到shell，也可以查看攻击者的交互流量。此模式使用谨慎，请遵守 ctf 竞赛规则。如果您违反了比赛规则。后果由你们自己承担。 FORWARD模式只是简单的转发攻击者的流量去打别人，我们可以在中间过程抓到攻击者的流量。当然，如果攻击者成功获取到flag，我们也可以在日志文件中获取到flag。 FORWARD_MUTIL 是基于 FORWARD 模式的，它主要从 hosts.txt 文件中循环获取受害者的主机信息，然后将攻击者的流量转发给受害者。



pwn_waf是一套用于在AWD的PWN攻防中使用的流量捕获工具，便于在PWN对抗中进行分析和编写对应脚本。这套工具共有四个模块，或者说这个工具有4种模式。

1、CATCH 程序/模式，捕获模式，只捕获PWN被攻击时产生的交互流量，并保存在指定的日志文件夹（默认为/tmp/.waf/，可以通过在编译前修改makefile文件进行更改）里。日志格式命名规则为：攻击发生的时间（本地系统时间）+ 微妙的hex值 + .log后缀
2、I0GAN 程序/模式，防守模式，可以阻止攻击者通过PWN程序获取SHELL，还可以通过保存的日志查看攻击流量（当前版本，攻击被拦截后，日志文件内仅有“AVOID”标识，无法看到本次攻击的payload信息。）
3、FORWARD 程序/模式，单目标转发模式，将攻击者对自己的流量转发到指定地址的指定端口（需要在编译FORWAR程序时，修改makefile文件），如果攻击者获得了被转发目标的flag，可以在本地保存的日志中查看到。
4、FORWARD_MUTIL 程序/模式 多目标转发模式，基于forward程序，从hosts.txt文件（默认情况下，需要放置在日志文件夹里，格式为“IP:PORT”为一行）中循环读取被转发的目标，将攻击者流量转发至这些目标。

### 原理

捕获和防守模式：

通过创建一个子进程来执行被防护的pwn程序，父进程使用ptrace监控子进程的系统调用。如果调用了系统I/O接口，则读取对应的数据，并保存到日志文件中。如果发现了危险的系统调用，也会记录到日志文件中。

单目标及多目标转发模式：

从标准I/O接口捕获流量，转发至对应目标。可以通过日志文件查看攻击者和被转发目标之间的通信，还可以看到攻击者成功获取的flag信息。

### 配置说明

1、找到自己要防守的程序
登录自己要防守的机器，可以使用cat /etc/xinetd.d/pwn 命令查找（该方法并不通用），可以通过server字段找到pwn程序路径。

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



如果你的结果回显如下，server字段显示为“chroot”程序，则需要查看“server_args”参数，在该示例中，要防守的程序路径为：/home/pwn/pwn

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

可以通过touch命令，核对找到的文件路径是否正确。

2、寻找一个当前用户拥有读写权限的目录，通常可以使用 /tmp 目录。如果寻找不到这类目录，则只能使用 转发模式。

3、修改配置文件，编译程序。打开makefile文件进行配置的修改。

捕获及防守模式：
```
# 配置
# log path
LOG_METHOD  := OPEN   # 是否生成log文件，如果找不到拥有写权限的目录，可以使用“CLOSE”进行关闭
LOG_PATH    := /tmp/.waf  # 日志文件路径，注意使用找到的当前用户拥有读写权限的目录替换
ARCH        := 64  # 被防守程序是32位/64位的，（可以使用  file /pwn 查看）
```



转发模式：

主要配置如下两个选项：

```
# Just used in FORWARD mode
FORWARD_IP   := 127.0.0.1  # 要将流量转发到的目标地址
FORWARD_PORT := 20000      # 目标地址的业务端口地址，一般是将其转发到目标pwn程序监听的端口
```

如果是多目标转发模式，需要将目标地址（IP:PORT）写入hosts.txt文件，放置在配置的文件夹里面。



### 编译

将四个模块全部编译生成：

```
make # or make all
```



或者单独生成某个模块：

```
make catch
make i0gan
make forward
make forward_multi
```





### 运行使用

编译完成后，可以使用 ./catch  ./i0gan 运行程序进行测试，

以/pwn 文件为要防守的程序，当前用户拥有/tmp 目录 为例：
将 catch/i0gan/forward/forward_multi 程序上传到/tmp 目录，如果要进行批量的转发，则将hosts.txt文件也上传。

```
mkdir /tmp/.waf        # 创建一个目录，注意要和编译时的配置文件一致。
chmod 777 /tmp/.waf     # 修改对应的权限
cp /pwn /tmp/.waf      # 将要防护的pwn文件，复制到对应的目录
cp /tmp/catch /pwn      # 用catch 或者 i0gan 、 forward 替换原始的pwn文件
cp /tmp/hosts.txt /tmp/.waf/hosts.txt  # 将多目标防护的对象文件放到正确位置
```



