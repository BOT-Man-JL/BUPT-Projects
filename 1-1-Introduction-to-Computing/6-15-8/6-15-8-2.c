#include <stdio.h>

int main ()
{
	int arr[1000] = {0}, p, i, j, count;
	
	for (i = 2; i * i < 1000; i++)
		if (!arr[i])
			for (j = i * 2; j < 1000; j += i)
				arr[j] = 1;
	
	printf ("The prime numbers between 1 and 999 are:\n");
	for (i = 2, count = 0; i < 1000; i++)
		if (!arr[i])
		{
			printf ("%d", i);
			count++;
			if (count % 5) printf ("\t");
			else printf ("\n");
		}
	
	return 0;
}

