#include <stdio.h>
#define LEXTEST

int main (int argc, char *argv[])
{
	/*
		Valid Input
	*/
	int a = 10, 0x90, 0070, 0, 00, 0x0;
	double b = 0.5, 1.5, .0, .1, 0., 1., 56e-3, 0.7e+8;
	auto c = a +b = a/**//b;;;
	auto d = dd0.xxx, dd.xx, d1.;
	char ch = '@this is a \'Char\'';
	const char *str = "$this is a \"String\"",
		"String\n\r\v\t\
		2";
	printf ("LexTest\n\r\v\t");
	[ ] ( ) { } . ->
	++ -- & * + - ~ !
	/ % << >> < > <= >= == != ^ | && ||
	? : , ;
	= *= /= %= += -= <<= >>= &= ^= |=

	// Invalid Input
	int a = 0x;
	double b = 5.0e, 65e+, 72e-;
	char ch = '', ';
	const char *str = "haha;
	scanf ("Lex\eT\*e\|s\ht");
		@ $ `
	return 0;
}
