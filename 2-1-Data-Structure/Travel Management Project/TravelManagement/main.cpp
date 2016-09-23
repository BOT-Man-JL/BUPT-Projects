
#include "Driver.h"
#include <thread>

int main (int argc, char *argArrCity[])
{	
	if (argc >= 2)
		Init (argArrCity[1]);
	else
		Init ("input.txt");			// Default input file name
	std::thread input_thread {fnInput};
	std::thread time_thread {fnTimeTick};
	input_thread.join ();
	time_thread.join ();
	
	return 0;
}


