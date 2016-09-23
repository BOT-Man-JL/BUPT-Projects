#include <stdio.h>
#include <string.h>

int main()
{
	int i, t ;
	char str[1000] ;
	
	printf ("Please input an integer:") ;
	
	scanf ("%s", &str) ;
	printf ("The result is:") ;
	
	for (i = 0, t = strlen (str) - 1; i < strlen (str); i++, t--)
		if (t%3 || i == strlen(str) - 1) printf ("%c", str[i]) ;
		else printf ("%c,", str[i]) ;
	printf ("\n") ;
	
	return 0 ;
}
