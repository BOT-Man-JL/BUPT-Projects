# 羽毛球馆预定系统

> 2019/5/11

## 运行

基于 C++ 14 + STL 编写，将 `main/booking_item/booking_manager.cc` 加入工程即可编译运行。

也可以直接使用 clang 编译：

``` bash
clang++ -std=c++14 booking_item.cc booking_manager.cc main.cc -Wall
```

## 测试

基于 [catch2](https://github.com/catchorg/Catch2) 编写，每个 `*_unittest.cc` 对应一个模块的单元测试代码，`unittest_main.cc` 对应单元测试的主入口。

可以直接使用 clang 编译：

``` bash
clang++ -std=c++14 booking_*.cc unittest_main.cc -Wall
```

## 限制

- 超出 09:00~22:00 的预定/取消不接受（目前在输入时拒绝）
- 重复预订/取消同一时间的一个场地，行为未定义（目前不会重复记录）
