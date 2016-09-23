#include <stdio.h>

int main ()
{
	int n, m, num[1000], query[100], i, j;
	printf ("Please input the number of integers:");
	scanf ("%d", &n);
	printf ("Please input the integers:");
	for (i = 0; i < n; i++)
		scanf ("%d", &num[i]);
	printf ("Please input the times of query:");
	scanf ("%d", &m);
	printf ("Please input the integers you want to query:");
	for (i = 0; i < m; i++)
		scanf ("%d", &query[i]);
	printf ("The result is:\n");
	for (i = 0; i < m; i++)
	{
		for (j = 0; j < n; j++)
			if (query[i] == num[j])
			{	if (j == 0) printf ("%d\n", num[1]);
				else if (j == n - 1) printf ("%d\n", num[n - 2]);
				else printf ("%d %d\n", num[j - 1], num[j + 1]);
				break;
			}
		if (j == n) printf ("NULL\n");
	}
	return 0;
}
