
#include <thread>
#include <iostream>
#include <iomanip>

#include "PokemonClient.h"
#define IPADDR "127.0.0.1"
#define PORT 5768

int main (int argc, char *argv[])
{
	std::cout << "Pokemon Client\n";

	auto fnConnect = [] ()
	{
		return PokemonGame::PokemonClient (IPADDR, PORT);
	};

	auto fnPrintPokemon = [] (const PokemonGame::Pokemon *pokemon)
	{
		std::cout << "\t" << std::setw (10)
			<< pokemon->GetName ()
			<< ": [LV:"
			<< pokemon->GetLevel ()
			<< ", Exp:"
			<< pokemon->GetExp ()
			<< ", Atk:"
			<< pokemon->GetAtk ()
			<< ", Def:"
			<< pokemon->GetDef ()
			<< ", HP:"
			<< pokemon->GetHP ()
			<< ", FHP:"
			<< pokemon->GetFullHP ()
			<< ", Gap:"
			<< pokemon->GetTimeGap ()
			<< "]\n";
	};

	auto fnLogin = [] (PokemonGame::PokemonClient &client,
					   std::string uid, std::string pwd)
	{
		if (client.Login (uid, pwd))
			std::cout << "Login Successfully\n";
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnRegister = [] (PokemonGame::PokemonClient &client,
						  std::string uid, std::string pwd)
	{
		if (client.Register (uid, pwd))
			std::cout << "Registered Successfully\n";
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnLogout = [] (PokemonGame::PokemonClient &client)
	{
		if (client.Logout ())
			std::cout << "Logout Successfully\n";
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnUsersWonRate = [] (PokemonGame::PokemonClient &client,
							  const std::string &uid)
	{
		double result;
		std::cout << "WonRate of User: " << uid << std::endl;
		if (client.UsersWonRate (uid, result))
			std::cout << '\t' << result << std::endl;
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnUsersBadges = [] (PokemonGame::PokemonClient &client,
							 const std::string &uid)
	{
		std::vector<std::string> result;
		std::cout << "Badges of User: " << uid << std::endl;
		if (client.UsersBadges (uid, result))
			for (const auto user : result)
				std::cout << '\t' << user << std::endl;
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnUsersAll = [] (PokemonGame::PokemonClient &client)
	{
		std::vector<std::string> result;
		std::cout << "All Users\n";
		if (client.UsersAll (result))
			for (const auto user : result)
				std::cout << '\t' << user << std::endl;
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnUsersOnline = [] (PokemonGame::PokemonClient &client)
	{
		std::vector<std::string> result;
		std::cout << "Online Users\n";
		if (client.UsersOnline (result))
			for (const auto user : result)
				std::cout << '\t' << user << std::endl;
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnUsersPokemons = [&fnPrintPokemon] (
		PokemonGame::PokemonClient &client,
		const std::string &uid)
	{
		std::vector<std::unique_ptr<PokemonGame::Pokemon>> result;
		std::cout << "Pokemons of User: " << uid << std::endl;
		if (client.UsersPokemons (uid, result))
			for (const auto &pokemon : result)
				fnPrintPokemon (pokemon.get ());
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnPokemonAll = [&fnPrintPokemon] (
		PokemonGame::PokemonClient &client)
	{
		std::vector<std::unique_ptr<PokemonGame::Pokemon>> result;
		std::cout << "All Pokemons: " << std::endl;
		if (client.PokemonAll (result))
			for (const auto &pokemon : result)
				fnPrintPokemon (pokemon.get ());
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	std::cout << "Client 1:\n";
	auto client = fnConnect ();
	fnRegister (client, "Xuzhu", "xuzhu");
	fnRegister (client, "Xuzhu", "xuzhu");
	fnLogin (client, "Xuzhu", "xuzhu");
	fnLogin (client, "Xuzhu", "xuzhu");
	fnLogin (client, "Xuzhu", "Hack");
	fnLogout (client);
	fnLogout (client);
	fnUsersAll (client);
	fnUsersOnline (client);
	fnUsersPokemons (client, "Xuzhu");
	fnUsersPokemons (client, "xuzhu");

	std::cout << "Client 2:\n";
	auto client2 = fnConnect ();
	fnRegister (client2, "John", "Lee");
	fnLogin (client2, "John", "Lee");
	fnPokemonAll (client2);

	std::cout << "Client 3:\n";
	auto client3 = fnConnect ();
	fnRegister (client3, "BOT", "Man");
	fnLogin (client3, "BOT", "Man");
	fnUsersAll (client3);
	fnUsersOnline (client3);
	fnUsersWonRate (client3, "BOT");
	fnUsersBadges (client3, "BOT");
	fnUsersPokemons (client3, "BOT");
	fnPokemonAll (client2);

	fnLogout (client2);
	fnLogout (client3);

	getchar ();
	return 0;
}