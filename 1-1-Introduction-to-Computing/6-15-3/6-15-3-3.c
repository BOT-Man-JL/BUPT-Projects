#include <stdio.h>
#include <math.h>

int main()
{
	int a, b, c, i, j, sum, count=0;
	printf ("Input three integers:");
	scanf ("%d%d%d", &a, &b, &c);
	printf ("The result is:\n");
	
	for (i=a; i<=b; i++)
	{
		if (i==1) continue;
		sum=1;
		for (j=2; j<i; j++)
			if (!(i%j)) sum+=j;
		if (abs(i-sum)<=c)
		{
			count++;
			if (count%5) printf ("%d\t", i);
			else printf ("%d\n", i);
		}
	}
	if (count==0) printf("This is no proper number in the interval.\n");
	
	return 0;
}

