# DNS 服务器实验报告

> by BOT Man, 2017

[heading-numbering]

## TOC [toc-heading] [no-number]

[TOC]

## 系统的功能设计

设计一个 DNS 服务器程序，读入 **IP 地址-域名** _(hosts)_ 对照表，当客户端查询域名对应的 IP 地址时，用域名检索该对照表，有三种可能检索结果：

- 检索结果 IP 地址 `0.0.0.0`，则向客户端返回 **域名不存在** 的报错消息（不良网站拦截功能）
- 检索结果为普通 IP 地址，则向客户端返回该地址（服务器功能）
- 表中未检到该域名，则向因特网 DNS 服务器发出查询，并将结果返给客户端（中继功能）
  - 考虑多个计算机上的客户端会同时查询，需要进行消息 ID 的转换

## 模块划分

程序主要分为 5 个模块：日志模块、配置模块、数据包模块、Socket 模块 和 主程序模块。

### 日志模块 `DnsRelay::Logger`

- 封装 `std::ostream` 的所有操作
- 构造时，加入时间戳
- 析构时，刷空缓冲区（便于日志写入文件）

### 配置模块 `DnsRelay::Config`

- 获取命令行参数提示字符串
- 提出命令行参数配置（全部可选）
  - 日志文件名 (`-o <log-file-name>`)
  - 因特网 DNS 服务器 IP (`-n <name-server-ip>`)
  - **IP 地址-域名** _(hosts)_ 表文件名 (`-h <hosts-file-name>`)
  - 查询因特网服务器超时时间 (`-t <timeout-seconds>`)
- 将所有参数输出到 `std::ostream`

### 数据包模块 `DnsRelay::Packet`

- 定义数据包内容
  - 传输地址
  - 包头段
  - 查询报文段
  - 资源报文段
- 从 Socket 字节流提取数据包
- 将数据包生成 Socket 字节流
- 将数据包内容输出到 `std::ostream`

#### 资源报文段 `DnsRelay::Packet::Resource`

- 针对不同类型的资源，定义不同的解析方式
- 针对不同类型的资源，以不同格式输出到 `std::ostream`

#### 严格数据包 `DnsRelay::StrictPacket`

- 包含两部分：
  - 数据包
  - Socket 字节流
- 仅允许修改数据包的：
  - 传输地址
  - 包头段
- 将严格数据包内容输出到 `std::ostream`

### Socket 模块 `DnsRelay::Connector`

- 建立、关闭 Socket
- 接收 Socket 字节流，转为严格数据包
- 发送数据包、严格数据包

### 主程序模块 `main`

- 初始化
  - 加载配置模块
  - 因特网 DNS 服务器地址
  - **IP 地址-域名** _(hosts)_ 表
  - 日志模块
  - ID 转换表
  - Socket 模块
- 处理业务逻辑（[sec|软件流程图]）

[page-break]

## 软件流程图

[img=max-width:85%]

[align-center]

![Flowchart](flowchart.svg)

<!--
recv=>inputoutput: 接收数据，
生成严格数据包
isResponse=>condition: 查询报文？
isStandardQuery=>condition: 正向查询？
isAQuery=>condition: IPv4 查询？
isInTable=>condition: 找到表项？
isBlocked=>condition: `0.0.0.0`？
fromNameServer=>condition: 请求超时？
fromNameServer2=>operation: 还原转换前 ID 和地址
toNameServer=>operation: 保存转换前 ID 和地址
toNameServer2=>operation: 设置转换后 ID，
设置因特网 DNS 服务器地址
getPacket=>operation: 生成普通数据包
setAns=>operation: 写入答案
setBlocked=>operation: 写入 未找到
sendStrict=>inputoutput: 发送严格数据包
sendNew=>inputoutput: 发送普通数据包
endRecv=>operation: 清空缓存
start=>start: 开始

start->recv->isResponse
isResponse(yes,right)->fromNameServer
isResponse(no)->isStandardQuery
isStandardQuery(no)->toNameServer
isStandardQuery(yes)->isAQuery
isAQuery(no)->toNameServer
isAQuery(yes)->isInTable
isInTable(no)->toNameServer
isInTable(yes)->getPacket->isBlocked
isBlocked(yes,right)->setBlocked->sendNew
isBlocked(no)->setAns->sendNew
toNameServer->toNameServer2->sendStrict
fromNameServer(yes,right)->endRecv
fromNameServer(no)->fromNameServer2->sendStrict
sendStrict->endRecv
sendNew->endRecv
endRecv(left)->recv
-->

## 测试用例以及运行结果

- 测试工具
  - `nslookup`
- 测试平台
  - `Ubuntu 16.04`
  - 由于 `Windows 10` 下的 `nslookup` 会发送多余的数据包，所以不使用。
- 测试方法
  - 在 `Windows` / `Ubuntu` 上启动 `dnsrelay`
    - `Windows` 上使用 `Visual Studio 2017` 编译
    - `Ubuntu` 上使用 `clang++ -std=c++14 dnsrelay.cpp -o dnsrelay` 编译
    - 运行命令行为 `dnsrelay`（使用默认参数）
  - 在 `Ubuntu` 上启动 `nslookup - 127.0.0.1` （交互方式）
  - 输入要查询的域名，以回车结束
- IP 地址-域名 _(hosts)_ 对照表包含以下几项
  - `0.0.0.0 008.cn`
  - `0.0.0.0 test0`
  - `11.111.11.111 test1.cn`
  - `22.22.222.222 test2.cn`
  - `123.127.134.10 bupt`

### 拦截域名

- `008.cn`
  - `** server can't find 008.cn: NXDOMAIN`
- `008.cN`
  - `** server can't find 008.cN: NXDOMAIN`
- `008.CN`
  - `** server can't find 008.CN: NXDOMAIN`
- `test0`
  - `** server can't find test0: NXDOMAIN`

### 已知域名

- `test1.cn`
  - `Name:    test1.cn`
  - `Address: 11.111.11.111`
- `test2.cn`
  - `Name:    test2.cn`
  - `Address: 22.22.222.222`
- `Test2.cn`
  - `Name:    Test2.cn`
  - `Address: 22.22.222.222`
- `bupt`
  - `Name:    bupt`
  - `Address: 123.127.134.10`

### 未知域名

- `baidu.com`
  - `Non-authoritative answer:`
  - `Name:    baidu.com`
  - `Address: 111.13.101.208`
  - `Name:    baidu.com`
  - `Address: 123.125.114.144`
  - `Name:    baidu.com`
  - `Address: 220.181.57.217`
  - `Name:    baidu.com`
  - `Address: 180.149.132.47`
- `github.com`
  - `Non-authoritative answer:`
  - `Name:    github.com`
  - `Address: 192.30.255.113`
  - `Name:    github.com`
  - `Address: 192.30.255.112`
- `bot-man-jl.github.io`, `BOT-Man-JL.github.io`
  - `Non-authoritative answer:`
  - `bot-man-jl.github.io canonical name = github.map.fastly.net.`
  - `Name:    github.map.fastly.net`
  - `Address: 151.101.72.133`

## 调试中遇到并解决的问题

由于最初的实现是：先把收到的包解析成各个字段，然后再修改内容并组装，最后发出去。遇到的问题主要在解析包的时候，有两个关键的问题：域名压缩和资源类型识别。最后的实现引入了严格数据包（[sec|严格数据包 `DnsRelay::StrictPacket`]），解决了解析不完全的问题。

### 域名压缩

#### 问题描述

域名压缩是用一个指针指向之前出现过的域名后缀，从而避免重复的记录这些后缀。例如，`bupt.edu.cn` 和 `www.bupt.edu.cn` 拥有公共后缀；如果前者先出现，那么后者可以压缩为 `www.` 和一个指向前者开头的指针；如果后者先出现，那么前者可以压缩为一个指向 `www.bupt.edu.cn` 中 `bupt` 位置的指针。

最开始的版本没有考虑到这个问题，导致了解析出来的域名不正确。

#### 解决方法

目前解决的办法是，将所有压缩的域名还原为完整的域名。

### 资源类型识别

#### 问题描述

域名压缩不仅出现在查询段、资源段的域名字段，还可能出现在资源段的数据字段。如果还原了域名字段的域名，但不还原资源数据的域名，就可能导致资源数据失效。例如，还原了前边的一些域名，使得整个包的体积变大，但后边域名里的指针指向的位置还是还原前的位置，所以这些指针会全部失效。

不仅如此，对于不同的资源，域名出现的位置和次数都不相同。例如，`A` 和 `AAAA` 资源没有域名字段；`NS`、`CNAME` 和 `PTR` 资源就是一个域名；`SOA` 资源开头有两个域名字段，之后跟着 5 个 16 位数值。

最开始的版本没有考虑到这个问题，而之后的版本考虑不周，导致了从因特网 DNS 服务器发回的数据包重新组装后，被客户端识别为不能解析的包。

#### 解决方法

最初的解决方法是，将所有的常见资源类型都分别解析。例如，`A` 和 `AAAA` 资源不还原域名；`NS`、`CNAME` 和 `PTR` 资源还原一次域名；`SOA` 资源还原开头的两个域名，然后保留之后的 5 个字段。

但是由于资源的种类繁多，结构复杂，代码难以覆盖，所以最终的解决方法是，引入严格数据包（[sec|严格数据包 `DnsRelay::StrictPacket`]）的概念 —— 仅允许修改包的 ID 和 发送地址，不能修改包的其他内容。对于所有 **未知域名**，用严格数据包保存，然后直接转发；对于 **拦截域名** 和 **已知域名**，从严格数据包解析出数据包，然后填充这个数据包，最后发送填充后的数据包。

## 课程设计工作总结

本次实验中，我学会了 UDP 通信的基本方法、DNS 协议的基本内容 以及 有关计算机网络编程的调试和分析等。另外，在不断地遇到问题，然后解决问题的过程中，我学会了如何用最小的代价，获得最好的效果，收获很大。