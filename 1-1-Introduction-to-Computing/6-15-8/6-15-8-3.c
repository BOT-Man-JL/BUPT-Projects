#include <stdio.h>

int maxLen (int a[], int n)
{
	int i, max = 0, curmax = 1, cur = a[0];
	
	for (i = 1; i < n; i++)
		if (a[i] == cur) curmax++;
		else
		{
			cur = a[i];
			max = curmax > max ? curmax : max;
			curmax = 1;
		}
	
	return max;
}

int main ()
{
	int n, a[1000], i;
	
	printf ("Please input the number of intergers:");
	scanf ("%d", &n);
	printf ("Please input the integers:");
	for (i = 0; i < n; i++)
		scanf ("%d", &a[i]);
		
	printf ("The longest level is:%d\n", maxLen (a, n));
	
	return 0;
}

