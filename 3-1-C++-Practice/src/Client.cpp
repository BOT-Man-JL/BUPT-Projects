
#include <thread>
#include <iostream>

#include "Protocol.h"

int main (int argc, char *argv[])
{
	auto fnLogin = [] (std::string uid, std::string pwd)
	{
		std::cout << "Pokemon Client\n";
		PokemonGame::PokemonClient client ("127.0.0.1", 5768);

		if (client.Login (uid, pwd))
			std::cout << "Login Successfully\n";
		else
		{
			std::cerr << client.ErrMsg () << std::endl;
			return;
		}

		if (client.Logout ())
			std::cout << "Logout Successfully\n";
		else
		{
			std::cerr << client.ErrMsg () << std::endl;
			return;
		}
	};

	auto fnReg = [] (std::string uid, std::string pwd)
	{
		std::cout << "Pokemon Client\n";
		PokemonGame::PokemonClient client ("127.0.0.1", 5768);

		if (client.Register (uid, pwd))
			std::cout << "Registered Successfully\n";
		else
		{
			std::cerr << client.ErrMsg () << std::endl;
			return;
		}

		if (client.Logout ())
			std::cout << "Logout Successfully\n";
		else
		{
			std::cerr << client.ErrMsg () << std::endl;
			return;
		}
	};

	std::thread (fnReg, "John", "Lee").detach ();
	std::thread (fnLogin, "John", "Lee").detach ();
	std::thread (fnLogin, "Admin", "AdminJohn").detach ();

	getchar ();
	return 0;
}