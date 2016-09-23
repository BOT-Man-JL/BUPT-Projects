#include <iostream>
#include <cmath>

namespace My_Shape {  //namespace My_Shape ->

class Shape
{
public:
	double area () const { return -1; }
};

class Rect : public Shape
{
	double top, left, bottom, right;
public:
	Rect (double a, double b, double c, double d)
		: top (a), left (b), bottom (c), right (d)
		{ std::cout << "Constructing Rect" << std::endl; }
	double area () const { return abs ((top - bottom) * (left - right)); }
	~Rect () { std::cout << "Destructing Rect" << std::endl; }
};

class Square : public Rect
{
public:
	Square (double a, double b, double len)
		: Rect (a, b, a + len, b + len)
		{ std::cout << "Constructing Square" << std::endl; }
	double area () const { return Rect::area (); }
	~Square () { std::cout << "Destructing Square" << std::endl; }
};

class Circle : public Shape
{
	double x, y, rad;
	const static double PI;
public:
	Circle (double _x, double _y, double _rad)
		: x (_x), y (_y), rad (_rad)
		{ std::cout << "Constructing Circle" << std::endl; }
	double area () const { return PI * rad * rad; }
	~Circle () { std::cout << "Destructing Circle" << std::endl; }
};
const double Circle::PI = 3.14159;

double GetArea (const Shape &s)
{
	return s.area ();
}

}  //<- namespace My_Shape

int main ()
{
	{
	using namespace My_Shape;
	Circle c (0, 0, 1);
	Square s (0, 0, 3);
	Rect rect (1, 2, 3, 4);

	std::cout << GetArea (c) << std::endl;
	std::cout << GetArea (s) << std::endl;
	std::cout << GetArea (rect) << std::endl;
	}

	return 0;
}
