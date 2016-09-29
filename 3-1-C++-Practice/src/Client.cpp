
#include <iostream>
#include <thread>

#include "Socket.h"

int main (int argc, char *argv[])
{
	auto fnThread = [] ()
	{
		PokemonGame_Impl::Client client ("127.0.0.1", 5768);
		std::cout << "Pokemon Client" << std::endl;

		auto request = "haha";
		std::cout << "I said '" << request << "'\n";

		auto response = client.Request (request);
		std::cout << "I got '" << response << "'\n";
	};

	std::thread (fnThread).detach ();
	std::thread (fnThread).detach ();
	std::thread (fnThread).detach ();
	std::thread (fnThread).detach ();
	std::thread (fnThread).detach ();

	getchar ();
	return 0;
}