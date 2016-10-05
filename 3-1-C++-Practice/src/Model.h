
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
		long level;
		long expPoint;
		long atk;
		long def;
		long hp;
		long fullHP;
		long timeGap;

		static PokemonModel *NewFromPokemon (
			std::string uid,
			const PokemonGame::Pokemon &pokemon)
		{
			return new PokemonModel
			{
				0, uid, pokemon.GetName (),
				(long) pokemon.GetLevel (), (long) pokemon.GetExp (),
				(long) pokemon.GetAtk (), (long) pokemon.GetDef (),
				(long) pokemon.GetHP (), (long) pokemon.GetFullHP (),
				(long) pokemon.GetTimeGap ()
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
		long won;
		long los;
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