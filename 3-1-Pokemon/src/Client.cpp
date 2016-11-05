
#include <thread>
#include <iostream>
#include <iomanip>

#include "PokemonClient.h"
#include "GUIGame.h"
#include "GUIAccounting.h"
#include "GUIRoom.h"

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
		std::vector<PokemonGame::PokemonID> pokemonIds;
		for (const auto &pokemon : client.MyPokemons ())
		{
			if (pokemonIds.size () == 3)
				break;
			pokemonIds.emplace_back (pokemon.first);
		}

		if (pokemonIds.size () >= 3 &&
			client.RoomEnter (roomId,
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

	auto fnRoomReady = [&fnPrintPokemon] (
		PokemonGame::PokemonClient &client)
	{
		if (client.RoomReady ())
		{
			const auto &players = client.ViewPlayers ();
			if (players.empty ())
			{
				std::cout << "No Player in this Room\n";
				return;
			}

			std::cout << "Players in this Room:\n";
			for (const auto &player : players)
			{
				std::cout << "\t" << player.uid << ", "
					<< std::boolalpha << player.isReady << ", "
					<< player.x << ", "
					<< player.y << "\n";

				for (const auto &pokemon : player.pokemons)
				{
					std::cout << "\t";
					fnPrintPokemon (pokemon.second.get ());
				}
			}
		}
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto guiLogin = [] (PokemonGame::PokemonClient &client)
	{
		auto hasLogin = false;
		PokemonGameGUI::AccountingWindow wnd;

		wnd.OnLogin ([&wnd, &client, &hasLogin] (
			const std::string &uid, const std::string &pwd)
		{
			wnd.Prompt ("Loging in...");
			if (client.Login (uid, pwd))
			{
				wnd.Prompt ("Login Successfully");
				hasLogin = true;
			}
			else
				wnd.Prompt (client.ErrMsg ());
		});

		wnd.OnRegister ([&wnd, &client] (
			const std::string &uid, const std::string &pwd)
		{
			wnd.Prompt ("Registering...");
			if (client.Register (uid, pwd))
				wnd.Prompt ("Registered Successfully");
			else
				wnd.Prompt (client.ErrMsg ());
		});

		while (!wnd.IsClosed () && !hasLogin)
			std::this_thread::sleep_for (
				std::chrono::milliseconds (200));

		return hasLogin;
	};

	auto guiSelectRoom = [] (PokemonGame::PokemonClient &client)
	{
		auto hasEntered = false;
		auto isReady = false;
		PokemonGameGUI::RoomWindow wnd;

		std::vector<PokemonGame::PokemonID> pokemonIds;
		for (const auto &pokemon : client.MyPokemons ())
		{
			if (pokemonIds.size () == 3)
				break;
			pokemonIds.emplace_back (pokemon.first);
		}

		wnd.OnSelectRoom ([&wnd, &client, &hasEntered, &pokemonIds] (
			const std::string &roomId)
		{
			if (pokemonIds.size () >= 3 &&
				client.RoomEnter (roomId,
								  pokemonIds[0], pokemonIds[1], pokemonIds[2]))
			{
				wnd.Prompt ("You Entered " + roomId);
				hasEntered = true;
			}
			else
				wnd.Prompt (client.ErrMsg ());
		});

		while (!wnd.IsClosed () && !isReady)
		{
			std::vector<std::string> roomList;
			if (!hasEntered)
			{
				if (client.RoomQuery (roomList))
					wnd.SetRooms (std::move (roomList));
				else
					wnd.Prompt (client.ErrMsg ());
			}
			else
			{
				if (client.RoomReady ())
				{
					isReady = true;
					const auto &players = client.ViewPlayers ();
					for (const auto &player : players)
						isReady = isReady && player.isReady;
					wnd.SetPlayers (players);
				}
				else
					wnd.Prompt (client.ErrMsg ());
			}
			std::this_thread::sleep_for (std::chrono::seconds (1));
		}

		return hasEntered;
	};

	// Test

	std::cout << "Client 1:\n";
	auto client1 = fnConnect ();
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
	auto client2 = fnConnect ();
	fnRegister (client2, "John", "LeeLee");
	fnLogin (client2, "John", "LeeLee");
	fnPokemonAll (client2);

	std::cout << "Client 3:\n";
	auto client3 = fnConnect ();
	fnRegister (client3, "BOT", "ManMan");
	fnLogin (client3, "BOT", "ManMan");
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
	fnRoomLeave (client3);
	fnRoomEnter (client3, "Hello");
	fnRoomQuery (client3);

	fnRoomReady (client3);
	fnRoomReady (client2);
	fnRoomLeave (client2);
	fnRoomReady (client3);

	PokemonGame::PokemonClient client_gui (IPADDR, PORT);

	if (!guiLogin (client_gui))
	{
		PokemonGameGUI::MsgBox ("You haven't Login in GUI\n");
		return 0;
	}

	// Simulate multi-players
	fnRoomEnter (client2, "Hello");
	std::thread thd ([&] ()
	{
		getchar ();
		fnRoomReady (client2);
		getchar ();
	});

	if (!guiSelectRoom (client_gui))
		PokemonGameGUI::MsgBox ("You haven't Entered a Room\n");

	if (thd.joinable ())
		thd.join ();

	fnRoomLeave (client2);
	fnRoomLeave (client3);
	fnRoomLeave (client_gui);

	fnLogout (client2);
	fnLogout (client3);
	fnLogout (client_gui);

	getchar ();
	return 0;
}