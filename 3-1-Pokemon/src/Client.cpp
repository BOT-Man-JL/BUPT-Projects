
#include <thread>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <mutex>

#include "PokemonClient.h"
#include "GUIGame.h"
#include "GUIAccounting.h"
#include "GUIRoom.h"

#define IPADDR "127.0.0.1"
#define PORT 5768

int main (int argc, char *argv[])
{
	std::cout << "Pokemon Client\n";

	auto fnPrintPokemon = [] (
		std::ostream &os,
		const PokemonGame::Pokemon *pokemon)
	{
		os << "\t" << std::setw (10)
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

	auto fnPrintPlayer = [&fnPrintPokemon] (
		std::ostream &os,
		const PokemonGame::UserID &uid,
		const PokemonGame::Player &player)
	{
		os << "\t" << uid << ", "
			<< std::boolalpha << player.isReady << ", "
			<< player.x << ", "
			<< player.y << "\n";

		for (const auto &pokemon : player.pokemons)
		{
			os << "\t";
			fnPrintPokemon (os, pokemon.second.get ());
		}
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
			fnPrintPokemon (std::cout, pokemon.second.get ());
	};

	auto fnUsersPokemons = [&fnPrintPokemon] (
		PokemonGame::PokemonClient &client,
		const std::string &uid)
	{
		PokemonGame::PokemonsWithID result;
		std::cout << "Pokemons of User: " << uid << std::endl;
		if (client.UsersPokemons (uid, result))
			for (const auto &pokemon : result)
				fnPrintPokemon (std::cout, pokemon.second.get ());
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnPokemonAll = [&fnPrintPokemon] (
		PokemonGame::PokemonClient &client)
	{
		PokemonGame::PokemonsWithID result;
		std::cout << "All Pokemons: " << std::endl;
		if (client.PokemonAll (result))
			for (const auto &pokemon : result)
				fnPrintPokemon (std::cout, pokemon.second.get ());
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
		std::array<PokemonGame::PokemonID, 3>
			pokemonIds { PokemonGame::PokemonID () };

		auto index = 0;
		for (const auto &pokemon : client.MyPokemons ())
		{
			if (index == pokemonIds.size ()) break;
			pokemonIds[index++] = pokemon.first;
		}

		if (client.RoomEnter (roomId, pokemonIds))
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

	auto fnRoomReady = [&fnPrintPlayer] (
		PokemonGame::PokemonClient &client)
	{
		if (client.RoomReady () && client.RoomState ())
		{
			std::cout << "I'am :" << client.MyUID () << std::endl;
			std::cout << "Players in this Room:\n";
			for (const auto &player : client.GetPlayers ())
				fnPrintPlayer (std::cout, player.first, player.second);
		}
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	auto fnLockStep = [&fnPrintPlayer] (
		PokemonGame::PokemonClient &client,
		PokemonGame::Action &action)
	{
		if (client.Lockstep (action))
		{
			std::cout << "I'am :" << client.MyUID () << std::endl;
			std::cout << "Players' State:\n";
			for (const auto &player : client.GetPlayers ())
				fnPrintPlayer (std::cout, player.first, player.second);
		}
		else
			std::cerr << client.ErrMsg () << std::endl;
	};

	// GUI Logic

	auto guiLogin = [] (PokemonGame::PokemonClient &client)
	{
		auto hasLogin = false;
		PokemonGameGUI::AccountingWindow wnd;

		std::mutex mtx;

		wnd.OnLogin ([&wnd, &client, &mtx, &hasLogin] (
			const std::string &uid, const std::string &pwd)
		{
			std::lock_guard<std::mutex> lg (mtx);
			wnd.Prompt ("Loging in...");
			if (client.Login (uid, pwd))
			{
				wnd.Prompt ("Login Successfully");
				hasLogin = true;
			}
			else
				wnd.Prompt (client.ErrMsg ());
		});

		wnd.OnRegister ([&wnd, &client, &mtx] (
			const std::string &uid, const std::string &pwd)
		{
			std::lock_guard<std::mutex> lg (mtx);
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

	auto guiSelectRoom = [&fnPrintPlayer] (
		PokemonGame::PokemonClient &client)
	{
		auto hasEntered = false;
		auto isReady = false;
		auto isAllReady = false;
		PokemonGameGUI::RoomWindow wnd;

		std::array<PokemonGame::PokemonID, 3>
			pokemonIds { PokemonGame::PokemonID () };

		auto index = 0;
		for (const auto &pokemon : client.MyPokemons ())
		{
			if (index == pokemonIds.size ()) break;
			pokemonIds[index++] = pokemon.first;
		}

		std::mutex mtx;

		wnd.SetState (hasEntered, isReady);
		wnd.OnClickBtn (
			[&wnd, &client, &mtx, &hasEntered, &isReady, &pokemonIds] (
				const std::string &roomId)
		{
			std::lock_guard<std::mutex> lg (mtx);
			if (!hasEntered)
			{
				if (client.RoomEnter (roomId, pokemonIds))
				{
					wnd.SetState (hasEntered = true, isReady);
					wnd.Prompt ("You Entered " + roomId);
				}
				else
					wnd.Prompt (client.ErrMsg ());
			}
			else
			{
				if (client.RoomLeave ())
				{
					wnd.SetState (hasEntered = false, isReady = false);
					wnd.Prompt ("You Left the Room");
				}
				else
					wnd.Prompt (client.ErrMsg ());
			}
		});
		wnd.OnClickReadyBtn (
			[&wnd, &client, &mtx, &hasEntered, &isReady, &isAllReady] ()
		{
			std::lock_guard<std::mutex> lg (mtx);
			if (!hasEntered)
				return;

			if (client.RoomReady ())
			{
				if (!isReady)
				{
					wnd.SetState (hasEntered, isReady = true);
					wnd.Prompt ("You are Ready");
				}
				else
				{
					wnd.SetState (hasEntered, isAllReady = isReady = false);
					wnd.Prompt ("You are Unready");
				}
			}
			else
				wnd.Prompt (client.ErrMsg ());
		});

		while (!wnd.IsClosed () && !isAllReady)
		{
			auto tBeg = std::chrono::system_clock::now ();
			if (!hasEntered)
			{
				std::vector<std::string> roomList;
				auto isErr = false;

				// Lock Client
				{
					std::lock_guard<std::mutex> lg (mtx);
					isErr = !client.RoomQuery (roomList);
				}

				if (isErr)
					wnd.Prompt (client.ErrMsg ());
				wnd.SetRooms (std::move (roomList));
			}
			else
			{
				std::stringstream playerInfo;
				auto isErr = false;

				// Lock Client
				{
					std::lock_guard<std::mutex> lg (mtx);
					isErr = !client.RoomState ();

					if (!isErr)
					{
						const auto &players = client.GetPlayers ();

						isAllReady = true;
						for (const auto &player : players)
						{
							isAllReady = isAllReady && player.second.isReady;
							fnPrintPlayer (playerInfo, player.first, player.second);
						}
					}
				}

				if (isErr)
					wnd.Prompt (client.ErrMsg ());
				wnd.SetTexts (PokemonGame_Impl::SplitStr (playerInfo.str (), "\n"));
			}

			// Avoid Deadlock
			wnd.Refresh ();

			std::this_thread::sleep_for (
				std::chrono::milliseconds (200) -
				(std::chrono::system_clock::now () - tBeg)
			);
		}

		return isAllReady;
	};

	auto fnPlay = [] (PokemonGame::PokemonClient &client)
	{
		using ActionType = PokemonGame::ActionType;

		PokemonGameGUI::GameWindow wnd (PokemonGame::Player::maxX,
										PokemonGame::Player::maxY);

		auto &players = client.GetPlayers ();
		auto &actionQueue = client.GetActionQueue ();
		PokemonGame::GamePhysics gamePhysics (players, actionQueue);

		std::mutex mtx;

		// Key Input
		auto cA = 0, cD = 0, cW = 0, cS = 0;
		constexpr auto cMax = 100;

		// Mouse Input
		auto isMouseDown = false;
		auto mosX = 0, mosY = 0;

		// Position
		auto posX = 0, posY = 0;
		auto getPos = [&] ()
		{
			const auto &rect = gamePhysics.playersPhysics
				.at (client.MyUID ()).GetRect ();
			posX = wnd.XX (rect.x + rect.w / 2);
			posY = wnd.YY (rect.y + rect.h / 2);
		};
		getPos ();

		wnd.GetWindow ().OnLButtonDown ([&] (EggAche::Window *, int x, int y)
		{
			isMouseDown = true;
			mosX = x;
			mosY = y;
		});

		wnd.GetWindow ().OnLButtonUp ([&] (EggAche::Window *, int x, int y)
		{
			isMouseDown = false;
		});

		wnd.GetWindow ().OnMouseMove ([&] (EggAche::Window *, int x, int y)
		{
			mosX = x;
			mosY = y;
		});

		wnd.GetWindow ().OnKeyDown ([&] (EggAche::Window *, char ch)
		{
			switch (ch)
			{
			case 'A':
				cA++;
				break;
			case 'D':
				cD++;
				break;
			case 'W':
				cW++;
				break;
			case 'S':
				cS++;
				break;
			default:
				break;
			}

			if (cA) cA++;
			if (cD) cD++;
			if (cW) cW++;
			if (cS) cS++;

			if (cA > cMax) cA = cMax;
			if (cD > cMax) cD = cMax;
			if (cW > cMax) cW = cMax;
			if (cS > cMax) cS = cMax;
		});

		wnd.GetWindow ().OnKeyUp ([&] (EggAche::Window *, char ch)
		{
			switch (ch)
			{
			case 'A':
				cA = 0;
				break;
			case 'D':
				cD = 0;
				break;
			case 'W':
				cW = 0;
				break;
			case 'S':
				cS = 0;
				break;
			default:
				break;
			}
		});

		constexpr auto Lps = 10;
		constexpr auto Fps = 30;
		constexpr auto Fpl = (double) Fps / Lps;
		constexpr auto tSleep = std::chrono::milliseconds (1000) / Fps;

		auto cLock = 0;
		auto cFrame = 0;

		// Game Loop
		while (!wnd.IsClosed ())
		{
			auto tBeg = std::chrono::system_clock::now ();

			// Render
			wnd.Render (gamePhysics, (double) cFrame / Fpl);

			// Frame Control
			if (cFrame < Fpl)
				cFrame++;
			else
				cFrame = 0;

			auto tElapse = std::chrono::system_clock::now () - tBeg;
			if (tSleep > tElapse)
				std::this_thread::sleep_for (tSleep - tElapse);

			// Lockstep action
			if (cFrame == 0)
			{
				PokemonGame::Action action { ActionType::None };
				if (isMouseDown)
				{
					getPos ();
					action.type = ActionType::Move;
					action.x = mosX - posX;
					action.y = mosY - posY;
				}
				else if (cA || cD || cW || cS)
				{
					action.type = ActionType::Move;
					action.x = cD - cA;
					action.y = cS - cW;
				}
				else
				{
					action.type = ActionType::Move;
					action.x = action.y = 0;
				}

				client.Lockstep (action);
				gamePhysics.Update ();
			}
		}
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
	}

	PokemonGame::PokemonClient client_gui (IPADDR, PORT);

	if (true)
	{
		if (!guiLogin (client_gui))
		{
			PokemonGameGUI::MsgBox ("You haven't Login in GUI\n");
			return 0;
		}
	}

	// Test
	if (true)
	{
		// Simulate multi-players
		std::thread thd ([&] ()
		{
			getchar ();
			fnRoomReady (client2);
		});

		if (!guiSelectRoom (client_gui))
			PokemonGameGUI::MsgBox ("You haven't Entered a Room\n");

		if (thd.joinable ())
			thd.join ();

		fnRoomReady (client3);
		fnRoomReady (client2);
	}

	// Test
	if (true)
	{
		fnPlay (client_gui);
	}

	// Test
	if (true)
	{
		fnRoomLeave (client2);
		fnRoomLeave (client3);
		fnRoomLeave (client_gui);

		fnLogout (client2);
		fnLogout (client3);
		fnLogout (client_gui);
	}

	getchar ();
	return 0;
}