Stanford CS 144 Networking Lab
==============================

## 环境部署

建议直接使用虚拟机, 为了你方便操作, 建议不要使用你收藏的陈年老古董, 比如 `CentOS7`, `Ubantu` 低版本等

> 这些老版本的镜像所下载的 `cmake` 和 `yum` 源都十分的老旧, 本实验不支持

建议直接在 Ubantu 官网下载最新版的镜像, 我下的是 `Ubantu23` :  [下载Ubuntu桌面系统 | Ubuntu](https://cn.ubuntu.com/download/desktop)

之后便是执行下面的命令用来安装依赖:

```
sudo apt update && sudo apt install git cmake gdb build-essential clang \
 	clang-tidy clang-format gcc-doc pkg-config glibc-doc tcpdump tshark
```

如果你没有报错, 那么恭喜你, 第一部分完成

之后克隆镜像网站, 执行克隆命令

```
git clone https://github.com/CS144/minnow.git
```

> 原仓库地址为 : [CS144/minnow at check7-startercode (github.com)](https://github.com/CS144/minnow/tree/check7-startercode)
>
> 原仓库是分为 $7$ 个分支的, 为了方便实验, 建议你直接下载或克隆最后一个分支
>
> 这样你没完成一个实验就不用合并了

之后进入项目的源文件内, 新建一个 `build` 文件夹

之后在项目根目录中依次执行一下命令

```
cmake -S . -B build
```

```
cmake --build build
```

OK, 恭喜你现在已经成功部署好环境可以正常运行了, 你可以尝试运行一下测试看看能否成功

```
cmake --build build --target check_webget
```

> 某些时候运行会报错, 也不知道为啥, 报错内容为内存泄漏, 目前无解决方法, 多刷几遍就成功了
>
> ```
> root@yyym:~/Stanford-CS144# cmake --build build --target check1
> Test project /root/Stanford-CS144/build
>       Start  1: compile with bug-checkers
>  1/10 Test  #1: compile with bug-checkers ........   Passed    0.95 sec
>       Start  9: reassembler_single
>  2/10 Test  #9: reassembler_single ...............***Exception: SegFault  0.26 sec
> AddressSanitizer:DEADLYSIGNAL
> AddressSanitizer:DEADLYSIGNAL
> AddressSanitizer:DEADLYSIGNAL
> AddressSanitizer:DEADLYSIGNAL
> AddressSanitizer:DEADLYSIGNAL
> AddressSanitizer:DEADLYSIGNAL
> ```



---

## Lab 概述

Lab 总共有 $7$ 个部分,  逐步实现计算机网络中的核心部分, 现在将各个 Lab 内容总结如下

* **Lab0**

  > **Part I :**  完成 `webget.cc` , 实现与 `telnet` 一样的功能
  >
  > **Part II :**  完成一个字节流类 `byte_stream`, 用于传输数据

* **Lab1**

  > 实现一个流重组器 `Reassembler` , 用来重新排序乱序到来的数据

* **Lab2**

  > **Part I :** 实现一个工具类 `Wrap32` 以实现 TCP 数据报中的 seq 和 absolute seq 的相互转换
  >
  > **Part II :** 实现 TCP 接收方的相关函数和代码 `TCPReceiver`

* **Lab3**

  > 实现 TCP 发送方的相关代码 `TCPSender`

* **Lab4**

  > **Part I :** 阶段性测试, 检验之前代码是否可正常运行
  >
  > **Part II :** 对 `ping` 命令返回的数据进行分析

* **Lab5**

  > 实现路由选择协议 ARP

* **Lab6**

  > 实现路由表的生成以及记忆功能

* **Lab7**

  > **Part I :** 阶段性测试, 测试之前代码是否正确, 使用外部服务器检验自己的代码是否可以正常发送数据
  >
  > **Part II :** 阶段性测试, 测试使用外部服务器传输大文件是否能保持一致性



---

## Lab 完成时间表

- [x] **Lab1 : 2024-03-21**
- [x] **Lab2 : 2024-04-10**
- [x] **Lab3 : 2024-04-22**
- [x] **Lab4 : 2024-04-23**
- [x] **Lab5 : 2024-04-26**
- [x] **Lab6 : 2024-04-29**
- [x] **Lab7 : 2024-04-30**
