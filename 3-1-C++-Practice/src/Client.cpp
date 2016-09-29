
#include <string>
#include <iostream>
#include "Socket.h"

int main (int argc, char *argv[])
{
	PokemonGame_Impl::Client client;
	std::cout << "Pokemon Client" << std::endl;

	auto response = client.Request ("127.0.0.1", 5768, "haha");
	std::cout << response << std::endl;

	getchar ();
	return 0;
}