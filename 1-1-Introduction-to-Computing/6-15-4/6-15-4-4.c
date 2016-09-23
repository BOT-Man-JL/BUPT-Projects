#include <stdio.h>
#include <math.h>

int main()
{
	int i, t = 0 ;
	double sum = 0 ;
	char ch, s[1000] ;
	
	printf ("Please input the binary string:") ;
	scanf ("%s", s) ;
	
	for (i = 0; s[i] != '.' && s[i] != '#'; i++) ;
		t = i ;
	for (i = 0; i < t; i++)
		sum += (s[i] - '0') * pow (2, t - i - 1) ;
	
	if (s[i] == '.')
	{	for (i = t + 1; s[i] != '#'; i++)
			sum += (s[i] - '0') * pow (.5, i - t) ;
		printf ("%.6f\n", sum) ;
	}
	else printf ("%d\n", (int) sum) ;
	
	return 0 ;
}
