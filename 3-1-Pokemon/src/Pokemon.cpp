
#include "Pokemon.h"

#define SCAFFOLD_NEW_POKEMON_FROM_NAME_WITH_ATTR(CLASSNAME)	\
if (name == #CLASSNAME)										\
	return std::unique_ptr<Pokemon> {						\
        new CLASSNAME (level, expPoint, atk, def,			\
					   hp, timeGap) }						\

#define SCAFFOLD_NEW_POKEMON_FROM_NAME(CLASSNAME)			\
if (name == #CLASSNAME)										\
	return std::unique_ptr<Pokemon> {						\
        new CLASSNAME () }									\

#define NAMEOF(CLASSNAME) #CLASSNAME

// Note that:
// All of these member functions could not be inline :-)
// http://stackoverflow.com/questions/3992980/c-inline-member-function-in-cpp-file/3993003#3993003

namespace PokemonGame
{
	// Define _GetDamagePoint

	Pokemon::HealthPoint StrengthPokemon::_GetDamagePoint (
		const Pokemon &opPokemon) const
	{
		HealthPoint ret = 0;
		if (dynamic_cast<const TankPokemon *> (&opPokemon))
			ret = (unsigned) (this->GetAtk () - opPokemon.GetDef () * 1.2);
		else if (dynamic_cast<const DefendingPokemon *> (&opPokemon))
			ret = this->GetAtk () - opPokemon.GetDef () / 2;
		else if (dynamic_cast<const SwiftPokemon *> (&opPokemon))
			ret = this->GetAtk () - _Rand (0, opPokemon.GetDef () * 2);
		else
			ret = this->GetAtk () - opPokemon.GetDef ();
		return std::max (ret, HealthPoint {});
	}

	Pokemon::HealthPoint DefendingPokemon::_GetDamagePoint (
		const Pokemon &opPokemon) const
	{
		HealthPoint ret = 0;
		if (dynamic_cast<const StrengthPokemon *> (&opPokemon))
			ret = this->GetAtk ();
		else if (dynamic_cast<const TankPokemon *> (&opPokemon))
			ret = this->GetAtk ();
		else if (dynamic_cast<const SwiftPokemon *> (&opPokemon))
			ret = this->GetAtk () - _Rand (
				opPokemon.GetDef () / 2, (unsigned) (opPokemon.GetDef () * 1.5));
		else
			ret = this->GetAtk () - opPokemon.GetDef ();
		return std::max (ret, HealthPoint {});
	}

	Pokemon::HealthPoint TankPokemon::_GetDamagePoint (
		const Pokemon &opPokemon) const
	{
		HealthPoint ret = 0;
		if (dynamic_cast<const StrengthPokemon *> (&opPokemon))
			ret = this->GetAtk () - opPokemon.GetDef () / 2;
		else if (dynamic_cast<const DefendingPokemon *> (&opPokemon))
			ret = this->GetAtk () - opPokemon.GetDef () * 3;
		else if (dynamic_cast<const SwiftPokemon *> (&opPokemon))
			ret = (unsigned) (this->GetAtk () - (unsigned) (opPokemon.GetDef () * 1.5));
		else
			ret = this->GetAtk () - opPokemon.GetDef ();
		return std::max (ret, HealthPoint {});
	}

	Pokemon::HealthPoint SwiftPokemon::_GetDamagePoint (
		const Pokemon &opPokemon) const
	{
		HealthPoint ret = 0;
		if (dynamic_cast<const StrengthPokemon *> (&opPokemon))
			ret = this->GetAtk () - _Rand (0, opPokemon.GetDef () * 2);
		else if (dynamic_cast<const TankPokemon *> (&opPokemon))
			ret = _Rand (this->GetAtk (), this->GetAtk () * 2) - opPokemon.GetDef ();
		else if (dynamic_cast<const DefendingPokemon *> (&opPokemon))
			ret = _Rand (this->GetAtk (), this->GetAtk () * 2) - opPokemon.GetDef () / 2;
		else
			ret = this->GetAtk () - opPokemon.GetDef ();
		return std::max (ret, HealthPoint {});
	}

	// Define _OnUpgrade

	void Pikachu::_OnUpgrade ()
	{
		// Currently No Attribute Improvement
	}

	void Charmander::_OnUpgrade ()
	{
		// Currently No Attribute Improvement
	}

	// Define Pokemon Factory

	const std::vector<std::string> &Pokemon::PokemonNames ()
	{
		static const std::vector<std::string> pokemonNames
		{
			NAMEOF (Pikachu),
			NAMEOF (Charmander)
		};
		return pokemonNames;
	}

	std::unique_ptr<Pokemon> Pokemon::NewPokemon (
		const std::string &name,
		Level level,
		ExpPoint expPoint,
		HealthPoint atk,
		HealthPoint def,
		HealthPoint hp,
		TimeGap timeGap)
	{
		SCAFFOLD_NEW_POKEMON_FROM_NAME_WITH_ATTR (Pikachu);
		SCAFFOLD_NEW_POKEMON_FROM_NAME_WITH_ATTR (Charmander);

		throw std::runtime_error ("No Such Pokemon");
	}

	std::unique_ptr<Pokemon> Pokemon::NewPokemon (
		const std::string &name)
	{
		SCAFFOLD_NEW_POKEMON_FROM_NAME (Pikachu);
		SCAFFOLD_NEW_POKEMON_FROM_NAME (Charmander);

		throw std::runtime_error ("No Such Pokemon");
	}

	std::unique_ptr<Pokemon> Pokemon::NewPokemon ()
	{
		return Pokemon::NewPokemon (PokemonNames ()[
			Pokemon::_Rand (0, PokemonNames ().size ())]);
	}
}