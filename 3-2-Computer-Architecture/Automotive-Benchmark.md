# 实验 2：5 级超标量流水线

> 2017/6/2

本次实验采用的模拟器 sim-outorder 为 5 级超标量流水的乱序模拟器。要求大家通过执行 simplescalar 自带的测试程序，并搞清楚各模拟器参数与统计量的含义，以加深对流水线技术的理解。

## 实验目的

1. 通过实验，加深对流水线技术的理解。
2. 初步掌握 SimpleScalar 的使用。

## 实验要求

1. 利用 SimpleScalar 中的模拟器 sim-outorder，运行其自带的测试程序：`./tests-pisa/bin.little` 目录下的 `test-math`, `test-fmath` 及 `test-printf`。
2. 记录结果及生成的统计量，注意观察流水线相关的统计量。
3. 使用 gcc-2.7.2.3 交叉编译器，构建 Mibench benchmark（基准测试程序）的 automotive 包（今后的实验中会用到）。在此过程中，需要将各文件夹下 Makefile 中的 gcc 替换为 SimpleScalar 中的交叉编译器的绝对路径。
4. 修改各文件夹下的 `runme_small.sh`，使其能够调用 sim-outorder 模拟器以完成 benchmark 的运行，并观察输出结果。

## 实验环境

- Ubuntu 16.04（docker `botmanjl/simplescalar`）
- SimpleSim
- gcc cross
- Mibench benchmark - Automotive

## 实验步骤

### 进入实验环境

- 下载实验 1 中部署好的镜像
- 进入该镜像，进行实验
- 配置环境变量

``` bash
docker pull botmanjl/simplescalar
docker run -i -t botmanjl/simplescalar

export IDIR=/home/john
```

### 使用 sim-outorder 运行 tests-pisa 测试

- 分别运行 `test-math`, `test-fmath` 及 `test-printf`
- 将输出重定向到 `*_o.txt`，统计量重定向到 `*_s.txt`

``` bash
cd $IDIR/simplesim-3.0/tests-pisa/bin.little
sim-outorder test-math > math_o.txt 2> math_s.txt
sim-outorder test-fmath > fmath_o.txt 2> fmath_s.txt
sim-outorder test-printf > printf_o.txt 2> printf_s.txt
```

输出结果见 [sec|结果一 `test-math` `test-fmath` `test-printf`]

### 使用 gcc cross 构建 Mibench benchmark

- 下载 `automotive`
- 修改原始的 `makefile`
- 构建 `automotive` 下的四个程序

``` bash
cd $IDIR
curl http://vhosts.eecs.umich.edu/mibench/automotive.tar.gz -o automotive.tar.gz
tar -xf automotive.tar.gz
rm automotive.tar.gz

## Setup libc.a path
export LIBDIR=$(echo "$IDIR/lib/libc.a" | sed 's/\//\\\//g')

cd $IDIR/automotive
sed 's/gcc/sgcc/g' */Makefile -i
sed "s/-lm/$LIBDIR/g" */Makefile -i

find . -type d -exec make -C {} \;
```

### 使用 sim-outorder 运行 automotive

- 使用 `sim-outorder` 分别运行 `basicmath`, `bitcount`, `susan` 和 `qsort`
- 由于这些跑分的结果不需要记录，所以将输出重定向到 `/dev/null`
- 将统计量重定向到 `*_s.txt`

``` bash
cd $IDIR/automotive

sim-outorder ./basicmath/basicmath_small > /dev/null 2> basicmath_s.txt

sim-outorder ./bitcount/bitcnts 75000 > /dev/null 2> bitcount_s.txt

sim-outorder ./susan/susan ./susan/input_small.pgm output_small.smoothing.pgm -s 2> susan_ss.txt
sim-outorder ./susan/susan ./susan/input_small.pgm output_small.edges.pgm -e 2> susan_es.txt
sim-outorder ./susan/susan ./susan/input_small.pgm output_small.corners.pgm -c 2> susan_cs.txt
rm *.pgm

sim-outorder ./qsort/qsort_small ./qsort/input_small.dat > /dev/null 2> qsort_s.txt
```

## 实验总结

本次实验中，学会了如何使用 SimpleSim 的 SimOutorder 模拟流水线，并统计运行结果；还学会了如何使用 gcc 交叉编译器生成用于 SimpleSim 的二进制文件。

## 实验结果

- 由于结果内容比较多，不截图，仅复制。
- 由于配置信息相同，不复制；仅复制统计量。

### 结果一 `test-math` `test-fmath` `test-printf`

#### 输出结果

```
root@636b338fc4bb:/home/john/simplesim-3.0/tests-pisa/bin.little# cat math_o.txt
pow(12.0, 2.0) == 144.000000
pow(10.0, 3.0) == 1000.000000
pow(10.0, -3.0) == 0.001000
str: 123.456
x: 123.000000
str: 123.456
x: 123.456000
str: 123.456
x: 123.456000
123.456 123.456000 123 1000
sinh(2.0) = 3.62686
sinh(3.0) = 10.01787
h=3.60555
atan2(3,2) = 0.98279
pow(3.60555,4.0) = 169
169 / exp(0.98279 * 5) = 1.24102
3.93117 + 5*log(3.60555) = 10.34355
cos(10.34355) = -0.6068,  sin(10.34355) = -0.79486
x     0.5x
x0.5     x
x   0.5x
-1e-17 != -1e-17 Worked!
root@636b338fc4bb:/home/john/simplesim-3.0/tests-pisa/bin.little# cat fmath_o.txt
q=4 (int)x=12 (int)y=29
z=144
z=841
z=13
z=13
l=6
l=36
*lp=216
z=144.000000
q=4 x=12.000000 (int)x=12 y=29.000000 (int)y=29
q = 16  x = 11.700001  y = 23.400000
root@636b338fc4bb:/home/john/simplesim-3.0/tests-pisa/bin.little# cat printf_o.txt
%.4x:   `0012'
%04x:   `0012'
%4.4x:  `0012'
%04.4x: `0012'
%4.3x:  ` 012'
%04.3x: `0012'
%.*x:   `0012'
%0*x:   `0012'
%*.*x:  `0012'
%0*.*x: `0012'
bad format:     "%z"
nil pointer (padded):   "     (nil)"
decimal negative:       "-2345"
octal negative: "37777773327"
hex negative:   "fffff6d7"
long decimal number:    "-123456"
long octal negative:    "37777773327"
long unsigned decimal number:   "4294843840"
zero-padded LDN:        "-000123456"
left-adjusted ZLDN:     "-123456   "
space-padded LDN:       "   -123456"
left-adjusted SLDN:     "-123456   "
zero-padded string:     "    Hi, Z."
left-adjusted Z string: "Hi, Z.    "
space-padded string:    "    Hi, Z."
left-adjusted S string: "Hi, Z.    "
null string:    "(null)"
e-style >= 1:   "1.234000e+01"
e-style >= .1:  "1.234000e-01"
e-style < .1:   "1.234000e-03"
e-style big:    "1.000000000000000000000000000000000000000000000000000000000000e+20"
e-style == .1:  "1.000000e-01"
f-style >= 1:   "12.340000"
f-style >= .1:  "0.123400"
f-style < .1:   "0.001234"
g-style >= 1:   "12.34"
g-style >= .1:  "0.1234"
g-style < .1:   "0.00123"
g-style big:    "100000000000000000000"
:new test: 99.85:
 0.10000
 0.10000
x0.5000x
0x1
|      0.0000|  0.0000e+00|           0|
|      1.0000|  1.0000e+00|           1|
|     -1.0000| -1.0000e+00|          -1|
|    100.0000|  1.0000e+02|         100|
|   1000.0000|  1.0000e+03|        1000|
|  10000.0000|  1.0000e+04|       1e+04|
|  12345.0000|  1.2345e+04|   1.235e+04|
| 100000.0000|  1.0000e+05|       1e+05|
| 123456.0000|  1.2346e+05|   1.235e+05|

Formatted output test
prefix  6d      6o      6x      6X      6u
%-+#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+# |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+0 |-123   |377    |ff     |FF     |4294967295 |
  %-+ |-123   |377    |ff     |FF     |4294967295 |
 %-#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-# |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-0 |-123   |377    |ff     |FF     |4294967295 |
   %- |-123   |377    |ff     |FF     |4294967295 |
 %+#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
  %+# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
  %+0 |-00123 |000377 |0000ff |0000FF |4294967295 |
   %+ |  -123 |   377 |    ff |    FF |4294967295 |
  %#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
   %# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
   %0 |-00123 |000377 |0000ff |0000FF |4294967295 |
    % |  -123 |   377 |    ff |    FF |4294967295 |
    (null)
(null)

Formatted output test
prefix  6d      6o      6x      6X      6u
%-+#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+# |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+0 |-123   |377    |ff     |FF     |4294967295 |
  %-+ |-123   |377    |ff     |FF     |4294967295 |
 %-#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-# |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-0 |-123   |377    |ff     |FF     |4294967295 |
   %- |-123   |377    |ff     |FF     |4294967295 |
 %+#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
  %+# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
  %+0 |-00123 |000377 |0000ff |0000FF |4294967295 |
   %+ |  -123 |   377 |    ff |    FF |4294967295 |
  %#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
   %# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
   %0 |-00123 |000377 |0000ff |0000FF |4294967295 |
    % |  -123 |   377 |    ff |    FF |4294967295 |
    (null)
(null)

Formatted output test
prefix  6d      6o      6x      6X      6u
%-+#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+# |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+0 |-123   |377    |ff     |FF     |4294967295 |
  %-+ |-123   |377    |ff     |FF     |4294967295 |
 %-#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-# |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-0 |-123   |377    |ff     |FF     |4294967295 |
   %- |-123   |377    |ff     |FF     |4294967295 |
 %+#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
  %+# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
  %+0 |-00123 |000377 |0000ff |0000FF |4294967295 |
   %+ |  -123 |   377 |    ff |    FF |4294967295 |
  %#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
   %# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
   %0 |-00123 |000377 |0000ff |0000FF |4294967295 |
    % |  -123 |   377 |    ff |    FF |4294967295 |
    (null)
(null)

Formatted output test
prefix  6d      6o      6x      6X      6u
%-+#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+# |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+0 |-123   |377    |ff     |FF     |4294967295 |
  %-+ |-123   |377    |ff     |FF     |4294967295 |
 %-#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-# |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-0 |-123   |377    |ff     |FF     |4294967295 |
   %- |-123   |377    |ff     |FF     |4294967295 |
 %+#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
  %+# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
  %+0 |-00123 |000377 |0000ff |0000FF |4294967295 |
   %+ |  -123 |   377 |    ff |    FF |4294967295 |
  %#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
   %# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
   %0 |-00123 |000377 |0000ff |0000FF |4294967295 |
    % |  -123 |   377 |    ff |    FF |4294967295 |
    (null)
(null)

Formatted output test
prefix  6d      6o      6x      6X      6u
%-+#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+# |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+0 |-123   |377    |ff     |FF     |4294967295 |
  %-+ |-123   |377    |ff     |FF     |4294967295 |
 %-#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-# |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-0 |-123   |377    |ff     |FF     |4294967295 |
   %- |-123   |377    |ff     |FF     |4294967295 |
 %+#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
  %+# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
  %+0 |-00123 |000377 |0000ff |0000FF |4294967295 |
   %+ |  -123 |   377 |    ff |    FF |4294967295 |
  %#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
   %# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
   %0 |-00123 |000377 |0000ff |0000FF |4294967295 |
    % |  -123 |   377 |    ff |    FF |4294967295 |
    (null)
(null)

Formatted output test
prefix  6d      6o      6x      6X      6u
%-+#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+# |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+0 |-123   |377    |ff     |FF     |4294967295 |
  %-+ |-123   |377    |ff     |FF     |4294967295 |
 %-#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-# |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-0 |-123   |377    |ff     |FF     |4294967295 |
   %- |-123   |377    |ff     |FF     |4294967295 |
 %+#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
  %+# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
  %+0 |-00123 |000377 |0000ff |0000FF |4294967295 |
   %+ |  -123 |   377 |    ff |    FF |4294967295 |
  %#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
   %# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
   %0 |-00123 |000377 |0000ff |0000FF |4294967295 |
    % |  -123 |   377 |    ff |    FF |4294967295 |
    (null)
(null)

Formatted output test
prefix  6d      6o      6x      6X      6u
%-+#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+# |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+0 |-123   |377    |ff     |FF     |4294967295 |
  %-+ |-123   |377    |ff     |FF     |4294967295 |
 %-#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-# |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-0 |-123   |377    |ff     |FF     |4294967295 |
   %- |-123   |377    |ff     |FF     |4294967295 |
 %+#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
  %+# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
  %+0 |-00123 |000377 |0000ff |0000FF |4294967295 |
   %+ |  -123 |   377 |    ff |    FF |4294967295 |
  %#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
   %# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
   %0 |-00123 |000377 |0000ff |0000FF |4294967295 |
    % |  -123 |   377 |    ff |    FF |4294967295 |
    (null)
(null)

Formatted output test
prefix  6d      6o      6x      6X      6u
%-+#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+# |-123   |0377   |0xff   |0XFF   |4294967295 |
 %-+0 |-123   |377    |ff     |FF     |4294967295 |
  %-+ |-123   |377    |ff     |FF     |4294967295 |
 %-#0 |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-# |-123   |0377   |0xff   |0XFF   |4294967295 |
  %-0 |-123   |377    |ff     |FF     |4294967295 |
   %- |-123   |377    |ff     |FF     |4294967295 |
 %+#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
  %+# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
  %+0 |-00123 |000377 |0000ff |0000FF |4294967295 |
   %+ |  -123 |   377 |    ff |    FF |4294967295 |
  %#0 |-00123 |000377 |0x00ff |0X00FF |4294967295 |
   %# |  -123 |  0377 |  0xff |  0XFF |4294967295 |
   %0 |-00123 |000377 |0000ff |0000FF |4294967295 |
    % |  -123 |   377 |    ff |    FF |4294967295 |
    (null)
(null)
1.234568e+06 should be 1.234568e+06
1234567.800000 should be 1234567.800000
1.23457e+06 should be 1.23457e+06
123.456 should be 123.456
1e+06 should be 1e+06
10 should be 10
0.02 should be 0.02
testing parametric fields: 0.7000
```

#### `test-math` 统计信息

```
sim: ** simulation statistics **
sim_num_insn                 189253 # total number of instructions committed
sim_num_refs                  47607 # total number of loads and stores committed
sim_num_loads                 27079 # total number of loads committed
sim_num_stores           20528.0000 # total number of stores committed
sim_num_branches              32123 # total number of branches committed
sim_elapsed_time                  1 # total simulation time in seconds
sim_inst_rate           189253.0000 # simulation speed (in insts/sec)
sim_total_insn               207606 # total number of instructions executed
sim_total_refs                52295 # total number of loads and stores executed
sim_total_loads               30245 # total number of loads executed
sim_total_stores         22050.0000 # total number of stores executed
sim_total_branches            36081 # total number of branches executed
sim_cycle                    202089 # total simulation time in cycles
sim_IPC                      0.9365 # instructions per cycle
sim_CPI                      1.0678 # cycles per instruction
sim_exec_BW                  1.0273 # total instructions (mis-spec + committed) per cycle
sim_IPB                      5.8915 # instruction per branch
IFQ_count                    323445 # cumulative IFQ occupancy
IFQ_fcount                    69203 # cumulative IFQ full count
ifq_occupancy                1.6005 # avg IFQ occupancy (insn's)
ifq_rate                     1.0273 # avg IFQ dispatch rate (insn/cycle)
ifq_latency                  1.5580 # avg IFQ occupant latency (cycle's)
ifq_full                     0.3424 # fraction of time (cycle's) IFQ was full
RUU_count                   1318983 # cumulative RUU occupancy
RUU_fcount                    43178 # cumulative RUU full count
ruu_occupancy                6.5267 # avg RUU occupancy (insn's)
ruu_rate                     1.0273 # avg RUU dispatch rate (insn/cycle)
ruu_latency                  6.3533 # avg RUU occupant latency (cycle's)
ruu_full                     0.2137 # fraction of time (cycle's) RUU was full
LSQ_count                    268742 # cumulative LSQ occupancy
LSQ_fcount                     4570 # cumulative LSQ full count
lsq_occupancy                1.3298 # avg LSQ occupancy (insn's)
lsq_rate                     1.0273 # avg LSQ dispatch rate (insn/cycle)
lsq_latency                  1.2945 # avg LSQ occupant latency (cycle's)
lsq_full                     0.0226 # fraction of time (cycle's) LSQ was full
sim_slip                    1767008 # total number of slip cycles
avg_sim_slip                 9.3368 # the average slip between issue and retirement
bpred_bimod.lookups           37365 # total number of bpred lookups
bpred_bimod.updates           32123 # total number of updates
bpred_bimod.addr_hits         28354 # total number of address-predicted hits
bpred_bimod.dir_hits          28958 # total number of direction-predicted hits (includes addr-hits)
bpred_bimod.misses             3165 # total number of misses
bpred_bimod.jr_hits            2990 # total number of address-predicted hits for JR's
bpred_bimod.jr_seen            3132 # total number of JR's seen
bpred_bimod.jr_non_ras_hits.PP           28 # total number of address-predicted hits for non-RAS JR's
bpred_bimod.jr_non_ras_seen.PP           41 # total number of non-RAS JR's seen
bpred_bimod.bpred_addr_rate    0.8827 # branch address-prediction rate (i.e., addr-hits/updates)
bpred_bimod.bpred_dir_rate    0.9015 # branch direction-prediction rate (i.e., all-hits/updates)
bpred_bimod.bpred_jr_rate    0.9547 # JR address-prediction rate (i.e., JR addr-hits/JRs seen)
bpred_bimod.bpred_jr_non_ras_rate.PP    0.6829 # non-RAS JR addr-pred rate (ie, non-RAS JR hits/JRs seen)
bpred_bimod.retstack_pushes         3430 # total number of address pushed onto ret-addr stack
bpred_bimod.retstack_pops         4098 # total number of address popped off of ret-addr stack
bpred_bimod.used_ras.PP         3091 # total number of RAS predictions used
bpred_bimod.ras_hits.PP         2962 # total number of RAS hits
bpred_bimod.ras_rate.PP    0.9583 # RAS prediction rate (i.e., RAS hits/used RAS)
il1.accesses                 228199 # total number of accesses
il1.hits                     214197 # total number of hits
il1.misses                    14002 # total number of misses
il1.replacements              13492 # total number of replacements
il1.writebacks                    0 # total number of writebacks
il1.invalidations                 0 # total number of invalidations
il1.miss_rate                0.0614 # miss rate (i.e., misses/ref)
il1.repl_rate                0.0591 # replacement rate (i.e., repls/ref)
il1.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
il1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
dl1.accesses                  47499 # total number of accesses
dl1.hits                      46940 # total number of hits
dl1.misses                      559 # total number of misses
dl1.replacements                 74 # total number of replacements
dl1.writebacks                   70 # total number of writebacks
dl1.invalidations                 0 # total number of invalidations
dl1.miss_rate                0.0118 # miss rate (i.e., misses/ref)
dl1.repl_rate                0.0016 # replacement rate (i.e., repls/ref)
dl1.wb_rate                  0.0015 # writeback rate (i.e., wrbks/ref)
dl1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
ul2.accesses                  14631 # total number of accesses
ul2.hits                      13431 # total number of hits
ul2.misses                     1200 # total number of misses
ul2.replacements                  0 # total number of replacements
ul2.writebacks                    0 # total number of writebacks
ul2.invalidations                 0 # total number of invalidations
ul2.miss_rate                0.0820 # miss rate (i.e., misses/ref)
ul2.repl_rate                0.0000 # replacement rate (i.e., repls/ref)
ul2.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
ul2.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
itlb.accesses                228199 # total number of accesses
itlb.hits                    228176 # total number of hits
itlb.misses                      23 # total number of misses
itlb.replacements                 0 # total number of replacements
itlb.writebacks                   0 # total number of writebacks
itlb.invalidations                0 # total number of invalidations
itlb.miss_rate               0.0001 # miss rate (i.e., misses/ref)
itlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
itlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
itlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
dtlb.accesses                 48166 # total number of accesses
dtlb.hits                     48156 # total number of hits
dtlb.misses                      10 # total number of misses
dtlb.replacements                 0 # total number of replacements
dtlb.writebacks                   0 # total number of writebacks
dtlb.invalidations                0 # total number of invalidations
dtlb.miss_rate               0.0002 # miss rate (i.e., misses/ref)
dtlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
dtlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
dtlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
sim_invalid_addrs                 0 # total non-speculative bogus addresses seen (debug var)
ld_text_base             0x00400000 # program text (code) segment base
ld_text_size                  91744 # program text (code) size in bytes
ld_data_base             0x10000000 # program initialized data segment base
ld_data_size                  13028 # program init'ed `.data' and uninit'ed `.bss' size in bytes
ld_stack_base            0x7fffc000 # program stack segment base (highest address in stack)
ld_stack_size                 16384 # program initial stack size
ld_prog_entry            0x00400140 # program entry point (initial PC)
ld_environ_base          0x7fff8000 # program environment base address address
ld_target_big_endian              0 # target executable endian-ness, non-zero if big endian
mem.page_count                   33 # total number of pages allocated
mem.page_mem                   132k # total size of memory pages allocated
mem.ptab_misses                  37 # total first level page table misses
mem.ptab_accesses           1832598 # total page table accesses
mem.ptab_miss_rate           0.0000 # first level page table miss rate
```

#### `test-fmath` 统计信息

```
sim: ** simulation statistics **
sim_num_insn                  43301 # total number of instructions committed
sim_num_refs                  12322 # total number of loads and stores committed
sim_num_loads                  5627 # total number of loads committed
sim_num_stores            6695.0000 # total number of stores committed
sim_num_branches               7733 # total number of branches committed
sim_elapsed_time                  1 # total simulation time in seconds
sim_inst_rate            43301.0000 # simulation speed (in insts/sec)
sim_total_insn                47571 # total number of instructions executed
sim_total_refs                13370 # total number of loads and stores executed
sim_total_loads                6334 # total number of loads executed
sim_total_stores          7036.0000 # total number of stores executed
sim_total_branches             8582 # total number of branches executed
sim_cycle                     59115 # total simulation time in cycles
sim_IPC                      0.7325 # instructions per cycle
sim_CPI                      1.3652 # cycles per instruction
sim_exec_BW                  0.8047 # total instructions (mis-spec + committed) per cycle
sim_IPB                      5.5995 # instruction per branch
IFQ_count                     78525 # cumulative IFQ occupancy
IFQ_fcount                    17099 # cumulative IFQ full count
ifq_occupancy                1.3283 # avg IFQ occupancy (insn's)
ifq_rate                     0.8047 # avg IFQ dispatch rate (insn/cycle)
ifq_latency                  1.6507 # avg IFQ occupant latency (cycle's)
ifq_full                     0.2892 # fraction of time (cycle's) IFQ was full
RUU_count                    308379 # cumulative RUU occupancy
RUU_fcount                     9850 # cumulative RUU full count
ruu_occupancy                5.2166 # avg RUU occupancy (insn's)
ruu_rate                     0.8047 # avg RUU dispatch rate (insn/cycle)
ruu_latency                  6.4825 # avg RUU occupant latency (cycle's)
ruu_full                     0.1666 # fraction of time (cycle's) RUU was full
LSQ_count                     65361 # cumulative LSQ occupancy
LSQ_fcount                     1895 # cumulative LSQ full count
lsq_occupancy                1.1057 # avg LSQ occupancy (insn's)
lsq_rate                     0.8047 # avg LSQ dispatch rate (insn/cycle)
lsq_latency                  1.3740 # avg LSQ occupant latency (cycle's)
lsq_full                     0.0321 # fraction of time (cycle's) LSQ was full
sim_slip                     416677 # total number of slip cycles
avg_sim_slip                 9.6228 # the average slip between issue and retirement
bpred_bimod.lookups            8840 # total number of bpred lookups
bpred_bimod.updates            7733 # total number of updates
bpred_bimod.addr_hits          6598 # total number of address-predicted hits
bpred_bimod.dir_hits           6899 # total number of direction-predicted hits (includes addr-hits)
bpred_bimod.misses              834 # total number of misses
bpred_bimod.jr_hits             619 # total number of address-predicted hits for JR's
bpred_bimod.jr_seen             661 # total number of JR's seen
bpred_bimod.jr_non_ras_hits.PP            4 # total number of address-predicted hits for non-RAS JR's
bpred_bimod.jr_non_ras_seen.PP           19 # total number of non-RAS JR's seen
bpred_bimod.bpred_addr_rate    0.8532 # branch address-prediction rate (i.e., addr-hits/updates)
bpred_bimod.bpred_dir_rate    0.8922 # branch direction-prediction rate (i.e., all-hits/updates)
bpred_bimod.bpred_jr_rate    0.9365 # JR address-prediction rate (i.e., JR addr-hits/JRs seen)
bpred_bimod.bpred_jr_non_ras_rate.PP    0.2105 # non-RAS JR addr-pred rate (ie, non-RAS JR hits/JRs seen)
bpred_bimod.retstack_pushes          729 # total number of address pushed onto ret-addr stack
bpred_bimod.retstack_pops          742 # total number of address popped off of ret-addr stack
bpred_bimod.used_ras.PP          642 # total number of RAS predictions used
bpred_bimod.ras_hits.PP          615 # total number of RAS hits
bpred_bimod.ras_rate.PP    0.9579 # RAS prediction rate (i.e., RAS hits/used RAS)
il1.accesses                  52091 # total number of accesses
il1.hits                      48590 # total number of hits
il1.misses                     3501 # total number of misses
il1.replacements               3007 # total number of replacements
il1.writebacks                    0 # total number of writebacks
il1.invalidations                 0 # total number of invalidations
il1.miss_rate                0.0672 # miss rate (i.e., misses/ref)
il1.repl_rate                0.0577 # replacement rate (i.e., repls/ref)
il1.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
il1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
dl1.accesses                  12375 # total number of accesses
dl1.hits                      11899 # total number of hits
dl1.misses                      476 # total number of misses
dl1.replacements                 30 # total number of replacements
dl1.writebacks                   24 # total number of writebacks
dl1.invalidations                 0 # total number of invalidations
dl1.miss_rate                0.0385 # miss rate (i.e., misses/ref)
dl1.repl_rate                0.0024 # replacement rate (i.e., repls/ref)
dl1.wb_rate                  0.0019 # writeback rate (i.e., wrbks/ref)
dl1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
ul2.accesses                   4001 # total number of accesses
ul2.hits                       3164 # total number of hits
ul2.misses                      837 # total number of misses
ul2.replacements                  0 # total number of replacements
ul2.writebacks                    0 # total number of writebacks
ul2.invalidations                 0 # total number of invalidations
ul2.miss_rate                0.2092 # miss rate (i.e., misses/ref)
ul2.repl_rate                0.0000 # replacement rate (i.e., repls/ref)
ul2.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
ul2.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
itlb.accesses                 52091 # total number of accesses
itlb.hits                     52071 # total number of hits
itlb.misses                      20 # total number of misses
itlb.replacements                 0 # total number of replacements
itlb.writebacks                   0 # total number of writebacks
itlb.invalidations                0 # total number of invalidations
itlb.miss_rate               0.0004 # miss rate (i.e., misses/ref)
itlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
itlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
itlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
dtlb.accesses                 12523 # total number of accesses
dtlb.hits                     12513 # total number of hits
dtlb.misses                      10 # total number of misses
dtlb.replacements                 0 # total number of replacements
dtlb.writebacks                   0 # total number of writebacks
dtlb.invalidations                0 # total number of invalidations
dtlb.miss_rate               0.0008 # miss rate (i.e., misses/ref)
dtlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
dtlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
dtlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
sim_invalid_addrs                 0 # total non-speculative bogus addresses seen (debug var)
ld_text_base             0x00400000 # program text (code) segment base
ld_text_size                  79920 # program text (code) size in bytes
ld_data_base             0x10000000 # program initialized data segment base
ld_data_size                  12288 # program init'ed `.data' and uninit'ed `.bss' size in bytes
ld_stack_base            0x7fffc000 # program stack segment base (highest address in stack)
ld_stack_size                 16384 # program initial stack size
ld_prog_entry            0x00400140 # program entry point (initial PC)
ld_environ_base          0x7fff8000 # program environment base address address
ld_target_big_endian              0 # target executable endian-ness, non-zero if big endian
mem.page_count                   30 # total number of pages allocated
mem.page_mem                   120k # total size of memory pages allocated
mem.ptab_misses                  34 # total first level page table misses
mem.ptab_accesses            798168 # total page table accesses
mem.ptab_miss_rate           0.0000 # first level page table miss rate
```

#### `test-printf` 统计信息

```
sim: ** simulation statistics **
sim_num_insn                1259740 # total number of instructions committed
sim_num_refs                 295664 # total number of loads and stores committed
sim_num_loads                180168 # total number of loads committed
sim_num_stores          115496.0000 # total number of stores committed
sim_num_branches             254414 # total number of branches committed
sim_elapsed_time                  1 # total simulation time in seconds
sim_inst_rate          1259740.0000 # simulation speed (in insts/sec)
sim_total_insn              1419401 # total number of instructions executed
sim_total_refs               336278 # total number of loads and stores executed
sim_total_loads              209207 # total number of loads executed
sim_total_stores        127071.0000 # total number of stores executed
sim_total_branches           300478 # total number of branches executed
sim_cycle                   1157278 # total simulation time in cycles
sim_IPC                      1.0885 # instructions per cycle
sim_CPI                      0.9187 # cycles per instruction
sim_exec_BW                  1.2265 # total instructions (mis-spec + committed) per cycle
sim_IPB                      4.9515 # instruction per branch
IFQ_count                   2774721 # cumulative IFQ occupancy
IFQ_fcount                   608943 # cumulative IFQ full count
ifq_occupancy                2.3976 # avg IFQ occupancy (insn's)
ifq_rate                     1.2265 # avg IFQ dispatch rate (insn/cycle)
ifq_latency                  1.9549 # avg IFQ occupant latency (cycle's)
ifq_full                     0.5262 # fraction of time (cycle's) IFQ was full
RUU_count                  11069352 # cumulative RUU occupancy
RUU_fcount                   463074 # cumulative RUU full count
ruu_occupancy                9.5650 # avg RUU occupancy (insn's)
ruu_rate                     1.2265 # avg RUU dispatch rate (insn/cycle)
ruu_latency                  7.7986 # avg RUU occupant latency (cycle's)
ruu_full                     0.4001 # fraction of time (cycle's) RUU was full
LSQ_count                   1620944 # cumulative LSQ occupancy
LSQ_fcount                    22566 # cumulative LSQ full count
lsq_occupancy                1.4007 # avg LSQ occupancy (insn's)
lsq_rate                     1.2265 # avg LSQ dispatch rate (insn/cycle)
lsq_latency                  1.1420 # avg LSQ occupant latency (cycle's)
lsq_full                     0.0195 # fraction of time (cycle's) LSQ was full
sim_slip                   13684914 # total number of slip cycles
avg_sim_slip                10.8633 # the average slip between issue and retirement
bpred_bimod.lookups          314531 # total number of bpred lookups
bpred_bimod.updates          254414 # total number of updates
bpred_bimod.addr_hits        231371 # total number of address-predicted hits
bpred_bimod.dir_hits         233813 # total number of direction-predicted hits (includes addr-hits)
bpred_bimod.misses            20601 # total number of misses
bpred_bimod.jr_hits           21142 # total number of address-predicted hits for JR's
bpred_bimod.jr_seen           23189 # total number of JR's seen
bpred_bimod.jr_non_ras_hits.PP          354 # total number of address-predicted hits for non-RAS JR's
bpred_bimod.jr_non_ras_seen.PP         2191 # total number of non-RAS JR's seen
bpred_bimod.bpred_addr_rate    0.9094 # branch address-prediction rate (i.e., addr-hits/updates)
bpred_bimod.bpred_dir_rate    0.9190 # branch direction-prediction rate (i.e., all-hits/updates)
bpred_bimod.bpred_jr_rate    0.9117 # JR address-prediction rate (i.e., JR addr-hits/JRs seen)
bpred_bimod.bpred_jr_non_ras_rate.PP    0.1616 # non-RAS JR addr-pred rate (ie, non-RAS JR hits/JRs seen)
bpred_bimod.retstack_pushes        23909 # total number of address pushed onto ret-addr stack
bpred_bimod.retstack_pops        22692 # total number of address popped off of ret-addr stack
bpred_bimod.used_ras.PP        20998 # total number of RAS predictions used
bpred_bimod.ras_hits.PP        20788 # total number of RAS hits
bpred_bimod.ras_rate.PP    0.9900 # RAS prediction rate (i.e., RAS hits/used RAS)
il1.accesses                1544798 # total number of accesses
il1.hits                    1488927 # total number of hits
il1.misses                    55871 # total number of misses
il1.replacements              55361 # total number of replacements
il1.writebacks                    0 # total number of writebacks
il1.invalidations                 0 # total number of invalidations
il1.miss_rate                0.0362 # miss rate (i.e., misses/ref)
il1.repl_rate                0.0358 # replacement rate (i.e., repls/ref)
il1.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
il1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
dl1.accesses                 300938 # total number of accesses
dl1.hits                     300245 # total number of hits
dl1.misses                      693 # total number of misses
dl1.replacements                181 # total number of replacements
dl1.writebacks                  177 # total number of writebacks
dl1.invalidations                 0 # total number of invalidations
dl1.miss_rate                0.0023 # miss rate (i.e., misses/ref)
dl1.repl_rate                0.0006 # replacement rate (i.e., repls/ref)
dl1.wb_rate                  0.0006 # writeback rate (i.e., wrbks/ref)
dl1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
ul2.accesses                  56741 # total number of accesses
ul2.hits                      55570 # total number of hits
ul2.misses                     1171 # total number of misses
ul2.replacements                  0 # total number of replacements
ul2.writebacks                    0 # total number of writebacks
ul2.invalidations                 0 # total number of invalidations
ul2.miss_rate                0.0206 # miss rate (i.e., misses/ref)
ul2.repl_rate                0.0000 # replacement rate (i.e., repls/ref)
ul2.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
ul2.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
itlb.accesses               1544798 # total number of accesses
itlb.hits                   1544779 # total number of hits
itlb.misses                      19 # total number of misses
itlb.replacements                 0 # total number of replacements
itlb.writebacks                   0 # total number of writebacks
itlb.invalidations                0 # total number of invalidations
itlb.miss_rate               0.0000 # miss rate (i.e., misses/ref)
itlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
itlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
itlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
dtlb.accesses                305408 # total number of accesses
dtlb.hits                    305399 # total number of hits
dtlb.misses                       9 # total number of misses
dtlb.replacements                 0 # total number of replacements
dtlb.writebacks                   0 # total number of writebacks
dtlb.invalidations                0 # total number of invalidations
dtlb.miss_rate               0.0000 # miss rate (i.e., misses/ref)
dtlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
dtlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
dtlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
sim_invalid_addrs                 0 # total non-speculative bogus addresses seen (debug var)
ld_text_base             0x00400000 # program text (code) segment base
ld_text_size                  74640 # program text (code) size in bytes
ld_data_base             0x10000000 # program initialized data segment base
ld_data_size                  13636 # program init'ed `.data' and uninit'ed `.bss' size in bytes
ld_stack_base            0x7fffc000 # program stack segment base (highest address in stack)
ld_stack_size                 16384 # program initial stack size
ld_prog_entry            0x00400140 # program entry point (initial PC)
ld_environ_base          0x7fff8000 # program environment base address address
ld_target_big_endian              0 # target executable endian-ness, non-zero if big endian
mem.page_count                   28 # total number of pages allocated
mem.page_mem                   112k # total size of memory pages allocated
mem.ptab_misses                  32 # total first level page table misses
mem.ptab_accesses           8605812 # total page table accesses
mem.ptab_miss_rate           0.0000 # first level page table miss rate
```

### 结果二

#### `basicmath` 统计信息

```
sim: ** simulation statistics **
sim_num_insn              158075378 # total number of instructions committed
sim_num_refs               41466673 # total number of loads and stores committed
sim_num_loads              25118907 # total number of loads committed
sim_num_stores         16347766.0000 # total number of stores committed
sim_num_branches           27004256 # total number of branches committed
sim_elapsed_time                129 # total simulation time in seconds
sim_inst_rate          1225390.5271 # simulation speed (in insts/sec)
sim_total_insn            170010362 # total number of instructions executed
sim_total_refs             44650359 # total number of loads and stores executed
sim_total_loads            27396715 # total number of loads executed
sim_total_stores       17253644.0000 # total number of stores executed
sim_total_branches         29699554 # total number of branches executed
sim_cycle                 162559114 # total simulation time in cycles
sim_IPC                      0.9724 # instructions per cycle
sim_CPI                      1.0284 # cycles per instruction
sim_exec_BW                  1.0458 # total instructions (mis-spec + committed) per cycle
sim_IPB                      5.8537 # instruction per branch
IFQ_count                 243164252 # cumulative IFQ occupancy
IFQ_fcount                 50604153 # cumulative IFQ full count
ifq_occupancy                1.4959 # avg IFQ occupancy (insn's)
ifq_rate                     1.0458 # avg IFQ dispatch rate (insn/cycle)
ifq_latency                  1.4303 # avg IFQ occupant latency (cycle's)
ifq_full                     0.3113 # fraction of time (cycle's) IFQ was full
RUU_count                1048670382 # cumulative RUU occupancy
RUU_fcount                 28206448 # cumulative RUU full count
ruu_occupancy                6.4510 # avg RUU occupancy (insn's)
ruu_rate                     1.0458 # avg RUU dispatch rate (insn/cycle)
ruu_latency                  6.1683 # avg RUU occupant latency (cycle's)
ruu_full                     0.1735 # fraction of time (cycle's) RUU was full
LSQ_count                 235317859 # cumulative LSQ occupancy
LSQ_fcount                  2684901 # cumulative LSQ full count
lsq_occupancy                1.4476 # avg LSQ occupancy (insn's)
lsq_rate                     1.0458 # avg LSQ dispatch rate (insn/cycle)
lsq_latency                  1.3841 # avg LSQ occupant latency (cycle's)
lsq_full                     0.0165 # fraction of time (cycle's) LSQ was full
sim_slip                 1447903715 # total number of slip cycles
avg_sim_slip                 9.1596 # the average slip between issue and retirement
bpred_bimod.lookups        30554524 # total number of bpred lookups
bpred_bimod.updates        27004256 # total number of updates
bpred_bimod.addr_hits      24590284 # total number of address-predicted hits
bpred_bimod.dir_hits       24677365 # total number of direction-predicted hits (includes addr-hits)
bpred_bimod.misses          2326891 # total number of misses
bpred_bimod.jr_hits         2714621 # total number of address-predicted hits for JR's
bpred_bimod.jr_seen         2803613 # total number of JR's seen
bpred_bimod.jr_non_ras_hits.PP        24844 # total number of address-predicted hits for non-RAS JR's
bpred_bimod.jr_non_ras_seen.PP        27667 # total number of non-RAS JR's seen
bpred_bimod.bpred_addr_rate    0.9106 # branch address-prediction rate (i.e., addr-hits/updates)
bpred_bimod.bpred_dir_rate    0.9138 # branch direction-prediction rate (i.e., all-hits/updates)
bpred_bimod.bpred_jr_rate    0.9683 # JR address-prediction rate (i.e., JR addr-hits/JRs seen)
bpred_bimod.bpred_jr_non_ras_rate.PP    0.8980 # non-RAS JR addr-pred rate (ie, non-RAS JR hits/JRs seen)
bpred_bimod.retstack_pushes      2949349 # total number of address pushed onto ret-addr stack
bpred_bimod.retstack_pops      3495713 # total number of address popped off of ret-addr stack
bpred_bimod.used_ras.PP      2775946 # total number of RAS predictions used
bpred_bimod.ras_hits.PP      2689777 # total number of RAS hits
bpred_bimod.ras_rate.PP    0.9690 # RAS prediction rate (i.e., RAS hits/used RAS)
il1.accesses              190945478 # total number of accesses
il1.hits                  175678633 # total number of hits
il1.misses                 15266845 # total number of misses
il1.replacements           15266335 # total number of replacements
il1.writebacks                    0 # total number of writebacks
il1.invalidations                 0 # total number of invalidations
il1.miss_rate                0.0800 # miss rate (i.e., misses/ref)
il1.repl_rate                0.0800 # replacement rate (i.e., repls/ref)
il1.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
il1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
dl1.accesses               41286741 # total number of accesses
dl1.hits                   41286011 # total number of hits
dl1.misses                      730 # total number of misses
dl1.replacements                218 # total number of replacements
dl1.writebacks                  210 # total number of writebacks
dl1.invalidations                 0 # total number of invalidations
dl1.miss_rate                0.0000 # miss rate (i.e., misses/ref)
dl1.repl_rate                0.0000 # replacement rate (i.e., repls/ref)
dl1.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
dl1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
ul2.accesses               15267785 # total number of accesses
ul2.hits                   15266569 # total number of hits
ul2.misses                     1216 # total number of misses
ul2.replacements                  0 # total number of replacements
ul2.writebacks                    0 # total number of writebacks
ul2.invalidations                 0 # total number of invalidations
ul2.miss_rate                0.0001 # miss rate (i.e., misses/ref)
ul2.repl_rate                0.0000 # replacement rate (i.e., repls/ref)
ul2.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
ul2.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
itlb.accesses             190945478 # total number of accesses
itlb.hits                 190945455 # total number of hits
itlb.misses                      23 # total number of misses
itlb.replacements                 0 # total number of replacements
itlb.writebacks                   0 # total number of writebacks
itlb.invalidations                0 # total number of invalidations
itlb.miss_rate               0.0000 # miss rate (i.e., misses/ref)
itlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
itlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
itlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
dtlb.accesses              41779597 # total number of accesses
dtlb.hits                  41779587 # total number of hits
dtlb.misses                      10 # total number of misses
dtlb.replacements                 0 # total number of replacements
dtlb.writebacks                   0 # total number of writebacks
dtlb.invalidations                0 # total number of invalidations
dtlb.miss_rate               0.0000 # miss rate (i.e., misses/ref)
dtlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
dtlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
dtlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
sim_invalid_addrs                 0 # total non-speculative bogus addresses seen (debug var)
ld_text_base             0x00400000 # program text (code) segment base
ld_text_size                  91008 # program text (code) size in bytes
ld_data_base             0x10000000 # program initialized data segment base
ld_data_size                  12848 # program init'ed `.data' and uninit'ed `.bss' size in bytes
ld_stack_base            0x7fffc000 # program stack segment base (highest address in stack)
ld_stack_size                 16384 # program initial stack size
ld_prog_entry            0x00400140 # program entry point (initial PC)
ld_environ_base          0x7fff8000 # program environment base address address
ld_target_big_endian              0 # target executable endian-ness, non-zero if big endian
mem.page_count                   33 # total number of pages allocated
mem.page_mem                   132k # total size of memory pages allocated
mem.ptab_misses                  37 # total first level page table misses
mem.ptab_accesses        1065855782 # total page table accesses
mem.ptab_miss_rate           0.0000 # first level page table miss rate
```

#### `bitcount` 统计信息

```
sim: ** simulation statistics **
sim_num_insn               43613054 # total number of instructions committed
sim_num_refs                5115877 # total number of loads and stores committed
sim_num_loads               3833009 # total number of loads committed
sim_num_stores         1282868.0000 # total number of stores committed
sim_num_branches            8911507 # total number of branches committed
sim_elapsed_time                 21 # total simulation time in seconds
sim_inst_rate          2076812.0952 # simulation speed (in insts/sec)
sim_total_insn             46169549 # total number of instructions executed
sim_total_refs              5417480 # total number of loads and stores executed
sim_total_loads             3984150 # total number of loads executed
sim_total_stores       1433330.0000 # total number of stores executed
sim_total_branches          9437639 # total number of branches executed
sim_cycle                  16608320 # total simulation time in cycles
sim_IPC                      2.6260 # instructions per cycle
sim_CPI                      0.3808 # cycles per instruction
sim_exec_BW                  2.7799 # total instructions (mis-spec + committed) per cycle
sim_IPB                      4.8940 # instruction per branch
IFQ_count                  53966527 # cumulative IFQ occupancy
IFQ_fcount                 10050149 # cumulative IFQ full count
ifq_occupancy                3.2494 # avg IFQ occupancy (insn's)
ifq_rate                     2.7799 # avg IFQ dispatch rate (insn/cycle)
ifq_latency                  1.1689 # avg IFQ occupant latency (cycle's)
ifq_full                     0.6051 # fraction of time (cycle's) IFQ was full
RUU_count                 209966005 # cumulative RUU occupancy
RUU_fcount                  6341140 # cumulative RUU full count
ruu_occupancy               12.6422 # avg RUU occupancy (insn's)
ruu_rate                     2.7799 # avg RUU dispatch rate (insn/cycle)
ruu_latency                  4.5477 # avg RUU occupant latency (cycle's)
ruu_full                     0.3818 # fraction of time (cycle's) RUU was full
LSQ_count                  26412281 # cumulative LSQ occupancy
LSQ_fcount                   227170 # cumulative LSQ full count
lsq_occupancy                1.5903 # avg LSQ occupancy (insn's)
lsq_rate                     2.7799 # avg LSQ dispatch rate (insn/cycle)
lsq_latency                  0.5721 # avg LSQ occupant latency (cycle's)
lsq_full                     0.0137 # fraction of time (cycle's) LSQ was full
sim_slip                  278037998 # total number of slip cycles
avg_sim_slip                 6.3751 # the average slip between issue and retirement
bpred_bimod.lookups         9738002 # total number of bpred lookups
bpred_bimod.updates         8911507 # total number of updates
bpred_bimod.addr_hits       8609942 # total number of address-predicted hits
bpred_bimod.dir_hits        8685341 # total number of direction-predicted hits (includes addr-hits)
bpred_bimod.misses           226166 # total number of misses
bpred_bimod.jr_hits          975820 # total number of address-predicted hits for JR's
bpred_bimod.jr_seen         1050893 # total number of JR's seen
bpred_bimod.jr_non_ras_hits.PP           13 # total number of address-predicted hits for non-RAS JR's
bpred_bimod.jr_non_ras_seen.PP           43 # total number of non-RAS JR's seen
bpred_bimod.bpred_addr_rate    0.9662 # branch address-prediction rate (i.e., addr-hits/updates)
bpred_bimod.bpred_dir_rate    0.9746 # branch direction-prediction rate (i.e., all-hits/updates)
bpred_bimod.bpred_jr_rate    0.9286 # JR address-prediction rate (i.e., JR addr-hits/JRs seen)
bpred_bimod.bpred_jr_non_ras_rate.PP    0.3023 # non-RAS JR addr-pred rate (ie, non-RAS JR hits/JRs seen)
bpred_bimod.retstack_pushes      1200978 # total number of address pushed onto ret-addr stack
bpred_bimod.retstack_pops      1125983 # total number of address popped off of ret-addr stack
bpred_bimod.used_ras.PP      1050850 # total number of RAS predictions used
bpred_bimod.ras_hits.PP       975807 # total number of RAS hits
bpred_bimod.ras_rate.PP    0.9286 # RAS prediction rate (i.e., RAS hits/used RAS)
il1.accesses               47226526 # total number of accesses
il1.hits                   47221289 # total number of hits
il1.misses                     5237 # total number of misses
il1.replacements               4735 # total number of replacements
il1.writebacks                    0 # total number of writebacks
il1.invalidations                 0 # total number of invalidations
il1.miss_rate                0.0001 # miss rate (i.e., misses/ref)
il1.repl_rate                0.0001 # replacement rate (i.e., repls/ref)
il1.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
il1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
dl1.accesses                5041022 # total number of accesses
dl1.hits                    5040506 # total number of hits
dl1.misses                      516 # total number of misses
dl1.replacements                 44 # total number of replacements
dl1.writebacks                   40 # total number of writebacks
dl1.invalidations                 0 # total number of invalidations
dl1.miss_rate                0.0001 # miss rate (i.e., misses/ref)
dl1.repl_rate                0.0000 # replacement rate (i.e., repls/ref)
dl1.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
dl1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
ul2.accesses                   5793 # total number of accesses
ul2.hits                       4841 # total number of hits
ul2.misses                      952 # total number of misses
ul2.replacements                  0 # total number of replacements
ul2.writebacks                    0 # total number of writebacks
ul2.invalidations                 0 # total number of invalidations
ul2.miss_rate                0.1643 # miss rate (i.e., misses/ref)
ul2.repl_rate                0.0000 # replacement rate (i.e., repls/ref)
ul2.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
ul2.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
itlb.accesses              47226526 # total number of accesses
itlb.hits                  47226506 # total number of hits
itlb.misses                      20 # total number of misses
itlb.replacements                 0 # total number of replacements
itlb.writebacks                   0 # total number of writebacks
itlb.invalidations                0 # total number of invalidations
itlb.miss_rate               0.0000 # miss rate (i.e., misses/ref)
itlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
itlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
itlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
dtlb.accesses               5116215 # total number of accesses
dtlb.hits                   5116206 # total number of hits
dtlb.misses                       9 # total number of misses
dtlb.replacements                 0 # total number of replacements
dtlb.writebacks                   0 # total number of writebacks
dtlb.invalidations                0 # total number of invalidations
dtlb.miss_rate               0.0000 # miss rate (i.e., misses/ref)
dtlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
dtlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
dtlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
sim_invalid_addrs                 0 # total non-speculative bogus addresses seen (debug var)
ld_text_base             0x00400000 # program text (code) segment base
ld_text_size                  81728 # program text (code) size in bytes
ld_data_base             0x10000000 # program initialized data segment base
ld_data_size                   9472 # program init'ed `.data' and uninit'ed `.bss' size in bytes
ld_stack_base            0x7fffc000 # program stack segment base (highest address in stack)
ld_stack_size                 16384 # program initial stack size
ld_prog_entry            0x00400140 # program entry point (initial PC)
ld_environ_base          0x7fff8000 # program environment base address address
ld_target_big_endian              0 # target executable endian-ness, non-zero if big endian
mem.page_count                   29 # total number of pages allocated
mem.page_mem                   116k # total size of memory pages allocated
mem.ptab_misses                  33 # total first level page table misses
mem.ptab_accesses         218031420 # total page table accesses
mem.ptab_miss_rate           0.0000 # first level page table miss rate
```

#### `susan-smoothing` 统计信息

```
sim: ** simulation statistics **
sim_num_insn               25579883 # total number of instructions committed
sim_num_refs                4954663 # total number of loads and stores committed
sim_num_loads               4930895 # total number of loads committed
sim_num_stores           23768.0000 # total number of stores committed
sim_num_branches            1895931 # total number of branches committed
sim_elapsed_time                 15 # total simulation time in seconds
sim_inst_rate          1705325.5333 # simulation speed (in insts/sec)
sim_total_insn             26060774 # total number of instructions executed
sim_total_refs              5295272 # total number of loads and stores executed
sim_total_loads             5271040 # total number of loads executed
sim_total_stores         24232.0000 # total number of stores executed
sim_total_branches          1903643 # total number of branches executed
sim_cycle                  14854811 # total simulation time in cycles
sim_IPC                      1.7220 # instructions per cycle
sim_CPI                      0.5807 # cycles per instruction
sim_exec_BW                  1.7544 # total instructions (mis-spec + committed) per cycle
sim_IPB                     13.4920 # instruction per branch
IFQ_count                  54515138 # cumulative IFQ occupancy
IFQ_fcount                 12828170 # cumulative IFQ full count
ifq_occupancy                3.6699 # avg IFQ occupancy (insn's)
ifq_rate                     1.7544 # avg IFQ dispatch rate (insn/cycle)
ifq_latency                  2.0918 # avg IFQ occupant latency (cycle's)
ifq_full                     0.8636 # fraction of time (cycle's) IFQ was full
RUU_count                 229029044 # cumulative RUU occupancy
RUU_fcount                 11807183 # cumulative RUU full count
ruu_occupancy               15.4178 # avg RUU occupancy (insn's)
ruu_rate                     1.7544 # avg RUU dispatch rate (insn/cycle)
ruu_latency                  8.7883 # avg RUU occupant latency (cycle's)
ruu_full                     0.7948 # fraction of time (cycle's) RUU was full
LSQ_count                  42009505 # cumulative LSQ occupancy
LSQ_fcount                    18025 # cumulative LSQ full count
lsq_occupancy                2.8280 # avg LSQ occupancy (insn's)
lsq_rate                     1.7544 # avg LSQ dispatch rate (insn/cycle)
lsq_latency                  1.6120 # avg LSQ occupant latency (cycle's)
lsq_full                     0.0012 # fraction of time (cycle's) LSQ was full
sim_slip                  299942872 # total number of slip cycles
avg_sim_slip                11.7257 # the average slip between issue and retirement
bpred_bimod.lookups         1904060 # total number of bpred lookups
bpred_bimod.updates         1895931 # total number of updates
bpred_bimod.addr_hits       1779216 # total number of address-predicted hits
bpred_bimod.dir_hits        1779506 # total number of direction-predicted hits (includes addr-hits)
bpred_bimod.misses           116425 # total number of misses
bpred_bimod.jr_hits            2595 # total number of address-predicted hits for JR's
bpred_bimod.jr_seen            2617 # total number of JR's seen
bpred_bimod.jr_non_ras_hits.PP            1 # total number of address-predicted hits for non-RAS JR's
bpred_bimod.jr_non_ras_seen.PP            5 # total number of non-RAS JR's seen
bpred_bimod.bpred_addr_rate    0.9384 # branch address-prediction rate (i.e., addr-hits/updates)
bpred_bimod.bpred_dir_rate    0.9386 # branch direction-prediction rate (i.e., all-hits/updates)
bpred_bimod.bpred_jr_rate    0.9916 # JR address-prediction rate (i.e., JR addr-hits/JRs seen)
bpred_bimod.bpred_jr_non_ras_rate.PP    0.2000 # non-RAS JR addr-pred rate (ie, non-RAS JR hits/JRs seen)
bpred_bimod.retstack_pushes         2731 # total number of address pushed onto ret-addr stack
bpred_bimod.retstack_pops         2732 # total number of address popped off of ret-addr stack
bpred_bimod.used_ras.PP         2612 # total number of RAS predictions used
bpred_bimod.ras_hits.PP         2594 # total number of RAS hits
bpred_bimod.ras_rate.PP    0.9931 # RAS prediction rate (i.e., RAS hits/used RAS)
il1.accesses               26526091 # total number of accesses
il1.hits                   26524813 # total number of hits
il1.misses                     1278 # total number of misses
il1.replacements                789 # total number of replacements
il1.writebacks                    0 # total number of writebacks
il1.invalidations                 0 # total number of invalidations
il1.miss_rate                0.0000 # miss rate (i.e., misses/ref)
il1.repl_rate                0.0000 # replacement rate (i.e., repls/ref)
il1.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
il1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
dl1.accesses                4954748 # total number of accesses
dl1.hits                    4953204 # total number of hits
dl1.misses                     1544 # total number of misses
dl1.replacements               1032 # total number of replacements
dl1.writebacks                  836 # total number of writebacks
dl1.invalidations                 0 # total number of invalidations
dl1.miss_rate                0.0003 # miss rate (i.e., misses/ref)
dl1.repl_rate                0.0002 # replacement rate (i.e., repls/ref)
dl1.wb_rate                  0.0002 # writeback rate (i.e., wrbks/ref)
dl1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
ul2.accesses                   3658 # total number of accesses
ul2.hits                       2473 # total number of hits
ul2.misses                     1185 # total number of misses
ul2.replacements                  0 # total number of replacements
ul2.writebacks                    0 # total number of writebacks
ul2.invalidations                 0 # total number of invalidations
ul2.miss_rate                0.3239 # miss rate (i.e., misses/ref)
ul2.repl_rate                0.0000 # replacement rate (i.e., repls/ref)
ul2.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
ul2.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
itlb.accesses              26526091 # total number of accesses
itlb.hits                  26526069 # total number of hits
itlb.misses                      22 # total number of misses
itlb.replacements                 0 # total number of replacements
itlb.writebacks                   0 # total number of writebacks
itlb.invalidations                0 # total number of invalidations
itlb.miss_rate               0.0000 # miss rate (i.e., misses/ref)
itlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
itlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
itlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
dtlb.accesses               4954784 # total number of accesses
dtlb.hits                   4954764 # total number of hits
dtlb.misses                      20 # total number of misses
dtlb.replacements                 0 # total number of replacements
dtlb.writebacks                   0 # total number of writebacks
dtlb.invalidations                0 # total number of invalidations
dtlb.miss_rate               0.0000 # miss rate (i.e., misses/ref)
dtlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
dtlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
dtlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
sim_invalid_addrs                 0 # total non-speculative bogus addresses seen (debug var)
ld_text_base             0x00400000 # program text (code) segment base
ld_text_size                 137744 # program text (code) size in bytes
ld_data_base             0x10000000 # program initialized data segment base
ld_data_size                  12976 # program init'ed `.data' and uninit'ed `.bss' size in bytes
ld_stack_base            0x7fffc000 # program stack segment base (highest address in stack)
ld_stack_size                 16384 # program initial stack size
ld_prog_entry            0x00400140 # program entry point (initial PC)
ld_environ_base          0x7fff8000 # program environment base address address
ld_target_big_endian              0 # target executable endian-ness, non-zero if big endian
mem.page_count                   54 # total number of pages allocated
mem.page_mem                   216k # total size of memory pages allocated
mem.ptab_misses                  54 # total first level page table misses
mem.ptab_accesses         117945118 # total page table accesses
mem.ptab_miss_rate           0.0000 # first level page table miss rate
```

#### `susan-edges` 统计信息

```
sim: ** simulation statistics **
sim_num_insn                2078413 # total number of instructions committed
sim_num_refs                 801747 # total number of loads and stores committed
sim_num_loads                733973 # total number of loads committed
sim_num_stores           67774.0000 # total number of stores committed
sim_num_branches              92261 # total number of branches committed
sim_elapsed_time                  1 # total simulation time in seconds
sim_inst_rate          2078413.0000 # simulation speed (in insts/sec)
sim_total_insn              2116977 # total number of instructions executed
sim_total_refs               815008 # total number of loads and stores executed
sim_total_loads              745744 # total number of loads executed
sim_total_stores         69264.0000 # total number of stores executed
sim_total_branches            96352 # total number of branches executed
sim_cycle                    934075 # total simulation time in cycles
sim_IPC                      2.2251 # instructions per cycle
sim_CPI                      0.4494 # cycles per instruction
sim_exec_BW                  2.2664 # total instructions (mis-spec + committed) per cycle
sim_IPB                     22.5275 # instruction per branch
IFQ_count                   3498342 # cumulative IFQ occupancy
IFQ_fcount                   843939 # cumulative IFQ full count
ifq_occupancy                3.7452 # avg IFQ occupancy (insn's)
ifq_rate                     2.2664 # avg IFQ dispatch rate (insn/cycle)
ifq_latency                  1.6525 # avg IFQ occupant latency (cycle's)
ifq_full                     0.9035 # fraction of time (cycle's) IFQ was full
RUU_count                  13392211 # cumulative RUU occupancy
RUU_fcount                   577453 # cumulative RUU full count
ruu_occupancy               14.3374 # avg RUU occupancy (insn's)
ruu_rate                     2.2664 # avg RUU dispatch rate (insn/cycle)
ruu_latency                  6.3261 # avg RUU occupant latency (cycle's)
ruu_full                     0.6182 # fraction of time (cycle's) RUU was full
LSQ_count                   5081937 # cumulative LSQ occupancy
LSQ_fcount                   262541 # cumulative LSQ full count
lsq_occupancy                5.4406 # avg LSQ occupancy (insn's)
lsq_rate                     2.2664 # avg LSQ dispatch rate (insn/cycle)
lsq_latency                  2.4006 # avg LSQ occupant latency (cycle's)
lsq_full                     0.2811 # fraction of time (cycle's) LSQ was full
sim_slip                   21068697 # total number of slip cycles
avg_sim_slip                10.1369 # the average slip between issue and retirement
bpred_bimod.lookups           97789 # total number of bpred lookups
bpred_bimod.updates           92261 # total number of updates
bpred_bimod.addr_hits         88532 # total number of address-predicted hits
bpred_bimod.dir_hits          88848 # total number of direction-predicted hits (includes addr-hits)
bpred_bimod.misses             3413 # total number of misses
bpred_bimod.jr_hits            2489 # total number of address-predicted hits for JR's
bpred_bimod.jr_seen            2511 # total number of JR's seen
bpred_bimod.jr_non_ras_hits.PP            1 # total number of address-predicted hits for non-RAS JR's
bpred_bimod.jr_non_ras_seen.PP            5 # total number of non-RAS JR's seen
bpred_bimod.bpred_addr_rate    0.9596 # branch address-prediction rate (i.e., addr-hits/updates)
bpred_bimod.bpred_dir_rate    0.9630 # branch direction-prediction rate (i.e., all-hits/updates)
bpred_bimod.bpred_jr_rate    0.9912 # JR address-prediction rate (i.e., JR addr-hits/JRs seen)
bpred_bimod.bpred_jr_non_ras_rate.PP    0.2000 # non-RAS JR addr-pred rate (ie, non-RAS JR hits/JRs seen)
bpred_bimod.retstack_pushes         2567 # total number of address pushed onto ret-addr stack
bpred_bimod.retstack_pops         2520 # total number of address popped off of ret-addr stack
bpred_bimod.used_ras.PP         2506 # total number of RAS predictions used
bpred_bimod.ras_hits.PP         2488 # total number of RAS hits
bpred_bimod.ras_rate.PP    0.9928 # RAS prediction rate (i.e., RAS hits/used RAS)
il1.accesses                2130379 # total number of accesses
il1.hits                    2128692 # total number of hits
il1.misses                     1687 # total number of misses
il1.replacements               1185 # total number of replacements
il1.writebacks                    0 # total number of writebacks
il1.invalidations                 0 # total number of invalidations
il1.miss_rate                0.0008 # miss rate (i.e., misses/ref)
il1.repl_rate                0.0006 # replacement rate (i.e., repls/ref)
il1.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
il1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
dl1.accesses                 796434 # total number of accesses
dl1.hits                     792534 # total number of hits
dl1.misses                     3900 # total number of misses
dl1.replacements               3388 # total number of replacements
dl1.writebacks                 2122 # total number of writebacks
dl1.invalidations                 0 # total number of invalidations
dl1.miss_rate                0.0049 # miss rate (i.e., misses/ref)
dl1.repl_rate                0.0043 # replacement rate (i.e., repls/ref)
dl1.wb_rate                  0.0027 # writeback rate (i.e., wrbks/ref)
dl1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
ul2.accesses                   7709 # total number of accesses
ul2.hits                       5956 # total number of hits
ul2.misses                     1753 # total number of misses
ul2.replacements                  0 # total number of replacements
ul2.writebacks                    0 # total number of writebacks
ul2.invalidations                 0 # total number of invalidations
ul2.miss_rate                0.2274 # miss rate (i.e., misses/ref)
ul2.repl_rate                0.0000 # replacement rate (i.e., repls/ref)
ul2.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
ul2.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
itlb.accesses               2130379 # total number of accesses
itlb.hits                   2130355 # total number of hits
itlb.misses                      24 # total number of misses
itlb.replacements                 0 # total number of replacements
itlb.writebacks                   0 # total number of writebacks
itlb.invalidations                0 # total number of invalidations
itlb.miss_rate               0.0000 # miss rate (i.e., misses/ref)
itlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
itlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
itlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
dtlb.accesses                809215 # total number of accesses
dtlb.hits                    809189 # total number of hits
dtlb.misses                      26 # total number of misses
dtlb.replacements                 0 # total number of replacements
dtlb.writebacks                   0 # total number of writebacks
dtlb.invalidations                0 # total number of invalidations
dtlb.miss_rate               0.0000 # miss rate (i.e., misses/ref)
dtlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
dtlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
dtlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
sim_invalid_addrs                 0 # total non-speculative bogus addresses seen (debug var)
ld_text_base             0x00400000 # program text (code) segment base
ld_text_size                 137744 # program text (code) size in bytes
ld_data_base             0x10000000 # program initialized data segment base
ld_data_size                  12976 # program init'ed `.data' and uninit'ed `.bss' size in bytes
ld_stack_base            0x7fffc000 # program stack segment base (highest address in stack)
ld_stack_size                 16384 # program initial stack size
ld_prog_entry            0x00400140 # program entry point (initial PC)
ld_environ_base          0x7fff8000 # program environment base address address
ld_target_big_endian              0 # target executable endian-ness, non-zero if big endian
mem.page_count                   60 # total number of pages allocated
mem.page_mem                   240k # total size of memory pages allocated
mem.ptab_misses                  60 # total first level page table misses
mem.ptab_accesses          12434922 # total page table accesses
mem.ptab_miss_rate           0.0000 # first level page table miss rate
```

#### `susan-corners` 统计信息

```
sim: ** simulation statistics **
sim_num_insn                 963476 # total number of instructions committed
sim_num_refs                 335791 # total number of loads and stores committed
sim_num_loads                310535 # total number of loads committed
sim_num_stores           25256.0000 # total number of stores committed
sim_num_branches              55326 # total number of branches committed
sim_elapsed_time                  1 # total simulation time in seconds
sim_inst_rate           963476.0000 # simulation speed (in insts/sec)
sim_total_insn               989211 # total number of instructions executed
sim_total_refs               342149 # total number of loads and stores executed
sim_total_loads              316665 # total number of loads executed
sim_total_stores         25484.0000 # total number of stores executed
sim_total_branches            58169 # total number of branches executed
sim_cycle                    455582 # total simulation time in cycles
sim_IPC                      2.1148 # instructions per cycle
sim_CPI                      0.4729 # cycles per instruction
sim_exec_BW                  2.1713 # total instructions (mis-spec + committed) per cycle
sim_IPB                     17.4145 # instruction per branch
IFQ_count                   1650469 # cumulative IFQ occupancy
IFQ_fcount                   401853 # cumulative IFQ full count
ifq_occupancy                3.6228 # avg IFQ occupancy (insn's)
ifq_rate                     2.1713 # avg IFQ dispatch rate (insn/cycle)
ifq_latency                  1.6685 # avg IFQ occupant latency (cycle's)
ifq_full                     0.8821 # fraction of time (cycle's) IFQ was full
RUU_count                   6432004 # cumulative RUU occupancy
RUU_fcount                   311053 # cumulative RUU full count
ruu_occupancy               14.1182 # avg RUU occupancy (insn's)
ruu_rate                     2.1713 # avg RUU dispatch rate (insn/cycle)
ruu_latency                  6.5022 # avg RUU occupant latency (cycle's)
ruu_full                     0.6828 # fraction of time (cycle's) RUU was full
LSQ_count                   2240420 # cumulative LSQ occupancy
LSQ_fcount                    72722 # cumulative LSQ full count
lsq_occupancy                4.9177 # avg LSQ occupancy (insn's)
lsq_rate                     2.1713 # avg LSQ dispatch rate (insn/cycle)
lsq_latency                  2.2649 # avg LSQ occupant latency (cycle's)
lsq_full                     0.1596 # fraction of time (cycle's) LSQ was full
sim_slip                    9844324 # total number of slip cycles
avg_sim_slip                10.2175 # the average slip between issue and retirement
bpred_bimod.lookups           58932 # total number of bpred lookups
bpred_bimod.updates           55326 # total number of updates
bpred_bimod.addr_hits         52596 # total number of address-predicted hits
bpred_bimod.dir_hits          52909 # total number of direction-predicted hits (includes addr-hits)
bpred_bimod.misses             2417 # total number of misses
bpred_bimod.jr_hits            2499 # total number of address-predicted hits for JR's
bpred_bimod.jr_seen            2523 # total number of JR's seen
bpred_bimod.jr_non_ras_hits.PP            1 # total number of address-predicted hits for non-RAS JR's
bpred_bimod.jr_non_ras_seen.PP            5 # total number of non-RAS JR's seen
bpred_bimod.bpred_addr_rate    0.9507 # branch address-prediction rate (i.e., addr-hits/updates)
bpred_bimod.bpred_dir_rate    0.9563 # branch direction-prediction rate (i.e., all-hits/updates)
bpred_bimod.bpred_jr_rate    0.9905 # JR address-prediction rate (i.e., JR addr-hits/JRs seen)
bpred_bimod.bpred_jr_non_ras_rate.PP    0.2000 # non-RAS JR addr-pred rate (ie, non-RAS JR hits/JRs seen)
bpred_bimod.retstack_pushes         2581 # total number of address pushed onto ret-addr stack
bpred_bimod.retstack_pops         2539 # total number of address popped off of ret-addr stack
bpred_bimod.used_ras.PP         2518 # total number of RAS predictions used
bpred_bimod.ras_hits.PP         2498 # total number of RAS hits
bpred_bimod.ras_rate.PP    0.9921 # RAS prediction rate (i.e., RAS hits/used RAS)
il1.accesses                 998582 # total number of accesses
il1.hits                     997054 # total number of hits
il1.misses                     1528 # total number of misses
il1.replacements               1036 # total number of replacements
il1.writebacks                    0 # total number of writebacks
il1.invalidations                 0 # total number of invalidations
il1.miss_rate                0.0015 # miss rate (i.e., misses/ref)
il1.repl_rate                0.0010 # replacement rate (i.e., repls/ref)
il1.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
il1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
dl1.accesses                 339029 # total number of accesses
dl1.hits                     336167 # total number of hits
dl1.misses                     2862 # total number of misses
dl1.replacements               2350 # total number of replacements
dl1.writebacks                 1633 # total number of writebacks
dl1.invalidations                 0 # total number of invalidations
dl1.miss_rate                0.0084 # miss rate (i.e., misses/ref)
dl1.repl_rate                0.0069 # replacement rate (i.e., repls/ref)
dl1.wb_rate                  0.0048 # writeback rate (i.e., wrbks/ref)
dl1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
ul2.accesses                   6023 # total number of accesses
ul2.hits                       4433 # total number of hits
ul2.misses                     1590 # total number of misses
ul2.replacements                  0 # total number of replacements
ul2.writebacks                    0 # total number of writebacks
ul2.invalidations                 0 # total number of invalidations
ul2.miss_rate                0.2640 # miss rate (i.e., misses/ref)
ul2.repl_rate                0.0000 # replacement rate (i.e., repls/ref)
ul2.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
ul2.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
itlb.accesses                998582 # total number of accesses
itlb.hits                    998559 # total number of hits
itlb.misses                      23 # total number of misses
itlb.replacements                 0 # total number of replacements
itlb.writebacks                   0 # total number of writebacks
itlb.invalidations                0 # total number of invalidations
itlb.miss_rate               0.0000 # miss rate (i.e., misses/ref)
itlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
itlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
itlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
dtlb.accesses                339058 # total number of accesses
dtlb.hits                    339028 # total number of hits
dtlb.misses                      30 # total number of misses
dtlb.replacements                 0 # total number of replacements
dtlb.writebacks                   0 # total number of writebacks
dtlb.invalidations                0 # total number of invalidations
dtlb.miss_rate               0.0001 # miss rate (i.e., misses/ref)
dtlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
dtlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
dtlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
sim_invalid_addrs                 0 # total non-speculative bogus addresses seen (debug var)
ld_text_base             0x00400000 # program text (code) segment base
ld_text_size                 137744 # program text (code) size in bytes
ld_data_base             0x10000000 # program initialized data segment base
ld_data_size                  12976 # program init'ed `.data' and uninit'ed `.bss' size in bytes
ld_stack_base            0x7fffc000 # program stack segment base (highest address in stack)
ld_stack_size                 16384 # program initial stack size
ld_prog_entry            0x00400140 # program entry point (initial PC)
ld_environ_base          0x7fff8000 # program environment base address address
ld_target_big_endian              0 # target executable endian-ness, non-zero if big endian
mem.page_count                   64 # total number of pages allocated
mem.page_mem                   256k # total size of memory pages allocated
mem.ptab_misses                  64 # total first level page table misses
mem.ptab_accesses           5980082 # total page table accesses
mem.ptab_miss_rate           0.0000 # first level page table miss rate
```

#### `qsort` 统计信息

```
sim: ** simulation statistics **
sim_num_insn               41903737 # total number of instructions committed
sim_num_refs               21923482 # total number of loads and stores committed
sim_num_loads              11838775 # total number of loads committed
sim_num_stores         10084707.0000 # total number of stores committed
sim_num_branches            6724738 # total number of branches committed
sim_elapsed_time                 32 # total simulation time in seconds
sim_inst_rate          1309491.7812 # simulation speed (in insts/sec)
sim_total_insn             44926754 # total number of instructions executed
sim_total_refs             23061100 # total number of loads and stores executed
sim_total_loads            12648839 # total number of loads executed
sim_total_stores       10412261.0000 # total number of stores executed
sim_total_branches          7477481 # total number of branches executed
sim_cycle                  27039174 # total simulation time in cycles
sim_IPC                      1.5497 # instructions per cycle
sim_CPI                      0.6453 # cycles per instruction
sim_exec_BW                  1.6615 # total instructions (mis-spec + committed) per cycle
sim_IPB                      6.2313 # instruction per branch
IFQ_count                  93678133 # cumulative IFQ occupancy
IFQ_fcount                 21926202 # cumulative IFQ full count
ifq_occupancy                3.4645 # avg IFQ occupancy (insn's)
ifq_rate                     1.6615 # avg IFQ dispatch rate (insn/cycle)
ifq_latency                  2.0851 # avg IFQ occupant latency (cycle's)
ifq_full                     0.8109 # fraction of time (cycle's) IFQ was full
RUU_count                 274232405 # cumulative RUU occupancy
RUU_fcount                  4426054 # cumulative RUU full count
ruu_occupancy               10.1420 # avg RUU occupancy (insn's)
ruu_rate                     1.6615 # avg RUU dispatch rate (insn/cycle)
ruu_latency                  6.1040 # avg RUU occupant latency (cycle's)
ruu_full                     0.1637 # fraction of time (cycle's) RUU was full
LSQ_count                 152195439 # cumulative LSQ occupancy
LSQ_fcount                 13183531 # cumulative LSQ full count
lsq_occupancy                5.6287 # avg LSQ occupancy (insn's)
lsq_rate                     1.6615 # avg LSQ dispatch rate (insn/cycle)
lsq_latency                  3.3876 # avg LSQ occupant latency (cycle's)
lsq_full                     0.4876 # fraction of time (cycle's) LSQ was full
sim_slip                  479421154 # total number of slip cycles
avg_sim_slip                11.4410 # the average slip between issue and retirement
bpred_bimod.lookups         7787723 # total number of bpred lookups
bpred_bimod.updates         6724738 # total number of updates
bpred_bimod.addr_hits       6259005 # total number of address-predicted hits
bpred_bimod.dir_hits        6278160 # total number of direction-predicted hits (includes addr-hits)
bpred_bimod.misses           446578 # total number of misses
bpred_bimod.jr_hits          786834 # total number of address-predicted hits for JR's
bpred_bimod.jr_seen          805730 # total number of JR's seen
bpred_bimod.jr_non_ras_hits.PP       154865 # total number of address-predicted hits for non-RAS JR's
bpred_bimod.jr_non_ras_seen.PP       154869 # total number of non-RAS JR's seen
bpred_bimod.bpred_addr_rate    0.9307 # branch address-prediction rate (i.e., addr-hits/updates)
bpred_bimod.bpred_dir_rate    0.9336 # branch direction-prediction rate (i.e., all-hits/updates)
bpred_bimod.bpred_jr_rate    0.9765 # JR address-prediction rate (i.e., JR addr-hits/JRs seen)
bpred_bimod.bpred_jr_non_ras_rate.PP    1.0000 # non-RAS JR addr-pred rate (ie, non-RAS JR hits/JRs seen)
bpred_bimod.retstack_pushes       790596 # total number of address pushed onto ret-addr stack
bpred_bimod.retstack_pops       755180 # total number of address popped off of ret-addr stack
bpred_bimod.used_ras.PP       650861 # total number of RAS predictions used
bpred_bimod.ras_hits.PP       631969 # total number of RAS hits
bpred_bimod.ras_rate.PP    0.9710 # RAS prediction rate (i.e., RAS hits/used RAS)
il1.accesses               46692073 # total number of accesses
il1.hits                   46570145 # total number of hits
il1.misses                   121928 # total number of misses
il1.replacements             121464 # total number of replacements
il1.writebacks                    0 # total number of writebacks
il1.invalidations                 0 # total number of invalidations
il1.miss_rate                0.0026 # miss rate (i.e., misses/ref)
il1.repl_rate                0.0026 # replacement rate (i.e., repls/ref)
il1.wb_rate                  0.0000 # writeback rate (i.e., wrbks/ref)
il1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
dl1.accesses               22018943 # total number of accesses
dl1.hits                   20749996 # total number of hits
dl1.misses                  1268947 # total number of misses
dl1.replacements            1268435 # total number of replacements
dl1.writebacks               670871 # total number of writebacks
dl1.invalidations                 0 # total number of invalidations
dl1.miss_rate                0.0576 # miss rate (i.e., misses/ref)
dl1.repl_rate                0.0576 # replacement rate (i.e., repls/ref)
dl1.wb_rate                  0.0305 # writeback rate (i.e., wrbks/ref)
dl1.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
ul2.accesses                2061746 # total number of accesses
ul2.hits                    1736408 # total number of hits
ul2.misses                   325338 # total number of misses
ul2.replacements             321242 # total number of replacements
ul2.writebacks               180036 # total number of writebacks
ul2.invalidations                 0 # total number of invalidations
ul2.miss_rate                0.1578 # miss rate (i.e., misses/ref)
ul2.repl_rate                0.1558 # replacement rate (i.e., repls/ref)
ul2.wb_rate                  0.0873 # writeback rate (i.e., wrbks/ref)
ul2.inv_rate                 0.0000 # invalidation rate (i.e., invs/ref)
itlb.accesses              46692073 # total number of accesses
itlb.hits                  46692056 # total number of hits
itlb.misses                      17 # total number of misses
itlb.replacements                 0 # total number of replacements
itlb.writebacks                   0 # total number of writebacks
itlb.invalidations                0 # total number of invalidations
itlb.miss_rate               0.0000 # miss rate (i.e., misses/ref)
itlb.repl_rate               0.0000 # replacement rate (i.e., repls/ref)
itlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
itlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
dtlb.accesses              22069285 # total number of accesses
dtlb.hits                  22065068 # total number of hits
dtlb.misses                    4217 # total number of misses
dtlb.replacements              4089 # total number of replacements
dtlb.writebacks                   0 # total number of writebacks
dtlb.invalidations                0 # total number of invalidations
dtlb.miss_rate               0.0002 # miss rate (i.e., misses/ref)
dtlb.repl_rate               0.0002 # replacement rate (i.e., repls/ref)
dtlb.wb_rate                 0.0000 # writeback rate (i.e., wrbks/ref)
dtlb.inv_rate                0.0000 # invalidation rate (i.e., invs/ref)
sim_invalid_addrs                 0 # total non-speculative bogus addresses seen (debug var)
ld_text_base             0x00400000 # program text (code) segment base
ld_text_size                 101552 # program text (code) size in bytes
ld_data_base             0x10000000 # program initialized data segment base
ld_data_size                  12288 # program init'ed `.data' and uninit'ed `.bss' size in bytes
ld_stack_base            0x7fffc000 # program stack segment base (highest address in stack)
ld_stack_size                 16384 # program initial stack size
ld_prog_entry            0x00400140 # program entry point (initial PC)
ld_environ_base          0x7fff8000 # program environment base address address
ld_target_big_endian              0 # target executable endian-ness, non-zero if big endian
mem.page_count                  663 # total number of pages allocated
mem.page_mem                  2652k # total size of memory pages allocated
mem.ptab_misses                 677 # total first level page table misses
mem.ptab_accesses         356344298 # total page table accesses
mem.ptab_miss_rate           0.0000 # first level page table miss rate
```
