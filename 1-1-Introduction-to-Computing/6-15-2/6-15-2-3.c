#include <stdio.h>

int main()
{
	int i, n, m, t, sum, count=0;
	printf ("Please input two integers:");
	scanf ("%d %d", &n, &m);
	printf ("The result is:\n");
	for (i=1; i<=n; i++)
	{
		sum=0; t=i;	sum=(t%10)*(t%10);
		while (t/=10) sum+=(t%10)*(t%10);
		if (i/m==sum)
		{
			count++;
			if (count%5) printf("%d\t", i);
			else printf("%d\n", i);
		}
	}
	return 0;
}
