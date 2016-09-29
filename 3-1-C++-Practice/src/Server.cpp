
#include <string>
#include <iostream>
#include "Socket.h"

int main (int argc, char *argv[])
{
	std::cout << "Pokemon Server" << std::endl;

	PokemonGame_Impl::Server (5768, [] (std::string request,
							 std::string &response)
	{
		std::cout << request << std::endl;
		response = "Echo: " + request;
	});

	getchar ();
	return 0;
}