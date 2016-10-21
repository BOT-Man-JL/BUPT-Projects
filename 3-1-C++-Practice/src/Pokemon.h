#ifndef POKEMON_H
#define POKEMON_H

#include <random>
#include <string>

#define POKEMONCONSTRUCTOR(CLASSNAME)				  \
CLASSNAME (Level level,								  \
ExpPoint expPoint,									  \
HealthPoint atk,									  \
HealthPoint def,									  \
HealthPoint hp,										  \
TimeGap timeGap)									  \
: Pokemon (level, expPoint, atk, def, hp, timeGap)	  \
{}													  \

namespace PokemonGame
{
	class Pokemon
	{
	public:
		// this Pokemon Attack opPokemon
		// Return <isKilling, isUpgraded>
		std::pair<bool, bool> Attack (Pokemon &opPokemon)
		{
			auto &thisPokemon = *this;
			auto damage = thisPokemon.GetDamagePoint (opPokemon);
			auto isKilling = opPokemon.Hurt (damage);
			auto isUpgraded = false;

			if (isKilling)
			{
				opPokemon._hp = opPokemon._fullHP;
				opPokemon.OnKilled ();
			}
			else
			{
				isUpgraded = thisPokemon.Upgrade (opPokemon.GetLevel () * 100);
				if (isUpgraded) thisPokemon.OnUpgrade ();
			}

			return std::make_pair (isKilling, isUpgraded);
		}

		// Attribute
		typedef unsigned Level;
		typedef unsigned ExpPoint;
		typedef unsigned HealthPoint;
		typedef unsigned TimeGap;

		// Get Attr
		virtual std::string GetName () const = 0;

		Level GetLevel () const { return _level; }
		Level GetExp () const { return _expPoint; }
		HealthPoint GetAtk () const { return _atk; }
		HealthPoint GetDef () const { return _def; }
		TimeGap GetTimeGap () const { return _timeGap; }
		HealthPoint GetHP () const { return _hp; }
		HealthPoint GetFullHP () const { return _fullHP; }

	protected:
		Level _level;
		ExpPoint _expPoint;
		HealthPoint _atk;
		HealthPoint _def;
		TimeGap _timeGap;
		HealthPoint _hp;
		HealthPoint _fullHP;

		// Constructor
		Pokemon (Level level,
				 ExpPoint expPoint,
				 HealthPoint atk,
				 HealthPoint def,
				 HealthPoint hp,
				 TimeGap timeGap)
			: _level (level), _expPoint (expPoint),
			_atk (atk), _def (def), _timeGap (timeGap),
			_hp (hp), _fullHP (hp)
		{}

		virtual HealthPoint GetDamagePoint (Pokemon &opPokemon) = 0;
		virtual void OnKilled () = 0;
		virtual void OnUpgrade () = 0;

	private:
		// Hurt
		// Return true if Killed
		// Return false otherwise
		bool Hurt (HealthPoint damage)
		{
			_hp -= damage;
			if (_hp <= 0)
				return true;
			return false;
		}

		// Upgrade
		// Return true if Upgraded
		// Return false otherwise
		bool Upgrade (ExpPoint exp)
		{
			_expPoint += exp;
			if (_level >= 15)
				return false;

			const auto thresholdPerLV = 100;
			auto isUpgraded = false;
			while (_expPoint >= thresholdPerLV * _level)
			{
				_expPoint -= thresholdPerLV * _level;
				_level++;
				isUpgraded = true;
			}
			return isUpgraded;
		}
	};

	struct StrengthPokemon : public Pokemon
	{
		using Pokemon::HealthPoint;
		using Pokemon::TimeGap;

		// Constructor
		POKEMONCONSTRUCTOR (StrengthPokemon)
	};

	struct DefendingPokemon : public Pokemon
	{
		using Pokemon::HealthPoint;
		using Pokemon::TimeGap;

		// Constructor
		DefendingPokemon (Level level,
						  ExpPoint expPoint,
						  HealthPoint atk,
						  HealthPoint def,
						  HealthPoint hp,
						  TimeGap timeGap)
			: Pokemon (level, expPoint, atk, def, hp, timeGap)
		{}
	};

	struct TankPokemon : public Pokemon
	{
		using Pokemon::HealthPoint;
		using Pokemon::TimeGap;

		// Constructor
		TankPokemon (Level level,
					 ExpPoint expPoint,
					 HealthPoint atk,
					 HealthPoint def,
					 HealthPoint hp,
					 TimeGap timeGap)
			: Pokemon (level, expPoint, atk, def, hp, timeGap)
		{}
	};

	struct SwiftPokemon : public Pokemon
	{
		using Pokemon::HealthPoint;
		using Pokemon::TimeGap;

		// Constructor
		SwiftPokemon (Level level,
					  ExpPoint expPoint,
					  HealthPoint atk,
					  HealthPoint def,
					  HealthPoint hp,
					  TimeGap timeGap)
			: Pokemon (level, expPoint, atk, def, hp, timeGap)
		{}
	};

	class Pikachu : public SwiftPokemon
	{
	public:
		Pikachu ()
			: SwiftPokemon (70, 50, 1000, 10)
		{}

		std::string GetName () const
		{
			return "Pikachu";
		}

		bool Attack (Pokemon &opPokemon) override
		{
			auto damage = _atk;
			if (opPokemon.GetType () == Type::Tank)
				damage += opPokemon.GetDef ();
			return opPokemon.Hurt (damage);
		}
	};

	class Charmander : public Pokemon
	{
	public:
		Charmander ()
			: Pokemon (100, 40, 1000, 18)
		{}

		std::string GetName () const
		{
			return "Charmander";
		}

		bool Attack (Pokemon &opPokemon) override
		{
			auto damage = _atk;
			if (opPokemon.GetType () == Type::Defending)
				damage += opPokemon.GetAtk ();
			return opPokemon.Hurt (damage);
		}
	};

}

namespace PokemonGame_Impl
{
	PokemonGame::Pokemon* NewPokemonRandly ()
	{
		const auto POKEMON_COUNT = 2;
		switch (std::random_device ()() % POKEMON_COUNT)
		{
		case 0:
			return new PokemonGame::Pikachu;
		case 1:
			return new PokemonGame::Charmander;
		default:
			return nullptr;
		}
	}
}

#endif // !POKEMON_H