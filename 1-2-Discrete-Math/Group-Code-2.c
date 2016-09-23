//
//2：针对 (8, 12)编码e，找出最小距离最大的群码编码函数，输出H及最小距离。
//无输入文件
//输出文件：out.txt，示例，矩阵按行输出
//1 1 0 0
//0 0 1 0
//....
//0 1 0 1
//H的最小距离是：3
//

/*
Sollution:
	The maximum minimum distance of e is 3.
	The following program will be running in a low pace, so that we would rather check it by proof.
	(1)	Denote the word with weight = i as xi.
		We can sort out the words with weight = 1, i.e. 00000001, 00000010, ...10000000 (.
	(2)	First suppose the minimun distance is 5, then the rest four bits of e(b), b \in { x1 } will be 1111;
		Now do XOR on two of e(b)'s, the result will be like x2-0000, as 1111 XOR 1111 = 0000;
		according to this, the minimum distance should be no more than 2, which goes against to the premise.
	(3)	Second suppose the minimum distance is 4, then the rest bits of e(b), b \in { x1 } will be one of
		0111, 1011, 1101 or 1110 (four kinds) or 1111 (eight kinds totally). According to the Pigeonhole
		Principle, there should be no less than 2 x1's share the same in such four kinds. The XOR on them
		will be like x2-0000 with weight = 2. Wrong again.
	(4)	Finally suppose the minimum distance is 3, then the rest bits of e(b), b \in { x1 } will be one of
		1100, 1010, 1001, 0110, 0101 or 0011 (six kinds). Now we additional can provide 0111, 1011, 1101, 1110
		and 1111 to fill the rest four bits (eleven kinds total). We can choose 8 of them the fill the rest
		four bits and then the XORs will still weigh no less than 3. For x2's, x3's ..., it can be proved too.
	(5)	Therefore, the maximum minimum distance of e is 3
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <assert.h>

#define B(x,y) (_arrBm[m * x + y])
#define IH(x,y) (_arrIH[n * x + y])
#define eH(x,y) (_arreH[n * x + y])

const int m = 8, n = 12, r = 4;
int tick;
int *_arrIH, *_arrBm, *_arreH;
int InitH (int row, int col);

int Init ()
{
	_arrBm = (int *) malloc ((int) pow (2, m) * m * sizeof (int));
	_arrIH = (int *) malloc (m * n * sizeof (int));
	_arreH = (int *) malloc ((int) pow (2, m) * n * sizeof (int));
	if (!_arrIH || !_arrBm || !_arreH)
	{
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

	// Initialize Im
	row = m; col = m;
	for (int i = 0; i < row; ++i)
		for (int j = 0; j < col; ++j)
		{
			if (i == j) IH (i, j) = 1;
			else IH (i, j) = 0;
		}

	// Recursive calc
	return InitH (0, m);
}

int Calc ()
{
	int row = (int) pow (2, m), col = n, tMin, min = n, tmp;
	for (int i = 0; i < row; ++i)
	{
		tMin = 0;
		for (int j = 0; j < col; ++j)
		{
			eH (i, j) = 0;
			for (int k = 0; k < m; ++k)
				tmp = eH (i, j) += (B (i, k) * IH (k, j));
			eH (i, j) %= 2;
			if (eH (i, j)) ++tMin;
		}
		if (tMin < min && tMin) min = tMin;
	}

	return min;
}

int InitH (int row, int col)
{
	int min = 0, tmp;

	if (row == m - 1 && col == n - 1)
	{
		IH (row, col) = 0;
		tmp = Calc ();
		min = tmp < min ? min : tmp;

		IH (row, col) = 1;
		tmp = Calc ();
		min = tmp < min ? min : tmp;

		return min;
	}

	if (col == n - 1)
	{
		IH (row, col) = 0;
		tmp = InitH (row + 1, m);
		min = tmp < min ? min : tmp;

		IH (row, col) = 1;
		tmp = InitH (row + 1, m);
		min = tmp < min ? min : tmp;

		//if (row < 3) printf ("%d/%d\n", ++tick, 4096);
		return min;
	}

	IH (row, col) = 0;
	tmp = InitH (row, col + 1);
	min = tmp < min ? min : tmp;

	IH (row, col) = 1;
	tmp = InitH (row, col + 1);
	min = tmp < min ? min : tmp;

	return min;
}

int main ()
{
	FILE *pf;

	pf = fopen ("out.txt", "w");
	fprintf (pf, "The max min distance is: %d\n", Init ());

	free (_arrBm);
	free (_arrIH);
	free (_arreH);

	fclose (pf);
	return 0;
}
