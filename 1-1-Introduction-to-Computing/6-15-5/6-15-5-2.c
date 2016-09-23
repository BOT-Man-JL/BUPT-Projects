#include <stdio.h>

int mypow (int x, int n)
{
	int i, mypi = 1 ;
	for (i = 0; i < n; i++)
		mypi *= x ;
	return mypi ;
}

int main ()
{
	int n, m ;
	
	printf ("Please input two integers:") ;
	scanf ("%d%d", &n, &m) ;
	printf ("The result is %d.\n", mypow(n, m)) ;
	
	return 0 ;
}
