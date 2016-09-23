#include <stdio.h>

int gcd (int a, int b)
{
	if (a == 0)		return b ;
	else if(b == 0)	return a ;
	else			return gcd (b, a % b) ;
}

int lcm (int a, int b)
{
	return a * b / gcd (a, b) ;
}

int main()
{
	int a, b ;
	
	printf ("Please input two integers:") ;
	scanf ("%d%d", &a, &b) ;
	printf ("The greatest common divisor is %d and the lowest common multiple is %d.\n", gcd (a, b), lcm (a, b)) ;
	
	return 0 ;
}
