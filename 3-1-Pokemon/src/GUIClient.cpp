
//
// Pokemon Game - Client Main
// BOT Man, 2016
//

#include <iostream>

#include "Client.h"
#include "GUIClient.h"

#define IPADDR "127.0.0.1"
#define PORT 5768

enum class GUIState
{
	Quit,
	Login,
	Start,
	Users,
	Pokemons,
	Rooms,
	Game,
	GameResult
};

int main (int argc, char *argv[])
{
	using namespace PokemonGame;
	using namespace PokemonGameGUI;

	// Read from start-up args
	auto clientIP = IPADDR;
	if (argc > 1) clientIP = argv[1];

	// Try until succeed to connect
	std::unique_ptr<Client> client;
	while (!client)
	{
		try { client = std::make_unique<Client> (clientIP, PORT); }
		catch (const std::exception &ex)
		{ std::cerr << ex.what () << std::endl; }
	}

	// Navigation Data
	bool isPassiveOffline = false;
	PokemonID pidToPlay;
	std::pair<size_t, size_t> worldSize;
	std::vector<RoomPlayer> roomPlayers;

	// Start at Login Page
	auto guiState = GUIState::Login;
	while (guiState != GUIState::Quit)
	{
		switch (guiState)
		{
		case GUIState::Quit:
			// Do nothing just Quit
			break;

		case GUIState::Login:
			try
			{
				GUIClient::LoginWindow (
					(*client), isPassiveOffline);
				isPassiveOffline = false;
				guiState = GUIState::Start;
			}
			catch (const std::exception &)
			{
				try { (*client).Logout (); }
				catch (...) {}
				guiState = GUIState::Quit;
			}
			break;

		case GUIState::Start:
			try
			{
				size_t action;
				std::tie (action, pidToPlay) =
					GUIClient::StartWindow ((*client));
				switch (action)
				{
				case 0: guiState = GUIState::Rooms; break;
				case 1: guiState = GUIState::Users; break;
				case 2: guiState = GUIState::Pokemons; break;
				default: break;
				}
			}
			catch (const std::exception &ex)
			{
				if (std::string (ex.what ()) == BadSession)
					isPassiveOffline = true;
				else
				{
					try { (*client).Logout (); }
					catch (...) {}
				}
				guiState = GUIState::Login;
			}
			break;

		case GUIState::Users:
			try
			{
				GUIClient::UsersWindow ((*client));
				guiState = GUIState::Start;
			}
			catch (const std::exception &)
			{
				isPassiveOffline = true;
				guiState = GUIState::Login;
			}
			break;

		case GUIState::Pokemons:
			try
			{
				GUIClient::PokemonsWindow ((*client));
				guiState = GUIState::Start;
			}
			catch (const std::exception &)
			{
				isPassiveOffline = true;
				guiState = GUIState::Login;
			}
			break;

		case GUIState::Rooms:
			try
			{
				std::tie (worldSize, roomPlayers) =
					GUIClient::RoomWindow ((*client), pidToPlay);
				guiState = GUIState::Game;
			}
			catch (const std::exception &ex)
			{
				if (std::string (ex.what ()) == BadSession)
				{
					isPassiveOffline = true;
					guiState = GUIState::Login;
				}
				else
				{
					try { (*client).RoomLeave (); }
					catch (...) {}
					guiState = GUIState::Start;
				}
			}
			break;

		case GUIState::Game:
			try
			{
				GUIClient::GameWindow (
					(*client), roomPlayers,
					worldSize.first, worldSize.second);
				guiState = GUIState::GameResult;
			}
			catch (const std::exception &ex)
			{
				if (std::string (ex.what ()) == BadSession)
				{
					isPassiveOffline = true;
					guiState = GUIState::Login;
				}
				else
				{
					try { (*client).RoomLeave (); }
					catch (...) {}
					guiState = GUIState::GameResult;
				}
			}
			break;

		case GUIState::GameResult:
			try
			{
				GUIClient::ResultWindow ((*client));
				guiState = GUIState::Start;

				// Leave Here
				(*client).RoomLeave ();
			}
			catch (const std::exception &ex)
			{
				if (std::string (ex.what ()) == BadSession)
				{
					isPassiveOffline = true;
					guiState = GUIState::Login;
				}
				else
				{
					try { (*client).RoomLeave (); }
					catch (...) {}
					guiState = GUIState::Start;
				}
			}
			break;

		default:
			throw std::runtime_error (
				"This will not happend unless your Compiler has a fatal BUG :-(");
			break;
		}
	}

	return 0;
}