#ifndef POKEMON_H
#define POKEMON_H

#include <vector>
#include <utility>
#include <exception>
#include <random>
#include <algorithm>
#include <string>
#include <memory>

// Fix for the pollution by <windows.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

// Scaffolding Macros
#define SCAFFOLD_POKEMON_TYPE(CLASSNAME)					\
struct CLASSNAME : public Pokemon {							\
protected:													\
	using Pokemon::HealthPoint;								\
	using Pokemon::TimeGap;									\
	using Pokemon::Level;									\
	using Pokemon::ExpPoint;								\
	CLASSNAME (Level level,									\
		ExpPoint expPoint,									\
		HealthPoint atk,									\
		HealthPoint def,									\
		HealthPoint hp,										\
		TimeGap timeGap)									\
	: Pokemon (level, expPoint, atk, def, hp, timeGap) {}	\
public:														\
	std::string GetType () const override final				\
	{														\
		return #CLASSNAME;									\
	}														\
	HealthPoint _GetDamagePoint (							\
		const Pokemon &opPokemon) const override;			\
}															\

#define SCAFFOLD_POKEMON(									\
	CLASSNAME, TYPE, W, H, V, ATK, DEF, HP, TG)				\
class CLASSNAME : public TYPE {								\
public:														\
	CLASSNAME (Level level,									\
		ExpPoint expPoint,									\
		HealthPoint atk,									\
		HealthPoint def,									\
		HealthPoint hp,										\
		TimeGap timeGap)									\
	: TYPE (level, expPoint, atk, def, hp, timeGap) {}		\
	CLASSNAME ()											\
	: TYPE (1, 0, _RandAttr (ATK), _RandAttr (DEF),			\
			_RandAttr (HP), _RandAttr (TG)) {}				\
	std::string GetName () const override final				\
	{														\
		return #CLASSNAME;									\
	}														\
	std::pair<size_t, size_t> GetSize (						\
		) const override final								\
	{														\
		return std::make_pair (W, H);						\
	}														\
	size_t GetVelocity () const override final				\
	{														\
		return V;											\
	}														\
protected:													\
	void _OnUpgrade () override final;						\
}															\

namespace PokemonGame
{
	class Pokemon
	{
	public:
		// Attribute Types
		using Level = unsigned;
		using ExpPoint = unsigned;
		using HealthPoint = unsigned;
		using TimeGap = unsigned;

		// Factory
		static const std::vector<std::string> &PokemonNames ();
		static std::unique_ptr<Pokemon> NewPokemon ();
		static std::unique_ptr<Pokemon> NewPokemon (
			const std::string &name);
		static std::unique_ptr<Pokemon> NewPokemon (
			const std::string &name,
			Level level,
			ExpPoint expPoint,
			HealthPoint atk,
			HealthPoint def,
			HealthPoint hp,
			TimeGap timeGap);

		// Get Type & Name
		virtual std::string GetType () const = 0;
		virtual std::string GetName () const = 0;

		// Get Physics
		virtual std::pair<size_t, size_t> GetSize () const = 0;
		virtual size_t GetVelocity () const = 0;

		// Get Attributes
		Level GetLevel () const { return _level; }
		Level GetExp () const { return _expPoint; }
		HealthPoint GetAtk () const { return _atk; }
		HealthPoint GetDef () const { return _def; }
		TimeGap GetTimeGap () const { return _timeGap; }
		HealthPoint GetHP () const { return _hp; }

		// Runtime Attr
		HealthPoint GetCurHP () const { return _curHp; }

		// this Pokemon Attack opPokemon
		// Return <isKilling, isUpgraded>
		std::pair<bool, bool> Attack (Pokemon &opPokemon)
		{
			auto &thisPokemon = *this;
			auto damage = thisPokemon._GetDamagePoint (opPokemon);
			auto isKilling = opPokemon._Hurt (damage);
			auto isUpgraded = false;

			if (isKilling)
			{
				isUpgraded = thisPokemon._Upgrade (
					opPokemon.GetLevel () * _Rand (50, 100));
			}
			else
				isUpgraded = thisPokemon._Upgrade (
					opPokemon.GetLevel () * _Rand (0, 5));
			if (isUpgraded) thisPokemon._OnUpgrade ();

			return std::make_pair (isKilling, isUpgraded);
		}

		// Reset cur Hp to Full Hp
		void Recover ()
		{
			_curHp = _hp;
		}

	protected:
		// Properties
		Level _level;
		ExpPoint _expPoint;
		HealthPoint _atk;
		HealthPoint _def;
		TimeGap _timeGap;
		HealthPoint _hp;

		// Runtime
		HealthPoint _curHp;

		// Constructor
		Pokemon (Level level,
				 ExpPoint expPoint,
				 HealthPoint atk,
				 HealthPoint def,
				 HealthPoint hp,
				 TimeGap timeGap)
			: _level (level), _expPoint (expPoint),
			_atk (atk), _def (def), _timeGap (timeGap),
			_hp (hp), _curHp (hp)
		{}

		// Abstract Functions
		virtual HealthPoint _GetDamagePoint (
			const Pokemon &opPokemon) const = 0;
		virtual void _OnUpgrade () = 0;

		// Helper Functions
		static int _Rand (int min, int max)
		{
			static std::random_device rand;
			if (max == min)
				return max;
			else
				return rand () % (max - min) + min;
		};

		static unsigned _RandAttr (unsigned attr)
		{
			// Vary from -20% to 20%
			auto delta = (unsigned) (attr / 5.0);
			return _Rand (attr - delta, attr + delta);
		}

	private:
		// Hurt
		// Return true if Killed, false otherwise
		bool _Hurt (HealthPoint damage)
		{
			if (_curHp >= damage)
				_curHp -= damage;
			else
				_curHp = 0;

			return (_curHp == 0);
		}

		// Upgrade
		// Return true if Upgraded, false otherwise
		bool _Upgrade (ExpPoint exp)
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

	// Scaffold 4 Types of Pokemons
	SCAFFOLD_POKEMON_TYPE (StrengthPokemon);
	SCAFFOLD_POKEMON_TYPE (DefendingPokemon);
	SCAFFOLD_POKEMON_TYPE (TankPokemon);
	SCAFFOLD_POKEMON_TYPE (SwiftPokemon);

	// Scaffold Pokemons
	SCAFFOLD_POKEMON (Pikachu, SwiftPokemon,
					  10, 20, 20,
					  10, 7, 45, 4);
	SCAFFOLD_POKEMON (Charmander, StrengthPokemon,
					  20, 25, 14,
					  15, 7, 55, 6);
}

#undef SCAFFOLD_POKEMON_TYPE
#undef SCAFFOLD_POKEMON

#endif // !POKEMON_H