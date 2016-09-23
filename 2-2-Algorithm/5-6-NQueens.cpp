// N-皇后问题的快速局部搜索算法
// 参考：Rok Sosic and Jun Gu. A Polynomial Time Algorithm for the N-Queens Problem. SIGART Bulletin, 1(3):7-11, 1990.

#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "math.h"

long *pSolution;    // 解 
long N;	            // 皇后个数
int sqrt_N;         // 用于产生范围在0 ~ N-1的随机数
long NumCollisions; // 当前解的冲突总数

// 正对角线, 同一条对角线上 i - pSolution[i] 为常数，下标范围 -(N-1) ~ (N-1),映射到 0 ~ (2N-2) [通过 + (N-1)]
long *pPosDiagonal; 

// 负对角线, 同一条对角线上 i + pSolution[i] 为常数，下标范围 0 ~ (2N-2) 
long *pNegDiagonal; 

/*----------------------------------------------
本函数交换两个单元
 ----------------------------------------------*/
inline void Swap(long &x, long &y)
{
	long temp;

	temp = y;
	y = x;
	x = temp;
}

/*----------------------------------------------
本函数用于生成一个随机排列，作为初始解
 ----------------------------------------------*/
void GeneratePermutation()
{
	for (long i = 0; i < N; i++) pSolution[i] = i;

	int high, low;
	long rand_index;

	for (i = 0; i < N; i++)
	{
		// 考虑到N的数值可能很大，随机数由两段拼接而成
		high = rand() % sqrt_N;
		low = rand()% sqrt_N;		
		rand_index = high * sqrt_N + low;
		
		Swap(pSolution[i],pSolution[rand_index]);
	}
}

/*----------------------------------------------
本函数计算冲突的个数
 ----------------------------------------------*/
long CountCollisions()
{	
	long max = 2 * N - 1;

	NumCollisions = 0;

	for (long i = 0; i < max; i++)
	{
		NumCollisions += pPosDiagonal[i] > 1 ? pPosDiagonal[i] - 1: 0;
		NumCollisions += pNegDiagonal[i] > 1 ? pNegDiagonal[i] - 1: 0;
	}

	return NumCollisions;
}

/*----------------------------------------------
本函数根据解，设置两条对角线对应的皇后数的数组
 ----------------------------------------------*/
void SetDiagonals()
{
	// 首先给两个对角线的数组赋初始值 0
	memset(pPosDiagonal,0,sizeof(long)*(2*N-1));
	memset(pNegDiagonal,0,sizeof(long)*(2*N-1));

	for (long i = 0; i < N; i++)
	{
		pPosDiagonal[i - pSolution[i] + N -1]++;
		pNegDiagonal[i + pSolution[i]]++;
	}
}

/*----------------------------------------------
本函数检查一下：如果交换 pSolution[i], pSolution[j], 冲突是否会减少？
 ----------------------------------------------*/
long SwapEvaluate(long i, long j)
{
	long ret = 0;

	// 两个解在同一个条正对角线上
	if (i - pSolution[i] == j - pSolution[j])
	{
		ret = (pPosDiagonal[i - pSolution[i] + N -1] > 2 ? 2: 1) + (pNegDiagonal[i + pSolution[i]] > 1 ? 1 : 0) +
			   (pNegDiagonal[j + pSolution[j]] > 1 ? 1 : 0) -
			   (pNegDiagonal[i + pSolution[j]] > 0 ? 2 : 1) -
			   (pPosDiagonal[i - pSolution[j] + N -1] > 0 ? 1: 0) - (pPosDiagonal[j - pSolution[i] + N -1] > 0 ? 1: 0);			   

		return ret;
	}

	// 两个解在同一条负对角线上
	if (i + pSolution[i] == j + pSolution[j])
	{
		ret = (pNegDiagonal[i + pSolution[i]] > 2 ? 2: 1) + (pPosDiagonal[i - pSolution[i] + N -1] > 1 ? 1 : 0) +
			  (pPosDiagonal[j - pSolution[j] + N -1] > 1 ? 1 : 0) -
			  (pPosDiagonal[i - pSolution[j] + N -1] > 0 ? 2 : 1) -
			  (pNegDiagonal[i + pSolution[j]] > 0 ? 1: 0) - (pNegDiagonal[j + pSolution[i]] > 0 ? 1: 0);			   

		return ret;
	}


	// 一般情况，即两个解在不同的对角线上
	ret = (pPosDiagonal[i - pSolution[i] + N -1] > 1 ? 1 : 0) + (pNegDiagonal[i + pSolution[i]] > 1 ? 1 : 0) + 
		  (pPosDiagonal[j - pSolution[j] + N -1] > 1 ? 1 : 0) + (pNegDiagonal[j + pSolution[j]] > 1 ? 1 : 0) - 
		  (pPosDiagonal[i - pSolution[j] + N -1] > 0 ? 1 : 0) - (pNegDiagonal[i + pSolution[j]] > 0 ? 1 : 0) - 
		  (pPosDiagonal[j - pSolution[i] + N -1] > 0 ? 1 : 0) - (pNegDiagonal[j + pSolution[i]] > 0 ? 1 : 0);

	return ret; // ret > 0 表明冲突减少，ret < 0 表示冲突增加
}

/*----------------------------------------------
本函数交换 pSolution[i], pSolution[j], 更新对角线数组及冲突总数
 ----------------------------------------------*/
void SwapQueens(long i, long j, long gain)
{
	pPosDiagonal[i - pSolution[i] + N -1] --;
	pNegDiagonal[i + pSolution[i]] --;
	pPosDiagonal[j - pSolution[j] + N -1] --;
	pNegDiagonal[j + pSolution[j]] --;

	Swap(pSolution[i], pSolution[j]);

	pPosDiagonal[i - pSolution[i] + N -1] ++;
	pNegDiagonal[i + pSolution[i]] ++;
	pPosDiagonal[j - pSolution[j] + N -1] ++;
	pNegDiagonal[j + pSolution[j]] ++;

	NumCollisions -= gain;
}

/*----------------------------------------------
主函数
 ----------------------------------------------*/
int main(int argc, char* argv[])
{
	clock_t start, finish;
	start = clock();

	sscanf(argv[1],"%ld",&N);

	sqrt_N = (int) sqrt(N);

	// 分配解的空间
	pSolution = new long[N]; // 使用下标 0 - (N-1)

	// 分配正对角线上的数组空间，并赋初始值为0
	pPosDiagonal = new long[2*N-1];

	// 分配负对角线上的数组空间，并赋初始值为0
	pNegDiagonal = new long[2*N-1];

	do {
		// 产生一个随机解
		GeneratePermutation();

		// 给两个对角线对应的皇后数数组赋值
		SetDiagonals();

		CountCollisions();

		bool flag = true;
		long gain;

		while (flag)
		{
			flag = false;
			for (long i = 0; i < N; i++)
				for (long j = i; j < N; j++)
					// 若pSolution[i] 或 pSolution[j]对应的对角线有冲突
					if (pPosDiagonal[i - pSolution[i] + N -1] > 1 || pNegDiagonal[i + pSolution[i]] > 1 || 
						pPosDiagonal[j - pSolution[j] + N -1] > 1 || pNegDiagonal[j + pSolution[j]] > 1)
					{
						// 判断一下，如果交换pSolution[i]和pSolution[j]，是否可以降低冲突总数
						gain = SwapEvaluate(i, j);

						// 若交换会带来冲突的减少，则值得交换
						if ( gain > 0)
						{
							flag = true; // 标记，证明本轮迭代做了交换

							SwapQueens(i, j, gain); // 交换 2个皇后，并更新冲突总数
						}	
					}					
		}
		
	} while (NumCollisions > 0);

	finish = clock();

	double duration = (double) (finish-start)/CLOCKS_PER_SEC;

	printf("time:%f seconds\n", duration);

	return 0;
}