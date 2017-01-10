# 内存管理

## 实验目的

> 在本次实验中，需要从不同的侧面了解 Windows 2000/XP 的虚拟内存机制。在 Windows 2000/XP 操作系统中，可以通过一些 API 操纵虚拟内存。主要需要了解以下几方面：
> - Windows 2000/XP 虚拟存储系统的组织
> - 如何控制虚拟内存空间
> - 如何编写内存追踪和显示工具
> - 详细了解与内存相关的API 函数的使用

## 实验内容

> 使用 Win32 API 函数，编写一个包含两个线程的进程，一个线程用于模拟内存分配活动，一个线程用于跟踪第一个线程的内存行为。模拟内存活动的线程可以从一个文件中读出要进行的内存操作，每个内存操作包含如下内容：
> - 时间：开始执行的时间；
> - 块数：分配内存的粒度；
> - 操作：包括保留一个区域、提交一个区域、释放一个区域、回收一个区域以及锁与解锁一个区域；可以将这些操作编号，存放于文件中。
> - 大小：指块的大小；
> - 访问权限：共五种PAGE_READONLY、PAGE_READWRITE、PAGE_EXCUTE、PAGE_EXECUTE_READ 和PAGE_ EXECUTE_READWRITE。可以将这些权限编号，存放于文件中。
> - 跟踪线程将页面大小、已使用的地址范围、物理内存总量以及虚拟内存总量等信息显示出来。

## 实验程序

由于实验样例程序不能运行，故使用 Modern C++ 重新编写。

[`Mem-Alloc.cpp`](Mem-Alloc.cpp)

## 实验结果

```
                PageSize 4096

Case: PAGE_READONLY
Reverse
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 1
                 Protect PAGE_UNKNOWN
                   State MEM_RESERVE
                    Type MEM_PRIVATE
Commit
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 1
                 Protect PAGE_READONLY
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Lock
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 1
                 Protect PAGE_READONLY
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Unlock
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 1
                 Protect PAGE_READONLY
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Decommit
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 1
                 Protect PAGE_UNKNOWN
                   State MEM_RESERVE
                    Type MEM_PRIVATE
Release
             BaseAddress 00090000
          AllocationBase 00000000
       AllocationProtect PAGE_UNKNOWN
   RegionSize / PageSize 16
                 Protect PAGE_NOACCESS
                   State MEM_FREE
                    Type MEM_UNKNOWN_TYPE

Case: PAGE_READWRITE
Reverse
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 2
                 Protect PAGE_UNKNOWN
                   State MEM_RESERVE
                    Type MEM_PRIVATE
Commit
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 2
                 Protect PAGE_READWRITE
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Lock
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 2
                 Protect PAGE_READWRITE
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Unlock
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 2
                 Protect PAGE_READWRITE
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Decommit
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 2
                 Protect PAGE_UNKNOWN
                   State MEM_RESERVE
                    Type MEM_PRIVATE
Release
             BaseAddress 00090000
          AllocationBase 00000000
       AllocationProtect PAGE_UNKNOWN
   RegionSize / PageSize 16
                 Protect PAGE_NOACCESS
                   State MEM_FREE
                    Type MEM_UNKNOWN_TYPE

Case: PAGE_EXECUTE
Reverse
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 3
                 Protect PAGE_UNKNOWN
                   State MEM_RESERVE
                    Type MEM_PRIVATE
Commit
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 3
                 Protect PAGE_EXECUTE
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Lock
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 3
                 Protect PAGE_EXECUTE
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Unlock
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 3
                 Protect PAGE_EXECUTE
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Decommit
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 3
                 Protect PAGE_UNKNOWN
                   State MEM_RESERVE
                    Type MEM_PRIVATE
Release
             BaseAddress 00090000
          AllocationBase 00000000
       AllocationProtect PAGE_UNKNOWN
   RegionSize / PageSize 16
                 Protect PAGE_NOACCESS
                   State MEM_FREE
                    Type MEM_UNKNOWN_TYPE

Case: PAGE_EXECUTE_READ
Reverse
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 4
                 Protect PAGE_UNKNOWN
                   State MEM_RESERVE
                    Type MEM_PRIVATE
Commit
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 4
                 Protect PAGE_EXECUTE_READ
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Lock
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 4
                 Protect PAGE_EXECUTE_READ
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Unlock
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 4
                 Protect PAGE_EXECUTE_READ
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Decommit
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 4
                 Protect PAGE_UNKNOWN
                   State MEM_RESERVE
                    Type MEM_PRIVATE
Release
             BaseAddress 00090000
          AllocationBase 00000000
       AllocationProtect PAGE_UNKNOWN
   RegionSize / PageSize 16
                 Protect PAGE_NOACCESS
                   State MEM_FREE
                    Type MEM_UNKNOWN_TYPE

Case: PAGE_EXECUTE_READWRITE
Reverse
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 5
                 Protect PAGE_UNKNOWN
                   State MEM_RESERVE
                    Type MEM_PRIVATE
Commit
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 5
                 Protect PAGE_EXECUTE_READWRITE
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Lock
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 5
                 Protect PAGE_EXECUTE_READWRITE
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Unlock
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 5
                 Protect PAGE_EXECUTE_READWRITE
                   State MEM_COMMIT
                    Type MEM_PRIVATE
Decommit
             BaseAddress 00090000
          AllocationBase 00090000
       AllocationProtect PAGE_NOACCESS
   RegionSize / PageSize 5
                 Protect PAGE_UNKNOWN
                   State MEM_RESERVE
                    Type MEM_PRIVATE
Release
             BaseAddress 00090000
          AllocationBase 00000000
       AllocationProtect PAGE_UNKNOWN
   RegionSize / PageSize 16
                 Protect PAGE_NOACCESS
                   State MEM_FREE
                    Type MEM_UNKNOWN_TYPE
```

## 实验结论

- 现在多数的计算机页大小为 4 KB (4096 Byte)；
- `VirtualQuery` 可以查看虚拟内存分配情况；
  - `BaseAddress` 表示查询的内存基址（64 位）；
  - `AllocationAddress` 表示系统为内存分配的基址（64 位）；
  - `AllocationProtect` 表示申请内存时的访问保护；
  - `RegionSize` 表示申请区域的大小（字节为单位）；
  - `Protect` 表示当前的内存访问保护；
  - `State` 表示当前内存分配状态；
  - `Type` 表示当前内存分配的类型；
- `VirtualAlloc` 可以申请系统分配虚拟内存；
  - 使用 `MEM_RESERVE` 可以申请**保留**一段空间；
  - 使用 `MEM_COMMIT` 表示**提交**申请，获取保留的空间；
- `VirtualLock` 用于**锁定**虚拟内存于物理内存中，保证之后对其访问 *不引起缺页中断*；
- `VirtualUnlock` **取消**刚刚的**锁定**；
- `VirtualFree` 释放申请的虚拟内存；
  - 使用 `MEM_DECOMMIT` 可以**撤销提交**，返回保留状态；
  - 使用 `MEM_RELEASE` 用于**撤销保留**，其他进程可以申请使用这块内存。