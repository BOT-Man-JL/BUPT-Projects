# 分布式温控系统面向对象设计

> 2014211306 班 F 组
>
> 李俊宁 董星彤 张有杰 赵亮 左旭彤
>
> 2017 年 05 月 24 日

[heading-numbering]

## [toc-heading] [no-number] Table of Contents

[TOC]

## [no-number] 迭代历史

> - v0.1 | 2017/5/24 | John Lee | 创建文档
> - v0.2 | 2017/5/27 | 董星彤 | 更新用例图，增加主控机导出报表和调节请求用例的协作图
> - v0.3 | 2017/5/28 | John Lee | 增加 类图
> - v0.4 | 2017/5/28 | 左旭彤 | 增加从控机操作契约和协作图
> - v0.5 | 2017/5/28 | 张有杰 | 主控机部分用例
> - v0.6 | 2017/5/28 | 张有杰 | 添加文档说明及部分系统操作说明
> - v0.7 | 2017/5/28 | 赵  亮 | 添加主控机操作契约及协作图
> - v0.8 | 2017/5/28 | 赵  亮 | 添加部分操作名称及说明
> - v1.0 | 2017/5/28 | John Lee | 发布文档

## 文档说明

### 文档目的

本文档使用面向对象的方法对在 OOA 模型的基础上分布式温控系统进行设计，从现实世界中抽象出系统的 OOD 模型以便理解问题域，为之后的软件开发奠定基础。

### 文档范围

根据系统的具体实现，运用OO方法进行系统设计:
1. 根据实现条件对 OOA 模型做必要的调整和修改，使其成为ODD 模型的一部分；
2. 根据具体实现条件，说明用例系统操作、设计软件体系结构设计、设计用例实现方案设计、类职责分配、创建设计类图。
 

### 读者对象

该面向对象设计文档的读者包括：项目设计人员、编码人员、测试人员和维护人员以及酒店用户等。
 

### 参考文档

- 《分布式温控系统用户需求说明书》
- 《用例模型说明书模板》
- 《分布式温控系统详细要求》

### 术语和缩写

| 术语 | 解释 |
|--|--|
| UC_M | 主控机的用例模型 |
| UC_S | 从控机的用例模型 |

## 项目背景

根据如下需求，设计中央空调系统：

1. 空调系统由中央空调和房间空调（从控机）两部分构成。中央空调由特定的管理人员操控，房间空调由用户来操控。
2. 中央空调具备开关按钮，只可人工开启和关闭，正常开启后处于待机状态。有冷暖两种工作模式，每种模式有特定的缺省温度以及温度区间，可根据季节进行工作模式的调整。中央空调能够实时监测个房间的温度和状态，并且能够根据实时刷新的频率进行配置。
3. 从控机只能人工方式开闭，可以通过控制面板设置目标温度，目标温度有上下限制。控制面板的温度调节可以连续变化也可以断续变化。每个从控机内有一个温度传感器，可以实时监测房间的温度。
4. 从控机的控制面板能够发送高、中、低风速的请求，各小组可以自定义高、中、低三种风速下的温度变化值。中央空调可以根据从控机的请求时长及高中低风速的供风量计算每个房间所消耗的能量以及所需支付的金额，并将对应信息发送给每个从控机进行在线显示，以便客户查询。
5. 中央空调监控具备统计功能，可以根据需要给出日报表、周报表和月报表；报表内容如下：房间号、从控机开关机的次数、温控请求起止时间（列出所有记录）、温控请求的起止温度及风量大小（列出所有记录）、每次温控请求所需费用、每日（周、月）所需总费用。
6. 中央空调同时只能处理三台从控机的请求，为此主机要有负载均衡的能力。如果有超过三台从控机请求，则需要对所有请求机器进行调度，调度算法自行定义。

## 用例模型

![user-cases-diagram](diagrams/user-cases-master-new.svg)

![user-cases-slave](diagrams/user-cases-slave.svg)

### 选项配置更改

| 操作名称 | 操作说明 |
| -- | -- |
| OpenConfigurationMenu() | 打开选项配置菜单 |
| StartMachine(GuestInfo guest) | 开启主控机系统，进行身份认证 |
| QuitSystem() | 退出主控机系统 |
| AdjustMode(Config TargetMode) | 按照目标模式及工作温度调整主控机工作模式 |
| AdjustHeartRate(Config NewHeartRate) | 按照输入的心跳频率调整从控机心跳 | 
| SetAllDefault() | 所有配置选项恢复默认 |
| CloseConfigMenu() | 关闭选项配置菜单 |

### 监测从控机状态

| 操作名称 | 操作说明 |
| -- | -- |
| OpenMonitorMenu() | 打开从控机监控菜单 |
| DisplayInfo(ClientInfo client) | 选择目标从控机，监控显示其信息 |
| CloseMonitorMenu() | 关闭从控机监控菜单 |

### 管理报表

| 操作名称 | 操作说明 |
| -- | -- |
| output_table(TimePoint timebegin,TimePoint timeend) | 输出起始时间点到终止时间点之间的报表信息 |

### 维护房客信息

| 操作名称 | 操作说明 |
| -- | -- |
| AddGuest(guest:GuestInfo) | 添加房客信息 |
| RemoveGuest (guest:GuestId) | 删除房客信息 |
| ViewGuestList ():list | 查看房客信息 |

### 汇报获取状态

| 操作名称 | 操作说明 |
| -- | -- |
| Auth (guest:GuestInfo) | 查看房客权限 |
| Request (req:GuestRequest):ClientInfo | 接受从控机请求 |
| Pulse (room:RoomInfo):ClientInfo | 定时刷新获取从控机房间信息 |
| AddEnergy (room:RoomId energy:Energy) | 添加房间号及能耗信息 |
| Cost GetCost (room:RoomId) | 反馈房间号及费用 |

### 接收从控机调控请求

| 操作名称 | 操作说明 |
| -- | -- |
| change_request(req:GuestRequest) | 根据从控机发送的调控请求（风速，温度）更改空调运行状态，并在日志中记录相应内容

### 从控机调控

| 操作名称 | 操作说明 |
| -- | -- |
| BootSlave()  | 启动从控机 |
| AdjustTandW() | 从控机发出调节请求 |

## 软件体系结构设计

系统采用分层的体系结构，其软件分层结构如下所示：

[align-center]

[img=max-width:50%]

![BUPT-Illformed-MVC](diagrams/bupt-mvc.png)

## 用例实现方案设计——类职责分配

### 选项配置更改

#### OpenConfigurationMenu()

> 操作契约

| 操作 | OpenConfigurationMenu() |
| :-- | :-- |
| 交叉引用 | 选项配置更改 UC_M_001 |
| 前置条件 | 主控机处于开启状态，运维人员管理主控机 |
| 后置条件 | 1. 一个新的（概念类）运维人员创建； |
| | 2. 一个新的（概念类）选项配置管理界面创建； |
| | 3. 运维人员与（概念类）主控机建立“关联”； |
| | 4. 运维人员与（概念类）选项配置管理界面建立“关联”； |

> 协作图

![OpenConfigurationMenu](diagrams/openConfigurationMenu.svg)

#### StartMachine(GuestInfo guest)

> 操作契约

| 操作 | StartMachine(GuestInfo guest)|
| :-- | :-- |
| 交叉引用 | 主控机开机 UC_M_001_1 |
| 前置条件 | 主控机处于关闭状态 |
| 后置条件 | 1. 一个新的（概念类）主控机欢迎界面创建；|
| | 2. 运维人员与主控机欢迎界面建立“关联”； |

> 协作图

![startMachine](diagrams/startMachine.svg)

#### QuitSystem()

> 操作契约

| 操作 | QuitSystem()|
| :-- | :-- |
| 交叉引用 | 主控机关机 UC_M_001_2 |
| 前置条件 | 主控机处于开启状态 |
| 后置条件 | 1. 运维人员与主控机功能界面解除“关联”； |
| | 2. (概念类)运维人员消除； |

> 协作图

![QuitSystem](diagrams/QuitSystem.svg)

#### AdjustMode(Config TargetMode)

> 操作契约

| 操作 | AdjustMode(Config TargetMode) |
| :-- | :-- |
| 交叉引用 | 选项配置更改 UC_M_002 |
| 前置条件 | 主控机处于开启状态，运维人员选择工作模式 |
| 后置条件 | 1. 一个新的（概念类）工作模式创建； |
| | 2. 工作模式与主控机建立“关联”； |
| | 3. 工作模式被修改； |

> 协作图

![AdjustMode](diagrams/AdjustMode.svg)

#### AdjustHeartRate(Config NewHeartRate)

> 操作契约

| 操作 | AdjustHeartRate(Config NewHeartRate) |
| :-- | :-- |
| 交叉引用 | 选项配置更改 UC_M_002 |
| 前置条件 | 主控机处于开启状态，运维人员设置从控机心跳频率 |
| 后置条件 | 1. 一个新的（概念类）从控机心跳频率创建； |
| | 2. 从控机心跳频率与主控机建立“关联”； |
| | 3. 设置从控机心跳频率； |

> 协作图

![AdjustHeartRate](diagrams/AdjustHeartRate.svg)

#### SetAllDefault()

> 操作契约

| 操作 | SetAllDefault() |
| :-- | :-- |
| 交叉引用 | 选项配置更改 UC_M_002 |
| 前置条件 | 主控机处于开启状态，运维人员配置主控机选项 |
| 后置条件 | 1. 一个新的（概念类）系统配置创建； |
| | 2. 系统配置与主控机建立“关联”； |
| | 3. 系统配置赋为默认值； |

> 协作图

![SetAllDefault](diagrams/SetAllDefault.svg)

#### CloseConfigMenu()

> 操作契约

| 操作 | CloseConfigMenu() |
| :-- | :-- |
| 交叉引用 | 选项配置更改 UC_M_002 |
| 前置条件 | 主控机处于开启状态，运维人员配置主控机选项 |
| 后置条件 | 1. 运维人员与（概念类）选项配置管理界面解除“关联”； |
| | 2. （概念类）选项配置管理界面消除； |
| | 3. 运维人员与（概念类）主控机解除“关联”； |
| | 4. （概念类）运维人员消除； |

> 协作图

![CloseConfigMenu](diagrams/CloseConfigMenu.svg)

### 监测从控机状态

#### OpenMonitorMenu()

> 操作契约

| 操作 | OpenMonitorMenu() |
| :-- | :-- |
| 交叉引用 | 监测从控机状态 UC_M_003 |
| 前置条件 | 主控机处于开启状态，运维人员监测从控机状态 |
| 后置条件 | 1. 一个新的（概念类）运维人员创建； |
| | 2. 一个新的（概念类）从控机状态监测界面创建； |
| | 3. 运维人员与（概念类）主控机建立“关联”； |
| | 4. 运维人员与（概念类）从控机状态监测界面建立“关联”； |

> 协作图

![OpenMonitorMenu](diagrams/OpenMonitorMenu.svg)

#### DisplayInfo(ClientInfo client)

> 操作契约

| 操作 | DisplayInfo(ClientInfo client) |
| :-- | :-- |
| 交叉引用 | 监测从控机状态 UC_M_003 |
| 前置条件 | 主控机成功获得从控机状态 |
| 后置条件 | 1. 从控机目标温度及设定风速显示 |
| | 2. 从控机当前温度显示|
| | 3. 从控机当前计费及耗能信息显示

> 协作图

![DisplayInfo](diagrams/DisplayInfo.svg)

#### CloseMonitorMenu()

> 操作契约

| 操作 | CloseMonitorMenu() |
| :-- | :-- |
| 交叉引用 | 监测从控机状态 UC_M_003 |
| 前置条件 | 主控机成功监测目标从控机状态 |
| 后置条件 | 1. 运维人员与（概念类）从控机状态监测界面解除“关联”； |
| | 2. （概念类）从控机状态监测界面消除； |
| | 3. 运维人员与（概念类）主控机解除“关联”； |
| | 4. （概念类）运维人员消除； |

> 协作图

![CloseMonitorMenu](diagrams/CloseMonitorMenu.svg)

### 管理报表

#### output_table(TimePoint timebegin,TimePoint timeend)

> 操作契约

| 操作 | output_table(TimePoint timebegin,TimePoint timeend) |
| :-- | :-- |
| 交叉引用 | 管理报表UC_M_004 |
| 前置条件 | 主控机处于开启状态，后台人员使用主控机并开启报表管理界面 |
| 后置条件 | 1. 一个新的（概念类）报表创建； |
| | 2. 报表与主控机建立“关联”； |
| | 3. 报表的范围属性初始化：时间范围，房间号，客户信息等； |
| | 4. 报表的内容属性被修改，存储了查询结果； |
| | 5. 报表被输出（打印或输出为某种格式） |

> 协作图

![output_table-collaboration-diagram](diagrams/output_table-collaboration-diagram.svg)

### 维护房客信息

#### manage_guestInfo(name,password)

> 操作契约

| 操作 | manage_guestInfo(name,password) |
| :-- | :-- |
| 交叉引用 | 维护房客信息UC_M_005 |
| 前置条件 | 主控机处于关闭状态，前台人员发出打开信息管理系统请求 |
| 后置条件 | 1.（概念类）信息管理系统打开； |
| |2.系统中键入新的（概念类）用户信息； |
| |3.(概念类)用户与相应房间的从控机建立关联；|

> 协作图

![Manage Guest Info Collaboration Diagram](diagrams/manage_guestInfo-collaboration-diagram.svg)

### 汇报获取信息

#### send_get_status(speed,temperature,energy)

> 操作契约

| 操作 | send_get_status(speed,temperature,energy) |
| :-- | :-- |
| 交叉引用 | 获取信息UC_M_006 |
| 前置条件 | 系统正常运行 |
| 后置条件 | 1. 主控机得到从控机温度调节请求和风速调节请求； |
| | 2. 主控机向从控机输送相应的风速； |
| | 3. 主控机向从控机返回能耗信息； |
| | 4. 主控机能够一直监测从控机的状态； |

> 协作图

![Auth And Energy Callaboration Diagram](diagrams/authAndEnergy-callaboration-diagram.svg)

### 接收从控机调控请求

#### change_request(req:GuestRequest)

> 操作契约

| 操作 | change_request(req:GuestRequest) |
| :-- | :-- |
| 交叉引用 | 从控机调控UC_M_007 |
| 前置条件 | 主控机处于开启状态，从控机向主控机发出调控请求 |
| 后置条件 | 1. （概念类）房间空调信息的属性被修改，存储了新的请求； |

> 协作图

![change_request-collaboration-diagram](diagrams/change_request-collaboration-diagram.svg)

### 从控机调控

#### BootSlave() 

> 操作契约

| 操作 | BootSlave() |
|:--|:--|
| 交叉引用 | 从控机调控 UC_S_001 |
| 前置条件 | 主控机已经被人工开启，用户去操作控制面板，从控机之前没有收到请求 |
| 后置条件 | 1.从控机与主控机建立关联 |
| | 2.从控机的状态被改变，变为开机状态
 
> 协作图

![bootslave-diagram](diagrams/bootslave-diagram.svg)

#### AdjustTandW()

> 操作契约

| 操作 | AdjustTandW() |
|:--|:--|
| 交叉引用 | 从控机调控 UC_S_001 |
| 前置条件 | 主控机和从控机都已经被人工开启，用户去操作控制面板 |
| 后置条件 | 1.从控机与房客建立“关联” |
| | 2. 房间温度或者风速被改变

> 协作图

![Adjust_tempreture_wind-diagram](diagrams/Adjust_tempreture_wind-diagram.svg)

## 用例实现方案设计——创建设计类图

> 图片均为 **矢量图**，请 **放大** 😉

![Class Diagram](diagrams/class-diagram.svg)

<!--
[ClientFacadeController|+Auth (guest:GuestInfo); +Request (req:GuestRequest); +Pulse (room:RoomInfo); +ViewClientInfo ():ClientInfo],
[AuthView|+Show (); +Hide (); +OnAuth ()]->[ClientFacadeController],
[ClientView|+Show (); +Hide (); +OnRequest (); +OnPulse ()]->[ClientFacadeController],

[DBFacade|+insert (); +delete (); +update (); +list ()]++-[ORMLite|+Insert\<T\> (); +Delete\<T\> (); +Update\<T\> (); +Query\<T\> ()],

[EnergyCostManager|+AddEnergy (room:RoomId energy:Energy); +Energy GetEnergy (room:RoomId); +Cost GetCost (room:RoomId)],
[GuestManager|+AddGuest (guest:GuestInfo); +RemoveGuest (guest:GuestId); +AuthGuest (guest:GuestInfo); +GetGuestList ():list\<GuestInfo\>]->[DBFacade],
[LogManager|+AddOnOff (onOff:LogOnOff); +AddRequest (LogRequest:request); +EndRequest (room:RoomId); +GetOnOff (from:TimePoint to:TimePoint):list\<LogOnOff\>; +GetRequest (from:TimePoint to:TimePoint):list\<LogRequest\>]->[DBFacade],
[ScheduleManager|-AdjustSchedule (); -CheckAlive ()| +SetConfig (config:Config); +Config GetConfig (); +Request (req:GuestRequest); +Pulse (room:RoomInfo); +GetClientList ():list\<ClientInfo\>]->[EnergyCostManager],
[ScheduleManager]->[LogManager],

[GuestInfoController|+AddGuest(guest:GuestInfo); +RemoveGuest (guest:GuestId); +ViewGuestList ():list\<GuestInfo\>]->[GuestManager],
[ConfigController|+SetConfig (config:Config); +ViewConfig ():Config]->[ScheduleManager],
[LogController|+GetDayOnOff (date:TimePoint):list\<LogOnOff\>; +GetWeekOnOff (date:TimePoint):list\<LogOnOff\>; +GetMonthOnOff (date:TimePoint):list\<LogOnOff\>; +GetDayRequest (date:TimePoint):list\<LogRequest\>; +GetWeekRequest (date:TimePoint):list\<LogRequest\>; +GetMonthRequest (date:TimePoint):list\<LogRequest\>]->[LogManager],
[ClientController|+Auth (guest:GuestInfo); +Request (req:GuestRequest):ClientInfo; +Pulse (room:RoomInfo):ClientInfo; +ViewClientList ():list\<ClientInfo\>]->[ScheduleManager],
[ClientController]->[GuestManager],

[GuestView|+Show (); +Hide (); +OnAdd (); +OnDel ()]->[GuestInfoController],
[ConfigView|+Show (); +Hide (); +OnSet ()]->[ConfigController],
[LogView|+Show (); +Hide (); +OnGet ()]->[LogController],
[ClientListView|+Show (); +Hide (); +Update ()]->[ClientController]
-->

### 模型层设计类图

![Model](diagrams/model.svg)

<!--
[Config|isOn:bool; mode:WorkingMode; defaultTemp:Temperature; pulseFrequency:PulseFreq],
[LogOnOff|room:RoomId; time:TimePoint],
[LogRequest|room:RoomId; wind:Wind; tempBeg:Temperature; tempEnd:Temperature; timeBeg:TimePoint; timeEnd:TimePoint; cost:Cost],
[GuestInfo|room:RoomId; guest:GuestId],
[RoomInfo|room:RoomId; temp:Temperature],
[GuestRequest|room:RoomId; temp:Temperature; wind:Wind],
[ClientInfo|room:RoomId; isAuth:bool; temp:Temperature; wind:Wind; energy:Energy; cost:Cost]
-->

### 用户界面层设计类图

![View](diagrams/view.svg)

<!--
[AuthView|+Show (); +Hide (); +OnAuth ()],
[ClientView|+Show (); +Hide (); +OnRequest (); +OnPulse ()],
[GuestView|+Show (); +Hide (); +OnAdd (); +OnDel ()],
[ConfigView|+Show (); +Hide (); +OnSet ()],
[LogView|+Show (); +Hide (); +OnGet ()],
[ClientListView|+Show (); +Hide (); +Update ()]
-->

### 控制器/处理层设计类图

![Controller](diagrams/controller.svg)

<!--
[GuestInfoController|+AddGuest(guest:GuestInfo); +RemoveGuest (guest:GuestId); +ViewGuestList ():list\<GuestInfo\>],
[ConfigController|+SetConfig (config:Config); +ViewConfig ():Config],
[LogController|+GetDayOnOff (date:TimePoint):list\<LogOnOff\>; +GetWeekOnOff (date:TimePoint):list\<LogOnOff\>; +GetMonthOnOff (date:TimePoint):list\<LogOnOff\>; +GetDayRequest (date:TimePoint):list\<LogRequest\>; +GetWeekRequest (date:TimePoint):list\<LogRequest\>; +GetMonthRequest (date:TimePoint):list\<LogRequest\>],
[ClientController|+Auth (guest:GuestInfo); +Request (req:GuestRequest):ClientInfo; +Pulse (room:RoomInfo):ClientInfo; +ViewClientList ():list\<ClientInfo\>],
[ClientFacadeController|+Auth (guest:GuestInfo); +Request (req:GuestRequest); +Pulse (room:RoomInfo); +ViewClientInfo ():ClientInfo]
-->

### 业务/领域层设计类图

![Service](diagrams/service.svg)

<!--
[LogManager|+AddOnOff (onOff:LogOnOff); +AddRequest (LogRequest:request); +EndRequest (room:RoomId); +GetOnOff (from:TimePoint to:TimePoint):list\<LogOnOff\>; +GetRequest (from:TimePoint to:TimePoint):list\<LogRequest\>],
[GuestManager|+AddGuest (guest:GuestInfo); +RemoveGuest (guest:GuestId); +AuthGuest (guest:GuestInfo); +GetGuestList ():list\<GuestInfo\>],
[EnergyCostManager|+AddEnergy (room:RoomId energy:Energy); +Energy GetEnergy (room:RoomId); +Cost GetCost (room:RoomId)],
[ScheduleManager|-AdjustSchedule (); -CheckAlive ()| +SetConfig (config:Config); +Config GetConfig (); +Request (req:GuestRequest); +Pulse (room:RoomInfo); +GetClientList ():list\<ClientInfo\>]
-->

### 持久化层设计类图

![Persistence](diagrams/persistence.svg)

<!--
[DBFacade|+insert (); +delete (); +update (); +list ()]++-[ORMLite|+Insert\<T\> (); +Delete\<T\> (); +Update\<T\> (); +Query\<T\> ()]
-->