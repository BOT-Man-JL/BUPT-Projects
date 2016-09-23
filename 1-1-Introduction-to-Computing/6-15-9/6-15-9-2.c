#include <stdio.h>

int RecurBianrySearch (int a[], int begin, int end, int key)
{
	int i = (begin + end) / 2;
	
	if (begin != end)
		if (a[i] >= key) return RecurBianrySearch (a, begin, i, key);
		else return RecurBianrySearch (a, i + 1, end, key);
	else
		if (a[begin] == key) return begin;
		else return -1;
}

int main ()
{
	int i, n, m, a[100000], b[100000];
	
	printf ("Please input the number of integers:");
	scanf ("%d", &n);
	printf ("Please input the number of you want to search:");
	scanf ("%d", &m);
	
	printf ("Please input the integers:");
	for (i = 0; i < n; i++)
		scanf ("%d", &a[i]);
	printf ("Please input the integers you want to search:");
	for (i = 0; i < m; i++)
		scanf ("%d", &b[i]);
	
	printf ("The elements' subscript in the array are:");
	for (i = 0; i < m; i++)
		printf ("%d ", RecurBianrySearch (a, 0, n - 1, b[i]));
	
	return 0;
}
