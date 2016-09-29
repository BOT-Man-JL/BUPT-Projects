
#include <iostream>

#include "Socket.h"

int main (int argc, char *argv[])
{
	{
		PokemonGame_Impl::Client client ("127.0.0.1", 5768);
		std::cout << "Pokemon Client" << std::endl;

		auto request = "haha";
		std::cout << "I said '" << request << "'\n";

		auto response = client.Request (request);
		std::cout << "I got '" << response << "'\n";
	}

	getchar ();
	return 0;
}