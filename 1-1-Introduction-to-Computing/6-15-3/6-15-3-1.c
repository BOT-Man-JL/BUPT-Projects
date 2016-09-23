#include <stdio.h>

int main()
{
	int n, i, j;
	printf ("Please input an integer:");
	scanf ("%d", &n);
	printf ("The result is:\n");
	for (i=1; i<=n; i++)
	{
		for (j=1; j<=2*n-1; j++)
			if (j>n-i && j<n+i)
				printf ("*");
			else printf (" ");
		printf("\n");
	}
	for (i=n+1; i<=2*n-1; i++)
	{
		for (j=1; j<=2*n-1; j++)
			if (j>i-n && j<3*n-i)
				printf ("*");
			else printf (" ");
		printf("\n");
	}
	return 0;
}

