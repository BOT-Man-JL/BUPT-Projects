//
// Window Mode for Crazy Train Simulation
// By LJN-BOT Man, 2015
//

#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "Scales.h"
#include "Element.h"
#include "Basic.h"
#include "GUI.h"

void ConsoleInput ();
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

static ThreadCtrl	tc;  //been set to 0
int g_cxMap, g_cyMap;

int main (int argc, char *argv[])
{
	int			iMode;
	HANDLE		hCompThread = NULL, hDispThread = NULL;

	TCHAR		szAppName[] = TEXT ("Crazy Train Simulation (Window Mode)");
	HWND		hwnd;
	MSG			msg;
	WNDCLASS	wndclass;
	HINSTANCE	hInstance;
	int			cxClient, cyClient;

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

	printf ("Welcome to Crazy Train Simulation\n\n"
			"Press C to Start Console Mode\n"
			"Press W to Start Window Mode\n"
			"Press Other Keys to Exit\n\n");
	iMode = getch ();

	switch (iMode)
	{
	case 'C':
	case 'c':
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
		break;

	case 'W':
	case 'w':
		hInstance = GetCurrentProcess ();

		wndclass.style			= CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc	= WndProc;
		wndclass.cbClsExtra		= 0;
		wndclass.cbWndExtra		= 0;
		wndclass.hInstance		= hInstance;
		wndclass.hIcon			= LoadIcon (NULL, IDI_APPLICATION);
		wndclass.hCursor		= LoadCursor (NULL, IDC_ARROW);
		wndclass.hbrBackground	= (HBRUSH) GetStockObject (WHITE_BRUSH);
		wndclass.lpszMenuName	= NULL;
		wndclass.lpszClassName	= szAppName;
		RegisterClass (&wndclass);

		cxClient = GetSystemMetrics (SM_CXSCREEN) * 3 / 4;
		cyClient = GetSystemMetrics (SM_CYSCREEN) * 3 / 4;

		if (cxClient * WORLDHEIGHT > cyClient * WORLDWIDTH) cxClient = cyClient * WORLDWIDTH / WORLDHEIGHT;
		else if (cxClient * WORLDHEIGHT < cyClient * WORLDWIDTH) cyClient = cxClient * WORLDHEIGHT / WORLDWIDTH;

		tc.hwnd = hwnd = CreateWindow (szAppName, szAppName,
						WS_OVERLAPPEDWINDOW,
						(GetSystemMetrics (SM_CXSCREEN) - cxClient) / 2,
						(GetSystemMetrics (SM_CYSCREEN) - cyClient) / 2,
						cxClient, cyClient + GetSystemMetrics (SM_CYCAPTION),
						NULL, NULL, hInstance, NULL);

		ShowWindow (hwnd, SW_NORMAL);
		UpdateWindow (hwnd);

		hCompThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) Compute, (LPVOID) &tc, 0, NULL);
		hDispThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) WDisplay, (LPVOID) &tc, 0, NULL);

		if (!hCompThread || !hDispThread)
			Err ("Creating Thread Failed");

		tc.hCompEvent = CreateEvent (NULL, FALSE, TRUE, NULL);
		tc.hDispEvent = CreateEvent (NULL, FALSE, FALSE, NULL);

		while (GetMessage (&msg, NULL, 0, 0))
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}

		CloseHandle (hInstance);
		break;

	default:
		printf ("Invalid Input\n");
		FreeAllResource (tc.lrtHead);
		//getch ();
		return 0;
	}

	WaitForSingleObject (hCompThread, INFINITE);  //Display Thread Exit
	WaitForSingleObject (hDispThread, INFINITE);  //Compute Thread Exit

	FreeAllResource (tc.lrtHead);  //Now, we can release the resource safely
	CloseHandle (tc.hCompEvent);
	CloseHandle (tc.hDispEvent);
	CloseHandle (hCompThread);
	CloseHandle (hDispThread);

	return 0;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;
	PAINTSTRUCT	ps;

	switch (message)
	{
	case WM_SIZE:
		g_cxMap = LOWORD (lParam);
		g_cyMap = HIWORD (lParam);

		if (g_cxMap * WORLDHEIGHT > g_cyMap * WORLDWIDTH) g_cxMap = g_cyMap * WORLDWIDTH / WORLDHEIGHT;
		else if (g_cxMap * WORLDHEIGHT < g_cyMap * WORLDWIDTH) g_cyMap = g_cxMap * WORLDHEIGHT / WORLDWIDTH;
		return 0;

	case WM_CHAR:
		switch (wParam)
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
			SendMessage (hwnd, WM_DESTROY, 0, 0);
			break;
		}
		return 0;

	case WM_PAINT:
		hdc = BeginPaint (hwnd, &ps);
#ifdef _DEBUG
		DrawLines (hdc);
#endif
		DrawAll (hdc, tc.lrtHead, tc.cFrame);  //draw again to avoid flash
		EndPaint (hwnd, &ps);
		return 0;

	case WM_DESTROY:
		tc.iRunningState = RS_QUIT;
		printf ("Exit\n");
		PostQuitMessage (0);
		return 0;
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
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

		case '\xD':
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
