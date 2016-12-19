
#include <iostream>
#include "Server.h"

#define PORT 5768

int main (int argc, char *argv[])
{
	std::cout << "Pokemon Server Started" << std::endl;
	PokemonGame::Server (PORT);

	return 0;
}