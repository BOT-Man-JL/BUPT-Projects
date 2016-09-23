#include <stdio.h>
#include <memory.h>

#define N 100

struct S
{
	int num[N];
	int sum;
};

/*
void QuickSort (struct S arr[], int L, int R)
{
	int l = L, r = R;
	struct S tmp;
	
	if (l < r)
	{
		memcpy (&tmp, &arr[l], sizeof tmp);
		while (l < r)
		{
			while (l < r && arr[r].sum >= tmp.sum)	r--;
			memcpy (&arr[l], &arr[r], sizeof tmp);
			while (l < r && arr[l].sum <= tmp.sum)	l++;
			memcpy (&arr[r], &arr[l], sizeof tmp);
		}
		memcpy (&arr[l], &tmp, sizeof tmp);
		
		QuickSort (arr, L, l - 1);
		QuickSort (arr, l + 1, R);	
	}
}
*/

void InsertSort (struct S arr[], int n, int size)
{
	int i;
	struct S tmp;
	
	memcpy (&tmp, &arr[n], sizeof tmp);
	for (i = n - 1; i >= 0; i--)
		if (arr[i].sum <= tmp.sum) break;
		else memcpy (&arr[i + 1], &arr[i], sizeof tmp);
	memcpy (&arr[i + 1], &tmp, sizeof tmp);
	
	if (n < size - 1) return InsertSort (arr, n + 1, size);
}

void sort (int a[][N], int n)
{
	int i, j;
	struct S arr[N];
	
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
		{
			arr[i].sum += a[i][j];
			arr[i].num[j] = a[i][j];
		}
	
	//QuickSort (arr, 0, n - 1, n);
	InsertSort (arr, 1, n);
	
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			a[i][j] = arr[i].num[j];
}

int main ()
{
	int i, j, a[N][N], size;
	
	printf ("Please input row of the matrix:");
	scanf ("%d", &size);
	
	printf ("Please input elements of the matrix:\n");
	for (i = 0; i < size; i++)
		for (j = 0; j < size; j++)
			scanf ("%d", &a[i][j]);

	sort (a, size);
	
	printf ("The result is:\n");
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
			printf ("%d ", a[i][j]);
		printf ("\n");
	}
	
	return 0;
}
