#ifndef POKEMON_GUI_GAME_H
#define POKEMON_GUI_GAME_H

#include <memory>
#include <thread>

#include "EggAche\EggAche.h"
#include "Pokemon.h"
#include "PokemonPhysics.h"

#define AssestPath "Assests/"

namespace PokemonGameGUI
{
	class GameWindow
	{
	public:
		GameWindow (size_t worldW, size_t worldH,
					const std::string &title,
					size_t width = 640, size_t height = 480)
			: _worldW (worldW), _worldH (worldH),
			_width (width), _height (height),
			_wnd (width, height, title.c_str ())
		{
			_wnd.OnResized ([this] (EggAche::Window *, size_t x, size_t y)
			{
				_width = x;
				_height = y;
			});

			// Set Bg
			SetBg ();
			_wnd.Refresh ();
		}

		void Render (
			const PokemonGame::GamePhysics &gamePhysics,
			double animationFraction)
		{
			// Get Actual Position to Render
			auto getRect = [&animationFraction, this] (
				const PokemonGame::Physics &physics)
			{
				auto rect = physics.GetRect ();
				const auto &velocity = physics.GetVelocity ();
				rect.x += (int) (animationFraction * std::get<0> (velocity));
				rect.y += (int) (animationFraction * std::get<1> (velocity));

				rect.x = XX (rect.x);
				rect.y = YY (rect.y);
				rect.w = XX (rect.w);
				rect.h = YY (rect.h);
				return std::move (rect);
			};

			// Update Bg
			SetBg ();

			// Render Players
			for (const auto &playerPair : gamePhysics.playersPhysics)
			{
				const auto &playerUid = playerPair.first;
				const auto &player = playerPair.second;
				const auto &pokemon = *player.GetPlayer ().pokemon;

				//auto imgName = AssestPath + pokemon.GetName ();

				auto rect = getRect (player);
				_bg->DrawRect (rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
				_bg->DrawTxt (rect.x, rect.y - 18, playerUid.c_str ());
				_bg->DrawTxt (rect.x, rect.y, (pokemon.GetName () + " / " +
					 std::to_string (pokemon.GetCurHP ())).c_str ());
			}

			// Render Damages
			for (const auto &damage : gamePhysics.damagesPhysics)
			{
				auto rect = getRect (damage);
				_bg->DrawElps (rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
			}

			// Refresh
			_wnd.Refresh ();
		}

		int XX (int x)
		{
			return x * _width / _worldW;
		}

		int YY (int y)
		{
			return y * _height / _worldH;
		}

		EggAche::Window &GetWindow ()
		{
			return _wnd;
		}

		bool IsClosed () const
		{
			return _wnd.IsClosed ();
		}

	private:
		EggAche::Window _wnd;
		std::unique_ptr<EggAche::Canvas> _bg;
		size_t _width, _height;
		size_t _worldW, _worldH;

		void SetBg ()
		{
			_bg = std::make_unique<EggAche::Canvas> (_width, _height);
			_wnd.SetBackground (_bg.get ());
			_bg->DrawImg (AssestPath "bg.bmp", 0, 0, _width, _height);
		}
	};
}

#endif // !POKEMON_GUI_GAME_H
