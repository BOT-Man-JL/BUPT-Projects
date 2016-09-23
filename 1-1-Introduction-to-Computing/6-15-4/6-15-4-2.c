#include <stdio.h>

int main()
{
	int i, n, f1 = 1, f2 = 1, ft ;
	double sum = 0 ;
	
	printf ("Please input an integer:") ;
	scanf ("%d", &n) ;
	for (i = 0; i < n; i++)
	{
		ft = f2 ;
		f2 += f1 ;
		f1 = ft ;
		
		sum += (double) f2 / f1 ;
	}
	printf ("The result is:%.2f\n", sum) ;
	return 0 ;
}
	
