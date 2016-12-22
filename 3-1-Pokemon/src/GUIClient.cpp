
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

	auto clientIP = IPADDR;
	if (argc > 1) clientIP = argv[1];
	Client client (clientIP, PORT);

	bool isPassiveOffline = false;
	UserModel curUser;
	PokemonID pidToPlay;
	std::pair<size_t, size_t> worldSize;
	std::vector<RoomPlayer> roomPlayers;
	std::vector<GameModel::ResultPlayer> gameResults;

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
				curUser = GUIClient::LoginWindow (
					client, isPassiveOffline);
				isPassiveOffline = false;
				guiState = GUIState::Start;
			}
			catch (const std::exception &)
			{
				try { client.Logout (); }
				catch (...) {}
				guiState = GUIState::Quit;
			}
			break;

		case GUIState::Start:
			try
			{
				size_t action;
				std::tie (action, pidToPlay) =
					GUIClient::StartWindow (curUser);
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
					try { client.Logout (); }
					catch (...) {}
				}
				guiState = GUIState::Login;
			}
			break;

		case GUIState::Users:
			GUIClient::UsersWindow (client);
			guiState = GUIState::Start;
			break;

		case GUIState::Pokemons:
			GUIClient::PokemonsWindow (client);
			guiState = GUIState::Start;
			break;

		case GUIState::Rooms:
			try
			{
				std::tie (worldSize, roomPlayers) =
					GUIClient::RoomWindow (client, curUser, pidToPlay);
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
					try { client.RoomLeave (); }
					catch (...) {}
					guiState = GUIState::Start;
				}
			}
			break;

		case GUIState::Game:
			try
			{
				gameResults = GUIClient::GameWindow (
					client, curUser, roomPlayers,
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
					try { client.RoomLeave (); }
					catch (...) {}
					guiState = GUIState::Start;
				}
			}
			break;

		case GUIState::GameResult:
			try
			{
				// Update curUser State
				auto updatedUsers = client.Users ();
				for (const auto &user : updatedUsers)
					if (user.uid == curUser.uid)
					{
						curUser = user;
						break;
					}

				GUIClient::ResultWindow (gameResults);
				guiState = GUIState::Start;

				// Begin to Leave
				client.RoomLeave ();
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
					try { client.RoomLeave (); }
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