#include <stdio.h>
#include <math.h>

int fac(int i)	{	return i>2?i*fac(i-1):i;	}

int main()
{
	double x, y=1, t;
	int i;
	printf ("Please input an float number:");
	scanf ("%lf", &x);
	for (i=1; fabs(pow(x,i)/fac(i))>=1e-8; i++)
		y+=fabs(pow(x,i)/fac(i));
	printf("The result is:%.4f\n", y);

	return 0;
}
