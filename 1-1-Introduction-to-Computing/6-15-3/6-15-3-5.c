#include <stdio.h>
#include <math.h>

double f(double);
int a, b, c, d;

int main()
{
	printf ("Please input four integers:");
	scanf ("%d%d%d%d", &a, &b, &c, &d);
	printf ("The solution is:%f\n", f(1));
	
	return 0;
}

double f(double x1)
{
	double x2=x1-(a*x1*x1*x1+b*x1*x1+pow (c,x1)+d)/(3*a*x1*x1+2*b*x1+pow (c,x1)*log (c)/log (exp (1)));
	if (fabs (x2-x1)<1e-5) return x2;
	else return f(x2);
}
