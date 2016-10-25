#ifndef POKEMON_H
#define POKEMON_H

#include <vector>
#include <exception>
#include <random>
#include <string>

#define SCAFFOLD_POKEMON_TYPE(CLASSNAME)					\
protected:													\
using Pokemon::HealthPoint;									\
using Pokemon::TimeGap;										\
using Pokemon::Level;										\
using Pokemon::ExpPoint;									\
CLASSNAME (Level level,										\
ExpPoint expPoint,											\
HealthPoint atk,											\
HealthPoint def,											\
HealthPoint hp,												\
HealthPoint fullHP,											\
TimeGap timeGap)											\
: Pokemon (level, expPoint, atk, def, hp, fullHP, timeGap)	\
{}															\
public:														\
std::string GetType () const override final					\
{															\
	return #CLASSNAME;										\
}															\

#define SCAFFOLD_POKEMON(CLASSNAME, TYPE)					\
public:														\
CLASSNAME (Level level,										\
ExpPoint expPoint,											\
HealthPoint atk,											\
HealthPoint def,											\
HealthPoint hp,												\
HealthPoint fullHP,											\
TimeGap timeGap)											\
: TYPE (level, expPoint, atk, def, hp, fullHP, timeGap)		\
{}															\
std::string GetName () const override final					\
{															\
	return #CLASSNAME;										\
}															\

#define SCAFFOLD_NEW_POKEMON_FROM_NAME(CLASSNAME)			\
if (name == #CLASSNAME)										\
	return new CLASSNAME (level, expPoint, atk, def,		\
						  hp, fullHP, timeGap)				\

#define NAMEOF(CLASSNAME) #CLASSNAME

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
			auto damage = thisPokemon._GetDamagePoint (opPokemon);
			auto isKilling = opPokemon.Hurt (damage);
			auto isUpgraded = false;

			if (isKilling)
			{
				opPokemon._hp = opPokemon._fullHP;
				opPokemon._OnKilled ();
			}
			else
			{
				isUpgraded = thisPokemon.Upgrade (opPokemon.GetLevel () * 100);
				if (isUpgraded) thisPokemon._OnUpgrade ();
			}

			return std::make_pair (isKilling, isUpgraded);
		}

		// Attribute
		typedef unsigned Level;
		typedef unsigned ExpPoint;
		typedef unsigned HealthPoint;
		typedef unsigned TimeGap;

		// Get Attr
		virtual std::string GetType () const = 0;
		virtual std::string GetName () const = 0;

		Level GetLevel () const { return _level; }
		Level GetExp () const { return _expPoint; }
		HealthPoint GetAtk () const { return _atk; }
		HealthPoint GetDef () const { return _def; }
		TimeGap GetTimeGap () const { return _timeGap; }
		HealthPoint GetHP () const { return _hp; }
		HealthPoint GetFullHP () const { return _fullHP; }

		// Factory
		static Pokemon *NewPokemon ();
		static Pokemon *NewPokemon (const std::string &name);
		static Pokemon *NewPokemon (const std::string &name,
									Level level,
									ExpPoint expPoint,
									HealthPoint atk,
									HealthPoint def,
									HealthPoint hp,
									HealthPoint fullHP,
									TimeGap timeGap);
		static const std::vector<std::string> &PokemonNames ();
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
				 HealthPoint fullHP,
				 TimeGap timeGap)
			: _level (level), _expPoint (expPoint),
			_atk (atk), _def (def), _timeGap (timeGap),
			_hp (hp), _fullHP (fullHP)
		{}

		// Abstract Function Members

		virtual HealthPoint _GetDamagePoint (Pokemon &opPokemon) = 0;
		virtual void _OnBorn () = 0;
		virtual void _OnKilled () = 0;
		virtual void _OnUpgrade () = 0;

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
		SCAFFOLD_POKEMON_TYPE (StrengthPokemon)
	public:
		HealthPoint _GetDamagePoint (Pokemon &opPokemon) override
		{
			return HealthPoint (0);
		}
	};

	struct DefendingPokemon : public Pokemon
	{
		SCAFFOLD_POKEMON_TYPE (DefendingPokemon)
	public:
		HealthPoint _GetDamagePoint (Pokemon &opPokemon) override
		{
			return HealthPoint (0);
		}
	};

	struct TankPokemon : public Pokemon
	{
		SCAFFOLD_POKEMON_TYPE (TankPokemon)
	public:
		HealthPoint _GetDamagePoint (Pokemon &opPokemon) override
		{
			return HealthPoint (0);
		}
	};

	struct SwiftPokemon : public Pokemon
	{
		SCAFFOLD_POKEMON_TYPE (SwiftPokemon)
	public:
		HealthPoint _GetDamagePoint (Pokemon &opPokemon) override
		{
			return HealthPoint (0);
		}
	};

	namespace Rand
	{
		int rand (int min, int max)
		{
			static std::random_device rand;
			return rand () % (max - min) + min;
		};
	}

	class Pikachu : public SwiftPokemon
	{
		SCAFFOLD_POKEMON (Pikachu, SwiftPokemon)
	protected:
		void _OnBorn () override
		{
			using namespace Rand;
			_atk = rand (8, 12);
			_def = rand (6, 8);
			_hp = _fullHP = rand (40, 50);
			_timeGap = rand (6, 9);
		}
		void _OnKilled () override
		{
		}
		void _OnUpgrade () override
		{
		}
	};

	class Charmander : public StrengthPokemon
	{
		SCAFFOLD_POKEMON (Charmander, StrengthPokemon)
	protected:
		void _OnBorn () override
		{
			using namespace Rand;
			_atk = rand (10, 15);
			_def = rand (6, 8);
			_hp = _fullHP = rand (50, 60);
			_timeGap = rand (8, 12);
		}
		void _OnKilled () override
		{
		}
		void _OnUpgrade () override
		{
		}
	};

	inline const std::vector<std::string> &Pokemon::PokemonNames ()
	{
		static const std::vector<std::string> pokemonNames
		{
			NAMEOF (Pikachu),
			NAMEOF (Charmander)
		};
		return pokemonNames;
	}

	inline Pokemon *Pokemon::NewPokemon (const std::string &name,
										 Level level,
										 ExpPoint expPoint,
										 HealthPoint atk,
										 HealthPoint def,
										 HealthPoint hp,
										 HealthPoint fullHP,
										 TimeGap timeGap)
	{
		SCAFFOLD_NEW_POKEMON_FROM_NAME (Pikachu);
		SCAFFOLD_NEW_POKEMON_FROM_NAME (Charmander);
		throw std::runtime_error ("No Such Pokemon");
	}

	Pokemon *Pokemon::NewPokemon (const std::string &name)
	{
		auto ret = Pokemon::NewPokemon (name,
			1, 0, 0, 0, 0, 0, 0);

		// Set Init Attr Here
		ret->_OnBorn ();
		return ret;
	}

	inline Pokemon *Pokemon::NewPokemon ()
	{
		return NewPokemon (
			PokemonNames ()[std::random_device ()()
			% PokemonNames ().size ()]);
	}
}

#endif // !POKEMON_H