
#include <thread>
#include <iostream>

#include "PokemonClientGUI.h"

#define IPADDR "127.0.0.1"
#define PORT 5768

int main (int argc, char *argv[])
{
	PokemonGameGUI::PokemonClientGUI client_gui (IPADDR, PORT);

	if (!client_gui.Login ())
	{
		PokemonGameGUI::MsgBox ("You haven't Login in GUI\n");
		return 0;
	}

	if (!client_gui.SelectRoom ())
	{
		PokemonGameGUI::MsgBox ("You haven't Entered a Room\n");
		return 0;
	}

	client_gui.Play ();

	return 0;
}