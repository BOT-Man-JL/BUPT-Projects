//
// Console Mode for Crazy Train Simulation
// By LJN-BOT Man, 2015
//

#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "Scales.h"
#include "Element.h"
#include "Basic.h"

void ConsoleInput ();

static ThreadCtrl tc;  //set to 0 already

int main (int argc, char *argv[])
{
	HANDLE hCompThread = NULL, hDispThread = NULL;

#ifdef _DEBUG
	InitMLTracer ();
#endif

	if (argc == 1)
	{
		if (!Init (&tc, "CTS_Map.dat"))
			Err ("Initializing Failed");
	}
	else
	{
		if (!Init (&tc, argv[1]))
			Err ("Initializing Failed");
	}

	printf ("Welcome to Crazy Train Simulation\n\nPress Any Key to Start\n\n");
	getch ();
	printf ("Press Enter to Pause/Continue\n"
			"Press Space to Switch Strategy\n"
			"Press Q/Esc to Quit\n\n");

	hCompThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) Compute, (LPVOID) &tc, 0, NULL);
	hDispThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) CDisplay, (LPVOID) &tc, 0, NULL);

	if (!hCompThread || !hDispThread)
		Err ("Creating Thread Failed");

	tc.hCompEvent = CreateEvent (NULL, FALSE, TRUE, NULL);
	tc.hDispEvent = CreateEvent (NULL, FALSE, FALSE, NULL);

	ConsoleInput (&tc);

	WaitForSingleObject (hCompThread, INFINITE);  //Display Thread Exit
	WaitForSingleObject (hDispThread, INFINITE);  //Compute Thread Exit

	FreeAllResource (tc.lrtHead);  //Now, we can release the resource safely
	CloseHandle (tc.hCompEvent);
	CloseHandle (tc.hDispEvent);
	CloseHandle (hCompThread);
	CloseHandle (hDispThread);

	//getchar ();
	return 0;
}

void ConsoleInput ()
{
	char ch;
	while (tc.iRunningState != RS_QUIT)
	{
		ch = getch ();
		switch (ch)
		{
		case ' ':
			tc.iStrategy = (tc.iStrategy + 1) % CSTRATEGY;
			printf ("Strategy changes to NO.%d\n", tc.iStrategy);
			break;

		case '\r':
			tc.iRunningState = !(tc.iRunningState - 1) + 1;
			printf (tc.iRunningState - 1 ? "Pause\n" : "Continue\n\n");
			break;

		case '\x1B':
		case 'Q':
		case 'q':
			tc.iRunningState = RS_QUIT;
			printf ("Exit\n");
			break;

		default:
			printf ("ERR: Invalid Input\n");
		}
	}
}
