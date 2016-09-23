#include <stdio.h>

void InsertSort (int a[], int n, int size)
{
	int i, t;
	
	t = a[n];
	for (i = n - 1; i >= 0; i--)
		if (a[i] <= t) break;
		else a[i + 1] = a[i];
	a[i + 1] = t;
	
	if (n == size - 1) return ;
	else return InsertSort (a, n + 1, size);
}

int main ()
{
	int a[1000], i, n;
	
	printf ("Please input the number of integers:");
	scanf ("%d", &n);
	printf ("Please input the integers:");
	for (i = 0; i < n; i++)
		scanf ("%d", &a[i]);
	
	InsertSort (a, 1, n);
	printf ("The result is:");
	for (i = 0; i < n; i++)
		printf ("%d ", a[i]);
	
	return 0;
}
