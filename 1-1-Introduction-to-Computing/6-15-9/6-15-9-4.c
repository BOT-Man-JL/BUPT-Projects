#include <stdio.h>

#define N 100

void getMatrix (int a[][N], int n)
{
	int i = 0, x = 0, y = 0;
	
	while (1)
	{
		a[x][y] = ++i;
		if ((x + y) % 2)
			if (y == 0 || a[x + 1][y - 1] == -1)
				if (a[x + 1][y] != -1) x++;
				else if (a[x][y + 1] != -1) y++;
				else break;
			else
				{ x++; y--;}
		else
			if (x == 0 || a[x - 1][y + 1] == -1)
				if (a[x][y + 1] != -1) y++;
				else if (a[x + 1][y] != -1) x++;
				else break;
			else
				{ x--; y++; }
	}
}

int main ()
{
	int n, i, j, a[101][101] = {0};
	
	printf ("Please input order of the snake matrix:");
	scanf ("%d", &n);
	
	for (i = 0; i <= n; i++)
		a[i][n] = a[n][i] = -1;
	getMatrix (a, n);
	
	printf ("The snake matrix is:\n");
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
			printf ("%d ", a[j][i]);
		printf ("\n");
	}
	return 0;
}
