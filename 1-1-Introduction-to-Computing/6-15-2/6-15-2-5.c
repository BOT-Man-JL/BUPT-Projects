#include <stdio.h>
#include <math.h>

int main()
{
	float a, b, c, d, x, y;
	printf ("Please input three float numbers:");
	scanf ("%f %f %f", &a, &b, &c);
	if (a==0) printf ("The equation is not quadratic.\n");
	else if (fabs(b*b-4*a*c)<=1e-6) printf ("The equation has two equal roots:%.4f\n", -b/2/a);
	else if (b*b>4*a*c) printf ("The equation has two real distinct roots:%.4f and %.4f\n", (-b+sqrt(b*b-4*a*c))/2/a, (-b-sqrt(b*b-4*a*c))/2/a);
	else if (b*b<4*a*c) printf ("The equation has two complex distinct roots:%.4f+%.4fi and %.4f-%.4fi\n", 
		-b/2/a, sqrt(4*a*c-b*b)/2/a, -b/2/a, sqrt(4*a*c-b*b)/2/a);
	else ;

	return 0;
}
