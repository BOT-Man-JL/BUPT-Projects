#include <stdio.h>

int convert (int n)
{
	if (n == 0 || n ==1) return n;
	else return n % 10 + convert (n / 10) * 2;
}

int main ()
{
	int n;
	printf ("Please input an integer:");
	scanf ("%d", &n);
	printf ("The result is %d\n", convert (n));
	return 0;
}
