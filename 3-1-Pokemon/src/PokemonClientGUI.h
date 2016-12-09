#ifndef POKEMON_CLIENT_GUI_H
#define POKEMON_CLIENT_GUI_H

#include "PokemonClient.h"
#include "GUIGame.h"
#include "GUIAccounting.h"
#include "GUIRoom.h"

namespace PokemonGameGUI
{
	class PokemonClientGUI
	{
	public:
		PokemonClientGUI (const std::string &ipAddr,
						  unsigned short port)
			: client (ipAddr, port)
		{}

		bool Login ()
		{
			auto hasLogin = false;
			PokemonGameGUI::AccountingWindow wnd;

			std::mutex mtx;

			wnd.OnLogin ([&wnd, this, &mtx, &hasLogin] (
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

			wnd.OnRegister ([&wnd, this, &mtx] (
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

			return _isLogin = hasLogin;
		}

		bool SelectRoom ()
		{
			auto hasEntered = false;
			auto isReady = false;
			auto isAllReady = false;
			PokemonGameGUI::RoomWindow wnd (
				client.MyUID () + " - Pick up a Room");

			auto pid = client.MyPokemons ().begin ()->first;

			std::mutex mtx;

			wnd.SetState (hasEntered, isReady);
			wnd.OnClickBtn (
				[&wnd, this, &mtx, &hasEntered, &isReady, pid] (
					const std::string &roomId)
			{
				std::lock_guard<std::mutex> lg (mtx);
				if (!hasEntered)
				{
					if (client.RoomEnter (roomId, pid))
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
				[&wnd, this, &mtx, &hasEntered, &isReady, &isAllReady] ()
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
								PrintPlayer (playerInfo, player.first, player.second);
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

			return _isInGame = isAllReady;
		};

		void Play ()
		{
			using ActionType = PokemonGame::ActionType;

			PokemonGameGUI::GameWindow wnd (PokemonGame::Player::maxX,
											PokemonGame::Player::maxY,
											client.MyUID () + " - In Game :-)");

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
				std::lock_guard<std::mutex> lg { mtx };
				isMouseDown = true;
				mosX = x;
				mosY = y;
			});

			wnd.GetWindow ().OnLButtonUp ([&] (EggAche::Window *, int x, int y)
			{
				std::lock_guard<std::mutex> lg { mtx };
				isMouseDown = false;
			});

			wnd.GetWindow ().OnMouseMove ([&] (EggAche::Window *, int x, int y)
			{
				std::lock_guard<std::mutex> lg { mtx };
				mosX = x;
				mosY = y;
			});

			wnd.GetWindow ().OnKeyDown ([&] (EggAche::Window *, char ch)
			{
				std::lock_guard<std::mutex> lg { mtx };
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
				std::lock_guard<std::mutex> lg { mtx };
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
					std::lock_guard<std::mutex> lg { mtx };
					PokemonGame::Action action { ActionType::Move, 0, 0 };
					if (isMouseDown)
					{
						getPos ();
						action.x = mosX - posX;
						action.y = mosY - posY;
					}
					else if (cA || cD || cW || cS)
					{
						action.x = cD - cA;
						action.y = cS - cW;
					}

					client.Lockstep (action);
					gamePhysics.Update ();
				}
			}
		};

		~PokemonClientGUI ()
		{
			if (_isInGame && !client.RoomLeave ())
				EggAche::MsgBox ("Error occured at Leaving Room");
			if (_isLogin && !client.Logout ())
				EggAche::MsgBox ("Error occured at Logout");
		}

	private:
		PokemonGame::PokemonClient client;
		bool _isLogin, _isInGame;
	};
}

#endif // !POKEMON_CLIENT_GUI_H