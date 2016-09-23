#include <stdio.h>

int main ()
{
	char str[10000], *addr[1000], *p, **pp;
	
	printf ("Please input a string:");
	gets (str);
	
	pp = addr;
	for (*pp = p = str; *p; p++)
		if (*p == ' ')
		{
			if (*(p + 1) != ' ')
				*(++pp) = p + 1 ? p + 1 : 0;
			*p = 0;
		}
	*(++pp) = 0;
	
	printf ("The result is:\n");
	for (pp = addr; *pp; pp++)
		printf ("%s\n", *pp);
	
	return 0;
}
