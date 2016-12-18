#ifndef POKEMON_GUI_CLIENT_H
#define POKEMON_GUI_CLIENT_H

#include <memory>
#include <mutex>
#include <thread>
#include <algorithm>

#include "EggAche/EggAche.h"

#include "PokemonClient.h"
#include "GUIShared.h"

namespace PokemonGameGUI
{
	class GUIClient
	{
	private:
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
			PokemonGame::PokemonClient &client,
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
			std::string promptStr = "Welcome to Pokemon Game";

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

		static std::vector<PokemonGame::RoomPlayer> RoomWindow (
			PokemonGame::PokemonClient &client,
			const PokemonGame::UserModel &curUser,
			const PokemonGame::PokemonID &pidToPlay,
			size_t width = 640, size_t height = 640)
		{
			const std::string ridHintStr = "<Input Room ID or Select Below>";
			const std::string invalidInputPrompt =
				"Please Input '_' / 'a-z' / 'A-Z' / '0-9' ...";

			const std::string enteringPrompt = "Entering Room...";
			const std::string leavingPrompt = "Leaving Room...";
			const std::string readyPrompt = "You are Ready Now";
			const std::string unreadyPrompt = "You are Unready Now";

			EggAche::Window wnd (width, height,
				(curUser.uid + " - Room").c_str ());
			std::unique_ptr<EggAche::Canvas> bg;

			std::vector<std::unique_ptr<Button>> roomBtns;
			std::unique_ptr<Button> enterBtn, readyBtn;
			std::unique_ptr<Input> ridInput;
			bool ridInputActivated = false;

			std::string ridStr;
			std::string promptStr = "Pick up a Room to Play";
			std::vector<std::string> roomList;
			std::vector<std::string> playerList;

			auto hasEntered = false;
			auto isReady = false;

			auto hasStart = false;
			std::vector<PokemonGame::RoomPlayer> ret;

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
					for (const auto &roomId : roomList)
					{
						roomBtns.emplace_back (std::make_unique<Button> (
							*bg, roomId,
							10, height * 3 / 10 + index * 30,
							width - bg->GetTxtWidth (roomId.c_str ()) - 20));
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
						promptStr = client.RoomEnter (ridStr, pidToPlay);
						hasEntered = true;
					}
					catch (const std::exception &ex)
					{ promptStr = ex.what (); }
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
					{ promptStr = ex.what (); }
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

				for (size_t i = 0; i < roomList.size (); i++)
					if (roomBtns[i]->TestClick (x, y))
					{
						ridStr = (std::string) roomList[i];
						fnEnter ();
						break;
					}

				refresh ();
			});

			while (!wnd.IsClosed () && !hasStart)
			{
				auto tBeg = std::chrono::system_clock::now ();

				{
					// Locking
					std::lock_guard<std::mutex> lg (mtx);

					if (!hasEntered)
					{
						try
						{ roomList = client.Rooms (); }
						catch (const std::exception &ex)
						{ promptStr = ex.what (); }
					}
					else
					{
						try
						{
							hasStart = true;
							ret = client.RoomReady (isReady);

							std::ostringstream oss;
							playerList.clear ();

							for (const auto &player : ret)
							{
								if (!player.isReady)
									hasStart = false;
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
						}
						catch (const std::exception &ex)
						{
							hasStart = false;
							promptStr = ex.what ();
						}
					}
					refresh ();
				}

				std::this_thread::sleep_for (
					std::chrono::milliseconds (500) -
					(std::chrono::system_clock::now () - tBeg)
				);
			}

			// Ensure callback function quit
			std::lock_guard<std::mutex> lg (mtx);

			if (!hasStart)
				throw std::runtime_error ("Close Window without Start Game");

			return ret;
		}
	};
}

#endif // !POKEMON_GUI_CLIENT_H
