#include "Pokemon.h"
#include <iostream>

int main (int argc, char *argv[])
{
	using namespace PokemonGame;

	Pikachu pika;
	Charmander cmd;

	auto tickPika = pika.GetTimeGap ();
	auto tickCmd = cmd.GetTimeGap ();
	auto fnAttack = [&] (Pokemon &p1, Pokemon &p2)
	{
		std::cout << p1.GetName () << " attacked "
			<< p2.GetName () << std::endl;
		if (p1.Attack (p2))
		{
			std::cout << p1.GetName () << " defeated "
				<< p2.GetName () << std::endl;
			return true;
		}
		return false;
	};

	while (true)
	{
		if (!tickPika)
		{
			if (fnAttack (pika, cmd))
				break;
			tickPika = pika.GetTimeGap ();
		}
		tickPika--;

		if (!tickCmd)
		{
			if (fnAttack (cmd, pika))
				break;
			tickCmd = cmd.GetTimeGap ();
		}
		tickCmd--;
	}

	getchar ();
	return 0;
}
