
#include <iostream>
#include "PokemonServer.h"

#define PORT 5768

int main (int argc, char *argv[])
{
	std::cout << "Pokemon Server Started" << std::endl;
	PokemonGame::PokemonServer (PORT);

	return 0;
}