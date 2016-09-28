#include <string>

namespace PokemonGame
{
	class Pokemon
	{
	public:
		Pokemon ()
		{}

		~Pokemon ()
		{}

		enum class Type
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
		typedef unsigned ATKGap;

	private:
		Type _type;
		Name _name;
		Level _level;
		ExpPoint _expPoint;
		HealthPoint _atk;
		HealthPoint _def;
		HealthPoint _lp;
		ATKGap _atkGap;
	};

}