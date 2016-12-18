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

			if (ch == '\x08' && !text.empty ())
				text.pop_back ();
			else if (text.size () < 16)
				text.push_back (ch);
			return true;
		}

	public:
		static PokemonGame::UserModel LoginWindow (
			PokemonGame::PokemonClient &client,
			size_t width = 480, size_t height = 360)
		{
			const std::string uidHintStr = "<Input your User ID>";
			const std::string pwdHintStr = "<Input your Password>";
			const std::string invalidInputPrompt =
				"Please Input '_' / 'a-z' / 'A-Z' / '0-9' ...";

			const std::string loginginPrompt = "Loging in...";
			const std::string logedinPrompt = "Login Successfully";

			const std::string registeringPrompt = "Registering...";
			const std::string registeredPrompt = "Registered Successfully";

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
				static const auto minInputWidth = bg->GetTxtWidth (pwdHintStr.c_str ());
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

				refresh ();
			};

			auto fnRegister = [&] ()
			{
				promptStr = registeringPrompt;
				refresh ();

				try
				{
					promptStr = client.Register (uidStr, pwdStr);
				}
				catch (const std::exception &ex)
				{ promptStr = ex.what (); }

				refresh ();
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
	};
}

#endif // !POKEMON_GUI_CLIENT_H
