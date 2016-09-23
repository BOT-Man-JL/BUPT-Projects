#include <stdio.h>

int main ()
{
	int n, m, i, j ;
	long long sq = 0, rc = 0 ;
	
	printf ("Please input two integers:") ;
	scanf ("%d%d", &n, &m) ;
	for (i = 1; i <= n; i++)
		for (j = 1; j <= m; j++)
			if (i == j) sq += (n - i + 1) * (m - j +1) ;
			else rc += (n - i + 1) * (m - j +1) ;
	printf ("The number of square is %lld, The number of rectangle is %lld.\n", sq, rc) ;
	return 0 ;
}
