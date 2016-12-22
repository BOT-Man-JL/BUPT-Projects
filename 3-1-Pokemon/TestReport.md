# 宠物小精灵对战游戏 测试报告

> 2014211306 李俊宁 2014211288
>
> 2016年 12月 22日

## 1. 范围

### 1.1. 项目概述

#### a. 被测软件名称

- 宠物小精灵对战游戏

#### b. 版本

- v0.1

#### c. 被测软件用途

- 适合作为简单的多人互动游戏，从而能为更多的玩家服务；
- 作为 程序设计实践 学习和实践的项目；

#### d. 被测软件的组成及功能

依赖库：

- *[ORM Lite](https://github.com/BOT-Man-JL/ORM-Lite)*
- *[EggAche GL](https://github.com/BOT-Man-JL/EggAche-GL)*
- *[json](https://github.com/nlohmann/json)*

程序模块：

- Socket 模块，负责基本的 Socket 通信；
- Pokemon 模块，负责与宠物小精灵相关的类的实现；
- Server 模块，负责服务器业务逻辑的实现；
- Client 模块，负责客户端协议的实现；
- GUI Client 模块，负责客户端业务逻辑的实现，即最终用户使用的部分；

测试模块：

- Pokemon Test 模块，负责 Pokemon 模块的单元测试；
- Client Test 模块，负责除 GUI 相关以外业务逻辑的单元测试；
- GUI Client 模块，负责综合测试；

### 1.2. 测试概述

#### Test 1： Pokemon Test

- 被测对象：Pokemon 模块
  - 两个Pokemon对战
- 测试环境：自动化测试

#### Test 2：Client Test

- 被测对象：Socket 模块，Server 模块，Client 模块
  - ...
- 测试环境：自动化测试

#### Test 3：Integrate Test

- 被测对象：所有模块
  - ...
- 测试环境：真实环境

## 2. 测试结果

### 2.1. Test 1： Pokemon Test

...