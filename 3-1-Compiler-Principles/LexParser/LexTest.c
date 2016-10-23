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
