//
// UI definition for Crazy Train Simulation
// By LJN-BOT Man, 2015
//

#include <windows.h>
#include <windowsx.h>
#include <stdarg.h>
#include <tchar.h>

#include "Scales.h"
#include "Element.h"
#include "GUI.h"

//========================================================================================
//Drawing Scales
//========================================================================================

#define DRAWWIDTH	100
#define DRAWHEIGHT	100

//========================================================================================
//Mouse Capture Function
//========================================================================================

void ClientToMap (POINT *pPt, LPARAM lParam)
{
	pPt->x = GET_X_LPARAM (lParam) * WORLDWIDTH / g_cxMap;
	pPt->y = GET_Y_LPARAM (lParam) * WORLDHEIGHT / g_cyMap;

	pPt->x = min (pPt->x, WORLDWIDTH - 1);
	pPt->x = max (0, pPt->x);
	pPt->y = min (pPt->y, WORLDHEIGHT - 1);
	pPt->y = max (0, pPt->y);
}

BOOL IsInMap (LPARAM lParam)
{
	if (GET_X_LPARAM (lParam) > g_cxMap ||
		GET_Y_LPARAM (lParam) > g_cyMap)
		return FALSE;
	else
		return TRUE;
}

Direction CorrectWayDirection (POINT ptBeg, POINT *ptEnd)
{
	if (ptEnd->x > ptBeg.x)
	{
		if (ptEnd->y > ptBeg.y)
		{
			if (ptEnd->y - ptBeg.y > ptEnd->x - ptBeg.x)
			{
				ptEnd->x = ptBeg.x;
				return SOUTH;
			}
			else
			{
				ptEnd->y = ptBeg.y;
				return EAST;
			}
		}
		else
		{
			if (ptBeg.y - ptEnd->y > ptEnd->x - ptBeg.x)
			{
				ptEnd->x = ptBeg.x;
				return NORTH;
			}
			else
			{
				ptEnd->y = ptBeg.y;
				return EAST;
			}
		}
	}
	else
	{
		if (ptEnd->y > ptBeg.y)
		{
			if (ptEnd->y - ptBeg.y > ptBeg.x - ptEnd->x)
			{
				ptEnd->x = ptBeg.x;
				return SOUTH;
			}
			else
			{
				ptEnd->y = ptBeg.y;
				return WEST;
			}
		}
		else
		{
			if (ptBeg.y - ptEnd->y > ptBeg.x - ptEnd->x)
			{
				ptEnd->x = ptBeg.x;
				return NORTH;
			}
			else
			{
				ptEnd->y = ptBeg.y;
				return WEST;
			}
		}
	}
}

//========================================================================================
//Print Function
//========================================================================================

void CDECL Print (HDC hdc, int x, int y, const TCHAR * szFormat, ...)
{
	int			cxChar, cxCaps, cyChar;
	TEXTMETRIC	tm;
	TCHAR		szBuffer[1024];
	va_list		pArgList;

	SaveDC (hdc);
	GetTextMetrics (hdc, &tm);
	cxChar = tm.tmAveCharWidth;
	cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
	cyChar = tm.tmHeight + tm.tmExternalLeading;

	va_start (pArgList, szFormat);
	_vsntprintf_s (szBuffer, sizeof (szBuffer) / sizeof (TCHAR),
							sizeof (szBuffer) / sizeof (TCHAR), szFormat, pArgList);
	va_end (pArgList);

	//SetBkMode (hdc, TRANSPARENT);
	TextOut (hdc, x, y, szBuffer, lstrlen (szBuffer));
	RestoreDC (hdc, -1);
}

//========================================================================================
//DrawAll Function
//========================================================================================

void DrawAll (HDC hdc, RouteList *lrtHead, int cFrame)
{
	RouteList *lrtCur;
	Route *pRoute;

	for (lrtCur = lrtHead; lrtCur != NULL; lrtCur = lrtCur->next)
		DrawRoute (hdc, lrtCur->elem, FALSE);

	for (lrtCur = lrtHead; lrtCur != NULL; lrtCur = lrtCur->next)
	{
		pRoute = lrtCur->elem;
		//Stations should be drawn upside
		DrawStation (hdc, pRoute->xBeg, pRoute->yBeg, TRUE, FALSE);
		if (pRoute->xBeg != pRoute->xEnd || pRoute->yBeg != pRoute->yEnd)
			DrawStation (hdc, pRoute->xEnd, pRoute->yEnd, FALSE, FALSE);

		if (pRoute->pTrain == NULL) continue;
		switch (pRoute->pTrain->state)
		{
		case TS_NDP:
		case TS_ARR:
			DrawTrain (hdc, pRoute->pTrain->x, pRoute->pTrain->y,
							EAST, -1, FALSE);  //offset == -1
			break;
		case TS_RUN:
		case TS_BRK:
			DrawTrain (hdc, pRoute->pTrain->x, pRoute->pTrain->y,
							pRoute->pTrain->lwyCur->elem->diTo,
							cFrame * pRoute->pTrain->cDist, FALSE);
			break;
		case TS_SUS:
			DrawTrain (hdc, pRoute->pTrain->x, pRoute->pTrain->y,
							pRoute->pTrain->lwyCur->elem->diTo,
							0, FALSE);  //static
			break;
		}
	}
}

//========================================================================================
//DrawRoute Function
//========================================================================================

void DrawRoute (HDC hdc, Route *pRoute, BOOL fRubber_Band_Line)
{
	int xBeg, yBeg, xEnd, yEnd;
	RailwayList *pCur, *pPre;

	for (pCur = pPre = pRoute->lwyHead; pCur != NULL; pCur = pCur->next)
	{
		if (pPre == pCur)
		{
			xBeg = pRoute->xBeg;
			yBeg = pRoute->yBeg;
		}
		else
		{
			xBeg = pPre->elem->x;
			yBeg = pPre->elem->y;
		}
		xEnd = pCur->elem->x;
		yEnd = pCur->elem->y;

		switch (pCur->elem->diTo)
		{
		case NORTH:
			if (yBeg - yEnd > 1) DrawWay (hdc, xEnd, yEnd + 1, xBeg, yBeg - 1,
												NORTH, fRubber_Band_Line);
			if (pPre != pCur && pPre->elem->diTo == EAST)
				DrawCorner (hdc, xBeg, yBeg, 2, fRubber_Band_Line);
			if (pPre != pCur && pPre->elem->diTo == WEST)
				DrawCorner (hdc, xBeg, yBeg, 1, fRubber_Band_Line);
			break;

		case SOUTH:
			if (yEnd - yBeg > 1) DrawWay (hdc, xBeg, yBeg + 1, xEnd, yEnd - 1,
												SOUTH, fRubber_Band_Line);
			if (pPre != pCur && pPre->elem->diTo == EAST)
				DrawCorner (hdc, xBeg, yBeg, 3, fRubber_Band_Line);
			if (pPre != pCur && pPre->elem->diTo == WEST)
				DrawCorner (hdc, xBeg, yBeg, 4, fRubber_Band_Line);
			break;

		case EAST:
			if (xEnd - xBeg > 1) DrawWay (hdc, xBeg + 1, yBeg, xEnd - 1, yEnd,
												EAST, fRubber_Band_Line);
			if (pPre != pCur && pPre->elem->diTo == NORTH)
				DrawCorner (hdc, xBeg, yBeg, 4, fRubber_Band_Line);
			if (pPre != pCur && pPre->elem->diTo == SOUTH)
				DrawCorner (hdc, xBeg, yBeg, 1, fRubber_Band_Line);
			break;

		case WEST:
			if (xBeg - xEnd > 1) DrawWay (hdc, xEnd + 1, yEnd, xBeg - 1, yBeg,
												WEST, fRubber_Band_Line);
			if (pPre != pCur && pPre->elem->diTo == NORTH)
				DrawCorner (hdc, xBeg, yBeg, 3, fRubber_Band_Line);
			if (pPre != pCur && pPre->elem->diTo == SOUTH)
				DrawCorner (hdc, xBeg, yBeg, 2, fRubber_Band_Line);
			break;
		}
		pPre = pCur;
	}
}

//========================================================================================
//DrawStation Function
//========================================================================================

void DrawStation (HDC hdc, int x, int y, BOOL fBeg, BOOL fRubber_Band_Line)
{
	HPEN hPen;

	SaveDC (hdc);
	SetMapMode (hdc, MM_ISOTROPIC);
	SetViewportOrgEx (hdc, x * g_cxMap / WORLDWIDTH, y * g_cyMap / WORLDHEIGHT, NULL);
	SetViewportExtEx (hdc, g_cxMap / WORLDWIDTH, g_cyMap / WORLDHEIGHT, NULL);
	SetWindowExtEx (hdc, DRAWWIDTH, DRAWHEIGHT, NULL);
	if (fRubber_Band_Line) SetROP2 (hdc, R2_NOT);

	if (fBeg) hPen = CreatePen (PS_SOLID, 13, RGB (0, 255, 0));
	else hPen = CreatePen (PS_SOLID, 13, RGB (255, 0, 0));
	SelectObject (hdc, hPen);
	SelectObject (hdc, GetStockObject (NULL_BRUSH));

	Rectangle (hdc, DRAWWIDTH / 5, 2 * DRAWHEIGHT / 5,
					4 * DRAWWIDTH / 5, 4 * DRAWHEIGHT / 5);

	MoveToEx (hdc, DRAWWIDTH / 2, 3 * DRAWHEIGHT / 50, NULL);
	LineTo (hdc, 3 * DRAWWIDTH / 50, 25 * DRAWHEIGHT / 50);

	MoveToEx (hdc, DRAWWIDTH / 2, 3 * DRAWHEIGHT / 50, NULL);
	LineTo (hdc, 47 * DRAWWIDTH / 50, 25 * DRAWHEIGHT / 50);

	DeleteObject (hPen);
	RestoreDC (hdc, -1);
}

//========================================================================================
//DrawWay Function
//========================================================================================

void DrawWay (HDC hdc, int xBeg, int yBeg, int xEnd, int yEnd,
						Direction dir, BOOL fRubber_Band_Line)
{
	HPEN hPen;
	int i;

	SaveDC (hdc);
	SetMapMode (hdc, MM_ISOTROPIC);
	SetViewportExtEx (hdc, g_cxMap, g_cyMap, NULL);  //Bug: big-integer division
	SetWindowExtEx (hdc, WORLDWIDTH * DRAWWIDTH, WORLDHEIGHT * DRAWHEIGHT, NULL);
	if (fRubber_Band_Line) SetROP2 (hdc, R2_NOT);

	hPen = CreatePen (PS_SOLID, 10, RGB (0, 0, 0));
	SelectObject (hdc, hPen);
	SelectObject (hdc, GetStockObject (BLACK_BRUSH));

	if (!fRubber_Band_Line)  //Final version
	{
		switch (dir)
		{
		case NORTH:
		case SOUTH:
			MoveToEx (hdc, xBeg * DRAWWIDTH + DRAWWIDTH / 5, yBeg * DRAWHEIGHT, NULL);
			LineTo (hdc, xBeg * DRAWWIDTH + DRAWWIDTH / 5, (yEnd + 1) * DRAWHEIGHT);
			MoveToEx (hdc, xBeg * DRAWWIDTH + 4 * DRAWWIDTH / 5, yBeg * DRAWHEIGHT, NULL);
			LineTo (hdc, xBeg * DRAWWIDTH + 4 * DRAWWIDTH / 5, (yEnd + 1) * DRAWHEIGHT);
			for (i = 0; i < (yEnd - yBeg + 1) * 3; i++)
			{
				MoveToEx (hdc, xBeg * DRAWWIDTH + DRAWWIDTH / 5,
					yBeg * DRAWHEIGHT + DRAWHEIGHT / 6 + i * DRAWHEIGHT / 3, NULL);
				LineTo (hdc, xBeg * DRAWWIDTH + 4 * DRAWWIDTH / 5,
					yBeg * DRAWHEIGHT + DRAWHEIGHT / 6 + i * DRAWHEIGHT / 3);
			}
			break;

		case EAST:
		case WEST:
			MoveToEx (hdc, xBeg * DRAWWIDTH, yBeg * DRAWHEIGHT + DRAWHEIGHT / 5, NULL);
			LineTo (hdc, (xEnd + 1) * DRAWWIDTH, yEnd * DRAWHEIGHT + DRAWHEIGHT / 5);
			MoveToEx (hdc, xBeg * DRAWWIDTH, yBeg * DRAWHEIGHT + 4 * DRAWHEIGHT / 5, NULL);
			LineTo (hdc, (xEnd + 1) * DRAWWIDTH, yEnd * DRAWHEIGHT + 4 * DRAWHEIGHT / 5);
			for (i = 0; i < (xEnd - xBeg + 1) * 3; i++)
			{
				MoveToEx (hdc, xBeg * DRAWWIDTH + DRAWWIDTH / 6 + i * DRAWWIDTH / 3,
					yEnd * DRAWHEIGHT + DRAWHEIGHT / 5, NULL);
				LineTo (hdc, xBeg * DRAWWIDTH + DRAWWIDTH / 6 + i * DRAWWIDTH / 3,
					yEnd * DRAWHEIGHT + 4 * DRAWHEIGHT / 5);
			}
			break;
		}
	}
	else  //Temp version
	{
		switch (dir)
		{
		case NORTH:
		case SOUTH:
			MoveToEx (hdc, (xBeg + xEnd + 1) * DRAWWIDTH / 2, yBeg * DRAWHEIGHT, NULL);
			LineTo (hdc, (xBeg + xEnd + 1) * DRAWWIDTH / 2, (yEnd + 1) * DRAWHEIGHT);
			break;

		case EAST:
		case WEST:
			MoveToEx (hdc, xBeg * DRAWWIDTH, (yBeg + yEnd + 1) * DRAWHEIGHT / 2, NULL);
			LineTo (hdc, (xEnd + 1) * DRAWWIDTH, (yBeg + yEnd + 1) * DRAWHEIGHT / 2);
			break;
		}
	}

	DeleteObject (hPen);
	RestoreDC (hdc, -1);
}

//========================================================================================
//DrawCorner Function
//========================================================================================

void DrawCorner (HDC hdc, int x, int y, int type, BOOL fRubber_Band_Line)
{	
	HPEN hPen;
	HRGN hRgnClip;  //, hRgnTmp;

	SaveDC (hdc);
	SetMapMode (hdc, MM_ISOTROPIC);
	SetViewportOrgEx (hdc, x * g_cxMap / WORLDWIDTH, y * g_cyMap / WORLDHEIGHT, NULL);
	SetViewportExtEx (hdc, g_cxMap / WORLDWIDTH, g_cyMap / WORLDHEIGHT, NULL);
	SetWindowExtEx (hdc, DRAWWIDTH, DRAWHEIGHT, NULL);
	if (fRubber_Band_Line) SetROP2 (hdc, R2_NOT);

	hPen = CreatePen (PS_SOLID, 10, RGB (0, 0, 0));
	SelectObject (hdc, hPen);
	SelectObject (hdc, GetStockObject (NULL_BRUSH));

	hRgnClip = CreateRectRgn (x * g_cxMap / WORLDWIDTH, y * g_cyMap / WORLDHEIGHT,
							(x + 1) * g_cxMap / WORLDWIDTH, (y + 1) * g_cyMap / WORLDHEIGHT);
	SelectClipRgn (hdc, hRgnClip);

	switch (type)
	{
	case 1:
		Ellipse (hdc, DRAWHEIGHT / 5, - 4 * DRAWHEIGHT / 5, 8 * DRAWWIDTH / 5, 4 * DRAWHEIGHT / 5);
		Ellipse (hdc, 4 * DRAWWIDTH / 5, - DRAWHEIGHT / 5, 6 * DRAWWIDTH / 5, DRAWHEIGHT / 5);
		//hRgnTmp = CreateEllipticRgn ();  //Use PENWIDTH to calc the circle
		//MoveToEx (hdc, DRAWWIDTH, 0, NULL);
		//LineTo (hdc, 0, DRAWHEIGHT);
		break;

	case 2:
		Ellipse (hdc, - 4 * DRAWWIDTH / 5, - 4 * DRAWHEIGHT / 5, 4 * DRAWWIDTH / 5, 4 * DRAWHEIGHT / 5);
		Ellipse (hdc, - DRAWWIDTH / 5, - DRAWHEIGHT / 5, DRAWWIDTH / 5, DRAWHEIGHT / 5);
		break;

	case 3:
		Ellipse (hdc, - 4 * DRAWWIDTH / 5, DRAWHEIGHT / 5, 4 * DRAWWIDTH / 5, 8 * DRAWHEIGHT / 5);
		Ellipse (hdc, - DRAWWIDTH / 5, 4 * DRAWHEIGHT / 5, DRAWWIDTH / 5, 6 * DRAWHEIGHT / 5);
		break;

	case 4:
		Ellipse (hdc, DRAWHEIGHT / 5, DRAWHEIGHT / 5, 8 * DRAWWIDTH / 5, 8 * DRAWHEIGHT / 5);
		Ellipse (hdc, 4 * DRAWHEIGHT / 5, 4 * DRAWHEIGHT / 5, 6 * DRAWWIDTH / 5, 6 * DRAWHEIGHT / 5);
		break;
	}
/*
	switch (type)
	{
	case 1:
		MoveToEx (hdc, DRAWWIDTH / 5, 0, NULL);
		LineTo (hdc, DRAWWIDTH, 4 * DRAWHEIGHT / 5);
		MoveToEx (hdc, 4 * DRAWWIDTH / 5, 0, NULL);
		LineTo (hdc, DRAWWIDTH, DRAWHEIGHT / 5);
		break;

	case 2:
		MoveToEx (hdc, DRAWWIDTH / 5, 0, NULL);
		LineTo (hdc, 0, DRAWHEIGHT / 5);
		MoveToEx (hdc, 4 * DRAWWIDTH / 5, 0, NULL);
		LineTo (hdc, 0, 4 * DRAWHEIGHT / 5);
		break;

	case 3:
		MoveToEx (hdc, 0, DRAWHEIGHT / 5, NULL);
		LineTo (hdc, 4 * DRAWWIDTH / 5, DRAWHEIGHT);
		MoveToEx (hdc, 0, 4 * DRAWHEIGHT / 5, NULL);
		LineTo (hdc, DRAWWIDTH / 5, DRAWHEIGHT);
		break;

	case 4:
		MoveToEx (hdc, DRAWWIDTH, DRAWHEIGHT / 5, NULL);
		LineTo (hdc, DRAWWIDTH / 5, DRAWHEIGHT);
		MoveToEx (hdc, DRAWWIDTH, 4 * DRAWHEIGHT / 5, NULL);
		LineTo (hdc, 4 * DRAWWIDTH / 5, DRAWHEIGHT);
		break;
	}
*/
	DeleteObject (hPen);
	DeleteObject (hRgnClip);
	//DeleteObject (hRgnTmp);
	RestoreDC (hdc, -1);
}

//========================================================================================
//DrawTrain Function
//========================================================================================

void DrawTrain (HDC hdc, int x, int y, Direction dir, int cOffset, BOOL fRubber_Band_Line)
{
	HPEN hPen;
	HBRUSH hBrush;
	int xOrg, yOrg;

	if (cOffset != -1)
	{
		hPen = CreatePen (PS_SOLID, 15, RGB (128, 64, 0));
		hBrush = CreateSolidBrush (RGB (157, 80, 0));
	}
	else
	{
		hPen = CreatePen (PS_DASH, 15, RGB (180, 128, 128));
		hBrush = CreateSolidBrush (RGB (170, 160, 150));
		cOffset = 0;
	}

	switch (dir)
	{
	case NORTH:
		xOrg = x * g_cxMap / WORLDWIDTH;
		yOrg = (y * g_cyMap - cOffset * g_cyMap / FPS) / WORLDHEIGHT;
		break;

	case SOUTH:
		xOrg = x * g_cxMap / WORLDWIDTH;
		yOrg = (y * g_cyMap + cOffset * g_cyMap / FPS) / WORLDHEIGHT;
		break;

	case EAST:
		xOrg = (x * g_cxMap + cOffset * g_cxMap / FPS) / WORLDWIDTH;
		yOrg = y * g_cyMap / WORLDHEIGHT;
		break;

	case WEST:
		xOrg = (x * g_cxMap - cOffset * g_cxMap / FPS) / WORLDWIDTH;
		yOrg = y * g_cyMap / WORLDHEIGHT;
		break;
	}

	SaveDC (hdc);
	SetMapMode (hdc, MM_ISOTROPIC);
	SetViewportOrgEx (hdc, xOrg, yOrg, NULL);
	SetViewportExtEx (hdc, g_cxMap / WORLDWIDTH, g_cyMap / WORLDHEIGHT, NULL);
	SetWindowExtEx (hdc, DRAWWIDTH, DRAWHEIGHT, NULL);
	if (fRubber_Band_Line) SetROP2 (hdc, R2_NOT);

	SelectObject (hdc, hPen);
	SelectObject (hdc, hBrush);

	switch (dir)
	{
	case NORTH:
	case SOUTH:
		//only Body
		RoundRect (hdc, DRAWWIDTH / 5, DRAWHEIGHT / 16,
						4 * DRAWWIDTH / 5, 15 * DRAWHEIGHT / 16,
						5 * DRAWWIDTH / 20, 5 * DRAWHEIGHT / 20);
		Rectangle (hdc, 4 * DRAWWIDTH / 10, DRAWHEIGHT / 3,
						6 * DRAWWIDTH / 10, 2 * DRAWHEIGHT / 3);
		break;

	case EAST:
	case WEST:
		//Body
		Rectangle (hdc, DRAWWIDTH / 8, DRAWHEIGHT / 16,
						7 * DRAWWIDTH / 8, DRAWHEIGHT / 8);
		RoundRect (hdc, 0, DRAWHEIGHT / 8,
						DRAWWIDTH, 5 * DRAWHEIGHT / 8,
						3 * DRAWWIDTH / 20, DRAWHEIGHT / 10);
		//Tires
		Ellipse (hdc, DRAWWIDTH / 8, 5 * DRAWHEIGHT / 8,
						3 * DRAWWIDTH / 8, 7 * DRAWHEIGHT / 8);
		Ellipse (hdc, 5 * DRAWWIDTH / 8, 5 * DRAWHEIGHT / 8,
						7 * DRAWWIDTH / 8, 7 * DRAWHEIGHT / 8);
		break;
	}

	//Head
	SelectObject (hdc, GetStockObject (BLACK_BRUSH));
	switch (dir)
	{
	case EAST:
		Rectangle (hdc, 6 * DRAWWIDTH / 8, - 3 * DRAWHEIGHT / 16,
						7 * DRAWWIDTH / 8, DRAWHEIGHT / 8);
		break;

	case WEST:
		Rectangle (hdc, DRAWWIDTH / 8, - 3 * DRAWHEIGHT / 16,
						2 * DRAWWIDTH / 8, DRAWHEIGHT / 8);
		break;
	}

	DeleteObject (hPen);
	DeleteObject (hBrush);
	RestoreDC (hdc, -1);
}

//========================================================================================
//DrawStop Function
//========================================================================================

void DrawStop (HDC hdc, int x, int y, BOOL fRubber_Band_Line)
{
	HPEN hPen;

	SaveDC (hdc);
	SetMapMode (hdc, MM_ISOTROPIC);
	SetViewportOrgEx (hdc, x * g_cxMap / WORLDWIDTH, y * g_cyMap / WORLDHEIGHT, NULL);
	SetViewportExtEx (hdc, g_cxMap / WORLDWIDTH, g_cyMap / WORLDHEIGHT, NULL);
	SetWindowExtEx (hdc, DRAWWIDTH, DRAWHEIGHT, NULL);
	if (fRubber_Band_Line) SetROP2 (hdc, R2_NOT);

	hPen = CreatePen (PS_SOLID, 12, RGB (0, 0, 0));
	SelectObject (hdc, hPen);
	SelectObject (hdc, GetStockObject (BLACK_BRUSH));

	Ellipse (hdc, DRAWWIDTH / 4, DRAWHEIGHT / 16,
					3 * DRAWWIDTH / 4, 9 * DRAWHEIGHT / 16);

	MoveToEx (hdc, DRAWWIDTH / 2, 9 * DRAWHEIGHT / 16, NULL);
	LineTo (hdc, DRAWWIDTH / 2, 15 * DRAWHEIGHT / 16);

	MoveToEx (hdc, DRAWWIDTH / 4, DRAWHEIGHT, NULL);
	LineTo (hdc, 3 * DRAWWIDTH / 4, DRAWHEIGHT);

	DeleteObject (hPen);
	RestoreDC (hdc, -1);
}

//========================================================================================
//DrawLines Function (For debug)
//========================================================================================

void DrawLines (HDC hdc)
{
	int i;

	SaveDC (hdc);
	SetMapMode (hdc, MM_ISOTROPIC);
	SetViewportExtEx (hdc, g_cxMap, g_cyMap, NULL);
	SetWindowExtEx (hdc, WORLDWIDTH, WORLDHEIGHT, NULL);

	for (i = 1; i <= WORLDWIDTH; i++)
	{
		MoveToEx (hdc, i, 0, NULL);
		LineTo (hdc, i, WORLDHEIGHT);
	}
	for (i = 1; i <= WORLDHEIGHT; i++)
	{
		MoveToEx (hdc, 0, i, NULL);
		LineTo (hdc, WORLDWIDTH, i);
	}

	RestoreDC (hdc, -1);
}

//========================================================================================
//DrawSignal Function (For debug)
//========================================================================================

void DrawSignal (HDC hdc, int xBeg, int yBeg, int xEnd, int yEnd, int State)
{
	HPEN hPen;

	SaveDC (hdc);
	SetMapMode (hdc, MM_ISOTROPIC);
	SetViewportExtEx (hdc, g_cxMap, g_cyMap, NULL);  //Bug: big-integer division
	SetWindowExtEx (hdc, WORLDWIDTH * DRAWWIDTH, WORLDHEIGHT * DRAWHEIGHT, NULL);

	switch (State)
	{
	case 0: hPen = CreatePen (PS_SOLID, 10, RGB (0, 255, 0)); break;  //Free
	case 1: hPen = CreatePen (PS_SOLID, 10, RGB (0, 0, 255)); break;  //Waiting
	case 2: hPen = CreatePen (PS_SOLID, 10, RGB (255, 0, 0)); break;  //Busy
	}

	SelectObject (hdc, hPen);
	SelectObject (hdc, GetStockObject (NULL_BRUSH));

	Rectangle (hdc, xBeg * DRAWWIDTH + DRAWWIDTH / 3,
					yBeg * DRAWHEIGHT + DRAWHEIGHT / 3,
					(xEnd + 1) * DRAWWIDTH - DRAWWIDTH / 3,
					(yEnd + 1) * DRAWHEIGHT - DRAWHEIGHT / 3);

	DeleteObject (hPen);
	RestoreDC (hdc, -1);
}
