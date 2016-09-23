#include <fstream>
#include <iostream>
#include <string>

int main ()
{
	std::string s;

	std::cout << "Input the input-file name: ";
	std::cin >> s;
	std::ifstream _if (s);
	if (!_if.is_open ())
	{
		std::cerr << "Can't not read";
		return 0;
	}

	std::cout << "Input the output-file name: ";
	std::cin >> s;
	std::ofstream _of (s);
	if (!_of.is_open ())
	{
		std::cerr << "Can't not write";
		return 0;
	}

	int lm = 1;  //line number
	_of << lm;
	for (int ch = _if.get (); ch != EOF; ch = _if.get ())
	{
		_of.put (ch);
		if (ch == '\n') _of << ++lm;
	}

	return 0;
}
