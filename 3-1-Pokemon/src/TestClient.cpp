
#include <thread>
#include <iostream>

#include "PokemonClient.h"

#define IPADDR "127.0.0.1"
#define PORT 5768

int main (int argc, char *argv[])
{
	std::cout << "Pokemon Client\n";

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

	auto fnMyPokemons = [] (
		PokemonGame::PokemonClient &client)
	{
		std::cout << "My Pokemons:" << std::endl;
		for (const auto &pokemon : client.MyPokemons ())
			PrintPokemon (std::cout, pokemon.second.get ());
	};

	auto fnUsersPokemons = [] (
		PokemonGame::PokemonClient &client,
		const std::string &uid)
	{
		PokemonGame::PokemonsWithID result;
		std::cout << "Pokemons of User: " << uid << std::endl;
		if (client.UsersPokemons (uid, result))
			for (const auto &pokemon : result)
				PrintPokemon (std::cout, pokemon.second.get ());
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnPokemonAll = [] (
		PokemonGame::PokemonClient &client)
	{
		PokemonGame::PokemonsWithID result;
		std::cout << "All Pokemons: " << std::endl;
		if (client.PokemonAll (result))
			for (const auto &pokemon : result)
				PrintPokemon (std::cout, pokemon.second.get ());
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
		auto pid = client.MyPokemons ().begin ()->first;
		if (client.RoomEnter (roomId, pid))
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

	auto fnRoomReady = [] (
		PokemonGame::PokemonClient &client)
	{
		if (client.RoomReady () && client.RoomState ())
		{
			std::cout << "I'am :" << client.MyUID () << std::endl;
			std::cout << "Players in this Room:\n";
			for (const auto &player : client.GetPlayers ())
				PrintPlayer (std::cout, player.first, player.second);
		}
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnLockStep = [] (
		PokemonGame::PokemonClient &client,
		PokemonGame::Action &action)
	{
		if (client.Lockstep (action))
		{
			std::cout << "I'am :" << client.MyUID () << std::endl;
			std::cout << "Players' State:\n";
			for (const auto &player : client.GetPlayers ())
				PrintPlayer (std::cout, player.first, player.second);
		}
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	PokemonGame::PokemonClient client1 (IPADDR, PORT);
	PokemonGame::PokemonClient client2 (IPADDR, PORT);
	PokemonGame::PokemonClient client3 (IPADDR, PORT);

	// Test
	if (true)
	{
		std::cout << "Client 1:\n";
		fnRegister (client1, "Johnny", "John");
		fnRegister (client1, "Johnny", "Johnny");
		fnRegister (client1, "Johnny", "Johnny");
		fnLogin (client1, "Johnny", "Johnny");
		fnLogin (client1, "Johnny", "Johnny");
		fnLogin (client1, "Johnny", "Hack");
		fnLogout (client1);
		fnLogout (client1);
		fnUsersAll (client1);
		fnUsersOnline (client1);
		fnUsersPokemons (client1, "Johnny");
		fnUsersPokemons (client1, "Johnny");

		std::cout << "Client 2:\n";
		fnRegister (client2, "John", "LeeLee");
		fnLogin (client2, "John", "LeeLee");

		std::cout << "Client 3:\n";
		fnRegister (client3, "BOT", "ManMan");
		fnLogin (client3, "BOT", "ManMan");
	}

	// Test
	if (true)
	{
		fnPokemonAll (client2);
		fnUsersAll (client3);
		fnUsersOnline (client3);
		fnUsersWonRate (client3, "BOT");
		fnUsersBadges (client3, "BOT");
		fnMyPokemons (client3);
		fnUsersPokemons (client3, "John");
		fnPokemonAll (client2);
	}

	// Test
	if (true)
	{
		fnRoomEnter (client2, "Hello");
		fnRoomLeave (client2);
		fnRoomEnter (client2, "Hello");
		fnRoomEnter (client2, "Hello2");

		fnRoomEnter (client3, "Hello2");
		fnRoomQuery (client3);
		fnRoomLeave (client3);
		fnRoomLeave (client3);

		fnRoomEnter (client3, "Hello");
		fnRoomQuery (client3);
		fnRoomReady (client3);

		fnRoomReady (client2);
		fnRoomLeave (client2);
		fnRoomEnter (client2, "Hello");
		fnRoomReady (client2);
	}

	// Test
	if (true)
	{
		fnRoomLeave (client2);
		fnRoomLeave (client3);

		fnLogout (client2);
		fnLogout (client3);
	}

	getchar ();
	return 0;
}