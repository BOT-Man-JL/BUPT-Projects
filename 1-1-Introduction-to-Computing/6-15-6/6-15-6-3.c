#include <stdio.h>

int countBinary (int n)
{
	if (n == 0) return n;
	else return countBinary (n>>1) + 1;
}

int main ()
{
	int n;
	printf ("Input an integer:");
	scanf ("%d", &n);
	printf ("The binary form of number %d has %d digits.\n", n, countBinary (n));
	return 0;
}
