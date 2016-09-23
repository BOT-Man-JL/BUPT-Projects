#include <stdio.h>
#include <malloc.h>

int *Create1 (int n)
{
	return (int *) malloc (sizeof (int) * n);
}

int main ()
{
	int n, *p, i;
	
	printf ("Please input n:");
	scanf ("%d", &n);
	p = Create1 (n);
	
	printf ("Please input %d integers:\n", n);
	for (i = 0; i < n; i++)
		scanf ("%d", &p[i]);
	
	for (i = 0; i < n; i++)
		printf ("%d ", - p[i] * 10);
	
	free (p);
	return 0;
}
