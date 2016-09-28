#ifndef POKEMON_H
#define POKEMON_H

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

		// Uninit
		virtual ~Pokemon ()
		{}

		// Upgrade
		virtual void Upgrade (ExpPoint exp)
		{
			_expPoint += exp;
			if (_level >= 15) return;

			const auto thresholdPerLV = 100;
			if (_expPoint >= thresholdPerLV * _level)
			{
				_expPoint -= thresholdPerLV * _level;
				_level++;
			}
		}

		// Hurt
		virtual bool Hurt (HealthPoint damage)
		{
			_hp -= damage;
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
		HealthPoint GetAtk () const { return _atk; }
		HealthPoint GetDef () const { return _def; }
		TimeGap GetTimeGap () const { return _timeGap; }

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

		void Upgrade (ExpPoint exp) override
		{
			Pokemon::Upgrade (exp);
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

		void Upgrade (ExpPoint exp) override
		{
			Pokemon::Upgrade (exp);
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

#endif // POKEMON_H