
//
// Pokemon Game - Server Main
// BOT Man, 2016
//

#include <iostream>
#include "Server.h"

#define PORT 5768

int main (int argc, char *argv[])
{
	std::cout << "Pokemon Server Started" << std::endl;

	// Run an instance of Server
	PokemonGame::Server (PORT);

	// Never Hit
	return 0;
}