#include <stdio.h>
#include <math.h>
//#define BACKTRACKING

void display ();
void calc (int);
int vs[4][4], c[4], sum=0;

int main()
{
	vs[0][3]=vs[1][0]=vs[1][2]=vs[3][2]=-1;
	printf("The result of ballot is:\n");
	calc (0);
	printf("There are %d ways to assign the players.", sum);
	return 0;
}

void display ()
{
	int i, j;
	sum++;
	
	printf ("Group %d:\n", sum);
	for (i=0; i<4; i++)
		for (j=0; j<4; j++)
			if (vs[i][j] && vs[i][j]!=-1)
			{
				switch (i)
				{
				case 0:
					printf ("A VS ");
					break;
				case 1:
					printf ("B VS ");
					break;
				case 2:
					printf ("C VS ");
					break;
				case 3:
					printf ("D VS ");
					break;
				}
				switch (j)
				{
				case 0:
					printf ("W");
					break;
				case 1:
					printf ("X");
					break;
				case 2:
					printf ("Y");
					break;
				case 3:
					printf ("Z");
					break;
				}
				printf ("\n");
			}
	printf("\n");
}
#ifdef BACKTRACKING
void calc(int i)
{
	int j;
	
	for (j=0; j<4; j++)
	if (vs[i][j]!=-1)
	{
		vs[i][j]=1;
		if (!c[j])
		{
			if (i==3)
				display();
			else
				c[j]=1;
				calc(i+1);
				c[j]=0;
		}
		vs[i][j]=0;
	}
}
#else
void calc(int i)
{
	int t[4], flag, j, k;
	
	for (t[0]=0; t[0]<4; t[0]++)
	for (t[1]=0; t[1]<4; t[1]++)
	for (t[2]=0; t[2]<4; t[2]++)
	for (t[3]=0; t[3]<4; t[3]++)
	{
		flag=1;
		for (i=0; i<4; i++)
		if (vs[i][t[i]]==-1)
			flag=0;
		for (j=0; j<4; j++)
		for (k=0; k<j; k++)
			if (t[j]==t[k])
				flag=0;
		if (flag)
		{
			sum++;
			printf ("Group %d:\n", sum);
			for (i=0; i<4; i++)
				printf ("%c VS %c\n", i+'A', t[i]+'W');
			printf ("\n");
		}
	}
}
#endif

/*	A	B	C	D
W	0	0	0	-1
X	-1	0	-1	0
Y	0	0	0	0
Z	0	0	-1	0
*/
