#include <iostream>

namespace Matrix_Assignment {  //namespace Matrix_Assignment ->

class size_not_matched {};  //Error class

class Matrix
{
	int rows, lines;
	int **arr;
public:
	Matrix () : rows (0), lines (0), arr (nullptr) {}
	Matrix (int row, int col);
	Matrix (const Matrix &mtx);
	Matrix (Matrix &&mtx);
	Matrix &operator= (const Matrix &mtx);
	Matrix &operator= (Matrix &&mtx);
	~Matrix ();
	Matrix operator+ (const Matrix &mtx) const;
	Matrix operator- (const Matrix &mtx) const;
	void Input () const;
	void Output () const;
};

inline Matrix::Matrix (int row, int col) : rows (row), lines (col), arr (new int*[rows])
{
	for (int i = 0; i < rows; ++i)
		arr[i] = new int[lines];
}

inline Matrix::Matrix (const Matrix &mtx) : rows (mtx.rows), lines (mtx.lines), arr (new int*[rows])
{
	for (int i = 0; i < rows; ++i)
	{
		arr[i] = new int[lines];
		for (int j = 0; j < lines; ++j)
			arr[i][j] = mtx.arr[i][j];
	}
}

inline Matrix::Matrix (Matrix &&mtx) : rows (mtx.rows), lines (mtx.lines), arr (mtx.arr)
{
	mtx.rows = mtx.lines = 0;
	mtx.arr = nullptr;
}

inline Matrix &Matrix::operator= (const Matrix &mtx)
{
	if (this != &mtx)
	{
		this -> ~Matrix ();

		rows = mtx.rows;
		lines = mtx.lines;
		arr = new int*[rows];
		for (int i = 0; i < rows; ++i)
		{
			arr[i] = new int[lines];
			for (int j = 0; j < lines; ++j)
				arr[i][j] = mtx.arr[i][j];
		}
	}
	return *this;
}

inline Matrix &Matrix::operator= (Matrix &&mtx)
{
	if (this != &mtx)
	{
		this -> ~Matrix ();

		rows = mtx.rows;
		lines = mtx.lines;
		arr = mtx.arr;

		mtx.rows = mtx.lines = 0;
		mtx.arr = nullptr;
	}
	return *this;
}

inline Matrix::~Matrix ()
{
	for (int i = 0; i < rows; ++i)
		delete[] arr[i];
	delete[] arr;
}

inline Matrix Matrix::operator+ (const Matrix &mtx) const
{
	if (rows != mtx.rows || lines != mtx.lines)
		throw size_not_matched ();
	Matrix res (*this);
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < lines; ++j)
			res.arr[i][j] += mtx.arr[i][j];
	return res;
}

inline Matrix Matrix::operator- (const Matrix &mtx) const
{
	if (rows != mtx.rows || lines != mtx.lines)
		throw size_not_matched ();
	Matrix res (*this);
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < lines; ++j)
			res.arr[i][j] -= mtx.arr[i][j];
	return res;
}

inline void Matrix::Input () const
{
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < lines; ++j)
			std::cin >> arr[i][j];
}

inline void Matrix::Output () const
{
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < lines; ++j)
			std::cout << arr[i][j] << "\t";
		std::cout << std::endl;
	}
}

}  //<- namespace Matrix_Assignment

int main ()
{
	using namespace Matrix_Assignment;

	int row, col;
	std::cout << "Please input the number of rows and lines:\n";
	std::cin >> row >> col;
	Matrix A1 (row, col), A2 (row, col), A3;

	std::cout << "Please input the " << row << " * " << col << " matrix A1:\n";
	A1.Input ();
	std::cout << "Please input the " << row << " * " << col << " matrix A2:\n";
	A2.Input ();

	A3 = A1 + A2;
	std::cout << "A1 + A2 =\n";
	A3.Output ();

	A3 = A1 - A2;
	std::cout << "A1 - A2 =\n";
	A3.Output ();

	Matrix *pA1 (new Matrix (A1)), *pA2 (new Matrix (A2)), *pA3;
	pA3 = new Matrix (*pA1 + *pA2);
	std::cout << "pA1 + pA2 =\n";
	pA3 -> Output ();
	delete pA3;

	pA3 = new Matrix (*pA1 - *pA2);
	std::cout << "pA1 - pA2 =\n";
	pA3 -> Output ();
	delete pA3;
	delete pA2;
	delete pA1;

	return 0;
}
