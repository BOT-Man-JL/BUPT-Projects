#include <stdio.h>

int main()
{
	int a, b, i;
	printf("Please input two integers:");
	scanf("%d %d", &a, &b);
	printf("number\tsquare\tcube\n");
	for(i=a; i<=b; i++)
		printf("%d\t%d\t%d\n", i, i*i, i*i*i);
	
	return 0;
}
