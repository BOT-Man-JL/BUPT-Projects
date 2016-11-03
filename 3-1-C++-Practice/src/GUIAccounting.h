#ifndef POKEMON_GUI_ACCOUNTING_H

#include <memory>
#include <thread>
#include <functional>

#include "EggAche\EggAche.h"
#include "GUIShared.h"

#define AssestPath "Assests/"

namespace PokemonGameGUI
{
	class AccountingWindow
	{
	public:
		AccountingWindow (size_t width = 480, size_t height = 360)
			: _width (width), _height (height),
			_wnd (width, height, "Pokemon Game")
		{
			InitLayout ();
			Prompt ("Welcome to Pokemon Game");
			_wnd.Refresh ();

			_wnd.OnResized ([this] (EggAche::Window *, size_t x, size_t y)
			{
				_width = x;
				_height = y;
				InitLayout ();
			});

			_wnd.OnPress ([this] (EggAche::Window *, char ch)
			{
				if (ch == '\t')
				{
					if ((
							!_uidInput->isActivated () &&
							!_pwdInput->isActivated ()
						) || _pwdInput->isActivated ())
					{
						_uidInput->Activate (true);
						_pwdInput->Activate (false);
					}
					else
					{
						_uidInput->Activate (false);
						_pwdInput->Activate (true);
					}
				}
				else if (ch == '\r')
				{
					if (_onLogin)
						_onLogin (_uidInput->GetText (), _pwdInput->GetText ());
				}
				else if (ch == '\n')
				{
					if (_onRegister)
						_onRegister (_uidInput->GetText (), _pwdInput->GetText ());
				}
				else if (ch != '\x08' && !isalnum (ch) && ch != '_')
					Prompt ("Please Input _/Alpha/Number...");
				else
				{
					_uidInput->InputKey (ch);
					_pwdInput->InputKey (ch);
				}

				_wnd.Refresh ();
			});

			_wnd.OnClick ([this] (EggAche::Window *, size_t x, size_t y)
			{
				_uidInput->Activate (false);
				_pwdInput->Activate (false);

				if (_uidInput->TestClick (x, y))
					_uidInput->Activate (true);
				if (_pwdInput->TestClick (x, y))
					_pwdInput->Activate (true);

				if (_loginBtn->TestClick (x, y) && _onLogin)
					_onLogin (_uidInput->GetText (), _pwdInput->GetText ());

				if (_registerBtn->TestClick (x, y) && _onRegister)
					_onRegister (_uidInput->GetText (), _pwdInput->GetText ());

				_wnd.Refresh ();
			});
		}

		using UidPwdCallback =
			std::function<void (const std::string &, const std::string &)>;

		void OnLogin (UidPwdCallback fn)
		{
			_onLogin = std::move (fn);
		}

		void OnRegister (UidPwdCallback fn)
		{
			_onRegister = std::move (fn);
		}

		void Prompt (const std::string &prompt)
		{
			if (_prompt != nullptr)
			{
				_prompt->Clear ();
				_prompt->DrawTxt (0, 0, prompt.c_str ());
			}
		}

		bool IsClosed () const
		{
			return _wnd.IsClosed ();
		}

	private:
		EggAche::Window _wnd;
		std::unique_ptr<EggAche::Canvas> _bg, _prompt;
		size_t _width, _height;

		UidPwdCallback _onLogin, _onRegister;
		std::string _strUid, _strPwd;

		std::unique_ptr<Button> _loginBtn, _registerBtn;
		std::unique_ptr<Input> _uidInput, _pwdInput;

		void InitLayout ()
		{
			_bg = std::make_unique<EggAche::Canvas> (_width, _height);
			_wnd.SetBackground (_bg.get ());

			if (_prompt.get () != nullptr)
				(*_bg) -= _prompt.get ();
			_prompt = std::make_unique<EggAche::Canvas> (
				_width / 2, _height / 2,
				_width * 3 / 10, _height * 2 / 10);
			(*_bg) += _prompt.get ();

			// Button
			_loginBtn = std::make_unique<Button> (
				*_bg, "Login",
				_width * 3 / 10, _height * 7 / 10);
			_registerBtn = std::make_unique<Button> (
				*_bg, "Register",
				_width * 5 / 10, _height * 7 / 10);

			// Input
			auto inputWidth = _width * 4 / 10;
			_uidInput = std::make_unique<Input> (
				*_bg, _strUid, "<Input your User ID>",
				_width * 3 / 10, _height * 3 / 10, inputWidth);
			_pwdInput = std::make_unique<Input> (
				*_bg, _strPwd, "<Input your Password>",
				_width * 3 / 10, _height * 5 / 10, inputWidth);

			_wnd.Refresh ();
		}
	};
}

#endif // !POKEMON_GUI_ACCOUNTING_H
