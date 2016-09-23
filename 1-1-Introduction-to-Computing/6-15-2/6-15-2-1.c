#include <stdio.h>

int main()
{
	char t;
	int i, ch=0, space=0, num=0, others=0;
	printf ("Please input a string:");
	while ((t=getchar())!='\n')
		if ((t>='a' && t<='z') || (t>='A' && t<='Z')) ch++;
		else if (t==' ') space++;
		else if (t>='0' && t<='9') num++;
		else others++;
	printf ("The number of English characters,spaces,numbers,other characters are:%d %d %d %d\n", 
		ch, space, num, others);

	return 0;
}
