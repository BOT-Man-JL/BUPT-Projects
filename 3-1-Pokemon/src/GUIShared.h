#ifndef POKEMON_GUI_SHARED_H
#define POKEMON_GUI_SHARED_H

#include "EggAche\EggAche.h"

namespace PokemonGameGUI
{
	namespace PokemonGameGUI_Impl
	{
		template <typename T>
		T Min (const T &a, const T &b)
		{
			return a < b ? a : b;
		}

		template <typename T>
		T Max (const T &a, const T &b)
		{
			return a > b ? a : b;
		}

		struct Rect
		{
			size_t x, y;
			size_t w, h;
		};
	}

	void MsgBox (const char *str)
	{
		EggAche::MsgBox (str, "Pokemon Game");
	}

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
		PokemonGameGUI_Impl::Rect _rect;
	};

	class Input
	{
	public:
		Input (EggAche::Canvas &canvas,
			   const std::string &text,
			   const std::string &hintText,
			   size_t x, size_t y,
			   size_t w, size_t h = 18)
			: _canvas (canvas), _rect { x, y, w + 4, h + 2 },
			_text (text), _hintText (hintText), _isActivated (false)
		{
			ReDraw ();
		}

		bool TestClick (size_t x, size_t y)
		{
			return x >= _rect.x && y >= _rect.y &&
				x <= _rect.x + _rect.w &&
				y <= _rect.y + _rect.h;
		}

		bool isActivated () const
		{
			return _isActivated;
		}

		void Activate (bool isActivated = true)
		{
			_isActivated = isActivated;
			ReDraw ();
		}

		const std::string &GetText () const
		{
			return _text;
		}

		void InputKey (char ch)
		{
			if (!_isActivated)
				return;

			if (ch == '\x08' && !_text.empty ())
				_text.pop_back ();
			else if ((isalnum (ch) || ch == '_') && _text.size () < 16)
				_text.push_back (ch);

			ReDraw ();
		}

	private:
		EggAche::Canvas &_canvas;
		PokemonGameGUI_Impl::Rect _rect;
		std::string _hintText;
		bool _isActivated;
		std::string _text;

		void ReDraw ()
		{
			// Clear and Draw Bounder
			_canvas.SetBrush (false, 255, 255, 255);
			_canvas.DrawRect (_rect.x, _rect.y,
							  _rect.x + _rect.w,
							  _rect.y + _rect.h);
			_canvas.SetBrush (true, 0, 0, 0);

			// Draw Text
			const auto &strToDraw =
				_text.empty () && !_isActivated ? _hintText : _text;
			_canvas.DrawTxt (_rect.x + 2, _rect.y + 1,
							 strToDraw.c_str ());

			// Draw Pipeline
			if (_isActivated)
			{
				auto posX = _rect.x + 2 +
					_canvas.GetTxtWidth (strToDraw.c_str ());
				_canvas.DrawLine (posX, _rect.y + 2,
								  posX, _rect.y - 2 + _rect.h);
			}
		}
	};

}

#endif // !POKEMON_GUI_SHARED_H
