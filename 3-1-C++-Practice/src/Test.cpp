#include "Pokemon.h"
#include <iostream>

int main (int argc, char *argv[])
{
	using namespace PokemonGame;

	Pikachu pika;
	Charmander cmd;

	auto fnAttack = [&] (Pokemon &p1, Pokemon &p2)
	{
		std::cout << p1.GetName () << " attacked "
			<< p2.GetName () << std::endl;
		auto preLevel = p1.GetLevel ();
		if (p1.Attack (p2))
		{
			std::cout << p1.GetName () << " defeated "
				<< p2.GetName () << std::endl;
			auto curLevel = p1.GetLevel ();
			if (preLevel != curLevel)
				std::cout << p1.GetName () << " upgraded from "
				<< preLevel << " to " << curLevel << std::endl;
			return true;
		}
		return false;
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
	};

	fnRound (pika, cmd);
	fnRound (cmd, pika);
	fnRound (pika, cmd);
	fnRound (cmd, pika);

	getchar ();
	return 0;
}
