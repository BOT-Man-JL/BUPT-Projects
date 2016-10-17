
#ifndef POKEMON_MODEL_H
#define POKEMON_MODEL_H

#include "ORM-Lite\ORMLite.h"
#include "Pokemon.h"

namespace PokemonGame_Impl
{
	struct PokemonModel
	{
		long id;
		std::string uid;

		std::string name;
		PokemonGame::Pokemon::Level level;
		PokemonGame::Pokemon::ExpPoint expPoint;
		PokemonGame::Pokemon::HealthPoint atk;
		PokemonGame::Pokemon::HealthPoint def;
		PokemonGame::Pokemon::HealthPoint hp;
		PokemonGame::Pokemon::HealthPoint fullHP;
		PokemonGame::Pokemon::TimeGap timeGap;

		static PokemonModel *NewFromPokemon (
			std::string uid,
			const PokemonGame::Pokemon &pokemon)
		{
			return new PokemonModel
			{
				0, uid, pokemon.GetName (),
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

	struct UserModel
	{
		std::string uid;
		std::string pwd;
		size_t won;
		size_t los;
		std::string badge;

	private:
		ORMAP (UserModel, uid, pwd, won, los, badge)
	};

	struct SessionModel
	{
		std::string uid;
		std::string sid;

	private:
		ORMAP (SessionModel, uid, sid)
	};
}

#endif // !POKEMON_MODEL_H