#include <stdio.h>
#include <malloc.h>

int **Create2 (int n, int m)
{
	int i, **p;
	p = (int **) malloc (sizeof (int *) * n);
	
	for (i = 0; i < n; i++)
		p[i] = (int *) malloc (sizeof (int) * m);
	
	return p;
}

int main ()
{
	int **p, m, n, i, j;
	
	printf ("Please input n and m:");
	scanf ("%d%d", &n, &m);
	p = Create2 (n, m);
	
	printf ("Please input %d integers:\n", n * m);
	for (i = 0; i < n; i++)
		for (j = 0; j < m; j++)
			scanf ("%d", &p[i][j]);
	
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < m; j++)
			printf ("%d ", p[i][j]);
		printf ("\n");
		free (p[i]);
	}
	free (p);
	
	return 0;
}
