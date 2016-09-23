//
//编程作业：
//给定群码 (m, n)编码函数e的H (读取文件, 读取文件方式，第一行两个整数m, n，
//第二行m * (n - m)个0或1，也就是矩阵H的上半部分，下半部单位矩阵自行生成)
//1，计算与e相关的极大似然法能纠错的比特数；2，交互方式给定的码字进行解码

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#define B(x,y) (_arrBm[m * x + y])
#define IH(x,y) (_arrIH[n * x + y])
#define N(x,y) (_arrN[n * x + y])
#define D(x,y) (_arrDT[(int) pow (2, m) * x + y])  // Decode table
#define M(x) (_arrMap[x])  // Map to the col; 0: non-mapped, 1-n: don't forget to add/minus 1
#define W(x) (_arrWgh[x])  // The weight of x; for finding the min weight

int m, n, r, *_arrIH, *_arrBm, *_arrN, *_arrDT, *_arrMap, *_arrWgh;

int Init ()
{
	FILE *pf;

	pf = fopen ("in.txt", "r");
	if (!pf)
	{
		printf ("Can not open the in.txt");
		return 0;
	}

	fscanf (pf, "%d%d", &m, &n);
	r = n - m;

	_arrBm = (int *) malloc ((int) pow (2, m) * m * sizeof (int));
	_arrIH = (int *) malloc (m * n * sizeof (int));
	_arrN = (int *) malloc ((int) pow (2, m) * n * sizeof (int));
	_arrDT = (int *) malloc ((int) pow (2, n) * sizeof (int));
	_arrMap = (int *) calloc ((int) pow (2, n), sizeof (int));
	_arrWgh = (int *) malloc ((int) pow (2, n) * sizeof (int));

	if (!_arrIH || !_arrBm || !_arrN || !_arrDT || !_arrMap)
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

	// Initialize Wgh
	row = pow (2, n);
	for (int i = 0; i < row; ++i)
	{
		int w = 0;
		int num = i;
		for (int j = 0; j < n; ++j)
		{
			w += num % 2;
			num /= 2;
		}
		W (i) = w;
	}

	fclose (pf);
	return 1;
}

void Calc ()
{
	// Calculate N
	int row = (int) pow (2, m), col = n, tMin, min = n, num;
	for (int i = 0; i < row; ++i)
	{
		tMin = 0;
		num = 0;
		for (int j = 0; j < col; ++j)
		{
			N (i, j) = 0;
			for (int k = 0; k < m; ++k)
				N (i, j) += (B (i, k) * IH (k, j));
			N (i, j) %= 2;
			if (N (i, j))
			{
				++tMin;
				num = num * 2 + 1;
			}
			else
				num = num * 2;
		}
		D (0, i) = num;
		M (num) = i + 1;
		if (tMin < min && tMin) min = tMin;
	}

	//printf ("N = \n");
	//for (int i = 0; i < row; ++i)
	//{
	//	for (int j = 0; j < col; ++j)
	//		printf ("%d ", N (i, j));
	//	printf ("\n");
	//}

	printf ("(e, d) can correct %d or fewer errors\n", (min - 1) / 2);

	// Calculate Decode Table
	row = (int) pow (2, r);
	col = (int) pow (2, m);
	int _pow = pow (2, n);  // the number of the table elems
	min = 1;  // current minimum weight
	num = 0;  // current element
	for (int i = 1; i < row; ++i)
	{
	Lab_again:
		for (; num < _pow; ++num)
			if (W (num) <= min && !M (num)) break;
		if (num == _pow)
		{
			num = 0;
			++min;
			goto Lab_again;
		}

		M (num) = 1;
		D (i, 0) = num;
		for (int j = 1; j < col; ++j)
		{
			D (i, j) = D (i, 0) ^ D (0, j);
			M (D (i, j)) = j + 1;
		}
	}

	//printf ("\n");
	//for (int i = 0; i < row; ++i)
	//{
	//	for (int j = 0; j < col; ++j)
	//	{
	//		num = D (i, j);
	//		char *tmp = (char *) malloc (n * sizeof (char));
	//		for (int i = 0; i < n; ++i)
	//		{
	//			tmp[n - i - 1] = num % 2;
	//			num /= 2;
	//		}
	//		for (int i = 0; i < n; ++i)
	//			printf ("%d", tmp[i]);
	//		//printf ("-%d ", W (D (i, j)));
	//		printf (" ");
	//		free (tmp);
	//	}
	//	printf ("\n");
	//}
}

void Interact ()
{
	printf ("\nInput the code word or E to Escape:\n");
	int num, ch = getchar ();
	while (ch != 'e' && ch != 'E')
	{
		// skip blank chars
		while (isblank (ch))
			ch = getchar ();

		num = 0;
		for (int i = 0; !isblank (ch) && i < n; ++i, ch = getchar ())
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

		printf ("The word is:\n");
		num = M (num) - 1;
		char *tmp = (char *) malloc (m * sizeof (char));
		for (int i = 0; i < m; ++i)
		{
			tmp[m - i - 1] = num % 2;
			num /= 2;
		}
		for (int i = 0; i < m; ++i)
			printf ("%d", tmp[i]);
		printf ("\n");

		free (tmp);
		printf ("\nInput the code word or E to Escape:\n");
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
	free (_arrN);
	free (_arrDT);
	free (_arrMap);

	return 0;
}
