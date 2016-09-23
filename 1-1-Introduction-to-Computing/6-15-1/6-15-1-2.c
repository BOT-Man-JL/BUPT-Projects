#include <stdio.h>

int main()
{
	char v1; short v2; int v3; long v4; long long v5; float v6; double v7; int i=0;
	int size[]={sizeof(char), sizeof(short), sizeof(int), sizeof(long), sizeof(long long), sizeof(float), sizeof(double)};
	char* type[]={"char", "short", "int", "long", "long long", "float", "double"};
	printf("Please input seven parts:\n");
	scanf("%c %hd %d %ld %lld %f %lf", &v1, &v2, &v3, &v4, &v5, &v6, &v7);
	printf("The result is:\n");
	printf("The \'%s\' variable is %c, it takes %d byte.\n", type[i], v1, size[i]); i++;
	printf("The \'%s\' variable is %hd, it takes %d bytes.\n", type[i], v2, size[i]); i++;
	printf("The \'%s\' variable is %d, it takes %d bytes.\n", type[i], v3, size[i]); i++;
	printf("The \'%s\' variable is %ld, it takes %d bytes.\n", type[i], v4, size[i]); i++;
	printf("The \'%s\' variable is %lld, it takes %d bytes.\n", type[i], v5, size[i]); i++;
	printf("The \'%s\' variable is %f, it takes %d bytes.\n", type[i], v6, size[i]); i++;
	printf("The \'%s\' variable is %lf, it takes %d bytes.\n", type[i], v7, size[i]); return 0;
}
