#include <stdio.h>

char *locatesubstr (char *str1, char *str2)
{
	char *p1, *p2;
	
	for (; *str1; str1++)
		if (*str1 == *str2)
		{
			for (p1 = str1, p2 = str2; *p2 && *p1; p1++, p2++)
				if (*p1 != *p2) break;
			if (*p2 == 0) return str1;
		}
	
	return 0;
}

int main ()
{
	char str1[500], str2[500], *p;
	
	printf ("Please input one string:");
	gets (str1);
	printf ("Please input another string:");
	gets (str2);
	
	if (p = locatesubstr(str1, str2))
		printf ("The result is:%s\n", p);
	else printf ("NULL!");
	
	return 0;
}
