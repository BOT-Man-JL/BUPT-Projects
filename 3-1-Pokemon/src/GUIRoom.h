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
		RoomWindow (const std::string &title,
					size_t width = 640, size_t height = 640)
			: _width (width), _height (height),
			_isEntered (false), _isReady (false),
			_promptStr ("Enter a Room to Play"),
			_wnd (width, height, title.c_str ())
		{
			InitLayout ();

			_wnd.OnResized ([this] (EggAche::Window *, size_t x, size_t y)
			{
				_width = x;
				_height = y;

				// Thread-unsafe
				//InitLayout ();
			});

			_wnd.OnPress ([this] (EggAche::Window *, char ch)
			{
				std::lock_guard<std::mutex> lg (mtx);

				if (ch == '\r')
				{
					if (!_isEntered)
					{
						// Enter
						if (_onClickBtn)
							_onClickBtn (_roomSelected);
						// Todo
					}
					else
					{
						// Ready
						if (_onClickReadyBtn)
							_onClickReadyBtn ();
					}
				}
				else if (ch == '\n')
				{
					if (_isEntered)
					{
						// Leave
						if (_onClickBtn)
							_onClickBtn (_roomSelected);
					}
				}
				else if (ch == '\t')
					_idInput->Activate (true);
				else if (ch != '\x08' && !isalnum (ch) && ch != '_')
					_promptStr = "Please Input _/Alpha/Number...";
				else
				{
					_idInput->InputKey (ch);
					_roomSelected = _idInput->GetText ();
				}

				InitLayout ();
			});

			_wnd.OnClick ([this] (EggAche::Window *, size_t x, size_t y)
			{
				std::lock_guard<std::mutex> lg (mtx);

				_idInput->Activate (false);
				if (_idInput->TestClick (x, y))
					_idInput->Activate (true);

				if (_enterBtn->TestClick (x, y) && _onClickBtn)
					_onClickBtn (_roomSelected);
				if (_readyBtn.get () != nullptr &&
					_readyBtn->TestClick (x, y) && _onClickReadyBtn)
					_onClickReadyBtn ();

				for (size_t i = 0; i < _roomList.size (); i++)
					if (_roomBtns[i]->TestClick (x, y))
					{
						_roomSelected = _roomList[i];
						_onClickBtn (_roomSelected);
						break;
					}

				InitLayout ();
			});
		}

		using SelectCallback =
			std::function<void (const std::string &)>;
		using SignalCallback =
			std::function<void ()>;

		void OnClickReadyBtn (SignalCallback fn)
		{
			_onClickReadyBtn = std::move (fn);
		}

		void OnClickBtn (SelectCallback fn)
		{
			_onClickBtn = std::move (fn);
		}

		bool IsClosed () const
		{
			return _wnd.IsClosed ();
		}

		void SetState (bool isEntered, bool isReady)
		{
			_isEntered = isEntered;
			_isReady = isReady;
		}

		void Prompt (std::string prompt)
		{
			// Todo
			//std::lock_guard<std::mutex> lg (mtx);
			_promptStr = std::move (prompt);
		}

		void SetRooms (std::vector<std::string> roomList)
		{
			std::lock_guard<std::mutex> lg (mtx);
			_roomList = std::move (roomList);
			_playerList.clear ();
		}

		void SetTexts (std::vector<std::string> textList)
		{
			std::lock_guard<std::mutex> lg (mtx);
			_playerList = std::move (textList);
			_roomList.clear ();
		}

		void Refresh ()
		{
			std::lock_guard<std::mutex> lg (mtx);
			InitLayout ();
		}

	private:
		EggAche::Window _wnd;
		std::unique_ptr<EggAche::Canvas> _bg;
		size_t _width, _height;

		SelectCallback _onClickBtn;
		SignalCallback _onClickReadyBtn;

		std::string _promptStr;
		std::string _roomSelected;
		bool _isEntered, _isReady;

		// For Room Selection
		std::vector<std::string> _roomList;

		// For User States
		std::vector<std::string> _playerList;

		std::vector<std::unique_ptr<Button>> _roomBtns;
		std::unique_ptr<Button> _enterBtn, _readyBtn;
		std::unique_ptr<Input> _idInput;

		// Sync
		std::mutex mtx;

		void InitLayout ()
		{
			_bg = std::make_unique<EggAche::Canvas> (_width, _height);
			_wnd.SetBackground (_bg.get ());

			// Prompt
			_bg->DrawTxt (
				(_width - _bg->GetTxtWidth (_promptStr.c_str ())) / 2,
				_height / 10,
				_promptStr.c_str ());

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
			if (!_isEntered)
			{
				_enterBtn = std::make_unique<Button> (
					*_bg, "Enter",
					_width * 7 / 10, _height * 2 / 10);
			}
			else
			{
				_enterBtn = std::make_unique<Button> (
					*_bg, "Leave",
					_width * 7 / 10, _height * 2 / 10 - 18);

				if (!_isReady)
					_readyBtn = std::make_unique<Button> (
						*_bg, "Ready",
						_width * 7 / 10, _height * 2 / 10 + 18);
				else
					_readyBtn = std::make_unique<Button> (
						*_bg, "Unready",
						_width * 7 / 10, _height * 2 / 10 + 18);
			}

			// Room List
			if (!_roomList.empty ())
			{
				auto index = 0;
				for (const auto &roomId : _roomList)
				{
					_roomBtns.emplace_back (std::make_unique<Button> (
						*_bg, roomId,
						10, _height * 3 / 10 + index * 30,
						_width - _bg->GetTxtWidth (roomId.c_str ()) - 20));
					index++;
				}
			}
			else if (!_playerList.empty ())
			{
				auto index = 0;
				for (const auto &playerInfo : _playerList)
				{
					_bg->DrawTxt ((_width - _bg->GetTxtWidth (playerInfo.c_str ())) / 2,
								  _height * 3 / 10 + index * 30,
								  playerInfo.c_str ());
					index++;
				}
			}

			_wnd.Refresh ();
		}
	};
}

#endif // !POKEMON_GUI_ROOM_H
