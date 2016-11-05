//
// Edit Mode for Crazy Train Simulation
// By LJN-BOT Man, 2015
//

#include <windows.h>

#include "Scales.h"
#include "Element.h"
#include "GUI.h"
#include "File.h"

#define ID_ROUTE 1
#define ID_TRAIN 2
#define ID_SPEED 3
#define ID_STIME 4
#define ID_SAVE  5

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

typedef enum _Drawtype { STATION = 1, ROUTE = 2, STOP = 4, TRAIN = 8 } Drawtype;
int		g_cxMap, g_cyMap;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT ("Crazy Train Simulation (Edit Mode)");
	HWND		hwnd;
	MSG			msg;
	WNDCLASS	wndclass;
	int			cxClient, cyClient;

#ifdef _DEBUG
	InitMLTracer ();
#endif

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

	if (cxClient * WORLDHEIGHT > cyClient * WORLDWIDTH)
		cxClient = cyClient * WORLDWIDTH / WORLDHEIGHT;
	else if (cxClient * WORLDHEIGHT < cyClient * WORLDWIDTH)
		cyClient = cxClient * WORLDHEIGHT / WORLDWIDTH;

	hwnd = CreateWindow (szAppName, szAppName,
					WS_OVERLAPPEDWINDOW,
					(GetSystemMetrics (SM_CXSCREEN) - cxClient) / 2,
					(GetSystemMetrics (SM_CYSCREEN) - cyClient) / 2,
					cxClient + GetSystemMetrics (SM_CYFRAME) * 8, cyClient + GetSystemMetrics (SM_CYCAPTION),
					NULL, NULL, hInstance, NULL);

	ShowWindow (hwnd, iCmdShow);
	UpdateWindow (hwnd);

	while (GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC				hdc;
	PAINTSTRUCT		ps;
	TEXTMETRIC		tm;
	static int		cxChar, cxCaps, cyChar;
	static BOOL		fBlocking, fEdited = FALSE;
	static POINT	ptBeg, ptEnd, ptTmp;
	static TCHAR	szHint[1024], szBuf[2];

	static HWND hwndRoute;
	static HWND hwndTrain;
	static HWND hwndSave;
	static HWND hwndSpeed;
	static HWND hwndStime;

	static Drawtype drType = ROUTE;
	static Direction diTmp, diCur = EAST;
	static int cStime = 0, cSpeed = 1;

	static RouteList *lrtHead = NULL, *lrtCur, *lrtTmp;
	static Route *prtCur;

	switch (message)
	{
	case WM_CREATE:
		hdc = GetDC (hwnd);

		GetTextMetrics (hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;

		ReleaseDC (hwnd, hdc);

		hwndRoute = CreateWindow (TEXT ("button"), TEXT ("Draw Route"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
						0, 0, 0, 0, hwnd, (HMENU) ID_ROUTE, ((LPCREATESTRUCT) lParam)->hInstance, NULL);
		hwndTrain = CreateWindow (TEXT ("button"), TEXT ("Draw Train"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
						0, 0, 0, 0, hwnd, (HMENU) ID_TRAIN, ((LPCREATESTRUCT) lParam)->hInstance, NULL);
		hwndSave = CreateWindow (TEXT ("button"), TEXT ("Save"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
						0, 0, 0, 0, hwnd, (HMENU) ID_SAVE, ((LPCREATESTRUCT) lParam)->hInstance, NULL);
		hwndSpeed = CreateWindow (TEXT ("edit"), TEXT (""), WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER,
						0, 0, 0, 0, hwnd, (HMENU) ID_SPEED, ((LPCREATESTRUCT) lParam)->hInstance, NULL);
		SetWindowText (hwndSpeed, TEXT ("1"));
		hwndStime = CreateWindow (TEXT ("edit"), TEXT (""), WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER,
						0, 0, 0, 0, hwnd, (HMENU) ID_STIME, ((LPCREATESTRUCT) lParam)->hInstance, NULL);
		SetWindowText (hwndStime, TEXT ("0"));
		SendMessage (hwndRoute, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage (hwndSpeed, EM_LIMITTEXT, 1, 0);
		SendMessage (hwndStime, EM_LIMITTEXT, 1, 0);

		return 0;

	case WM_SIZE:
		g_cxMap = LOWORD (lParam) - 2 * GetSystemMetrics (SM_CYFRAME) - cxChar * 15;
		g_cyMap = HIWORD (lParam) - cyChar - 1;

		if (g_cxMap * WORLDHEIGHT > g_cyMap * WORLDWIDTH) g_cxMap = g_cyMap * WORLDWIDTH / WORLDHEIGHT;
		else if (g_cxMap * WORLDHEIGHT < g_cyMap * WORLDWIDTH) g_cyMap = g_cxMap * WORLDHEIGHT / WORLDWIDTH;

		MoveWindow (hwndRoute, g_cxMap + GetSystemMetrics (SM_CYFRAME), cyChar * 5, cxChar * 15, cyChar, TRUE);
		MoveWindow (hwndTrain, g_cxMap + GetSystemMetrics (SM_CYFRAME), cyChar * 6 + 5, cxChar * 15, cyChar, TRUE);
		MoveWindow (hwndSpeed, g_cxMap + GetSystemMetrics (SM_CYFRAME), cyChar * 11, cxChar * 3, cyChar + 5, TRUE);
		MoveWindow (hwndStime, g_cxMap + GetSystemMetrics (SM_CYFRAME), cyChar * 13, cxChar * 3, cyChar + 5, TRUE);
		MoveWindow (hwndSave, g_cxMap + GetSystemMetrics (SM_CYFRAME), cyChar * 15, cxChar * 15, cyChar + 3, TRUE);

		szHint[0] = TEXT ('\0');  //Clear the hint buffer
		return 0;

	case WM_LBUTTONDOWN:
		SetFocus (hwnd);
		if (!IsInMap (lParam)) return 0;

		ClientToMap (&ptBeg, lParam);
		ClientToMap (&ptEnd, lParam);

		hdc = GetDC (hwnd);
		switch (drType)
		{
		case ROUTE:
			SetCursor (LoadCursor (NULL, IDC_CROSS));
			SetCapture (hwnd);
			fBlocking = TRUE;

			DrawStation (hdc, ptEnd.x, ptEnd.y, TRUE, TRUE);
			break;

		case TRAIN:
			SetCursor (LoadCursor (NULL, IDC_SIZEALL));
			SetCapture (hwnd);
			fBlocking = TRUE;

			DrawTrain (hdc, ptEnd.x, ptEnd.y, EAST, 0, TRUE);
			break;

		/*
		case STOP:
			SetCursor (LoadCursor (NULL, IDC_SIZEALL));
			SetCapture (hwnd);
			fBlocking = TRUE;

			DrawStop (hdc, ptEnd.x, ptEnd.y, TRUE);
			break;
		*/
		default:
			SetCursor (LoadCursor (NULL, IDC_ARROW));
		}

		ReleaseDC (hwnd, hdc);
		return 0;

	case WM_MOUSEMOVE:
		if (!fBlocking) return 0;

		ptTmp = ptEnd;
		ClientToMap (&ptEnd, lParam);
		hdc = GetDC (hwnd);

		switch (drType)
		{
		case ROUTE:
			diTmp = diCur;
			diCur = CorrectWayDirection (ptBeg, &ptEnd);

			if ((ptTmp.x != ptBeg.x) || (ptTmp.y != ptBeg.y)) DrawStation (hdc, ptTmp.x, ptTmp.y, FALSE, TRUE);  //Use R2_NOT to erase the old Graph
			if ((ptEnd.x != ptBeg.x) || (ptEnd.y != ptBeg.y)) DrawStation (hdc, ptEnd.x, ptEnd.y, FALSE, TRUE);
			DrawWay (hdc, ptBeg.x, ptBeg.y, ptTmp.x, ptTmp.y, diTmp, TRUE);  //Use R2_NOT to erase the old Graph
			DrawWay (hdc, ptBeg.x, ptBeg.y, ptEnd.x, ptEnd.y, diCur, TRUE);

			Print (hdc, g_cxMap / 2, g_cyMap + 1, TEXT ("Current Pos %4d %4d  "), ptEnd.x, ptEnd.y);
			break;

		case TRAIN:
			DrawTrain (hdc, ptTmp.x, ptTmp.y, EAST, 0, TRUE);  //Use R2_NOT to erase the old Graph
			DrawTrain (hdc, ptEnd.x, ptEnd.y, EAST, 0, TRUE);

			Print (hdc, g_cxMap / 2, g_cyMap + 1, TEXT ("New Train at %4d %4d  "), ptEnd.x, ptEnd.y);
			break;

		/*
		case STOP:
			DrawStop (hdc, ptTmp.x, ptTmp.y, TRUE);  //Use R2_NOT to erase the old Graph
			DrawStop (hdc, ptEnd.x, ptEnd.y, TRUE);

			Print (hdc, g_cxMap / 2, g_cyMap + 1, TEXT ("New Stop at %4d %4d  "), ptEnd.x, ptEnd.y);
			break;
		*/
		default:
			SetCursor (LoadCursor (NULL, IDC_ARROW));
		}

		ReleaseDC (hwnd, hdc);
		return 0;

	case WM_LBUTTONUP:
		if (!fBlocking) return 0;

		ClientToMap (&ptEnd, lParam);
		hdc = GetDC (hwnd);

		switch (drType)
		{
		case ROUTE:
			diTmp = CorrectWayDirection (ptBeg, &ptEnd);

			DrawStation (hdc, ptBeg.x, ptBeg.y, TRUE, TRUE);  //Use R2_NOT to erase the old Graph
			DrawStation (hdc, ptEnd.x, ptEnd.y, FALSE, TRUE);
			DrawWay (hdc, ptBeg.x, ptBeg.y, ptEnd.x, ptEnd.y, diTmp, TRUE);

			if (ptBeg.x == ptEnd.x && ptBeg.y == ptEnd.y)  //double click
			{
				lstrcpy (szHint, TEXT ("Beg and End Station can't be at the same position."));
				break;
			}

			for (lrtCur = lrtHead; lrtCur != NULL; lrtCur = lrtCur->next)
			{
				//Case 1.beg == beg
				if (lrtCur->elem->xBeg == ptBeg.x && lrtCur->elem->yBeg == ptBeg.y)
				{
					lstrcpy (szHint, TEXT ("The Beg Station can not be established."));
					break;
				}

				//Case 2.end == beg
				else if (lrtCur->elem->xEnd == ptBeg.x && lrtCur->elem->yEnd == ptBeg.y)
				{
					prtCur = lrtCur->elem;

					//Check !Case 3.
					for (lrtTmp = lrtCur->next; lrtTmp != NULL; lrtTmp = lrtTmp->next)
						if (lrtTmp->elem->xBeg == ptEnd.x && lrtTmp->elem->yBeg == ptEnd.y)
							break;
					if (lrtTmp != NULL)
					{
						lstrcpy (szHint, TEXT ("The End Station can not be established."));
						break;  //break before scanning every route, i.e. beg == end (Case 3)
					}

					switch (AddNewRailway (prtCur, ptEnd.x, ptEnd.y, diTmp))
					{
					case 1:
						if (prtCur->xBeg == ptEnd.x && prtCur->yBeg == ptEnd.y)  //beg == end itself
							lstrcpy (szHint, TEXT ("The Route has been turned into a Circle."));
						else  //normal merge
							lstrcpy (szHint, TEXT ("The Railway has been merged successfully."));

						fEdited = TRUE;
						break;

					case -1:
						lstrcpy (szHint, TEXT ("Oppsite direction is not allowed."));
						break;

					case 0:
						lstrcpy (szHint, TEXT ("Out of memory..."));
						break;
					}
					break;
				}

				//Case 3.beg == end
				else if (lrtCur->elem->xBeg == ptEnd.x && lrtCur->elem->yBeg == ptEnd.y)
				{
					lstrcpy (szHint, TEXT ("The End Station can not be established."));
					break;
				}

				//Case 4.end == end
				//Case 5.free space
				else continue;
			}

			//Case 6.new route
			if (lrtCur == NULL)
			{
				prtCur = NewRoute (ptBeg.x, ptBeg.y);
				if (!AddRoute (&lrtHead, prtCur))
				{
					if (prtCur) free (prtCur);
					lstrcpy (szHint, TEXT ("Out of memory..."));
					break;
				}

				switch (AddNewRailway (prtCur, ptEnd.x, ptEnd.y, diTmp))
				{
				case 1:
					fEdited = TRUE;
					lstrcpy (szHint, TEXT ("The new Route has been established successfully."));
					break;

				case 0:
					lstrcpy (szHint, TEXT ("Out of memory..."));
					break;

				case -1:
					MessageBox (NULL, TEXT ("Something unexpected happened..."),
						TEXT ("Crazy Train Simulation (Edit Mode)"), MB_ICONERROR);
					break;
				}
			}
			break;

		case TRAIN:
			DrawTrain (hdc, ptEnd.x, ptEnd.y, EAST, 0, TRUE);  //Use R2_NOT to erase the old Graph

			for (lrtCur = lrtHead; lrtCur != NULL; lrtCur = lrtCur->next)
				if (lrtCur->elem->xBeg == ptEnd.x && lrtCur->elem->yBeg == ptEnd.y)
				{
					if (lrtCur->elem->pTrain)
						lstrcpy (szHint, TEXT ("One Route for one Train in this version..."));
					else
					{
						GetWindowText (hwndSpeed, szBuf, 2);
						if (lstrlen (szBuf) == 0)
						{
							SetWindowText (hwndSpeed, TEXT ("1"));
							cSpeed = 1;
						}
						GetWindowText (hwndStime, szBuf, 2);
						if (lstrlen (szBuf) == 0)
						{
							SetWindowText (hwndStime, TEXT ("0"));
							cStime = 0;
						}

						if (AddNewTrain (lrtCur->elem, cStime, cSpeed))
						{
							fEdited = TRUE;
							lstrcpy (szHint, TEXT ("New Train has been built successfully."));
						}
						else
							lstrcpy (szHint, TEXT ("Out of memory..."));
					}
					break;
				}

			if (lrtCur == NULL) lstrcpy (szHint, TEXT ("The Train should be at a Beg Station."));
			break;

		/*
		case STOP:
			DrawStop (hdc, ptEnd.x, ptEnd.y, TRUE);  //Use R2_NOT to erase the old Graph

			lstrcpy (szHint, TEXT ("This function would be supported later..."));
			//Should be on railway, but couldn't be at station
			break;
		*/
		default:
			MessageBox (NULL, TEXT ("Something unexpected happened..."),
				TEXT ("Crazy Train Simulation (Edit Mode)"), MB_ICONERROR);
		}

		ReleaseDC (hwnd, hdc);
		SetCursor (LoadCursor (NULL, IDC_ARROW));
		ReleaseCapture ();

		fBlocking = FALSE;
		InvalidateRect (hwnd, NULL, TRUE);
		return 0;

	case WM_CHAR:
		switch (wParam)
		{
		case '\x1B':
			if (fBlocking) // i.e., Escape
			{
				ReleaseCapture ();
				fBlocking = FALSE;
			}
			break;
		}
		return 0;

	case WM_COMMAND:
		switch (LOWORD (wParam))
		{
		case ID_ROUTE:
			drType = ROUTE;
			break;

		case ID_TRAIN:
			drType = TRAIN;
			break;

		case ID_SAVE:
			if (WriteConf (lrtHead))
			{
				fEdited = FALSE;
				hdc = GetDC (hwnd);
				SelectObject (hdc, (HBRUSH) GetStockObject (WHITE_BRUSH));
				SelectObject (hdc, (HPEN) GetStockObject (NULL_PEN));
				Rectangle (hdc, -1, g_cyMap + 1, g_cxMap + 1,  g_cyMap + cyChar + 5);  //Clear the bkg.
				Print (hdc, 0, g_cyMap + 1, TEXT ("Map has been saved Successfully."));
				ReleaseDC (hwnd, hdc);
			}
			else
				MessageBox (NULL, TEXT ("Can not save the Map..."), TEXT ("Error"), MB_ICONERROR);
			break;

		case ID_SPEED:
			if (HIWORD (wParam) == EN_CHANGE)
			{
				GetWindowText (hwndSpeed, szBuf, 2);

#ifdef UNICODE
				cSpeed = _wtoi (szBuf);
#else
				cSpeed = atoi (szBuf);
#endif
				if (((*szBuf) < TEXT('1') || (*szBuf) > TEXT('9')) && lstrlen (szBuf) != 0)
				{
					SetWindowText (hwndSpeed, TEXT ("1"));
					cSpeed = 1;
				}
			}
			break;

		case ID_STIME:
			if (HIWORD (wParam) == EN_CHANGE)
			{
				GetWindowText (hwndStime, szBuf, 2);

#ifdef UNICODE
				cStime = _wtoi (szBuf);
#else
				cStime = atoi (szBuf);
#endif
				if (((*szBuf) < TEXT('0') || (*szBuf) > TEXT('9')) && lstrlen (szBuf) != 0)
				{
					SetWindowText (hwndStime, TEXT ("0"));
					cStime = 0;
				}
			}
			break;
		}
		return 0;

	case WM_PAINT:
		hdc = BeginPaint (hwnd, &ps);

		TextOut (hdc, g_cxMap + GetSystemMetrics (SM_CYFRAME), cyChar * 3 + 2,
						TEXT ("Draw Type:"), lstrlen (TEXT ("Draw Type:")));
		TextOut (hdc, g_cxMap + GetSystemMetrics (SM_CYFRAME), cyChar * 9 + 2,
						TEXT ("Train Options:"), lstrlen (TEXT ("Train Options:")));
		TextOut (hdc, g_cxMap + GetSystemMetrics (SM_CYFRAME) + cxChar * 4, cyChar * 11 + 2,
						TEXT ("Speed"), lstrlen (TEXT ("Speed")));
		TextOut (hdc, g_cxMap + GetSystemMetrics (SM_CYFRAME) + cxChar * 4, cyChar * 13 + 2,
						TEXT ("Start Time"), lstrlen (TEXT ("Start Time")));

		DrawLines (hdc);
		DrawAll (hdc, lrtHead, 0);
		if (lstrlen (szHint)) Print (hdc, 0, g_cyMap + 1, TEXT ("Hint: %s"), szHint);

		EndPaint (hwnd, &ps);
		return 0;

	case WM_DESTROY:
		if (fEdited)
			if (MessageBox (NULL, TEXT ("Save?"), TEXT ("Crazy Train Simulation (Edit Mode)"), MB_YESNO) == IDYES)
				if (!WriteConf (lrtHead))
					MessageBox (NULL, TEXT ("Can not save the Map..."),
						TEXT ("Crazy Train Simulation (Edit Mode)"), MB_ICONERROR);
		PostQuitMessage (0);

		FreeAllResource (lrtHead);
		return 0;
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}
