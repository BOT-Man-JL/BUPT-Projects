#include <iostream>

namespace Boat_and_Car {  //namespace Boat_and_Car ->
class Car;
class Boat
{
	double weight;
public:
	Boat (double i) : weight (i) {}
	friend double TotalWeight (const Boat &b, const Car &c);
};

class Car
{
	double weight;
public:
	Car (double i) : weight (i) {}
	friend double TotalWeight (const Boat &b, const Car &c);
};

double TotalWeight (const Boat &b, const Car &c)
{
	return b.weight + c.weight;
}
}  //<- namespace Boat_and_Car

int main ()
{
	double i, j;
	std::cout << "Input the weight of the boat and the car:\n";
	std::cin >> i >> j;
	
	using namespace Boat_and_Car;
	Boat b (i);
	Car c (j);
	std::cout << "The total weight of them is: " << TotalWeight (b, c) << std::endl;

	return 0;
}
