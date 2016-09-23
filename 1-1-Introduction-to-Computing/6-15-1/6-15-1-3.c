#include <stdio.h>

int main()
{
	char ch[3], i;
	for(i=0; i<3; i++)
	{
		ch[i]=getchar();
		//scanf("%c",&ch[i]);
		if(!((ch[i]>='a'&&ch[i]<='z')||(ch[i]>='A'&&ch[i]<='Z')))
		{
			printf("The letters are not adjacent with each other.\n");
			return 0;
		}
	}
	if(ch[0]+1==ch[1]&&ch[1]+1==ch[2])
		printf("The letters are adjacent with each other.\n");
	else printf("The letters are not adjacent with each other.\n");
	
	return 0;
}
