#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

int main ()
{
	std::string s;

	std::cout << "Input the output-file name: ";
	std::cin >> s;
	std::ofstream _of (s);
	if (!_of.is_open ())
	{
		std::cerr << "Can't not write";
		return 0;
	}

	_of << -1 << ' ' << 1 << ' ' << 1L << ' ' << 1.2 << ' ' << "string" << std::endl;
	_of << std::showbase << std::dec << 53 << ' ' << std::oct << 53 << ' ' << std::hex << 53 << std::endl;
	_of << 1 << std::setw (5) << 1 << " " << std::setprecision (10) << 3.141592653589 << std::endl;

	return 0;
}
