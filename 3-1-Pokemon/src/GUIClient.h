#ifndef POKEMON_GUI_CLIENT_H
#define POKEMON_GUI_CLIENT_H

#include <memory>
#include <mutex>
#include <thread>
#include <future>
#include <algorithm>
#include <cctype>

#include "EggAche/EggAche.h"

#include "Client.h"

// Fix for the pollution by <windows.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace PokemonGameGUI
{
	class GUIClient
	{
	private:
		struct Rect
		{
			size_t x, y;
			size_t w, h;
		};

		class Button
		{
		public:
			Button (EggAche::Canvas &canvas,
					const std::string &text,
					size_t x, size_t y,
					size_t w = 20, size_t h = 18)
				: _rect { x, y, w, h }
			{
				canvas.DrawTxt (_rect.x + _rect.w / 2,
								_rect.y + (_rect.h - 18) / 2,
								text.c_str ());

				_rect.w += canvas.GetTxtWidth (text.c_str ());
				canvas.DrawRdRt (_rect.x, _rect.y,
								 _rect.x + _rect.w,
								 _rect.y + _rect.h,
								 _rect.w / 4, _rect.h / 4);
			}

			bool TestClick (size_t x, size_t y)
			{
				return x >= _rect.x && y >= _rect.y &&
					x <= _rect.x + _rect.w &&
					y <= _rect.y + _rect.h;
			}

		private:
			Rect _rect;
		};

		class Input
		{
		public:
			Input (EggAche::Canvas &canvas,
				   const std::string &text,
				   const std::string &hintText,
				   bool isActivated,
				   size_t x, size_t y,
				   size_t w, size_t h = 18)
				: _rect { x, y, w + 4, h + 2 }
			{
				// Clear and Draw Bounder
				canvas.SetBrush (false, 255, 255, 255);
				canvas.DrawRect (_rect.x, _rect.y,
								 _rect.x + _rect.w,
								 _rect.y + _rect.h);
				canvas.SetBrush (true, 0, 0, 0);

				// Draw Text
				const auto &strToDraw =
					text.empty () && !isActivated ? hintText : text;
				canvas.DrawTxt (_rect.x + 2, _rect.y + 1,
								strToDraw.c_str ());

				// Draw Pipeline
				if (isActivated)
				{
					auto posX = _rect.x + 2 +
						canvas.GetTxtWidth (strToDraw.c_str ());
					canvas.DrawLine (posX, _rect.y + 2,
									 posX, _rect.y - 2 + _rect.h);
				}
			}

			bool TestClick (size_t x, size_t y)
			{
				return x >= _rect.x && y >= _rect.y &&
					x <= _rect.x + _rect.w &&
					y <= _rect.y + _rect.h;
			}

		private:
			Rect _rect;
		};

		static bool HandleInput (std::string &text, char ch)
		{
			if (ch != '\x08' && !isalnum (ch) && ch != '_')
				return false;

			if (ch == '\x08')
			{
				if (!text.empty ())
					text.pop_back ();
			}
			else if (text.size () < 16)
				text.push_back (ch);
			return true;
		}

	public:

		//
		// Login Window
		//

		static PokemonGame::UserModel LoginWindow (
			PokemonGame::Client &client,
			bool isPassiveOffline,
			size_t width = 480, size_t height = 360)
		{
			const std::string uidHintStr = "<Input your User ID>";
			const std::string pwdHintStr = "<Input your Password>";
			const std::string invalidInputPrompt =
				"Please Input '_' / 'a-z' / 'A-Z' / '0-9' ...";

			const std::string loginginPrompt = "Loging in...";
			const std::string registeringPrompt = "Registering...";
			const std::string logedinPrompt = "Login Successfully";

			EggAche::Window wnd (width, height, "Pokemon Game - Login");
			std::unique_ptr<EggAche::Canvas> bg;

			std::unique_ptr<Button> loginBtn, registerBtn;
			std::unique_ptr<Input> uidInput, pwdInput;
			bool uidInputActivated = false;
			bool pwdInputActivated = false;

			std::string uidStr, pwdStr;
			std::string promptStr = isPassiveOffline ?
				"Your Account is Passive Loged out..." :
				"Welcome to Pokemon Game";

			auto hasLogin = false;
			PokemonGame::UserModel ret;

			std::mutex mtx;

			auto refresh = [&] ()
			{
				bg = std::make_unique<EggAche::Canvas> (width, height);
				wnd.SetBackground (bg.get ());

				// Prompt
				bg->DrawTxt (width / 2 - bg->GetTxtWidth (promptStr.c_str ()) / 2,
							 height / 10 - 18 / 3,
							 promptStr.c_str ());

				// Input
				auto minInputWidth = bg->GetTxtWidth (pwdHintStr.c_str ());
				auto inputWidth = std::max (width * 4 / 10, minInputWidth);

				uidInput = std::make_unique<Input> (
					*bg, uidStr, uidHintStr, uidInputActivated,
					(width - inputWidth) / 2, height * 3 / 10, inputWidth);
				pwdInput = std::make_unique<Input> (
					*bg, pwdStr, pwdHintStr, pwdInputActivated,
					(width - inputWidth) / 2, height * 5 / 10, inputWidth);

				// Button
				loginBtn = std::make_unique<Button> (
					*bg, "Login",
					width * 3 / 10, height * 7 / 10);
				registerBtn = std::make_unique<Button> (
					*bg, "Register",
					width * 5 / 10, height * 7 / 10);

				// Refresh
				wnd.Refresh ();
			};

			refresh ();

			auto fnLogin = [&] ()
			{
				promptStr = loginginPrompt;
				refresh ();

				try
				{
					ret = client.Login (uidStr, pwdStr);
					hasLogin = true;
					promptStr = logedinPrompt;
				}
				catch (const std::exception &ex)
				{ promptStr = ex.what (); }
			};

			auto fnRegister = [&] ()
			{
				promptStr = registeringPrompt;
				refresh ();

				try
				{ promptStr = client.Register (uidStr, pwdStr); }
				catch (const std::exception &ex)
				{ promptStr = ex.what (); }
			};

			wnd.OnResized ([&] (EggAche::Window *, size_t x, size_t y)
			{
				std::lock_guard<std::mutex> lg (mtx);

				width = x;
				height = y;
				refresh ();
			});

			wnd.OnPress ([&] (EggAche::Window *, char ch)
			{
				std::lock_guard<std::mutex> lg (mtx);

				if (ch == '\t')
				{
					if ((!uidInputActivated && !pwdInputActivated) ||
						pwdInputActivated)
					{
						uidInputActivated = true;
						pwdInputActivated = false;
					}
					else
					{
						uidInputActivated = false;
						pwdInputActivated = true;
					}
				}
				else if (ch == '\r')
					fnLogin ();
				else if (ch == '\n')
					fnRegister ();
				else
				{
					bool isValid = true;
					if (uidInputActivated && !HandleInput (uidStr, ch))
						isValid = false;
					else if (pwdInputActivated && !HandleInput (pwdStr, ch))
						isValid = false;

					if (!isValid)
						promptStr = invalidInputPrompt;
				}

				refresh ();
			});

			wnd.OnClick ([&] (EggAche::Window *, size_t x, size_t y)
			{
				std::lock_guard<std::mutex> lg (mtx);

				uidInputActivated = false;
				pwdInputActivated = false;

				if (uidInput->TestClick (x, y))
					uidInputActivated = true;
				if (pwdInput->TestClick (x, y))
					pwdInputActivated = true;

				if (loginBtn->TestClick (x, y))
					fnLogin ();
				if (registerBtn->TestClick (x, y))
					fnRegister ();

				refresh ();
			});

			while (!wnd.IsClosed () && !hasLogin)
				std::this_thread::sleep_for (
					std::chrono::milliseconds (200));

			// Ensure callback function quit
			std::lock_guard<std::mutex> lg (mtx);

			if (!hasLogin)
				throw std::runtime_error ("Close Window without Login");

			return ret;
		}

		//
		// Room Window
		//

		static std::pair<
			std::pair<size_t, size_t>,
			std::vector<PokemonGame::RoomPlayer>
		> RoomWindow (
			PokemonGame::Client &client,
			const PokemonGame::UserModel &curUser,
			const PokemonGame::PokemonID &pidToPlay,
			size_t width = 640, size_t height = 640)
		{
			const std::string ridHintStr = "<Input Room ID or Select Below>";
			const std::string invalidInputPrompt =
				"Please Input '_' / 'a-z' / 'A-Z' / '0-9' ...";

			const std::string enteringPrompt = "Entering Room...";
			const std::string enteredPrompt = "Entered this Room";
			const std::string leavingPrompt = "Leaving Room...";
			const std::string readyPrompt = "You are Ready Now";
			const std::string unreadyPrompt = "You are Unready Now";

			constexpr auto tSleep = std::chrono::milliseconds (500);

			EggAche::Window wnd (width, height,
				(curUser.uid + " - Room").c_str ());
			std::unique_ptr<EggAche::Canvas> bg;

			std::vector<std::unique_ptr<Button>> roomBtns;
			std::unique_ptr<Button> enterBtn, readyBtn;
			std::unique_ptr<Input> ridInput;
			bool ridInputActivated = false;

			std::string ridStr;
			std::string promptStr = "Pick up a Room to Play";
			std::vector<std::string> playerList;

			auto hasEntered = false;
			auto isReady = false;

			auto hasStarted = false;
			std::vector<PokemonGame::RoomModel> roomModels;
			std::pair<
				std::pair<size_t, size_t>,
				std::vector<PokemonGame::RoomPlayer>
			> ret;

			std::mutex mtx;

			auto refresh = [&] ()
			{
				bg = std::make_unique<EggAche::Canvas> (width, height);
				wnd.SetBackground (bg.get ());

				// Prompt
				bg->DrawTxt (width / 2 - bg->GetTxtWidth (promptStr.c_str ()) / 2,
							 height / 10 - 18 / 3,
							 promptStr.c_str ());

				// Input
				auto minInputWidth = bg->GetTxtWidth (ridHintStr.c_str ());
				auto inputWidth = std::max (width * 4 / 10, minInputWidth);

				ridInput = std::make_unique<Input> (
					*bg, ridStr, ridHintStr, ridInputActivated,
					(width * 7 / 10 - inputWidth) / 2, height * 2 / 10, inputWidth);

				// Btn
				if (!hasEntered)
				{
					enterBtn = std::make_unique<Button> (
						*bg, "Enter",
						width * 7 / 10, height * 2 / 10);
				}
				else
				{
					enterBtn = std::make_unique<Button> (
						*bg, "Leave",
						width * 7 / 10, height * 2 / 10 - 18);

					if (!isReady)
						readyBtn = std::make_unique<Button> (
							*bg, "Ready",
							width * 7 / 10, height * 2 / 10 + 18);
					else
						readyBtn = std::make_unique<Button> (
							*bg, "Unready",
							width * 7 / 10, height * 2 / 10 + 18);
				}

				// List View
				if (!hasEntered)
				{
					auto index = 0;
					for (const auto &room : roomModels)
					{
						auto roomStr = room.rid + " - " +
							(room.isPending ? "Pending" : "In Game");
						roomBtns.emplace_back (std::make_unique<Button> (
							*bg, roomStr,
							10, height * 3 / 10 + index * 30,
							width - bg->GetTxtWidth (roomStr.c_str ()) - 20));
						index++;
					}
				}
				else
				{
					auto index = 0;
					for (const auto &player : playerList)
					{
						bg->DrawTxt ((width - bg->GetTxtWidth (player.c_str ())) / 2,
									 height * 3 / 10 + index * 30,
									 player.c_str ());
						index++;
					}
				}

				wnd.Refresh ();
			};

			refresh ();

			auto fnEnter = [&] ()
			{
				if (!hasEntered)
				{
					promptStr = enteringPrompt;
					refresh ();

					try
					{
						promptStr = enteredPrompt;
						ret.first = client.RoomEnter (ridStr, pidToPlay);
						hasEntered = true;
					}
					catch (const std::exception &ex)
					{
						if (std::string (ex.what ()) == PokemonGame::BadSession)
							throw;
						promptStr = ex.what ();
					}
				}
				else
				{
					promptStr = leavingPrompt;
					refresh ();

					try
					{
						promptStr = client.RoomLeave ();
						hasEntered = false;
					}
					catch (const std::exception &ex)
					{
						if (std::string (ex.what ()) == PokemonGame::BadSession)
							throw;
						promptStr = ex.what ();
					}
				}
			};

			auto fnReady = [&] ()
			{
				if (!hasEntered)
					return;

				isReady = !isReady;
				if (isReady)
					promptStr = readyPrompt;
				else
					promptStr = unreadyPrompt;
			};

			wnd.OnResized ([&] (EggAche::Window *, size_t x, size_t y)
			{
				std::lock_guard<std::mutex> lg (mtx);

				width = x;
				height = y;
				refresh ();
			});

			wnd.OnPress ([&] (EggAche::Window *, char ch)
			{
				std::lock_guard<std::mutex> lg (mtx);

				if (ch == '\t')
					ridInputActivated = true;
				else if (ch == '\r')
				{
					if (!hasEntered)
						fnEnter ();
					else
						fnReady ();
				}
				else if (ch == '\n')
				{
					if (hasEntered)
						fnEnter ();
				}
				else
				{
					bool isValid = true;
					if (ridInputActivated && !HandleInput (ridStr, ch))
						isValid = false;

					if (!isValid)
						promptStr = invalidInputPrompt;
				}

				refresh ();
			});

			wnd.OnClick ([&] (EggAche::Window *, size_t x, size_t y)
			{
				std::lock_guard<std::mutex> lg (mtx);

				ridInputActivated = false;
				if (ridInput->TestClick (x, y))
					ridInputActivated = true;

				if (enterBtn->TestClick (x, y))
					fnEnter ();
				if (readyBtn.get () != nullptr && readyBtn->TestClick (x, y))
					fnReady ();

				for (size_t i = 0; i < roomModels.size (); i++)
					if (roomBtns[i]->TestClick (x, y))
					{
						ridStr = (std::string) roomModels[i].rid;
						fnEnter ();
						break;
					}

				refresh ();
			});

			while (!wnd.IsClosed () && !hasStarted)
			{
				auto tBeg = std::chrono::system_clock::now ();

				{
					// Locking
					std::lock_guard<std::mutex> lg (mtx);

					if (!hasEntered)
					{
						try
						{
							roomModels.clear ();
							roomModels = client.Rooms ();
						}
						catch (const std::exception &ex)
						{
							if (std::string (ex.what ()) == PokemonGame::BadSession)
								throw;
							promptStr = ex.what ();
						}
					}
					else
					{
						try
						{
							hasStarted = true;
							ret.second = client.RoomReady (isReady);

							std::ostringstream oss;
							playerList.clear ();

							for (const auto &player : ret.second)
							{
								if (!player.isReady)
									hasStarted = false;

								playerList.emplace_back (player.uid + " " +
									(player.isReady ? "Ready" : "Unready"));

								const auto &pokemon = player.pokemon;
								oss.str ("");
								oss << pokemon.name
									<< " level: " << pokemon.level
									<< " exp: " << pokemon.expPoint
									<< " atk: " << pokemon.atk
									<< " def: " << pokemon.def
									<< " hp: " << pokemon.hp
									<< " timeGap: " << pokemon.timeGap;
								playerList.emplace_back (oss.str ());
							}

							if (ret.second.size () < 2)
								hasStarted = false;
						}
						catch (const std::exception &ex)
						{
							if (std::string (ex.what ()) == PokemonGame::BadSession)
								throw;

							hasStarted = false;
							promptStr = ex.what ();
						}
					}
					refresh ();
				}

				auto tElapse = std::chrono::system_clock::now () - tBeg;
				if (tSleep > tElapse)
					std::this_thread::sleep_for (tSleep - tElapse);
			}

			// Ensure callback function quit
			std::lock_guard<std::mutex> lg (mtx);

			if (!hasStarted)
				throw std::runtime_error ("Close Window without Start Game");

			return ret;
		}

		//
		// Game Window
		//

		static std::vector<PokemonGame::GameModel::ResultPlayer> GameWindow (
			PokemonGame::Client &client,
			const PokemonGame::UserModel &curUser,
			const std::vector<PokemonGame::RoomPlayer> &roomPlayers,
			size_t worldW, size_t worldH,
			size_t width = 640, size_t height = 480)
		{
			EggAche::Window wnd (width, height,
				(curUser.uid + " - Playing").c_str ());
			std::unique_ptr<EggAche::Canvas> bg;

			// Room Players
			std::unordered_map<
				PokemonGame::UserID,
				PokemonGame::RoomPlayer
			> roomPlayerMap;

			// Fetched Model
			PokemonGame::GameModel gameModel;

			// Task
			bool isTaskReady = false;
			std::future<PokemonGame::GameModel> task;

			// Error Prompt
			size_t promptTick;
			std::string promptStr;

			// Frame Control
			auto curFrame = 0;
			constexpr auto Fps = 30;
			constexpr auto Fpl = 10;
			constexpr auto tSleep = std::chrono::milliseconds (1000) / Fps;

			// Key Input
			auto cA = 0, cD = 0, cW = 0, cS = 0;
			constexpr auto cMax = 100;

			// Mouse Input
			auto isAtk = false;
			auto isDef = false;
			auto mosX = 0, mosY = 0;

			// Player Position
			size_t posX = 0, posY = 0;

			std::mutex mtx;

			// Init Room Players
			for (const auto &player : roomPlayers)
				roomPlayerMap.emplace (player.uid, player);

			auto fixRectToRender = [&] (Rect &rect)
			{
				auto scaleW = (double) width / worldW;
				auto scaleH = (double) height / worldH;

				rect.x = size_t (rect.x * scaleW);
				rect.w = size_t (rect.w * scaleW);
				rect.y = size_t (rect.y * scaleH);
				rect.h = size_t (rect.h * scaleH);
			};

			auto render = [&] ()
			{
				bg = std::make_unique<EggAche::Canvas> (width, height);
				wnd.SetBackground (bg.get ());

				// Render Prompt
				if (promptTick > 0)
				{
					bg->DrawTxt (0, 0, promptStr.c_str ());
					promptTick--;
				}

				auto fraction = (double) (curFrame % Fpl) / Fpl;

				// Render Players
				for (const auto &player : gameModel.players)
				{
					const auto &roomPlayer = roomPlayerMap[player.uid];
					Rect rect {
						(size_t) (player.x + fraction * player.vx),
						(size_t) (player.y + fraction * player.vy),
						roomPlayer.width, roomPlayer.height
					};
					fixRectToRender (rect);

					bg->DrawRect (rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
					bg->DrawTxt (rect.x, rect.y - 18, player.uid.c_str ());
					bg->DrawTxt (rect.x, rect.y,
						(roomPlayer.pokemon.name + " / " +
						 std::to_string (player.curHp)).c_str ());
				}

				// Render Damages
				for (const auto &damagePair : gameModel.damages)
				{
					const auto &damage = damagePair.second;
					Rect rect {
						(size_t) (damage.x + fraction * damage.vx),
						(size_t) (damage.y + fraction * damage.vy),
						10, 10
					};
					fixRectToRender (rect);

					bg->DrawElps (rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
				}

				wnd.Refresh ();
			};

			auto update = [&] ()
			{
				// Take a snapshot
				auto oldPlayers = gameModel.players;
				auto oldDamages = gameModel.damages;

				// Update Model
				gameModel = task.get ();

				// Fix Damage
				for (auto &damage : gameModel.damages)
				{
					if (oldDamages.find (damage.first) == oldDamages.end ())
						continue;

					auto newX = damage.second.x + damage.second.vx;
					auto newY = damage.second.y + damage.second.vy;

					damage.second.x = oldDamages[damage.first].x + oldDamages[damage.first].vx;
					damage.second.y = oldDamages[damage.first].y + oldDamages[damage.first].vy;
					damage.second.vx = newX - damage.second.x;
					damage.second.vy = newY - damage.second.y;
				}

				// Some Players Quit...
				if (oldPlayers.size () != gameModel.players.size ())
					return;

				// Fix Player
				for (size_t i = 0; i < oldPlayers.size (); ++i)
				{
					auto newX = gameModel.players[i].x + gameModel.players[i].vx;
					auto newY = gameModel.players[i].y + gameModel.players[i].vy;

					gameModel.players[i].x = oldPlayers[i].x + oldPlayers[i].vx;
					gameModel.players[i].y = oldPlayers[i].y + oldPlayers[i].vy;
					gameModel.players[i].vx = newX - gameModel.players[i].x;
					gameModel.players[i].vy = newY - gameModel.players[i].y;

					// Update Player Position
					if (gameModel.players[i].uid == curUser.uid)
					{
						posX = (gameModel.players[i].x + gameModel.players[i].vx) / 2;
						posY = (gameModel.players[i].y + gameModel.players[i].vy) / 2;
					}
				}
			};

			wnd.OnResized ([&] (EggAche::Window *, size_t x, size_t y)
			{
				std::lock_guard<std::mutex> lg (mtx);
				width = x; height = y;
			});

			// Move
			wnd.OnKeyDown ([&] (EggAche::Window *, char ch)
			{
				std::lock_guard<std::mutex> lg (mtx);
				switch (ch)
				{
				case 'A': cA++; break;
				case 'D': cD++; break;
				case 'W': cW++; break;
				case 'S': cS++; break;
				default: break;
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
			wnd.OnKeyUp ([&] (EggAche::Window *, char ch)
			{
				std::lock_guard<std::mutex> lg (mtx);
				switch (ch)
				{
				case 'A': cA = 0; break;
				case 'D': cD = 0; break;
				case 'W': cW = 0; break;
				case 'S': cS = 0; break;
				default: break;
				}
			});

			// Atk
			wnd.OnLButtonUp ([&] (EggAche::Window *, int x, int y)
			{
				std::lock_guard<std::mutex> lg (mtx);
				isAtk = true; mosX = x; mosY = y;
			});

			// Def
			wnd.OnRButtonDown ([&] (EggAche::Window *, int x, int y)
			{
				std::lock_guard<std::mutex> lg (mtx);
				isDef = true;
			});
			wnd.OnRButtonUp ([&] (EggAche::Window *, int x, int y)
			{
				std::lock_guard<std::mutex> lg (mtx);
				isDef = false;
			});

			// Game Loop
			while (!wnd.IsClosed () && !gameModel.isOver)
			{
				auto tBeg = std::chrono::system_clock::now ();

				// Frame Control
				if (curFrame < Fps) curFrame++;
				else curFrame = 0;

				// Sync
				if (curFrame % Fpl == Fpl / 2)
				{
					try
					{
						std::lock_guard<std::mutex> lg (mtx);

						auto atkx = 0, atky = 0;
						if (isAtk)
						{
							const auto &roomPlayer = roomPlayerMap[curUser.uid];
							Rect rect {
								posX, posY,
								roomPlayer.width, roomPlayer.height
							};
							fixRectToRender (rect);

							atkx = mosX - rect.x + rect.w / 2;
							atky = mosY - rect.y + rect.h / 2;

							// Todo
							std::cout << mosX << " " << mosY << std::endl;
							std::cout << rect.x << " " << rect.y << std::endl;

							isAtk = false;
						}

						task = std::async (std::launch::async, [=, &client] () {
							return client.GameSync (cD - cA, cS - cW, atkx, atky, isDef);
						});
					}
					catch (const std::exception &ex)
					{
						if (std::string (ex.what ()) == PokemonGame::BadSession)
							throw;
						promptStr = ex.what ();
						promptTick = Fps * 3;
					}
				}

				// Double Buffering
				if (isTaskReady && curFrame % Fpl == 0)
				{
					update ();
					isTaskReady = false;
				}

				// Render
				render ();

				auto tElapse = std::chrono::system_clock::now () - tBeg;
				if (tSleep > tElapse)
				{
					if (!isTaskReady && task.valid ())
					{
						auto waitResult = task.wait_for (tSleep - tElapse);
						isTaskReady = (waitResult == std::future_status::ready);

						// Sleep Again
						tElapse = std::chrono::system_clock::now () - tBeg;
						if (tSleep > tElapse)
							std::this_thread::sleep_for (tSleep - tElapse);
					}
					else
						std::this_thread::sleep_for (tSleep - tElapse);
				}
			}

			// Ensure callback function quit
			std::lock_guard<std::mutex> lg (mtx);

			if (!gameModel.isOver)
				throw std::runtime_error ("Close Window without Finish Game");

			return gameModel.results;
		}
	};
}

#endif // !POKEMON_GUI_CLIENT_H
