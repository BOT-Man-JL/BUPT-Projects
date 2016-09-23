#include <iostream>
#include <memory>

namespace My_Point {  //namespace My_Point ->

class Point
{
	int x, y;
public:
	Point (int i, int j) : x (i), y (j) {}
	Point &operator++ () { ++x; ++y; return *this; }
	Point operator++ (int)
	{
		Point tmp (*this);
		++x; ++y;
		return std::move (tmp);
	}
	Point &operator-- () { --x; --y; return *this; }
	Point operator-- (int)
	{
		Point tmp (*this);
		--x; --y;
		return std::move (tmp);
	}
	int GetX () const { return x; }
	int GetY () const { return y; }
};

}  //<- namespace My_Point

int main ()
{
	using namespace My_Point;
	Point p(1, 2);
	std::cout << (p++).GetX () << std::endl;
	std::cout << (++p).GetY () << std::endl;
	std::cout << (p--).GetX () << std::endl;
	std::cout << (--p).GetY () << std::endl;

	return 0;
}
