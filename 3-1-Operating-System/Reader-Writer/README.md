# 进程同步

## 要求

本课程实验内容引自《Windows 内核实验教程》(陈向群、林斌等编著，机械工业出版社，2002.9)。

在 Windows 环境下，创建一个包含 n 个线程的控制进程。用这 n 个线程来表示 n 个读者或写者。每个线程按相应测试数据文件的要求，进行读写操作。请用信号量机制分别实现读者优先和写者优先的读者-写者问题。

读者-写者问题的读写操作限制：
- 写-写互斥；
- 读-写互斥；
- 读-读允许；

读者优先的附加限制：如果一个读者申请进行读操作时已有另一读者正在进行读操作，则该读者可直接开始读操作。

写者优先的附加限制：如果一个读者申请进行读操作时已有另一写者在等待访问共享资源，则该读者必须等到没有写者处于等待状态后才能开始读操作。

运行结果显示要求：要求在每个线程创建、发出读写操作申请、开始读写操作和结束读写操作时分别显示一行提示信息，以确信所有处理都遵守相应的读写操作限制。

### 测试数据文件格式

测试数据文件包括n 行测试数据，分别描述创建的n 个线程是读者还是写者，以及读写操作的开始时间和持续时间。每行测试数据包括四个字段，各字段间用空格分隔。
- 第一字段为一个正整数，表示线程序号。第一字段表示相应线程角色，R 表示读者是，W 表示写者。
- 第二字段为一个正数，表示读写操作的开始时间。线程创建后，延时相应时间（单位为秒）后发出对共享资源的读写申请。
- 第三字段为一个正数，表示读写操作的持续时间。当线程读写申请成功后，开始对共享资源的读写操作，该操作持续相应时间后结束，并释放共享资源。

下面是一个测试数据文件的例子：

```
1 R 3 5
2 W 4 5
3 R 5 2
4 R 6 5
5 W 5.1 3
```

## 程序

由于实验样例程序不能运行，故使用 Modern C++ 重新编写。

[`Reader-Writer.cpp`](Reader-Writer.cpp)

## 算法

### 读者优先

``` cpp
// Resource to R/W
auto resource = CreateSemaphore (NULL, 1, 1, NULL);

// Reader Count
auto rmutex = CreateSemaphore (NULL, 1, 1, NULL);
size_t read_count = 0;
```

#### 读者

``` cpp
WaitForSingleObject (rmutex, INFINITE);
read_count++;
if (read_count == 1)
    WaitForSingleObject (resource, INFINITE);
ReleaseSemaphore (rmutex, 1, NULL);

// Critical Section

WaitForSingleObject (rmutex, INFINITE);
read_count--;
if (read_count == 0)
    ReleaseSemaphore (resource, 1, NULL);
ReleaseSemaphore (rmutex, 1, NULL);
```

#### 写者

``` cpp
WaitForSingleObject (resource, INFINITE);

// Critical Section

ReleaseSemaphore (resource, 1, NULL);
```

### 写者优先

``` cpp
// Resource to R/W
auto resource = CreateSemaphore (NULL, 1, 1, NULL);

// Reader trying to enter
auto readTry = CreateSemaphore (NULL, 1, 1, NULL);

// Reader Count
auto rmutex = CreateSemaphore (NULL, 1, 1, NULL);
size_t read_count = 0;

// Writer Count
auto wmutex = CreateSemaphore (NULL, 1, 1, NULL);
size_t write_count = 0;
```

#### 读者

``` cpp
WaitForSingleObject (readTry, INFINITE);

WaitForSingleObject (rmutex, INFINITE);
read_count++;
if (read_count == 1)
    WaitForSingleObject (resource, INFINITE);
ReleaseSemaphore (rmutex, 1, NULL);

ReleaseSemaphore (readTry, 1, NULL);

// Critical Section

WaitForSingleObject (rmutex, INFINITE);
read_count--;
if (read_count == 0)
    ReleaseSemaphore (resource, 1, NULL);
ReleaseSemaphore (rmutex, 1, NULL);
```

#### 写者

``` cpp
WaitForSingleObject (wmutex, INFINITE);
write_count++;
if (write_count == 1)
    WaitForSingleObject (readTry, INFINITE);
ReleaseSemaphore (wmutex, 1, NULL);

WaitForSingleObject (resource, INFINITE);

// Critical Section

ReleaseSemaphore (resource, 1, NULL);

WaitForSingleObject (wmutex, INFINITE);
write_count--;
if (write_count == 0)
    ReleaseSemaphore (readTry, 1, NULL);
ReleaseSemaphore (wmutex, 1, NULL);
```

## 输入输出

### 输入

```
1 R 3 5
2 W 4 5
3 R 5 2
4 R 6 5
5 W 5.1 3
```

### 输出

```
Reader Preference
Thread 1 - Reader  - Request
Thread 1 - Reader  - Begin
Thread 2 - Writer  - Request
Thread 3 - Reader  - Request
Thread 3 - Reader  - Begin
Thread 5 - Writer  - Request
Thread 4 - Reader  - Request
Thread 4 - Reader  - Begin
Thread 3 - Reader  - End
Thread 1 - Reader  - End
Thread 4 - Reader  - End
Thread 2 - Writer  - Begin
Thread 2 - Writer  - End
Thread 5 - Writer  - Begin
Thread 5 - Writer  - End
Writer Preference
Thread 1 - Reader  - Request
Thread 1 - Reader  - Begin
Thread 2 - Writer  - Request
Thread 3 - Reader  - Request
Thread 5 - Writer  - Request
Thread 4 - Reader  - Request
Thread 1 - Reader  - End
Thread 2 - Writer  - Begin
Thread 2 - Writer  - End
Thread 5 - Writer  - Begin
Thread 5 - Writer  - End
Thread 3 - Reader  - Begin
Thread 4 - Reader  - Begin
Thread 3 - Reader  - End
Thread 4 - Reader  - End
End
```