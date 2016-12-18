
#include <iostream>

#include "PokemonClient.h"
#include "GUIClient.h"

#define IPADDR "127.0.0.1"
#define PORT 5768

enum class GUIState
{
	Quit,
	Login,
	ViewInfo,
	Pokemons,
	Users,
	Rooms,
	Game,
	GameResult
};

int main (int argc, char *argv[])
{
	using namespace PokemonGame;
	using namespace PokemonGameGUI;

	PokemonClient client = PokemonClient (IPADDR, PORT);

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
				curUser = GUIClient::LoginWindow (client);
				guiState = GUIState::ViewInfo;
			}
			catch (const std::exception &)
			{
				guiState = GUIState::Quit;
			}
			break;

		case GUIState::ViewInfo:
			pidToPlay = curUser.pokemons.front ().pid;
			guiState = GUIState::Rooms;
			break;

		case GUIState::Pokemons:
			break;

		case GUIState::Users:
			break;

		case GUIState::Rooms:
			try
			{
				std::tie (worldSize, roomPlayers) =
					GUIClient::RoomWindow (client, curUser, pidToPlay);
				guiState = GUIState::Game;
			}
			catch (const std::exception &)
			{
				guiState = GUIState::ViewInfo;
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
			catch (const std::exception &)
			{
				client.RoomLeave ();
				guiState = GUIState::ViewInfo;
			}
			break;

		case GUIState::GameResult:
			break;

		default:
			throw std::runtime_error (
				"This will not happend unless your OS has a fatal BUG :-(");
			break;
		}
	}

	return 0;
}