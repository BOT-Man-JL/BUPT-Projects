#include <stdio.h>
#include <stdlib.h>

int cmp (const void *a, const void *b) { return *(int*) b - *(int*) a; }

int main ()
{
	int i, n, m, num[100], t[100];
	printf ("Please input the number of intrgers:");
	scanf ("%d", &n);
	printf ("Please input the intrgers:");
	for (i = 0; i < n; i++)
	{
		scanf ("%d", &num[i]);
		t[i] = num[i];
	}
	qsort (num, n, sizeof (int), cmp);
	printf ("Please input which large intrger you want to search:");
	scanf ("%d", &m);
	for (i = 0; i < n; i++)
		if (num[m - 1] == t[i]) printf ("The result is:A[%d]=%d\n", i, num[m - 1]);
	return 0;
}
