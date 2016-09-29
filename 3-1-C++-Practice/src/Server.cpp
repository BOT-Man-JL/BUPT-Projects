
#include <string>
#include <iostream>
#include <vector>

#include "Protocol.h"

int main (int argc, char *argv[])
{
	std::cout << "Pokemon Server Started" << std::endl;
	PokemonGame::PokemonServer (5768);

	return 0;
}