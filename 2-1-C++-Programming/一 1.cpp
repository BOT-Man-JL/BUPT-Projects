#include <iostream>
#include <random>

int main ()
{
	int price = std::random_device () () % 1000 + 1, guess;

	std::cin >> guess;
	while (guess != price)
	{
		if (!std::cin.good ())
			std::cout << "Bad Input" << std::endl;
		else {
			if (guess > 1000 || guess < 1)
				std::cout << "The price ranges from 1 to 1000" << std::endl;
			else {
				if (guess > price) std::cout << "Too High" << std::endl;
				else std::cout << "Too Low" << std::endl;
			}
		}
		std::cin.clear ();
		std::cin.sync ();
		std::cin >> guess;
	}
	std::cout << "You got it" << std::endl;

	return 0;
}
