//
// Basic functions declaration for Crazy Train Simulation
// By LJN-BOT Man, 2015
//

#ifndef CTS_BASIC
#define CTS_BASIC

#include <windows.h>

#include "Element.h"

//Is the Second Version
#define CTS_WINDOW

//Running State----------------------------------------------------------------------

#define RS_NOTBEG	0  //Not Begin
#define RS_RUNNING	1  //Running
#define RS_PAUSE	2  //Pause
#define RS_QUIT		3  //Quit

//Thread Control Data----------------------------------------------------------------------

typedef struct _ThreadCtrl
{
	HANDLE hCompEvent;		// To Sync Thread
	HANDLE hDispEvent;		// To Sync Thread
	int cTime;				// Global Time
	int iStrategy;			// Current Strategy
	int iRunningState;		// Application Running State
	RouteList *lrtHead;		// User Resource

	// For Window
	int cFrame;				// Current Frame
	HWND hwnd;				// Window Handle
} ThreadCtrl;

//Thread Functions----------------------------------------------------------------------

void WINAPI Compute (ThreadCtrl *ptc);		// Compute Thread
void WINAPI CDisplay (ThreadCtrl *ptc);		// Console Display Thread
void WINAPI WDisplay (ThreadCtrl *ptc);		// Window Display Thread

//Other Functions----------------------------------------------------------------------

int Init (ThreadCtrl *ptc, const char *szConf);		// Initialization
void Err (const char *str);							// Fatal Error Report

#endif  //CTS_BASIC
