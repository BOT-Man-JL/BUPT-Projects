#include <iostream>

void Input (int mtx[][5])
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 5; ++j)
			std::cin >> mtx[i][j];
}

void Output (int mtx[][5])
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 5; ++j)
			std::cout << mtx[i][j] << "\t";
		std::cout << std::endl;
	}
}

void Add (int a[][5], int b[][5], int res[][5])  //res = a + b
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 5; ++j)
			res[i][j] = a[i][j] + b[i][j];
}

void Sub (int a[][5], int b[][5], int res[][5])  //res = a - b
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 5; ++j)
			res[i][j] = a[i][j] - b[i][j];
}

int main ()
{
	int (*A1)[5] = new int[4][5],
		(*A2)[5] = new int[4][5],
		(*A3)[5] = new int[4][5];

	std::cout << "Please input the 4*5 matrix A1:\n";
	Input (A1);
	std::cout << "Please input the 4*5 matrix A2:\n";
	Input (A2);

	Add (A1, A2, A3);
	std::cout << "A1 + A2 =:\n";
	Output (A3);
	Sub (A1, A2, A3);
	std::cout << "A1 - A2 =:\n";
	Output (A3);

	delete[] A1;
	delete[] A2;
	delete[] A3;
	
	return 0;
}
