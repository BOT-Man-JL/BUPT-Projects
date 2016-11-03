#ifndef POKEMON_GUI_GAME_H

#include <memory>
#include <thread>

#include "EggAche\EggAche.h"
#include "Pokemon.h"

#define AssestPath "Assests/"

namespace PokemonGameGUI
{
	class GameWindow
	{
	public:
		GameWindow (size_t width = 640, size_t height = 480)
			: _width (width), _height (height),
			_wnd (width, height, "Pokemon Game")
		{
			SetBg ();

			_wnd.OnResized ([this] (EggAche::Window *, size_t x, size_t y)
			{
				_width = x;
				_height = y;
				SetBg ();
			});
		}

		void Join ()
		{
			while (!_wnd.IsClosed ())
			{
				std::this_thread::sleep_for (std::chrono::seconds (1));
			}
		}

	private:
		EggAche::Window _wnd;
		std::unique_ptr<EggAche::Canvas> _bg;
		size_t _width, _height;

		void SetBg ()
		{
			_bg = std::make_unique<EggAche::Canvas> (_width, _height);
			_wnd.SetBackground (_bg.get ());
			_bg->DrawImg (AssestPath "bg.bmp", 0, 0, _width, _height);
			_wnd.Refresh ();
		}
	};

	class PokemonGUI
	{
	public:
		PokemonGUI (PokemonGame::Pokemon &pokemon,
					size_t width = 50, size_t height = 50)
			: _pokemon (pokemon)
		{
			_canvas =
				std::make_unique<EggAche::Canvas> (width, height);
			auto imgName = AssestPath + pokemon.GetName ();
		}

	private:
		PokemonGame::Pokemon &_pokemon;
		std::unique_ptr<EggAche::Canvas> _canvas;
	};
}

#endif // !POKEMON_GUI_GAME_H
