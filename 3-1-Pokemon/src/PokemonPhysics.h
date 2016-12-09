#ifndef POKEMON_PHYSICS_H
#define POKEMON_PHYSICS_H

#include <cmath>
#include <memory>
#include <list>
#include <unordered_map>

#include "Pokemon.h"
#include "Shared.h"

namespace PokemonGame
{
	class Physics
	{
	public:
		struct Rect
		{
			int x, y;
			int w, h;
		};

		Physics (int x, int y, int w, int h, int vx, int vy)
			: _rect { x, y, w, h }, _vx (vx), _vy (vy)
		{}

		const Rect &GetRect () const
		{
			return _rect;
		}

		decltype (auto) GetVelocity () const
		{
			return std::forward_as_tuple (_vx, _vy);
		}

		bool IsOverlap (const Physics &other) const
		{
			const auto &a = this->_rect;
			const auto &b = other._rect;

			return
				a.x <= b.x + b.w &&
				a.y <= b.y + b.h &&
				b.x <= a.x + a.w &&
				b.y <= a.y + a.h;
		}

		bool IsInside (const Physics &other) const
		{
			const auto &a = this->_rect;
			const auto &b = other._rect;

			return
				a.x >= b.x &&
				a.y >= b.y &&
				a.x + a.w <= b.x + b.w &&
				a.y + a.h <= b.y + b.h;
		}

	protected:
		Rect _rect;
		int _vx, _vy;
	};

	class PlayerPhysics : public Physics
	{
	public:
		PlayerPhysics (Player &player)
			: Physics (player.x, player.y, 0, 0, 0, 0),
			_player (player)
		{
			auto size = _player.pokemon->GetSize ();
			_rect.w = size.first;
			_rect.h = size.second;
		}

		void Move (int x, int y, const Physics &worldMap)
		{
			// Last snapshot of Velocity
			_rect.x += _vx;
			_rect.y += _vy;

			// Write back Position
			_player.x = _rect.x;
			_player.y = _rect.y;

			// Try Move
			_rect.x += x;
			_rect.y += y;

			// Set Velocity
			if (IsInside (worldMap))
			{
				_vx = x;
				_vy = y;
			}
			else
				_vx = _vy = 0;

			// Rollback Move
			_rect.x -= x;
			_rect.y -= y;
		}

		decltype (auto) GetPlayer () const
		{
			return _player;
		}

	private:
		Player &_player;
	};

	class DamagePhysics : public Physics
	{
		static constexpr auto width = 10;
		static constexpr auto height = 10;

	public:
		DamagePhysics (const UserID &player,
					   int x, int y, int vx, int vy,
					   PokemonGame::Pokemon::HealthPoint damage,
					   size_t tick)
			: Physics (x, y, width, height, vx, vy),
			_player (player), _damage (damage), _tick (tick)
		{}

		bool Move ()
		{
			// Move in Velocity
			_rect.x += _vx;
			_rect.y += _vy;

			// Count down tick
			_tick--;

			if (_tick) return false;
		}

		auto GetDamage () const { return _damage; }

	private:
		UserID _player;
		PokemonGame::Pokemon::HealthPoint _damage;
		size_t _tick;
	};

	using PlayersPhysics = std::unordered_map<UserID, PlayerPhysics>;

	struct GamePhysics
	{
		GamePhysics (Players &players, ActionQueue &actionQueue)
			: _players (players), _actionQueue (actionQueue)
		{
			for (auto &playerPair : _players)
			{
				auto &player = playerPair.second;
				playersPhysics.emplace (playerPair.first, PlayerPhysics (player));
			}
		}

		void Update ()
		{
			// World map is also a Physics
			const static Physics worldMap (0, 0, Player::maxX, Player::maxY, 0, 0);

			while (!_actionQueue.empty ())
			{
				// Retrieve Action
				const auto &action = _actionQueue.top ();
				const auto &player = _players[action.uid];

				// Current Pokemon
				auto &pokemon = *(player.pokemon);

				// Rigid body
				auto &physics = playersPhysics.at (action.uid);

				// Get Distance to Move
				auto distX = 0, distY = 0;
				if (action.x != 0 || action.y != 0)
				{
					auto len = sqrt (action.x * action.x + action.y * action.y);
					distY = (int) ((int) pokemon.GetVelocity () * action.y / len);
					distX = (int) ((int) pokemon.GetVelocity () * action.x / len);
				}

				// Get Attack Property
				pokemon.GetTimeGap ();
				pokemon.GetAtk ();

				switch (action.type)
				{
				case ActionType::Move:
					physics.Move (distX, distY, worldMap);
					break;

				case ActionType::Attack:

					break;

				case ActionType::Defend:
					break;

				default:
					throw std::runtime_error ("WTF? (Will not hit)");
					break;
				}

				// Pop
				_actionQueue.pop ();
			}
		}

		PlayersPhysics playersPhysics;
		std::list<DamagePhysics> damagesPhysics;

	private:
		Players &_players;
		ActionQueue &_actionQueue;
	};
}

#endif // !POKEMON_PHYSICS_H
