#ifndef POKEMON_GUI_SHARED_H
#define POKEMON_GUI_SHARED_H

#include <string>
#include <cctype>
#include "EggAche/EggAche.h"

// Fix for the pollution by <windows.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace PokemonGameGUI
{
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

}

#endif // !POKEMON_GUI_SHARED_H
