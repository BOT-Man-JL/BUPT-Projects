
#include <thread>
#include <iostream>

#include "Client.h"

#define IPADDR "127.0.0.1"
#define PORT 5768

int main (int argc, char *argv[])
{
	std::cout << "Pokemon Client\n";

	auto printPokemon = [] (
		std::ostream &os,
		const PokemonGame::PokemonModel &pokemon)
	{
		os << pokemon.name;
	};

	auto printUser = [&printPokemon] (
		std::ostream &os,
		const PokemonGame::UserModel &user)
	{
		os << user.uid
			<< (user.online ? "\tOnline" : "\tOffline")
			<< "\tWon Rate:" << user.wonRate
			<< "\n\t" << "Badges:\n";
		for (const auto &badge : user.badges)
			os << "\t\t" << badge << "\n";
		os << "\tPokemons:\n";
		for (const auto &pokemon : user.pokemons)
		{
			os << "\t\t";
			printPokemon (os, pokemon);
			os << "\n";
		}
	};

	auto printRoomPlayer = [&printPokemon] (
		std::ostream &os,
		const PokemonGame::RoomPlayer &player)
	{
		os << player.uid
			<< (player.isReady ? "\tReady" : "\tUnready")
			<< "\n\tPokemon:\n\t\t";
		printPokemon (os, player.pokemon);
		os << std::endl;
	};

	auto fnRegister = [] (PokemonGame::Client &client,
						  const PokemonGame::UserID &uid,
						  const PokemonGame::UserPwd &pwd)
	{
		try
		{
			std::cout << client.Register (uid, pwd) << std::endl;
		}
		catch (const std::exception &ex)
		{ std::cerr << ex.what () << std::endl; }
	};

	auto fnLogin = [&printUser] (
		PokemonGame::Client &client,
		const PokemonGame::UserID &uid,
		const PokemonGame::UserPwd &pwd)
	{
		try
		{
			auto userModel = client.Login (uid, pwd);
			std::cout << "Login Successfully:\n";
			printUser (std::cout, userModel);
		}
		catch (const std::exception &ex)
		{ std::cerr << ex.what () << std::endl; }
	};

	auto fnLogout = [] (PokemonGame::Client &client)
	{
		try
		{
			std::cout << client.Logout () << std::endl;
		}
		catch (const std::exception &ex)
		{ std::cerr << ex.what () << std::endl; }
	};

	auto fnPokemons = [&printPokemon] (
		PokemonGame::Client &client)
	{
		try
		{
			auto pokemons = client.Pokemons ();
			std::cout << "All Pokemons:\n";
			for (const auto &pokemon : pokemons)
			{
				std::cout << "\t";
				printPokemon (std::cout, pokemon);
				std::cout << "\n";
			}
		}
		catch (const std::exception &ex)
		{ std::cerr << ex.what () << std::endl; }
	};

	auto fnUsers = [&printUser] (
		PokemonGame::Client &client)
	{
		try
		{
			auto users = client.Users ();
			std::cout << "All Users:\n";
			for (const auto &user : users)
				printUser (std::cout, user);
		}
		catch (const std::exception &ex)
		{ std::cerr << ex.what () << std::endl; }
	};

	auto fnRooms = [] (PokemonGame::Client &client)
	{
		try
		{
			auto rooms = client.Rooms ();
			std::cout << "Rooms:\n";
			for (const auto &room : rooms)
				std::cout << "\t" << room.rid << " - "
				<< (room.isPending ? "Pending" : "in Game") << std::endl;
		}
		catch (const std::exception &ex)
		{ std::cerr << ex.what () << std::endl; }
	};

	auto fnRoomEnter = [] (PokemonGame::Client &client,
						   const PokemonGame::RoomID &rid,
						   const PokemonGame::PokemonID &pid)
	{
		try
		{
			std::cout << "Enter the Room of size ";
			auto size = client.RoomEnter (rid, pid);
			std::cout << size.first << ", " << size.second << std::endl;
		}
		catch (const std::exception &ex)
		{ std::cerr << ex.what () << std::endl; }
	};

	auto fnRoomLeave = [] (PokemonGame::Client &client)
	{
		try
		{
			std::cout << client.RoomLeave () << std::endl;
		}
		catch (const std::exception &ex)
		{ std::cerr << ex.what () << std::endl; }
	};

	auto fnRoomReady = [&printRoomPlayer] (
		PokemonGame::Client &client,
		bool isReady)
	{
		try
		{
			std::cout << "Ready:\n";
			auto players = client.RoomReady (isReady);
			for (const auto &player : players)
				printRoomPlayer (std::cout, player);
		}
		catch (const std::exception &ex)
		{ std::cerr << ex.what () << std::endl; }
	};

	PokemonGame::Client client1 (IPADDR, PORT);
	PokemonGame::Client client2 (IPADDR, PORT);
	PokemonGame::Client client3 (IPADDR, PORT);

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

		std::cout << "Client 2:\n";
		fnRegister (client2, "John", "LeeLee");
		fnLogin (client2, "John", "LeeLee");

		std::cout << "Client 3:\n";
		fnRegister (client3, "BOT", "ManMan");
		fnLogin (client3, "BOT", "ManMan");
	}

	if (true)
	{
		fnPokemons (client1);
		fnUsers (client1);

		fnPokemons (client2);
		fnUsers (client3);
	}

	if (true)
	{
		fnRoomEnter (client2, "Hello", 1);
		fnRoomEnter (client2, "Hello", 5);
		fnRoomLeave (client2);
		fnRoomEnter (client2, "Hello", 5);
		fnRoomEnter (client2, "Hello2", 5);

		fnRoomEnter (client3, "Hello2", 8);
		fnRooms (client3);
		fnRoomLeave (client3);
		fnRoomLeave (client3);

		fnRoomEnter (client3, "Hello", 8);
		fnRooms (client3);
		fnRoomReady (client3, true);

		fnRoomReady (client2, true);
		fnRoomLeave (client2);
		fnRoomEnter (client2, "Hello", 5);
		fnRoomReady (client2, false);
	}

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