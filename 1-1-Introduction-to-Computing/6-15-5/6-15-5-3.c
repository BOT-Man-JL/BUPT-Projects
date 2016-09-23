#include <stdio.h>
#include <math.h>

int pr[100000], acc = 0 ;

int isPrime (int n)
{
	int i ;
	if (n == 2 || n == 1) return 1 ;
	if (n % 2 == 0) return 0 ;
	for (i = 3; i <= sqrt (n); i += 2)
		if (n % i == 0) return 0 ;
	return 1 ;
}

void printFactor (int n)
{
	int i, once = 0 ;
	printf ("%d = ", n) ;
	if (n == 1)
	{
		printf ("1\n") ;
		return ;
	}
	for (i = 0; n != 1;)
		if (n % pr[i] == 0)
		{
			if (once) printf (" * ") ;
			printf ("%d", pr[i]) ;
			n /= pr[i] ;
			once = 1 ;
		}
		else i++ ;	
	printf ("\n") ;
}
	
int main ()
{
	int a, b, i ;
	
	printf ("Please input two integers:") ;
	scanf ("%d%d", &a, &b) ;
	for (i = 2; i <= b; i++)
		if (isPrime (i))
			pr[acc++] = i ;
	printf ("The result is:\n") ;
	for (i = a; i <= b; i++)
		printFactor (i) ;
	
	return 0 ;
}
