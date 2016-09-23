#include <stdio.h>

int main()
{
	int a, b, n, i, i2, j, k, t[4], flag, count=0;
	printf ("Please input three integers:");
	scanf ("%d%d%d", &a, &b, &n);
	printf ("The result is:\n");
	
	//a=213; b=214; n=4;
	for (i=a; i<=b; i++)
	{
		i2=i;
		flag=1;
		for (j=0; i2; j++)
		{
			t[j]=i2%10;
			if (t[j]<=0 || t[j]>n)
				i2=flag=0;
			for (k=0; k<j; k++)
				if (t[k]==t[j])
					i2=flag=0;
			i2/=10;
		}
		if (flag)
		{
			count++;
			if (count%5) printf ("%d\t", i);
			else printf ("%d\n", i);
		}
	}
	if (count==0) printf("This is no proper number in the interval.\n");
	
	return 0;
}

