#ifndef POKEMON_H
#define POKEMON_H

#include <random>
#include <string>

namespace PokemonGame
{
	class Pokemon
	{
	public:
		// Attribute
		enum struct Type
		{
			Strength,
			Tank,
			Defending,
			Swift
		};
		typedef std::string Name;
		typedef unsigned Level;
		typedef unsigned ExpPoint;
		typedef unsigned HealthPoint;
		typedef unsigned TimeGap;

		// Init
		Pokemon (Type type, Name name,
				 HealthPoint atk,
				 HealthPoint def,
				 HealthPoint hp,
				 TimeGap timeGap)
			: _type (type), _name (name), _level (1), _expPoint (0),
			_atk (atk), _def (def), _hp (hp), _fullHP (hp), _timeGap (timeGap)
		{}

		// Upgrade
		virtual bool Upgrade (ExpPoint exp)
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

		// Hurt
		virtual bool Hurt (HealthPoint damage)
		{
			if (damage > _def)
				_hp -= damage - _def / 2;
			if (_hp <= 0)
				return true;
			return false;
		}

		// Attack
		virtual bool Attack (Pokemon &) = 0;

		// Get Attr
		Type GetType () const { return _type; }
		Name GetName () const { return _name; }
		Level GetLevel () const { return _level; }
		Level GetExp () const { return _expPoint; }
		HealthPoint GetAtk () const { return _atk; }
		HealthPoint GetDef () const { return _def; }
		TimeGap GetTimeGap () const { return _timeGap; }
		HealthPoint GetHP () const { return _hp; }
		HealthPoint GetFullHP () const { return _fullHP; }

	protected:
		Type _type;
		Name _name;
		Level _level;
		ExpPoint _expPoint;
		HealthPoint _atk;
		HealthPoint _def;
		HealthPoint _hp;
		HealthPoint _fullHP;
		TimeGap _timeGap;
	};

	class Pikachu : public Pokemon
	{
	public:
		Pikachu ()
			: Pokemon (Type::Swift, "Pikachu",
					   70, 50, 1000, 10)
		{}

		bool Upgrade (ExpPoint exp) override
		{
			return Pokemon::Upgrade (exp);
		}

		bool Hurt (HealthPoint damage) override
		{
			auto isDead = Pokemon::Hurt (damage);
			if (isDead)
				_hp = _fullHP;
			return isDead;
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
			: Pokemon (Type::Strength, "Charmander",
					   100, 40, 1000, 18)
		{}

		bool Upgrade (ExpPoint exp) override
		{
			return Pokemon::Upgrade (exp);
		}

		bool Hurt (HealthPoint damage) override
		{
			auto isDead = Pokemon::Hurt (damage);
			if (isDead)
				_hp = _fullHP;
			return isDead;
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