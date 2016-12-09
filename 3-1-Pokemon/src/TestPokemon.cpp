#include "Pokemon.h"
#include <iostream>

int main (int argc, char *argv[])
{
	using namespace PokemonGame;

	auto pika = Pokemon::NewPokemon ("Pikachu");
	auto cmd = Pokemon::NewPokemon ("Charmander");

	auto fnAttack = [&] (Pokemon &p1, Pokemon &p2)
	{
		std::cout << p1.GetName () << " attacked "
			<< p2.GetName () << std::endl;
		auto result = p1.Attack (p2);

		std::cout << "HP:\t" << p1.GetHP () << "\t"
			<< p2.GetHP () << std::endl
			<< "LV: \t" << p1.GetLevel () << "\t"
			<< p2.GetLevel () << std::endl;

		if (result.first)
			std::cout << p1.GetName () << " defeated "
			<< p2.GetName () << std::endl;
		if (result.second)
			std::cout << p1.GetName () << " upgraded" << std::endl;

		return result.first;
	};

	auto fnRound = [&] (Pokemon &p1, Pokemon &p2)
	{
		std::cout << "\nNew Round:\n\n";

		auto tickP1 = p1.GetTimeGap ();
		auto tickP2 = p2.GetTimeGap ();
		while (true)
		{
			if (!tickP1)
			{
				if (fnAttack (p1, p2))
					break;
				tickP1 = p1.GetTimeGap ();
			}
			tickP1--;

			if (!tickP2)
			{
				if (fnAttack (p2, p1))
					break;
				tickP2 = p2.GetTimeGap ();
			}
			tickP2--;
		}
		p1.Recover ();
		p2.Recover ();
	};

	fnRound (*pika, *cmd);
	fnRound (*cmd, *pika);
	fnRound (*pika, *cmd);
	fnRound (*cmd, *pika);

	getchar ();
	return 0;
}
