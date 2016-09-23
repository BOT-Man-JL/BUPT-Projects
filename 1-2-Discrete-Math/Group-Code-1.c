//
//1：给定H (读取文件方式，第一行两个整数m, n，第二行 m * (n - m)个0或1，也就是矩阵H的上半部分，下半部单位矩阵自行生成)，计算群码编码函数e_H，计算该编码函数能检测到多少位错误，交互输出字的码字
//输入文件：in.txt，示例：第一行两个整数，第二行累计m * r个整数。所有整数都用一个空格分隔
//3 5
//1 0 1 0 0 1
//无输出文件

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#define B(x,y) (_arrBm[m * x + y])
#define IH(x,y) (_arrIH[n * x + y])
#define eH(x,y) (_arreH[n * x + y])

int m, n, r, *_arrIH, *_arrBm, *_arreH;

int Init ()
{
	FILE *pf;

	pf = fopen ("in.txt", "r");

	fscanf (pf, "%d%d", &m, &n);
	r = n - m;

	_arrBm = (int *) malloc ((int) pow (2, m) * m * sizeof (int));
	_arrIH = (int *) malloc (m * n * sizeof (int));
	_arreH = (int *) malloc ((int) pow (2, m) * n * sizeof (int));
	if (!_arrIH || !_arrBm || !_arreH)
	{
		fclose (pf);
		printf ("Out of memory\n");
		return 0;
	}

	// Initialize Bm
	int row = (int) pow (2, m), col = m, tmp;
	for (int i = 0; i < row; ++i)
	{
		tmp = i;
		for (int j = col - 1; j >= 0; --j, tmp /= 2)
			B (i, j) = tmp % 2;
	}

	//for (int i = 0; i < row; ++i)
	//{
	//	for (int j = 0; j < col; ++j)
	//		printf ("%d ", B (i, j));
	//	printf ("\n");
	//}

	// Initialize Im
	row = m; col = m;
	for (int i = 0; i < row; ++i)
		for (int j = 0; j < col; ++j)
		{
			if (i == j) IH (i, j) = 1;
			else IH (i, j) = 0;
		}

	// Initialize H
	row = m; col = n;
	for (int i = 0; i < row; ++i)
		for (int j = m; j < col; ++j)
			fscanf (pf, "%d", &IH (i, j));

	//for (int i = 0; i < row; ++i)
	//{
	//	for (int j = 0; j < col; ++j)
	//		printf ("%d ", IH (i, j));
	//	printf ("\n");
	//}

	fclose (pf);
	return 1;
}

void Calc ()
{
	int row = (int) pow (2, m), col = n, tMin, min = n;
	for (int i = 0; i < row; ++i)
	{
		tMin = 0;
		for (int j = 0; j < col; ++j)
		{
			eH (i, j) = 0;
			for (int k = 0; k < m; ++k)
				eH (i, j) += (B (i, k) * IH (k, j));
			eH (i, j) %= 2;
			if (eH (i, j)) ++tMin;
		}
		if (tMin < min && tMin) min = tMin;
	}

	printf ("e_H = \n");
	for (int i = 0; i < row; ++i)
	{
		for (int j = 0; j < col; ++j)
			printf ("%d ", eH (i, j));
		printf ("\n");
	}

	printf ("e_H can detect %d or fewer errors\n", min - 1);
}

void Interact ()
{
	printf ("\nInput the word or E to Escape:\n");
	int num, ch = getchar ();
	while (ch != 'e' && ch != 'E')
	{
		// skip blank chars
		while (isblank (ch))
			ch = getchar ();

		num = 0;
		for (int i = 0; !isblank (ch) && i < m; ++i, ch = getchar ())
			switch (ch)
			{
			case '0':
				num = num * 2;
				break;
			case '1':
				num = num * 2 + 1;
				break;
			default:
				printf ("Bad Input\n");
				return;
			}

		printf ("The code word is:\n");
		for (int i = 0; i < n; ++i)
			printf ("%d", eH (num, i));
		printf ("\n");

		printf ("\nInput the word or E to Escape:\n");
		ch = getchar ();
	}
}

int main ()
{
	if (!Init ()) return 1;
	Calc ();
	Interact ();

	free (_arrBm);
	free (_arrIH);
	free (_arreH);

	return 0;
}
