#include <stdio.h>

void delcharfun (char str[], char ch)
{
	char *p, *t;
	
	for (p = str; *p;)
		if (*p == ch)
			for (t = p; *t; t++)
				*t = *(t + 1);
		else p++;
}
int main ()
{
	char str[100], ch;
	
	printf ("Please input the string:");
	scanf ("%s", str);
	fflush (stdin);
	printf ("Please input the letter you want to delete:");
	scanf ("%c", &ch);
	
	delcharfun (str, ch);
	printf ("The result is:%s\n", str);
	
	return 0;
}
