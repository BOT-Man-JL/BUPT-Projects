#ifndef POKEMON_GUI_ROOM_H
#define POKEMON_GUI_ROOM_H

#include <cctype>
#include <vector>
#include <memory>
#include <thread>
#include <functional>
#include <mutex>

#include "EggAche\EggAche.h"
#include "GUIShared.h"
#include "Shared.h"

#define AssestPath "Assests/"

namespace PokemonGameGUI
{
	class RoomWindow
	{
	public:
		using Players = std::vector<PokemonGame::Player>;

		RoomWindow (size_t width = 480, size_t height = 360)
			: _width (width), _height (height),
			_wnd (width, height, "Pokemon Room")
		{
			InitLayout ();

			_wnd.OnResized ([this] (EggAche::Window *, size_t x, size_t y)
			{
				_width = x;
				_height = y;
				InitLayout ();
			});

			_wnd.OnPress ([this] (EggAche::Window *, char ch)
			{
				if (ch == '\r')
				{
					if (_onSelectRoom)
						_onSelectRoom (_roomSelected);
				}
				else if (ch != '\x08' && !isalnum (ch) && ch != '_')
					Prompt ("Please Input _/Alpha/Number...");
				else
				{
					_idInput->InputKey (ch);
					_roomSelected = _idInput->GetText ();
				}

				_wnd.Refresh ();
			});

			_wnd.OnClick ([this] (EggAche::Window *, size_t x, size_t y)
			{
				_idInput->Activate (false);
				if (_idInput->TestClick (x, y))
					_idInput->Activate (true);

				if (_enterBtn->TestClick (x, y) && _onSelectRoom)
					_onSelectRoom (_roomSelected);

				for (size_t i = 0; i < _roomList.size (); i++)
					if (_roomBtns[i]->TestClick (x, y))
					{
						_roomSelected = _roomList[i];
						_onSelectRoom (_roomSelected);
						break;
					}

				_wnd.Refresh ();
			});
		}

		using SelectCallback =
			std::function<void (const std::string &)>;

		void OnSelectRoom (SelectCallback fn)
		{
			_onSelectRoom = std::move (fn);
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

		void SetRooms (std::vector<std::string> roomList)
		{
			_roomList = std::move (roomList);
			InitLayout ();
		}

		void SetPlayers (const Players &players)
		{
			// Todo
		}

	private:
		EggAche::Window _wnd;
		std::unique_ptr<EggAche::Canvas> _bg, _prompt;
		size_t _width, _height;

		SelectCallback _onSelectRoom;
		std::string _roomSelected;
		std::vector<std::string> _roomList;

		std::vector<std::unique_ptr<Button>> _roomBtns;
		std::unique_ptr<Button> _enterBtn;
		std::unique_ptr<Input> _idInput;

		void InitLayout ()
		{
			static std::mutex mtx;
			std::lock_guard<std::mutex> lg (mtx);

			_bg = std::make_unique<EggAche::Canvas> (_width, _height);
			_wnd.SetBackground (_bg.get ());

			// Prompt
			if (_prompt.get () != nullptr)
				(*_bg) -= _prompt.get ();
			_prompt = std::make_unique<EggAche::Canvas> (
				_width / 2, _height / 2,
				_width * 3 / 10, _height * 1 / 10);
			(*_bg) += _prompt.get ();
			Prompt ("Enter a Room to Play");

			// Input
			auto isFocused = false;
			if (_idInput.get () != nullptr)
				isFocused = _idInput->isActivated ();

			using namespace PokemonGameGUI_Impl;
			static const auto minInputWidth =
				_bg->GetTxtWidth ("<Input Room ID>");
			auto inputWidth = Max (_width * 5 / 10, minInputWidth);

			_idInput = std::make_unique<Input> (
				*_bg, _roomSelected, "<Input Room ID>",
				_width * 1 / 10, _height * 2 / 10, inputWidth);
			_idInput->Activate (isFocused);

			// Btn
			_enterBtn = std::make_unique<Button> (
				*_bg, "Enter",
				_width * 7 / 10, _height * 2 / 10);

			auto index = 0;
			for (const auto &roomId : _roomList)
			{
				_roomBtns.emplace_back (std::make_unique<Button> (
					*_bg, roomId,
					_width * 3 / 10,
					_height * 3 / 10 + index * 30));
			}

			_wnd.Refresh ();
		}
	};
}

#endif // !POKEMON_GUI_ROOM_H
