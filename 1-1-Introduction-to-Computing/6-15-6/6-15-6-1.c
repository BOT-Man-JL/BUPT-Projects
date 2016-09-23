#include <stdio.h>

int countBinary (int n)
{
	char t [32];
	int i, count;
	printf ("Its binary form of number is:");
	t[0] = n & 1;
	for (count = 1; n >>= 1; count++)
		t[count] = n & 1;
	for (i = count - 1; i >= 0; i--)
		printf ("%d", t[i]);
	return count;
}

int main ()
{
	int n;
	printf ("Please input an integer:");
	scanf ("%d", &n);
	printf ("\nIt has %d binary digits.\n", countBinary (n));
	return 0;
}
