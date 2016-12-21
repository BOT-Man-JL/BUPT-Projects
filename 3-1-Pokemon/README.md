# 宠物小精灵对战游戏

> 2014211306 李俊宁 2014211288

- 参考
  *[CppCoreGuidelines](https://github.com/isocpp/CppCoreGuidelines)*，
  使用Modern C++编写；
- 使用 更好的数据库ORM和图形库：
  *[ORM Lite](https://github.com/BOT-Man-JL/ORM-Lite)* &
  *[EggAche GL](https://github.com/BOT-Man-JL/EggAche-GL)*；
- 使用 lambda表达式 和 callback方式，实现**低耦合 可扩展**的 C/S 设计；
- 使用 `unique_ptr` 和 reference方式，**消除**潜在的**内存泄露**；
- 使用 Template Method 和 virtual function，实现**可扩展**的类设计；
- 使用 `exception` 实现灵活的**错误处理**；
- 使用 `json` 提高**协议**的**可扩展性**；

## 题目要求及项目文件

### 题目一：宠物小精灵的加入

> 设计宠物小精灵的类，为简化游戏设计，精灵的属性包括种类（力量型：高攻击； 肉盾型：高生命值； 防御型：高防御； 敏捷型：低攻击间隔，共四种）、名字、等级、经验值、攻击力、防御力、生命值、攻击间隔等（以上属性必须，其他属性可自行添加）（基本要求：本游戏中只有上面的4种类型。 进一步要求：上述4种类型可以进一步深入划分，比如力量型又可以在细分为：沙瓦朗、火爆猴、腕力等）
>
> 每个精灵初始等级为1，满级15级，每当精灵升级的时候，宠物对应的属性值会有少量增加（主属性增加量相对较多）
>
> 每个精灵有自己独特的攻击方式，如“闪电攻击”，“火焰攻击”等等，请设计一个精灵的基类，并将精灵的攻击方法设为虚方法以方便子类重写
>
> 请写一个测试程序对设计的精灵类的相关属性和方法（包括攻击函数，升级函数等）进行测试
>
> 题目考察点：类的继承，对象数据成员设计，成员函数设计

项目文件：

- Pokemon.h
- Pokemon.cpp
- TestPokemon.cpp

### 题目二：用户注册与平台登录

> 每个用户需要注册一个账号，用户名全局唯一，不能有任何两个用户名相同，要考虑注册失败的场景时的反馈
>
> 实现注册、登录、登出功能，均采用C/S模式，客户端和服务端用socket进行通信，服务端保存所有用户的信息（文件存储或数据库均可，数据库有额外加分）
>
> 每个用户拥有：用户名、拥有的精灵，两个属性。 用户注册成功时，系统自动随机分发三个1级精灵给用户
>
> 用户可以查看所有成功注册用户拥有的精灵，也可以查看所有当前在线的用户
>
> 如有界面设计可酌情加分
>
> 题目考察点：socket通信，交互场景反馈

项目依赖：

- *[ORM Lite](https://github.com/BOT-Man-JL/ORM-Lite)* (Server Only)
- *[json](https://github.com/nlohmann/json)*

项目文件：

- Socket.h
- Shared.h
- Server
  - Pokemon.h
  - Pokemon.cpp
  - Server.h
  - Server.cpp
- Client
  - Pokemon.h
  - Client.h
  - TestClient.cpp

### 题目三：游戏对战的设计

> 已经登录的在线用户可以和服务器进行虚拟决斗，决斗分两种：升级赛和决斗赛，两种比赛都能增长宠物经验值。服务器上有一个虚拟精灵的列表，用户可以挑选其中任意一个进行比赛（升级赛或者决斗赛）。另外决斗赛中用户胜出可以直接获得该战胜的的精灵，失败则系统从用户的精灵中随机选三个（不够三个精灵的情况就选择他所有的精灵），然后由用户选一个送出。
> > 升级赛 只是用户用来增加精灵经验值，规则开发者自定；
> >
> > 累积多少经验值升一级，规则开发者自定；
> >
> > 决斗赛的上述规则同升级赛，只是额外还可以赢得宠物一个。
>
> 请让你的系统自动模拟每场比赛的每次出招。另外，为了增加不确定性，可以加入概率闪避攻击和暴击伤害机制
> > 比赛的过程和结果由系统根据上述规则自动模拟完成，要求结果具有一定的随机性。

（以上内容以替换为**多人实时对战**）

> 用户如果没有精灵（比如总是失败，已经全部送出去），则系统会随机放给给他一个初级精灵。
>
> 用户增加新功能，可以查看某个用户的胜率
>
> 用户增加新属性，为宠物个数徽章（金银铜）和高级宠物徽章（金银铜），分别根据拥有的宠物个数的多少和拥有高级宠物（15级）个数的多少颁发
>
> 如有界面设计可酌情加分，如有新颖设计可酌情加分
>
> 题目考察点：客户端与服务器数据交互（可采用多进程或异步通信或其他方法均可），并发请求处理，类的方法设计，伤害计算方法设计。

项目依赖：

- *[ORM Lite](https://github.com/BOT-Man-JL/ORM-Lite)* (Server Only)
- *[EggAche GL](https://github.com/BOT-Man-JL/EggAche-GL)* (Client Only)
- *[json](https://github.com/nlohmann/json)*

项目文件：

- Socket.h
- Shared.h
- Server
  - Pokemon.h
  - Pokemon.cpp
  - Server.h
  - Server.cpp
- Client
  - Pokemon.h
  - Client.h
  - GUIClient.h
  - GUIClient.cpp

## 框架

### 服务器

### 客户端

### 传输协议

- 使用 `json` 交换信息；
- 使用 `stateless` 传输方式；
- 使用 `session` 保持状态；

``` json
"user":
{
  "uid": uid, "online": isonline, "wonrate": wonrate,
  "badges": [badge], "pokemons": [pokemon]
}

"pokemon":
{
  "pid": pid, "uid": uid, "name": name,
  "level": level, "exppoint": exppoint,
  "atk": atk, "def": def, "hp": hp, "timegap": timegap
}

"room":
{
  "rid": rid,
  "pending": not_over_and_not_started
}

"roomplayer":
{
  "uid": uid, "ready": isready,
  "width": width, "height": height,
  "pokemon": pokemon
}

"gameplayer":
{
  "uid": uid,
  "x": x, "y": y,
  "vx": vx, "vy": vy,
  "timegap": timegap,
  "hp": curHp
}

"gamedamage":
{
  "did": did,
  "x": x, "y": y,
  "vx": vx, "vy": vy
}

"resultplayer":
{
  "uid": uid,
  "won": iswon
}
```

#### 账户管理

``` json
// register

{"request": "register", "param": {"uid": uid, "pwd": pwd}}

{"success": true, "response": msg}
{"success": false, "response": msg}

// login

{"request": "login", "param": {"uid": uid, "pwd": pwd}}

{"success": true, "response": {"sid": sid, "user": thisuser}
{"success": false, "response": msg}

// logout

{"request": "logout", "param": {"sid": sid}}

{"success": true, "response": msg}
{"success": false, "response": msg}

// sesseion error

{"success": false, "response": "You haven't Login"}
```

#### 查询

``` json
// pokemons

{"request": "pokemons", "param": {"sid": sid}}

{"success": true, "response": [pokemon]}
{"success": false, "response": msg}

// users

{"request": "users", "param": {"sid": sid}}

{"success": true, "response": [user]}
{"success": false, "response": msg}
```

#### 房间 / 游戏

``` json
// rooms

{"request": "rooms", "param": {"sid": sid}}

{"success": true, "response": [rid]}
{"success": false, "response": msg}

// roomenter

{"request": "roomenter",
 "param": {"sid": sid, "rid": rid, "pid": pid}}

{"success": true, "response": {"width": width, "height": height}}
{"success": false, "response": msg}

// roomleave

{"request": "roomleave", "param": {"sid": sid}}

{"success": true, "response": msg}
{"success": false, "response": msg}

// roomready

{"request": "roomready",
 "param": {"sid": sid, "ready": isready}}

{"success": true, "response": [roomplayer]}
{"success": false, "response": msg}

// gamesync

{ "request": "gamesync",
  "param": {
    "sid": sid,
    "movex": movex, "movey": movey,
    "atkx": atkx, "atky": atky,
    "def": isdef
  }
}

{
  "success": true,
  "response": {
    "over": isover,
    "gameplayers": [gameplayer],     // not over
    "gamedamages": [gamedamage],     // not over
    "resultplayers": [resultplayer]  // over
  }
}
{"success": false, "response": msg}
```

## 设计

### `Socket.h`

- 代码组织于 `namespace BOT_Socket`；

#### `class Server`

``` cpp
Server (unsigned short port,
        std::function<void (const std::string &request,
                            std::string &response)> callback);
```

参数：
- `port` 为监听端口；
- `callback` 为业务逻辑处理**回调函数**；

返回值：
- 无 （构造函数）

功能：
- 构造一个 Socket Server，为每一个 accept 到的 socket 建立新的线程；
- 每个线程的每个请求，通过回调函数接受 `request`，写入 `response` 处理；
- 实现 `recv` 时，使用 `stringstream` 进行**动态buffer**接收；

#### `class Client`

``` cpp
Client (const std::string &ipAddr,
        unsigned short port);
```

参数：
- `ipAddr` 为服务器 IP；
- `port` 为服务器端口；

返回值：
- 无 （构造函数）

功能：
- 构造一个 Socket Client，连接到服务器的对应主机端口上；

``` cpp
std::string Client::Request (const std::string &request);
```

参数：
- `request` 发给 Server 的 string；

返回值：
- Server 响应该请求时返回的 string；

功能：
- 使用 `this` 的连接的 socket 进行数据收发；
- 实现 `recv` 时，使用 `stringstream` 进行**动态buffer**接收；

### `Shared.h`

- 代码组织于 `namespace PokemonGame`；
- 用于定义
  - 所有server和client共享的**数据格式**；
  - 关于**被迫掉线**的返回消息；

``` cpp
using PokemonID = size_t;
using PokemonName = std::string;

using UserID = std::string;
using UserPwd = std::string;
using UserBadge = std::string;

using SessionID = std::string;
using RoomID = std::string;

constexpr const char *BadSession = "You haven't Login";
```

### `Pokemon.h/cpp`

- 代码组织于 `namespace PokemonGame`；

#### 抽象类 `class Pokemon`

``` cpp
class Pokemon
{
public:
    // Get Type & Name
    virtual std::string GetType () const = 0;
    virtual std::string GetName () const = 0;

    // Get Physics
    virtual std::pair<size_t, size_t> GetSize () const = 0;
    virtual size_t GetVelocity () const = 0;

    // Attribute
    typedef unsigned Level;
    typedef unsigned ExpPoint;
    typedef unsigned HealthPoint;
    typedef unsigned TimeGap;

    // Get Attr
    Level GetLevel () const { return _level; }
    Level GetExp () const { return _expPoint; }
    HealthPoint GetAtk () const { return _atk; }
    HealthPoint GetDef () const { return _def; }
    TimeGap GetTimeGap () const { return _timeGap; }
    HealthPoint GetHP () const { return _hp; }

    // Runtime Attr
    HealthPoint GetCurHP () const { return _curHp; }

    // Factory
    static const std::vector<std::string> &PokemonNames ();
    static std::unique_ptr<Pokemon> NewPokemon ();
    static std::unique_ptr<Pokemon> NewPokemon (
        const std::string &name);
    static std::unique_ptr<Pokemon> NewPokemon (
        const std::string &name,
        Level level,
        ExpPoint expPoint,
        HealthPoint atk,
        HealthPoint def,
        HealthPoint hp,
        TimeGap timeGap);

    // this Pokemon Attack opPokemon
    // Return <isKilling, isUpgraded>
    std::pair<bool, bool> Attack (Pokemon &opPokemon);

    // Reset cur Hp to Full Hp
    void Recover ();

protected:
    // Properties
    Level _level;
    ExpPoint _expPoint;
    HealthPoint _atk;
    HealthPoint _def;
    TimeGap _timeGap;
    HealthPoint _hp;

    // Runtime
    HealthPoint _curHp;

    // Constructor
    Pokemon (Level level,
             ExpPoint expPoint,
             HealthPoint atk,
             HealthPoint def,
             HealthPoint hp,
             TimeGap timeGap)
        : _level (level), _expPoint (expPoint),
        _atk (atk), _def (def), _timeGap (timeGap),
        _hp (hp), _curHp (hp)
    {}

    // Abstract Functions
    virtual HealthPoint _GetDamagePoint (
        const Pokemon &opPokemon) const = 0;
    virtual void _OnUpgrade () = 0;
};
```

- `Attack` 使用 **Template Method Pattern**，通过重载 `_GetDamagePoint`
  和 `_OnUpgrade` 实现具体的伤害计算和升级；
- `NewPokemon` 作为精灵工厂，随机产生/根据名字随机产生/构造产生
  一个pokemon实例，并用 `unique_ptr` 标识所有权；

#### 宏展开派生类

- `SCAFFOLD_POKEMON_TYPE` 重写 `GetType`, `_GetDamagePoint`；
- `SCAFFOLD_POKEMON` 重写 `GetName`, `GetSize`, `GetVelocity`, `_OnUpgrade`；
- 具体的函数逻辑在 `Pokemon.cpp` 定义；

``` cpp
#define SCAFFOLD_POKEMON_TYPE(CLASSNAME)
#define SCAFFOLD_POKEMON(
    CLASSNAME, TYPE, W, H, V, ATK, DEF, HP, TG)

// Scaffold 4 Types of Pokemons
SCAFFOLD_POKEMON_TYPE (StrengthPokemon);
SCAFFOLD_POKEMON_TYPE (DefendingPokemon);
SCAFFOLD_POKEMON_TYPE (TankPokemon);
SCAFFOLD_POKEMON_TYPE (SwiftPokemon);
...

// Scaffold Pokemons
SCAFFOLD_POKEMON (Pikachu, SwiftPokemon,
                  10, 20, 20,
                  10, 7, 45, 4);
SCAFFOLD_POKEMON (Charmander, StrengthPokemon,
                  20, 25, 14,
                  15, 7, 55, 6);
...
```

而在 `Pokemon.cpp` 中，使用三个宏展开代码：

``` cpp
#define SCAFFOLD_NEW_POKEMON_FROM_NAME_WITH_ATTR(CLASSNAME)
#define SCAFFOLD_NEW_POKEMON_FROM_NAME(CLASSNAME)
#define NAMEOF(CLASSNAME)
```

### `Server.h/cpp`

- 依赖于
  - `ORM Lite` `json`；
  - `Socket.h` `Shared.h` `Pokemon.h`；
- 代码组织于 `namespace PokemonGame`；

#### 定义server-side模型

数据库模型：

- `TimePoint/TimePointHelper` 定义时间相关的数据类型；
- `struct PokemonModel` 定义用于**持久化**的 `Pokemon` 数据模型；
  - `ToPokemon` 用于从 `PokemonModel` 生成对应 `Pokemon`；
  - `FromPokemon` 用于从 `Pokemon` 生成对应 `PokemonModel`；
- `struct BadgeModel` 定义用于**持久化**的 Badge 数据模型；
- `struct UserModel` 定义用于**持久化**的 User 数据模型；
  - 每个 User 有多个 Badge 和 Pokemon；

运行时模型：

- `struct SessionModel` 定义 会话 模型；
- `struct RoomModel` 定义 房间/游戏时 模型；

#### `class Server` 实现业务逻辑

- 对象构造时产生Pokemon服务器对象；
- server的处理流程：
  - 初始化运行时 `SessionModel` 和 `RoomModel` 数组；
  - 初始化数据库；
  - 初始化各个业务的 `Handler`；
  - 运行一个 `BOT_Socket::Server` 实例
    并根据业务逻辑，将消息分发到不同的 `Handler`；

#### `Server.cpp` 运行服务器

生成 `PokemonGame::Server` 实例，运行服务器；

``` cpp
PokemonGame::Server (PORT);
```

### `Client.h`

- 依赖于
  - `json`；
  - `Socket.h` `Shared.h` `Pokemon.h`；
- 代码组织于 `namespace PokemonGame`；

#### 定义client-side模型

- `struct PokemonModel` 定义 `Pokemon` 数据模型；
- `struct UserModel` 定义 `User` 数据模型；
  - 包含了 `User` 的 `Badges` 和 `Pokemons`；
- `struct RoomModel` 定义 房间 数据模型；
- `struct RoomPlayer` 定义 房间内玩家 数据模型；
- `struct GameModel` 定义 游戏状态 数据模型；
  - `struct GamePlayer` 定义 游戏时玩家 模型；
  - `struct GameDamage` 定义 游戏时伤害 模型；
  - `struct ResultPlayer` 定义 游戏结束后结果 模型；
  - `bool isOver` 判断 游戏是否结束；

#### `class Client` 实现协议

- 使用socket装包/拆包json格式的数据；
- **构造**返回相应的client-side模型；

``` cpp
std::string Register (const UserID &uid,
                      const UserPwd &pwd);
UserModel Login (const UserID &uid,
                 const UserPwd &pwd);
std::string Logout ();

std::vector<PokemonModel> Pokemons ();
std::vector<UserModel> Users ();

std::vector<RoomModel> Rooms ();
std::pair<size_t, size_t> RoomEnter (const RoomID &rid,
                                     const PokemonID &pid);
std::string RoomLeave ();
std::vector<RoomPlayer> RoomReady (bool isReady);
GameModel GameSync (
    const int movex, const int movey,
    const int atkx, const int atky, const bool isDef);
```

### `GUIClient.h/cpp`

- 依赖于
  - `ORM Lite` `json`；
  - `Client.h`；
- 代码组织于 `namespace PokemonGameGUI`；

#### `class GUIClient` 实现业务逻辑

- 使用 GUI 封装 `class Client` 实现的协议，进一步实现业务逻辑；
- `GUIClient::LoginWindow` 传入 `Client`，
  返回当前用户的 `UserModel`；
- `GUIClient::StartWindow` 传入 `Client` 和 当前用户的 `UserModel`，
  返回 导航到的页面号 和 选中的小精灵的 `PokemonID`；
- `GUIClient::UsersWindow` 传入 `Client` 渲染所有用户列表；
- `GUIClient::PokemonsWindow` 传入 `Client` 渲染所有小精灵列表；
- `GUIClient::RoomWindow` 传入 `Client`，当前用户的 `UserModel` 和
  选中进入游戏的 `Pokemon`，返回 房间游戏地图大小 和 `RoomPlayer` 列表；
- `GUIClient::GameWindow` 传入 `Client`，当前用户的 `UserModel`，
  房间游戏地图大小 和 `RoomPlayer` 列表，返回 游戏结果；
- `GUIClient::ResultWindow` 传入 渲染 游戏结果；

#### `GUIClient.cpp` 运行客户端

- 使用 `enum class GUIState` 自动机，进行界面之间的导航；
- 在对应的状态，打开对应界面，根据返回值或异常判断如何导航；