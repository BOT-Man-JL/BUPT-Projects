
#ifndef POKEMON_MODEL_H
#define POKEMON_MODEL_H

#include <unordered_map>
#include <memory>
#include <mutex>

#include "ORM-Lite\ORMLite.h"
#include "Pokemon.h"
#include "Shared.h"

namespace PokemonGame_Impl
{
	// SQL Model
	struct PokemonModel
	{
		PokemonGame::PokemonID id;
		PokemonGame::UserID uid;

		std::string name;
		PokemonGame::Pokemon::Level level;
		PokemonGame::Pokemon::ExpPoint expPoint;
		PokemonGame::Pokemon::HealthPoint atk;
		PokemonGame::Pokemon::HealthPoint def;
		PokemonGame::Pokemon::HealthPoint hp;
		PokemonGame::Pokemon::HealthPoint fullHP;
		PokemonGame::Pokemon::TimeGap timeGap;

		inline PokemonGame::Pokemon *ToPokemon ()
		{
			return PokemonGame::Pokemon::NewPokemon (
				name, level, expPoint,
				atk, def, hp, fullHP, timeGap);
		}

		static PokemonModel *NewFromPokemon (
			PokemonGame::PokemonID id, std::string uid,
			const PokemonGame::Pokemon &pokemon)
		{
			return new PokemonModel
			{
				id, uid, pokemon.GetName (),
				pokemon.GetLevel (), pokemon.GetExp (),
				pokemon.GetAtk (), pokemon.GetDef (),
				pokemon.GetHP (), pokemon.GetFullHP (),
				pokemon.GetTimeGap ()
			};
		}

	private:
		ORMAP (PokemonModel, id, uid, name, level, expPoint,
			   atk, def, hp, fullHP, timeGap)
	};

	// User
	struct UserModel
	{
		PokemonGame::UserID uid;
		std::string pwd;
		size_t won;
		size_t los;
		std::string badge;

	private:
		ORMAP (UserModel, uid, pwd, won, los, badge)
	};

	// Session
	struct SessionModel
	{
		PokemonGame::UserID uid;
		PokemonGame::RoomID rid;
	};
	using Sessions = std::unordered_map<PokemonGame::SessionID, SessionModel>;

	// Room
	struct RoomModel
	{
		static const size_t maxPlayerPerRoom = 3;
		PokemonGame::Players players;
		
		using ActionQueues =
			std::unordered_map<PokemonGame::UserID, PokemonGame::ActionQueue>;
		ActionQueues actionQueues;

		std::mutex mtxSync;
	};
	using Rooms = std::unordered_map<PokemonGame::RoomID, RoomModel>;
}

#endif // !POKEMON_MODEL_H