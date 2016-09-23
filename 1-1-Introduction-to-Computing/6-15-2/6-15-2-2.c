#include <stdio.h>

int main()
{
	int i, n, s=0;
	printf ("Please input an integer:");
	scanf ("%d", &n);
	for (i=21; i<n; i+=21)
		s+=i;
	printf ("The result is:%d\n", s*s);

	return 0;
}
