#include <iostream>
#include <cmath>

namespace Point_and_Rect {  //namespace Point_and_Rect ->
class Point
{
	int x, y;
public:
	Point (int i, int j) : x (i), y (j)
		{ std::cout << "Constructing Point (" << x << ", " << y << ")" << std::endl; }
	Point () : x (0), y (0)
		{ std::cout << "Constructing Point (0, 0)" << std::endl; }
	friend class Rect;
	friend double distance (const Point &p, const Point &q);
	~Point ()
		{ std::cout << "Destructing Point (" << x << ", " << y << ")" << std::endl; }
};

double distance (const Point &p, const Point &q)
{
	return sqrt (double ((p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y)));
}

class Rect
{
	Point ptTL, ptBR;  //top-left, bottom-right
public:
	Rect (int a, int b, int c, int d) : ptTL (a, b), ptBR (c, d)
		{ std::cout << "Constructing Rect" << std::endl; }
	int area () const { return abs ((ptTL.x - ptBR.x) * (ptTL.y - ptBR.y)); }
	~Rect () { std::cout << "Destructing Rect" << std::endl; }
};
}  //<- namespace Point_and_Rect

int main ()
{
	namespace PR = Point_and_Rect;
	int a, b, c, d;

	std::cout << "Input the top-left & bottom-right position of the rectangle:\n";
	std::cin >> a >> b >> c >> d;
	PR::Rect rect(a, b, c, d);
	std::cout << "The area of this rectangle is: " << rect.area () << std::endl;

	return 0;
}
