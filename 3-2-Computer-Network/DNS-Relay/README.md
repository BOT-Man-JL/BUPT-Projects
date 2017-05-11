# DNS 中继服务器 实验报告

> by BOT Man, 2017

[heading-numbering]

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

## 软件流程图

![Flowchart](flowchart.svg)

<!--
recv=>inputoutput: 接收数据
isResponse=>condition: 查询报文？
isStandardQuery=>condition: 正向查询？
isAQuery=>condition: IPv4 查询？
isInTable=>condition: 找到表项？
isBlocked=>condition: `0.0.0.0`？
fromNameServer=>condition: 请求超时？
fromNameServer2=>operation: 还原 ID 和地址
toNameServer=>operation: 保存 ID 和地址
toNameServer2=>operation: 设置为新 ID，
设置因特网 DNS 服务器地址
setAns=>operation: 写入答案
setBlocked=>operation: 写入 未找到
sendStrict=>inputoutput: 发送严格数据包
sendNew=>inputoutput: 发送数据包
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
isInTable(yes)->isBlocked
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

- 测试平台
  - `Windows 10`
  - `Ubuntu 16.04`
- 测试工具
  - `nslookup`
- 测试对象
  - 拦截域名
  - 已知域名
  - 未知域名

### 拦截域名

### 已知域名

### 未知域名

## 调试中遇到并解决的问题

由于最初的实现是：先把收到的包解析成各个字段，然后再修改内容并组装，最后发出去。遇到的问题主要在解析包的时候，有两个关键的问题：域名压缩和资源类型识别。

### 域名压缩

#### 问题描述

域名压缩是用一个指针指向之前出现过的后缀，从而避免重复的记录这些后缀。例如，`bupt.edu.cn` 和 `www.bupt.edu.cn` 拥有公共后缀；如果前者先出现，那么后者可以压缩为 `www.` 和一个指向前者开头的指针；如果后者先出现，那么前者可以压缩为一个指向 `www.bupt.edu.cn` 中 `bupt` 位置的指针。

最开始的版本没有考虑到这个问题，导致了解析出来的域名不正确。

#### 解决方法

域名还原

### 资源类型识别

#### 问题描述

域名压缩不仅出现在查询段、资源段的域名字段，还可能出现在资源段的数据字段。如果还原了域名字段的域名，但不还原资源数据的域名，就可能导致资源数据失效。例如，还原了前边的一些域名，使得整个包的体积变大，但后边域名里的指针指向的位置还是还原前的位置，所以这些指针会全部失效。

最开始的版本没有考虑到这个问题，导致了从因特网 DNS 服务器发回的数据包重新组装后，被客户端识别为不能解析的包。

#### 解决方法

- 解析资源
- 设置严格数据包（[sec|严格数据包 `DnsRelay::StrictPacket`]）

## 课程设计工作总结
