#include <stdio.h>

int main()
{
	int t, n, odd=0, even=0;
	printf ("Please input an integer:");
	scanf ("%d", &n);
	do
	{
		if (n%2) odd=1;
		else even=1;
	}
	while (n/=10);
	if (odd && even) printf("both!");
	else if (odd) printf("odd number only!");
	else if (even) printf("even number only!");
	else ;

	return 0;
}
