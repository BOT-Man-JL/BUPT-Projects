#include <stdio.h>

int main ()
{
	int arr[200], n, i, j, t, k;
	
	printf ("Please input the number of integers:");
	scanf ("%d", &n);
	printf ("Please input the integers:");
	for (i = 0; i < n; i++)
		scanf ("%d", &arr[i]);
	
	for (i = 0; i < n; i++)
		if (!(arr[i] % 2))
		{
			t = arr[i];
			for (j = 0; j < i; j++)
				if (arr[j] % 2)
				{
					k = arr[j];
					arr[j] = t;
					t = k;
				}
			arr[i] = t;
		}
	
	printf ("The elements of the new array are:");
	for (i = 0; i < n; i++)
		printf ("%d ", arr[i]);
		
	return 0;
}

