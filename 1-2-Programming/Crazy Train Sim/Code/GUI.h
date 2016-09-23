//
// UI declaration for Crazy Train Simulation
// By LJN-BOT Man, 2015
//

#ifndef CTS_UI
#define CTS_UI

#include <windows.h>

#include "Element.h"

extern int g_cxMap, g_cyMap;

// For EditMain.c
void ClientToMap (POINT *pPt, LPARAM lParam);
BOOL IsInMap (LPARAM lParam);
Direction CorrectWayDirection (POINT ptBeg, POINT *ptEnd);

void CDECL Print (HDC hdc, int x, int y, const TCHAR * szFormat, ...);

void DrawAll (HDC hdc, RouteList *lrtHead, int cFrame);
void DrawRoute (HDC hdc, Route *pRoute, BOOL fRubber_Band_Line);

void DrawStation (HDC hdc, int x, int y, BOOL fBeg, BOOL fRubber_Band_Line);
void DrawCorner (HDC hdc, int x, int y, int type, BOOL fRubber_Band_Line);
void DrawWay (HDC hdc, int xBeg, int yBeg, int xEnd, int yEnd, Direction dir, BOOL fRubber_Band_Line);

void DrawTrain (HDC hdc, int x, int y, Direction dir, int cOffset, BOOL fRubber_Band_Line);
//void DrawStop (HDC hdc, int x, int y, BOOL fRubber_Band_Line);

//For debug
void DrawLines (HDC hdc);
void DrawSignal (HDC hdc, int xBeg, int yBeg, int xEnd, int yEnd, int State);

#endif  //CTS_UI
