# 词法分析实验报告

> 2014211306 2014211288
> 李俊宁

> 2016/10/21

## 实验目的

1. 实现使用DFA状态转换图分析C语言源程序的词法。
2. 深刻领会状态转换图的含义，逐步理解有限自动机。
3. 掌握手工生成词法分析器的方法，了解词法分析器的内部工作原理。

## 实验内容

设计并实现C语言的词法分析程序，实现如下功能。

1. 可以识别出用C语言编写的源程序中的每个单词符号，并以记号的形式输出每个单词符号。
2. 可以识别并跳过源程序中的注释。
3. 可以统计源程序中的语句行数、各类单词的个数、以及字符总数，并输出统计结果。
4. 检查源程序中存在的错误位置，并报告错误所在的位置。
5. 对源程序中出现的错误进行适当地恢复，使词法分析可以继续进行，对源程序进行一次扫描，即可检查并报告源程序中存在的所有词法错误。

## 算法分析

### 词法种类

```
token:
  keyword
  identifier
  constant
  string
  punctuator
```

Remarks:
- 目前完全支持**ANSI C89**词法

### 输入输出

首先 `int main (int argc, char *argv[])`

- 检查`argc` 是否合法 (`argc >= 2`)
- 打开输入/输出文件，并判断释放成功
- 将输入输出流传入`LexParsing`

然后 `void LexParsing (std::istream &is, std::ostream &os)`
进行所有分析

- 进入大循环，进入*大循环自动机* *开始状态*
- 从流中每次读入一个**字符`ch`**，并**超前扫描`nch`**
- 使用**对应算法 见下** 分析字符流
- 输出结果
  - 总行数
  - 总字符数
  - 各类单词统计
  - 符号表
  - Token表
  - 错误列表（如果不为空）

**Remarks:**
- 使用了字符**流**方式读入文件，不需要显式的创建**缓冲区**
- 每次读入字符时，自动计入**总字符数**
- 每次读到换行符时，自动计入**总行数**
- 根据当前字符`ch`判断进入哪种分析自动机
  - 在分析**十进制整数/浮点数/./...标点**时，使用**显式状态实现的**自动机
  - **其余**分析均使用**隐式状态实现的**自动机（由于状态较少）
- **错误类型** 见 *测试样例*

### 分析空字符

- 判断`ch`是否为**空字符**
  - 进入**空字符**的自动机，然后*退出*

``` C
white-space: one of
  ' '
  '\t' '\v' '\r' '\n' (escape-sequence)

new-line:
  '\n' (escape-sequence)
```

### 分析关键字/标识符

- 判断`ch`是否为**标识符/关键字**的*开始字符*
  - 进入**标识符/关键字**的自动机，**循环超前扫描`nch`**
    - 不为**标识符/关键字**的*合法字符*时*退出*
      - 退出前，用*hash表*判断是**关键字**还是**标识符**
- 识别**文法**

```
 keyword: one of
  auto      break     case      char      const   continue
  default   do        double    else      enum    extern
  float     for       goto      if        inline  int      long
  register  restrict  return    short     signed
  sizeof    static    struct    switch    typedef
  union     unsigned  void      volatile  while

identifier:
  identifier-nondigit
  identifier identifier-nondigit
  identifier digit

identifier-nondigit: one of
  _ a b c d e f g h i j k l m
    n o p q r s t u v w x y z
    A B C D E F G H I J K L M
    N O P Q R S T U V W X Y Z

digit: one of
  0 1 2 3 4 5 6 7 8 9
```

### 分析数值常量/./...标点

- 判断`ch`是否为**数字** 或 `.`
  - 如果`ch`是`.` 或 不是`0` 或 （`ch`是`0` 且 `nch`是`.`），进入**十进制数/./...标点**的自动机，**循环超前扫描`nch`**，进入下一轮前读入`nch`
    - 如果`ch`是`.`，自动机初始状态为 `1`；否则为初始状态为 `0`
    - 状态0
      - `nch`为**数字**时，不转移
      - `nch`为 `.` 时，转移到状态2
      - `nch`为 `e` 或 `E` 时，转移到状态3
      - 否则，*退出*
    - 状态1
      - `nch`为**数字**时，转移到状态2
      - `nch`为 `.` 时，转移到状态6
      - 否则，*退出*
    - 状态2
      - `nch`为**数字**时，不转移
      - `nch`为 `e` 或 `E` 时，转移到状态3
      - 否则，*退出*
    - 状态3
      - `nch`为 `+` 或 `-` 时，转移到状态4
      - `nch`为**数字**时，转移到状态5
      - 否则，*报错*（非法指数）
    - 状态4
      - `nch`为**数字**时，转移到状态5
      - 否则，*报错*（非法指数）
    - 状态5
      - `nch`为**数字**时，不转移
      - 否则，*退出*
    - 状态6
      - `nch`为 `.` 时，转移到状态7
      - 否则，*报错*（非法标点..）
    - 状态7
      - *退出*
    - 退出前，判断终止状态
      - 如果是 `1` 或 `7`，则输出**标点**
      - 否则，输出**常量**
  - 否则如果`nch`是`X` 或 `x`，进入**十六进制数**的自动机，**循环超前扫描`nch`**
    - 不为*十六进制有效字符*时*退出*
      - 退出前，判断*输出串*是不是 `0x`
        - 如果是，则*报错*（无效0x）
  - 否则，进入**八进制数**的自动机，**循环超前扫描`nch`**
    - 不为*八进制有效字符*时*退出*
  - 输出**常量**前，进入**数值常量后缀**的自动机，**循环超前扫描`nch`**，进入下一轮前读入`nch`
    - **整数常量**初始状态为 `3`；**浮点数常量**初始状态为 `2`
    - 状态0
      - `nch`为**数字/字母**时，不转移
      - 否则，*报错*（非法后缀）
    - 状态1
      - `nch`为**数字/字母**时，转移到状态0
      - 否则，*退出*
    - 状态2
      - `nch`为 `F f L l`时，转移到状态1
      - `nch`为**其他数字/字母**时，转移到状态0
      - 否则，*退出*
    - 状态3
      - `nch`为 `U u`时，转移到状态4
      - `nch`为 `L l`时，转移到状态5
      - `nch`为**其他数字/字母**时，转移到状态0
      - 否则，*退出*
    - 状态4
      - `nch`为 `L l`时，转移到状态1
      - `nch`为**其他数字/字母**时，转移到状态0
      - 否则，*退出*
    - 状态5
      - `nch`为 `U u`时，转移到状态1
      - `nch`为**其他数字/字母**时，转移到状态0
      - 否则，*退出*
- 识别**文法**

```
constant:
  integer-constant
  floating-constant
  character-constant

integer-constant:
  decimal-constant
  octal-constant
  hexadecimal-constant
  decimal-constant integer-suffix
  octal-constant integer-suffix
  hexadecimal-constant integer-suffix

decimal-constant:
  nonzero-digit
  decimal-constant digit

octal-constant:
  0
  octal-constant octal-digit

hexadecimal-constant:
  hexadecimal-prefix hexadecimal-digit
  hexadecimal-constant hexadecimal-digit

hexadecimal-prefix: one of
  0x 0X

digit: one of
  0 1 2 3 4 5 6 7 8 9

nonzero-digit: one of
  1 2 3 4 5 6 7 8 9

octal-digit: one of
  0 1 2 3 4 5 6 7

hexadecimal-digit: one of
  0 1 2 3 4 5 6 7 8 9
  a b c d e f
  A B C D E F

integer-suffix:
  unsigned-suffix
  unsigned-suffix long-suffix
  long-suffix
  long-suffix unsigned-suffix

unsigned-suffix: one of
  u U

long-suffix: one of
  l L

floating-constant:
  fractional-constant
  fractional-constant exponent-part
  fractional-constant floating-suffix
  fractional-constant exponent-part floating-suffix
  digit-sequence exponent-part
  digit-sequence exponent-part floating-suffix

fractional-constant:
  . digit-sequence
  digit-sequence .
  digit-sequence . digit-sequence

digit-sequence:
  digit
  digit-sequence digit

exponent-part:
  e sign digit-sequence
  E sign digit-sequence

sign: one of
  + -

floating-suffix: one of
  f l F L
```

### 分析字符常量/字符串文字量

- 判断`ch`是否为 `'` 或 `"`
  - 进入**字符常量/字符串文字量**的自动机，**循环扫描`ch`**
    - 扫描到 `'` 或 `"` 时，*退出*
      - 退出前，判断*输出串*是不是 `''`
        - 如果是，则*报错*（无效字符常量）
    - 扫描到**换行符**时，*报错*并*退出*（不闭合）
    - 扫描到 `\` 时，**超前扫描`nch`**
      - 如果`nch`是**换行符**，则跳过 `\` 和`nch`（语言特性）
      - 如果`nch`是**转义字符**，则读入`nch`
      - 否则，*报错*并*继续*（非法转义字符）
- 识别**文法**

```
constant:
  integer-constant
  floating-constant
  character-constant

character-constant:
  ' c-char-sequence '

string:
  " "
  " s-char-sequence "

c-char-sequence:
  c-char
  c-char-sequence c-char

s-char-sequence:
  s-char
  s-char-sequence s-char

c-char:
  any member of the source character set except
    the single-quote ', backslash \, or new-line character
  escape-sequence

s-char:
  any member of the source character set except
    the double-quote ", backslash \, or new-line character
  escape-sequence

escape-sequence: one of
  \' \" \? \\
  \a \b \f \n \r \t \v
```

### 分析块注释

- 判断`ch`是否为 `/` 且 `nch`是否为 `*`
  - 进入**块注释**的自动机
  - 扫描到`ch`为 `*` 且 `nch`为 `/` 时，读入`nch`，并*退出*

### 分析行注释

- 判断`ch`是否为 `/` 且 `nch`是否为 `/`
  - 进入**行注释**的自动机
  - 扫描到`ch`为**换行符**时，*退出*

### 分析预编译命令

- 判断`ch`是否为 `#`
  - 进入**预编译命令**的自动机
  - 扫描到`ch`为**换行符**时，*退出*

### 分析其他字符 (包括标点)

- 判断`ch`是否为**标点**
- **循环扫描`nch`**，判断 `当前串 + nch` 是否为**标点**
  （基于长为 *n* 的标点一定存在长为 *n-1* 的标点前缀，除了...）
  - 如果是，则当前串加入`nch`
  - 如果不是，则*退出*
- `当前串`不能构成**标点**，则*报错*
- 识别**文法**

```
punctuator: one of
  [ ] ( ) { } . ->
  ++ -- & * + - ~ !
  / % << >> < > <= >= == != ^ | && ||
  ? : , ; ...
  = *= /= %= += -= <<= >>= &= ^= |=
```

## 测试样例

### Windows MSVC 2015 (Visual Studio 2015)

运行 `Lexer.vcxproj`, 并使用参数 `LexTest.c`

### Unix/Unix-like

``` bash
g++ Lexer.cpp -std=c++11 -o Lexer
./Lexer LexTest.c
```

### Input : LexTest.c

``` C
#include <stdio.h>
#define LEXTEST

int main (int argc, char *argv[])
{
	/*
		Valid Input
	*/
	int a = 10, 0x90, 0070, 0, 00, 0x0;
	double b = 0.5, 1.5, .0, .1e3, 0., 1.e2, 56e-3, 0.7e+8;
	float c = a +b = a/**//b;;;
	long d = dd0.xxx, dd.xx, d1.;
	auto sf = 1lu, 1ul, 02u, 02lu, 0x1l, 0x2u, .3f, 4.l;
	char ch = '@this is a \'Char\'';
	const char *str = "$this is a \"String\"",
		"String\n\r\v\t\
		2";
	printf ("LexTest\n\r\v\t");
	[ ] ( ) { } . ->
	++ -- & * + - ~ !
	/ % << >> < > <= >= == != ^ | && ||
	? : , ; ...
	= *= /= %= += -= <<= >>= &= ^= |=

	// Invalid Input
	int a = 0x;
	double b = 5.0e, 65e+, 72e-;
	auto sf = 1uu, 1ull, 1llu, 1lul, 2lu2, 2ul2, .3fl, .4lf, .5l3l.6l;
	char ch = '', 'hahaha;
	const char *str = "haha;
	scanf ("Lex\eT\*e\|s\ht");
		@ $ ` ..
	return 0;
}

```

### Output : LexTest.c.output.txt

```
Total Lines: 35
Total Chars: 788

Total Words:
[   keyword]:	18
[identifier]:	26
[  constant]:	25
[    string]:	4

Symbols:
"$this is a \"String\""
"LexTest"
"LexTest\n\r\v\t"
"String\n\r\v\t		2"
'@this is a \'Char\''
.0
.1e3
.3f
.6
0
0.
0.5
0.7e+8
00
0070
02lu
02u
0x0
0x1l
0x2u
0x90
1.5
1.e2
10
1lu
1ul
4.l
56e-3
a
argc
argv
b
c
ch
d
d1
dd
dd0
main
printf
scanf
sf
str
xx
xxx

Tokens:
<   keyword>	int
<identifier>	main
<punctuator>	(
<   keyword>	int
<identifier>	argc
<punctuator>	,
<   keyword>	char
<punctuator>	*
<identifier>	argv
<punctuator>	[
<punctuator>	]
<punctuator>	)
<punctuator>	{
<   keyword>	int
<identifier>	a
<punctuator>	=
<  constant>	10
<punctuator>	,
<  constant>	0x90
<punctuator>	,
<  constant>	0070
<punctuator>	,
<  constant>	0
<punctuator>	,
<  constant>	00
<punctuator>	,
<  constant>	0x0
<punctuator>	;
<   keyword>	double
<identifier>	b
<punctuator>	=
<  constant>	0.5
<punctuator>	,
<  constant>	1.5
<punctuator>	,
<  constant>	.0
<punctuator>	,
<  constant>	.1e3
<punctuator>	,
<  constant>	0.
<punctuator>	,
<  constant>	1.e2
<punctuator>	,
<  constant>	56e-3
<punctuator>	,
<  constant>	0.7e+8
<punctuator>	;
<   keyword>	float
<identifier>	c
<punctuator>	=
<identifier>	a
<punctuator>	+
<identifier>	b
<punctuator>	=
<identifier>	a
<punctuator>	/
<identifier>	b
<punctuator>	;
<punctuator>	;
<punctuator>	;
<   keyword>	long
<identifier>	d
<punctuator>	=
<identifier>	dd0
<punctuator>	.
<identifier>	xxx
<punctuator>	,
<identifier>	dd
<punctuator>	.
<identifier>	xx
<punctuator>	,
<identifier>	d1
<punctuator>	.
<punctuator>	;
<   keyword>	auto
<identifier>	sf
<punctuator>	=
<  constant>	1lu
<punctuator>	,
<  constant>	1ul
<punctuator>	,
<  constant>	02u
<punctuator>	,
<  constant>	02lu
<punctuator>	,
<  constant>	0x1l
<punctuator>	,
<  constant>	0x2u
<punctuator>	,
<  constant>	.3f
<punctuator>	,
<  constant>	4.l
<punctuator>	;
<   keyword>	char
<identifier>	ch
<punctuator>	=
<  constant>	'@this is a \'Char\''
<punctuator>	;
<   keyword>	const
<   keyword>	char
<punctuator>	*
<identifier>	str
<punctuator>	=
<    string>	"$this is a \"String\""
<punctuator>	,
<    string>	"String\n\r\v\t		2"
<punctuator>	;
<identifier>	printf
<punctuator>	(
<    string>	"LexTest\n\r\v\t"
<punctuator>	)
<punctuator>	;
<punctuator>	[
<punctuator>	]
<punctuator>	(
<punctuator>	)
<punctuator>	{
<punctuator>	}
<punctuator>	.
<punctuator>	->
<punctuator>	++
<punctuator>	--
<punctuator>	&
<punctuator>	*
<punctuator>	+
<punctuator>	-
<punctuator>	~
<punctuator>	!
<punctuator>	/
<punctuator>	%
<punctuator>	<<
<punctuator>	>>
<punctuator>	<
<punctuator>	>
<punctuator>	<=
<punctuator>	>=
<punctuator>	==
<punctuator>	!=
<punctuator>	^
<punctuator>	|
<punctuator>	&&
<punctuator>	||
<punctuator>	?
<punctuator>	:
<punctuator>	,
<punctuator>	;
<punctuator>	...
<punctuator>	=
<punctuator>	*=
<punctuator>	/=
<punctuator>	%=
<punctuator>	+=
<punctuator>	-=
<punctuator>	<<=
<punctuator>	>>=
<punctuator>	&=
<punctuator>	^=
<punctuator>	|=
<   keyword>	int
<identifier>	a
<punctuator>	=
<punctuator>	;
<   keyword>	double
<identifier>	b
<punctuator>	=
<punctuator>	,
<punctuator>	,
<punctuator>	;
<   keyword>	auto
<identifier>	sf
<punctuator>	=
<punctuator>	,
<punctuator>	,
<punctuator>	,
<punctuator>	,
<punctuator>	,
<punctuator>	,
<punctuator>	,
<punctuator>	,
<  constant>	.6l
<punctuator>	;
<   keyword>	char
<identifier>	ch
<punctuator>	=
<punctuator>	,
<   keyword>	const
<   keyword>	char
<punctuator>	*
<identifier>	str
<punctuator>	=
<identifier>	scanf
<punctuator>	(
<    string>	"LexTest"
<punctuator>	)
<punctuator>	;
<   keyword>	return
<  constant>	0
<punctuator>	;
<punctuator>	}

Errors:
[26]	Invalid 0x
[27]	Bad Exponential: 5.0e
[27]	Bad Exponential: 65e+
[27]	Bad Exponential: 72e-
[28]	Bad Suffix: 1uu
[28]	Bad Suffix: 1ull
[28]	Bad Suffix: 1llu
[28]	Bad Suffix: 1lul
[28]	Bad Suffix: 2lu2
[28]	Bad Suffix: 2ul2
[28]	Bad Suffix: .3fl
[28]	Bad Suffix: .4lf
[28]	Bad Suffix: .5l3l
[29]	Too few chars inside Pair ''
[30]	No End Bracket: 'hahaha;
[31]	No End Bracket: "haha;
[31]	Invalid Escape Char: \e
[31]	Invalid Escape Char: \*
[31]	Invalid Escape Char: \|
[31]	Invalid Escape Char: \h
[32]	Invalid Punctuator: @
[32]	Invalid Punctuator: $
[32]	Invalid Punctuator: `
[32]	Invalid Punctuator: ..

```