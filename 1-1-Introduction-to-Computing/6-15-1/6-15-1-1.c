#include <stdio.h>

int main()
{
	double a, b, c, d;
	printf("Please input four float numbers:");
	scanf("%lf %lf %lf %lf", &a, &b, &c, &d);
	printf("The result is %lf\n", (a+b)*(a-b)+c/d);
	
	return 0;
}
