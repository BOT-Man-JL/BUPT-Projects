
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
		std::cout << "All Users:\n";
		if (client.UsersAll (result))
			for (const auto user : result)
				std::cout << '\t' << user << std::endl;
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnUsersOnline = [] (PokemonGame::PokemonClient &client)
	{
		std::vector<std::string> result;
		std::cout << "Online Users:\n";
		if (client.UsersOnline (result))
			for (const auto user : result)
				std::cout << '\t' << user << std::endl;
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnMyPokemons = [&fnPrintPokemon] (
		PokemonGame::PokemonClient &client)
	{
		std::cout << "My Pokemons:" << std::endl;
		for (const auto &pokemon : client.MyPokemons ())
			fnPrintPokemon (pokemon.second.get ());
	};

	auto fnUsersPokemons = [&fnPrintPokemon] (
		PokemonGame::PokemonClient &client,
		const std::string &uid)
	{
		PokemonGame::Pokemons result;
		std::cout << "Pokemons of User: " << uid << std::endl;
		if (client.UsersPokemons (uid, result))
			for (const auto &pokemon : result)
				fnPrintPokemon (pokemon.second.get ());
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnPokemonAll = [&fnPrintPokemon] (
		PokemonGame::PokemonClient &client)
	{
		PokemonGame::Pokemons result;
		std::cout << "All Pokemons: " << std::endl;
		if (client.PokemonAll (result))
			for (const auto &pokemon : result)
				fnPrintPokemon (pokemon.second.get ());
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnRoomQuery = [] (PokemonGame::PokemonClient &client)
	{
		std::vector<std::string> result;
		std::cout << "Rooms\n";
		if (client.RoomQuery (result))
			for (const auto roomId : result)
				std::cout << '\t' << roomId << std::endl;
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnRoomEnter = [] (
		PokemonGame::PokemonClient &client,
		std::string roomId)
	{
		std::vector<std::string> pokemonIds;
		for (const auto &pokemon : client.MyPokemons ())
		{
			if (pokemonIds.size () == 3)
				break;
			pokemonIds.emplace_back (std::to_string (pokemon.first));
		}

		if (client.RoomEnter (roomId,
							  pokemonIds[0], pokemonIds[1], pokemonIds[2]))
			std::cout << "Entered " << roomId << std::endl;
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnRoomLeave = [] (PokemonGame::PokemonClient &client)
	{
		if (client.RoomLeave ())
			std::cout << "Leaved the Room" << std::endl;
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnRoomReady = [] (PokemonGame::PokemonClient &client)
	{
		if (client.RoomReady ())
			std::cout << "I'am Ready" << std::endl;
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnViewPlayers = [&fnPrintPokemon] (PokemonGame::PokemonClient &client)
	{
		const auto &players = client.ViewPlayers ();
		std::cout << "Players in this Room:\n";
		for (const auto &player : players)
		{
			std::cout << "\t" << player.second.uid << ", "
				<< std::boolalpha << player.second.isReady << ", "
				<< player.second.x << ", "
				<< player.second.y << "\n";

			std::cout << "\t";
			fnPrintPokemon (player.second.pokemon1.get ());
			std::cout << "\t";
			fnPrintPokemon (player.second.pokemon2.get ());
			std::cout << "\t";
			fnPrintPokemon (player.second.pokemon3.get ());
		}
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
	fnMyPokemons (client3);
	fnUsersPokemons (client3, "John");
	fnPokemonAll (client2);

	fnRoomEnter (client2, "Hello");
	fnRoomLeave (client2);
	fnRoomEnter (client2, "Hello");
	fnRoomEnter (client2, "Hello2");

	fnRoomEnter (client3, "Hello2");
	fnRoomQuery (client3);
	fnRoomLeave (client3);
	fnRoomEnter (client3, "Hello");
	fnRoomQuery (client3);
	fnRoomReady (client3);
	fnViewPlayers (client3);

	fnLogout (client2);
	fnLogout (client3);

	getchar ();
	return 0;
}