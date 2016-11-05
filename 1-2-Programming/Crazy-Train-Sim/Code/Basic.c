//
// Basic functions definition for Crazy Train Simulation
// By LJN-BOT Man, 2015
//

#include <windows.h>
#include <stdio.h>

#include "Scales.h"
#include "Element.h"
#include "File.h"
#include "Basic.h"

//========================================================================================
//max and min Macro
//========================================================================================

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

//========================================================================================
//Compute Function
//========================================================================================

extern SignalList *g_lsgHead;  //For debug

static void MoveTrain (ThreadCtrl *ptc, Train *pTrain);
static void AllocateSignal (int iStrategy);

void WINAPI Compute (ThreadCtrl *ptc)
{
	RouteList *pCur;
	int fLocking, fArrived;

	while (ptc->iRunningState != RS_QUIT)
	{
		if (WaitForSingleObject (ptc->hCompEvent, INFINITE) == WAIT_OBJECT_0)
		{
			while (ptc->iRunningState == RS_PAUSE) Sleep (1000);
			if (ptc->iRunningState == RS_QUIT) break;

			ptc->cTime++;
			if (ptc->cTime >= INT_MAX) Err ("Running Too Long");

			fLocking = fArrived = 1;
			for (pCur = ptc->lrtHead; pCur != NULL; pCur = pCur->next)
			{
				if (pCur->elem->pTrain == NULL) continue;
				if (pCur->elem->pTrain->state != TS_ARR)
				{
					if (pCur->elem->pTrain->state != TS_SUS)
						fLocking = 0;
					else
						fArrived = 0;
				}
				MoveTrain (ptc, pCur->elem->pTrain);
			}
			AllocateSignal (ptc->iStrategy);
			if (fLocking && !fArrived)
			{
				//FreeAllResource (ptc->lrtHead);  //Others are using...
				Err ("(Deadlocking...) The map you designed has not possible solution "
					"in this version... It will be improved later [:-)");
			}
			SetEvent (ptc->hDispEvent);
		}
	}
	SetEvent (ptc->hDispEvent);  //In case Display () is sleeping...
}

//========================================================================================
//Display Function in Console Mode
//========================================================================================

void WINAPI CDisplay (ThreadCtrl *ptc)
{
	RouteList	*lrtCur;
	int			i;
	static char	*szState[5] = {"TS_NDP", "TS_ARR", "TS_SUS", "TS_RUN", "TS_BRK"};

	while (ptc->iRunningState != RS_QUIT)
	{
		if (WaitForSingleObject (ptc->hDispEvent, INFINITE) == WAIT_OBJECT_0)
		{
			if (ptc->iRunningState == RS_QUIT) break;
			printf ("Time = %d\t Strategy = %d\n", ptc->cTime, ptc->iStrategy);
			for (i = 1, lrtCur = ptc->lrtHead; lrtCur != NULL; i++, lrtCur = lrtCur->next)
			{
				if (lrtCur->elem->pTrain == NULL) continue;
				printf ("Train %d = (%d, %d)\t State = %s\n", i,
						lrtCur->elem->pTrain->x, lrtCur->elem->pTrain->y,
						szState[lrtCur->elem->pTrain->state]);
			}
			printf ("\n");

			WriteLog (ptc->cTime, ptc->iStrategy, ptc->lrtHead);

			Sleep (COMPUTETICK);
		}
		SetEvent (ptc->hCompEvent);
	}
	SetEvent (ptc->hCompEvent);  //In case Compute () is sleeping...
}

//========================================================================================
//Display Function in Window Mode
//========================================================================================

#ifdef CTS_WINDOW

#include "GUI.h"

void WINAPI WDisplay (ThreadCtrl *ptc)
{
	HDC hdcMem, hdcWnd;  //Draw Map once, draw trains then.
	HBITMAP hBitmap;

#ifdef _DEBUG
	SignalList *lsgCur;
#endif

	while (ptc->iRunningState != RS_QUIT)
	{
		if (WaitForSingleObject (ptc->hDispEvent, INFINITE) == WAIT_OBJECT_0)
		{
			if (ptc->iRunningState == RS_QUIT) break;
			for (ptc->cFrame = 0; ptc->cFrame < FPS; ptc->cFrame++)
			{
				hdcWnd = GetDC (ptc->hwnd);
				hdcMem = CreateCompatibleDC (hdcWnd);
				hBitmap = CreateCompatibleBitmap (hdcWnd, g_cxMap, g_cyMap);
				SelectObject (hdcMem, hBitmap);
				SelectObject (hdcMem, (HBRUSH) GetStockObject (WHITE_BRUSH));
				Rectangle (hdcMem, -1, -1, g_cxMap + 1, g_cyMap + 1);

#ifdef _DEBUG
				DrawLines (hdcMem);
				for (lsgCur = g_lsgHead; lsgCur != NULL; lsgCur = lsgCur->next)
				{
					if (lsgCur->elem->ptrUsing != NULL)
						DrawSignal (hdcMem, lsgCur->elem->xBeg, lsgCur->elem->yBeg,
											lsgCur->elem->xEnd, lsgCur->elem->yEnd, 2);
					else if (lsgCur->elem->ltrWaiting != NULL)
						DrawSignal (hdcMem, lsgCur->elem->xBeg, lsgCur->elem->yBeg,
											lsgCur->elem->xEnd, lsgCur->elem->yEnd, 1);
					else
						DrawSignal (hdcMem, lsgCur->elem->xBeg, lsgCur->elem->yBeg,
											lsgCur->elem->xEnd, lsgCur->elem->yEnd, 0);
				}
#endif

				DrawAll (hdcMem, ptc->lrtHead, ptc->cFrame);
				Sleep (1000 / FPS);

				BitBlt (hdcWnd, 0, 0, g_cxMap, g_cyMap, hdcMem, 0, 0, SRCCOPY);
				DeleteObject (hBitmap);
				DeleteDC (hdcMem);
				ReleaseDC (ptc->hwnd, hdcWnd);
			}
			WriteLog (ptc->cTime, ptc->iStrategy, ptc->lrtHead);
			SetEvent (ptc->hCompEvent);
		}
	}
	SetEvent (ptc->hCompEvent);  //In case Compute () is sleeping...
}

#endif

//========================================================================================
//Err Function
//========================================================================================

void Err (const char *str)
{
	printf ("ERR: %s... Exit in 3 sec.\n", str);
	Sleep (3000);
	exit (0);
}

//========================================================================================
//Init Function
//========================================================================================

static void GenerateSignal (RouteList *lrtHead);
//static void SplitSignal ();  //Finished later

int Init (ThreadCtrl *ptc, const char *szConf)
{
	if (!ReadConf (szConf, &ptc->lrtHead)) return 0;
	GenerateSignal (ptc->lrtHead);

	ptc->iRunningState = RS_RUNNING;
	return 1;
}

//========================================================================================
//Implementation of GenerateSignal Function
//========================================================================================

typedef struct _Endpoint  //Private
{
	int i;
	struct _Endpoint *next;
	struct _EndpointPair *father;
} Endpoint;

typedef struct _EndpointPair  //Private
{
	struct _Endpoint *beg, *end;
	struct _Route *pRoute;
} EndpointPair;

typedef struct _Line  //Private
{
	int xBeg, yBeg, xEnd, yEnd;
	struct _Route *pRoute;
	struct _Line *next;
} Line;

static Endpoint *NewEndpoint (int x)  //Private
{
	Endpoint *pNew;
	pNew = (Endpoint *) malloc (sizeof (Endpoint));
	if (pNew == NULL) return NULL;

	pNew->i = x;
	pNew->next = NULL;
	pNew->father = NULL;

	return pNew;
}

static EndpointPair *NewEndpointPair (Route *pRoute, int x, int y)  //Private
{
	EndpointPair *pNew;
	pNew = (EndpointPair *) malloc (sizeof (EndpointPair));
	if (pNew == NULL) return NULL;

	pNew->pRoute = pRoute;
	pNew->beg = NewEndpoint (min (x, y));  //Auto Correct
	pNew->end = NewEndpoint (max (x, y));
	pNew->beg->father = pNew->end->father = pNew;

	return pNew;
}

static void AddLine (Line **plHead, Route *pRoute, int xBeg, int yBeg, int xEnd, int yEnd)
{
	Line *pCur, *pPre;

	for (pCur = pPre = (*plHead); pCur != NULL; pCur = pCur->next)
		pPre = pCur;
	pCur = (Line *) malloc (sizeof (Line));
	//if (pCur == NULL) return 0;

	pCur->xBeg = min (xBeg, xEnd);  //Auto Correct
	pCur->yBeg = min (yBeg, yEnd);
	pCur->xEnd = max (xEnd, xBeg);
	pCur->yEnd = max (yEnd, yBeg);
	pCur->pRoute = pRoute;
	pCur->next = NULL;

	if ((*plHead) != NULL) pPre->next = pCur;
	else (*plHead) = pCur;
	//return 1;
}

static Endpoint *pvHead[WORLDWIDTH] = { NULL }, *pvTail[WORLDWIDTH] = { NULL },
			*phHead[WORLDHEIGHT] = { NULL }, *phTail[WORLDHEIGHT] = { NULL };  //Private

static void GetEndpointNCross (RouteList *lrtHead);  //Private
static void SortEndpoints (Endpoint *pL, Endpoint *pR);  //Private
static void SwapEndpoints (Endpoint *x, Endpoint *y);  //Private
static void CalcEndpoints (Endpoint *pHead, int fVertical, int order);  //Private

static void GenerateSignal (RouteList *lrtHead)
{
	Endpoint *pCur, *pTmp;
	EndpointPair *pFather;
	int i;

	GetEndpointNCross (lrtHead);

	for (i = 0; i < WORLDWIDTH; i++)
		if (pvHead[i])
		{
			SortEndpoints (pvHead[i], NULL);
			CalcEndpoints (pvHead[i], 1, i);
			for (pCur = pvHead[i]; pCur != NULL; )
			{
					pTmp = pCur;
					if (pCur->father)
					{
						pFather = pCur->father;
						pCur->father->beg->father = pCur->father->end->father = NULL;
						free (pFather);
					}
					pCur = pCur->next;
					free (pTmp);
			}
		}

	for (i = 0; i < WORLDHEIGHT; i++)
		if (phHead[i])
		{
			SortEndpoints (phHead[i], NULL);
			CalcEndpoints (phHead[i], 0, i);
			for (pCur = phHead[i]; pCur != NULL; )
			{
				pTmp = pCur;
				if (pCur->father)
				{
					pFather = pCur->father;
					pCur->father->beg->father = pCur->father->end->father = NULL;
					free (pFather);
				}
				pCur = pCur->next;
				free (pTmp);
			}
		}
}

static void GetEndpointNCross (RouteList *lrtHead)  //Private
{
	RouteList *lrtCur, *lrtTmp;
	RailwayList *lwyCur;
	Signal *psgCur;
	Line *plHead = NULL, *plCur, *plTmp;
	int xBeg, yBeg, xEnd, yEnd, i, j;
	RouteList *lrtArr[WORLDWIDTH][WORLDHEIGHT] = { NULL };

	for (lrtCur = lrtHead; lrtCur != NULL; lrtCur = lrtCur->next)
	{
		xBeg = lrtCur->elem->xBeg;
		yBeg = lrtCur->elem->yBeg;

		for (lwyCur = lrtCur->elem->lwyHead; lwyCur != NULL; lwyCur = lwyCur->next)
		{
			xEnd = lwyCur->elem->x;
			yEnd = lwyCur->elem->y;

			switch (lwyCur->elem->diTo)
			{
			case NORTH:
			case SOUTH:
				if (pvHead[xBeg] == NULL)
				{
					pvHead[xBeg] = NewEndpointPair (lrtCur->elem, yBeg, yEnd)->beg;
					pvHead[xBeg]->next = pvTail[xBeg] = pvHead[xBeg]->father->end;
				}
				else
				{
					pvTail[xBeg]->next = NewEndpointPair (lrtCur->elem, yBeg, yEnd)->beg;
					pvTail[xBeg] = pvTail[xBeg]->next;
					pvTail[xBeg]->next = pvTail[xBeg]->father->end;
					pvTail[xBeg] = pvTail[xBeg]->next;
				}
				AddLine (&plHead, lrtCur->elem, xBeg, yBeg, xEnd, yEnd);
				break;

			case EAST:
			case WEST:
				if (phHead[yBeg] == NULL)
				{
					phHead[yBeg] = NewEndpointPair (lrtCur->elem, xBeg, xEnd)->beg;
					phHead[yBeg]->next = phTail[yBeg] = phHead[yBeg]->father->end;
				}
				else
				{
					phTail[yBeg]->next = NewEndpointPair (lrtCur->elem, xBeg, xEnd)->beg;
					phTail[yBeg] = phTail[yBeg]->next;
					phTail[yBeg]->next = phTail[yBeg]->father->end;
					phTail[yBeg] = phTail[yBeg]->next;
				}
				AddLine (&plHead, lrtCur->elem, xBeg, yBeg, xEnd, yEnd);
				break;
			}
			xBeg = xEnd;
			yBeg = yEnd;
		}
	}

	//Use Lines to calc. crosses
	for (plCur = plHead; plCur != NULL; plCur = plCur->next)
		for (plTmp = plCur->next; plTmp != NULL; plTmp = plTmp->next)
		{
			if ((plTmp->xBeg < plCur->xBeg && plTmp->xEnd > plCur->xEnd) &&
					(plTmp->yBeg > plCur->yBeg && plTmp->yEnd < plCur->yEnd))
			{
				AddRoute (&lrtArr[plCur->xBeg][plTmp->yBeg], plCur->pRoute);
				AddRoute (&lrtArr[plCur->xBeg][plTmp->yBeg], plTmp->pRoute);
			}
			else if ((plTmp->xBeg > plCur->xBeg && plTmp->xEnd < plCur->xEnd) &&
					(plTmp->yBeg < plCur->yBeg && plTmp->yEnd > plCur->yEnd))
			{
				AddRoute (&lrtArr[plTmp->xBeg][plCur->yBeg], plCur->pRoute);
				AddRoute (&lrtArr[plTmp->xBeg][plCur->yBeg], plTmp->pRoute);
			}
		}

	//Free Lines
	for (plCur = plHead; plCur != NULL; )
	{
		plTmp = plCur;
		plCur = plCur->next;
		free (plTmp);
	}

	//Counting...
	for (i = 0; i < WORLDWIDTH; i++)
		for (j = 0; j < WORLDHEIGHT; j++)
			if (lrtArr[i][j])
			{
				if (lrtArr[i][j]->next == NULL) continue;  //One Route in it

				psgCur = NewSignal (i, j, i, j);
				for (lrtCur = lrtArr[i][j]; lrtCur != NULL; lrtCur = lrtCur->next)
					AddSignal (lrtCur->elem, psgCur);

				//Free Route Container List
				for (lrtCur = lrtArr[i][j]; lrtCur != NULL; )
				{
					lrtTmp = lrtCur;
					lrtCur = lrtCur->next;
					free (lrtTmp);
				}
			}
}

static void SwapEndpoints (Endpoint *x, Endpoint *y)  //Private
{
	int t;
	EndpointPair *pt;

	t = y->i; y->i = x->i; x->i = t;

	if (y->father->beg == y)
		y->father->beg = x;
	else
		y->father->end = x;

	if (x->father->beg ==x)
		x->father->beg = y;
	else
		x->father->end = y;

	pt = y->father;
	y->father = x->father;
	x->father = pt;
}

static void SortEndpoints (Endpoint *pL, Endpoint *pR)  //Private
{
	int key = pL->i;
	Endpoint *p = pL, *q = pL->next;

	while (q != pR)  //Quick Sort
	{
		//Beg point first if equal
		if (q->i < key || (q->father->beg == q && q->i == key))
		{
			p = p->next;
			if (p != q)
				SwapEndpoints (p, q);
		}
		q = q->next;
	}
	if (p != pL)
		SwapEndpoints (p, pL);

	if (pL != p)
		SortEndpoints (pL, p);
	if (p->next != pR)
		SortEndpoints (p->next, pR);
}

static void CalcEndpoints (Endpoint *pHead, int fVertical, int order)  //Private
{
	Signal *psgCur;
	Endpoint *pCur, *pTmp;
	int beg = -1;

	for (pCur = pHead; pCur != NULL; pCur = pCur->next)
		if (pCur->father->end == pCur)  //Only calc end
		{
			//For each previous Endpoint
			for (pTmp = pHead; pTmp != pCur; pTmp = pTmp->next)
			{
				if (pTmp->father->pRoute == pCur->father->pRoute)
					continue;  //Safe in one-Train case

				if (pTmp->i == -1 || pTmp->father->end == pTmp||
					pTmp->father->end == pCur) continue;  //not a valid beg

				if (pTmp->father->pRoute == pCur->father->pRoute)
					continue;  //One Route

				//Calc the actual beg
				beg = max (pTmp->i, pCur->father->beg->i);

				//Update Signal
				if (fVertical)
				{
					psgCur = FindSignal (order, beg, order, pCur->i);
					if (psgCur == NULL)
						psgCur = NewSignal (order, beg, order, pCur->i);
					AddSignal (pCur->father->pRoute, psgCur);
					AddSignal (pTmp->father->pRoute, psgCur);
				}
				else
				{
					psgCur = FindSignal (beg, order, pCur->i, order);
					if (psgCur == NULL)
						psgCur = NewSignal (beg, order, pCur->i, order);
					AddSignal (pCur->father->pRoute, psgCur);
					AddSignal (pTmp->father->pRoute, psgCur);
				}
			}
			pCur->father->beg->i = -1;  //Invalidate its beg
		}
}

//========================================================================================
//Implementation of MoveTrain Function
//========================================================================================

static void CheckSignal (Train *pTrain, int fAlloc);  //Private

static void MoveTrain (ThreadCtrl *ptc, Train *pTrain)  //Private
{
	switch (pTrain->state)
	{
	case TS_NDP:
		if (ptc->cTime > pTrain->tDep)
		{
			//Set Position
			if (pTrain->pRoute->lwyHead == NULL)
				Err ("Initialization not complete");
			pTrain->lwyCur = pTrain->pRoute->lwyHead;
			break;  //Checking
		}
		//Fall through for else
	case TS_ARR:
		return;

	case TS_BRK:
		pTrain->state = TS_SUS;
		//Fall through
	case TS_RUN:
		switch (pTrain->lwyCur->elem->diTo)
		{
		case NORTH:
			pTrain->y = pTrain->y - pTrain->cDist;
			break;

		case SOUTH:
			pTrain->y = pTrain->y + pTrain->cDist;
			break;

		case EAST:
			pTrain->x = pTrain->x + pTrain->cDist;
			break;

		case WEST:
			pTrain->x = pTrain->x - pTrain->cDist;
			break;
		}
		break;  //Checking

	case TS_SUS:
		break;
	}

	//---------------------Checking---------------------

	//Init the Actual Speed
	pTrain->cDist = pTrain->cSpeed;

	//Check the position on Current Railway to Update lwyCur
	if (pTrain->x == pTrain->lwyCur->elem->x && pTrain->y == pTrain->lwyCur->elem->y)
	{
		if (pTrain->lwyCur->next != NULL)  //Use the next railway
			pTrain->lwyCur = pTrain->lwyCur->next;

		else if (pTrain->pRoute->xBeg == pTrain->pRoute->xEnd &&
					pTrain->pRoute->yBeg == pTrain->pRoute->yEnd)  //Go circle
			pTrain->lwyCur = pTrain->pRoute->lwyHead;

		else  //Arrived
		{
			pTrain->state = TS_ARR;
			CheckSignal (pTrain, 0);
			pTrain->cDist = 0;
			pTrain->state = TS_ARR;
			return;
		}
	}

	//Check Corner
	switch (pTrain->lwyCur->elem->diTo)
	{
	case NORTH:
		if (pTrain->y < pTrain->lwyCur->elem->y + pTrain->cSpeed)
			pTrain->cDist = pTrain->y - pTrain->lwyCur->elem->y;
		break;

	case SOUTH:
		if (pTrain->y > pTrain->lwyCur->elem->y - pTrain->cSpeed)
			pTrain->cDist = pTrain->lwyCur->elem->y - pTrain->y;
		break;

	case EAST:
		if (pTrain->x > pTrain->lwyCur->elem->x - pTrain->cSpeed)
			pTrain->cDist = pTrain->lwyCur->elem->x - pTrain->x;
		break;

	case WEST:
		if (pTrain->x < pTrain->lwyCur->elem->x + pTrain->cSpeed)
			pTrain->cDist = pTrain->x - pTrain->lwyCur->elem->x;
		break;
	}

	//Check Signal
	CheckSignal (pTrain, 1);
}

//========================================================================================
//Implementation of CheckSignal Function
//========================================================================================

static int CalcDistToSignal (Train *pTrain, Signal *pSignal);  //Private

static void CheckSignal (Train *pTrain, int fAlloc)  //Private
{
	SignalList *lsgUsingHead = NULL;  //Record the Signal applied
	SignalList *lsgCur, *lsgTmp;
	int xBeg, yBeg, xEnd, yEnd;  //Move Rect
	int cDistTmp, fBlock = 0;

	//Init the Move Rect
	switch (pTrain->lwyCur->elem->diTo)
	{
	case NORTH:
		xBeg = xEnd = pTrain->x;
		yBeg = max (pTrain->y - pTrain->cDist, pTrain->lwyCur->elem->y);
		yEnd = pTrain->y;
		break;

	case SOUTH:
		xBeg = xEnd = pTrain->x;
		yBeg = pTrain->y;
		yEnd = min (pTrain->y + pTrain->cDist, pTrain->lwyCur->elem->y);
		break;

	case EAST:
		yBeg = yEnd = pTrain->y;
		xBeg = pTrain->x;
		xEnd = min (pTrain->x + pTrain->cDist, pTrain->lwyCur->elem->x);
		break;

	case WEST:
		yBeg = yEnd = pTrain->y;
		xEnd = pTrain->x;
		xBeg = max (pTrain->x - pTrain->cDist, pTrain->lwyCur->elem->x);
		break;
	}

	//Using cDistTmp to record the min Distance
	cDistTmp = pTrain->cDist;

	//Check All Signal
	for (lsgCur = pTrain->pRoute->lsgHead; lsgCur != NULL; lsgCur = lsgCur->next)
	{
		//Release All Signal when arrived
		if (pTrain->state == TS_ARR && lsgCur->elem->ptrUsing == pTrain)
			lsgCur->elem->ptrUsing = NULL;

		//Test Move Rect & Signal Rect
		if ((xBeg <= lsgCur->elem->xBeg && xEnd >= lsgCur->elem->xEnd &&
			 yBeg >= lsgCur->elem->yBeg && yEnd <= lsgCur->elem->yEnd) ||
			(xBeg >= lsgCur->elem->xBeg && xEnd <= lsgCur->elem->xEnd &&
			 yBeg <= lsgCur->elem->yBeg && yEnd >= lsgCur->elem->yEnd) ||
			(xBeg <= lsgCur->elem->xEnd && xEnd >= lsgCur->elem->xBeg &&
			 yBeg <= lsgCur->elem->yEnd && yEnd >= lsgCur->elem->yBeg))
		{
			if (lsgCur->elem->ptrUsing == NULL)
			{
				if (fAlloc)  //Apply to Wait Signal
					if (!AddSignal_Inner (&lsgUsingHead, lsgCur->elem))
						Err ("Out of Memory");
			}
			else if (lsgCur->elem->ptrUsing != pTrain)
			{
				//Some of the Railway has been used by other Trains
				fBlock = 1;
				cDistTmp = min (CalcDistToSignal (pTrain, lsgCur->elem), cDistTmp);
				if (fAlloc)  //Apply to Wait Signal
					if (!AddSignal_Inner (&lsgUsingHead, lsgCur->elem))
						Err ("Out of Memory");
			}
		}
		else if (lsgCur->elem->ptrUsing == pTrain)  //Release the Signal
			lsgCur->elem->ptrUsing = NULL;
	}

	if (cDistTmp < 1)
		//Before Starting... The Railway has been used!!!
		pTrain->state = TS_NDP;
	else if (!fBlock)
		//No Train is using its wanted resource
		pTrain->state = TS_RUN;
	else if (cDistTmp > 1)
	{
		//Not totally blocked
		cDistTmp--;
		pTrain->state = TS_BRK;
		pTrain->cDist = cDistTmp;
	}
	else
		//Blocked
		pTrain->state = TS_SUS;

	//Check each Signal applied
	for (lsgCur = lsgUsingHead; lsgCur != NULL;)
	{
		if (CalcDistToSignal (pTrain, lsgCur->elem) <= cDistTmp && fAlloc)
		{
			if (!AddTrain (&lsgCur->elem->ltrWaiting, pTrain))  //Wait the Signal
				Err ("Out of Memory");
		}
		lsgTmp = lsgCur;
		lsgCur = lsgCur->next;
		free (lsgTmp);
	}
}

static int CalcDistToSignal (Train *pTrain, Signal *pSignal)  //Private
{
	switch (pTrain->lwyCur->elem->diTo)
	{
	case NORTH:
		return pTrain->y - pSignal->yEnd;
	case SOUTH:
		return pSignal->yBeg - pTrain->y;
	case EAST:
		return pSignal->xBeg - pTrain->x;
	case WEST:
		return pTrain->x - pSignal->xEnd;
	}
	Err ("Something Unexpected happened...");
	return 0;
}

//========================================================================================
//Implementation of AllocateSignal Function
//========================================================================================

static void SortTrainList (TrainList *pL, TrainList *pR);  //Private

static void AllocateSignal (int iStrategy)  //Private
{
	SignalList *lsgCur;
	TrainList *ltrCur, *ltrTmp;

	for (lsgCur = g_lsgHead; lsgCur != NULL; lsgCur = lsgCur->next)
		if (lsgCur->elem->ptrUsing == NULL)
		{
			//Sort the Waiting Train List
			switch (iStrategy)
			{
			case 0:
				break;
			case 1:
				if (lsgCur->elem->ltrWaiting != NULL)
					SortTrainList (lsgCur->elem->ltrWaiting, NULL);
				break;
			}

			//Find the First valid Train
			for (ltrCur = lsgCur->elem->ltrWaiting; ltrCur != NULL; )
			{
				if (ltrCur->elem->state != TS_SUS) break;
				else
				{
					ltrTmp = ltrCur;
					ltrCur = lsgCur->elem->ltrWaiting = ltrCur->next;
					free (ltrTmp);
				}
			}

			//Alloc if possible
			if (ltrCur != NULL)
			{
				ltrTmp = ltrCur;
				lsgCur->elem->ptrUsing = ltrCur->elem;
				ltrCur = lsgCur->elem->ltrWaiting = ltrCur->next;
				free (ltrTmp);
			}

			//Re-check the following Trains
			if (ltrCur != NULL)
				for (; ltrCur != NULL; ltrCur = ltrCur->next)
					if (ltrCur->elem->state != TS_SUS)
						CheckSignal (ltrCur->elem, 0);
		}
}

static void SortTrainList (TrainList *pL, TrainList *pR)  //Private
{
	int key = pL->elem->cSpeed;
	TrainList *p = pL, *q = pL->next;
	Train *pTmp;

	while (q != pR)  //Quick Sort
	{
		if (q->elem->cSpeed < key)
		{
			p = p->next;
			if (p != q)
			{
				pTmp = p->elem;
				p->elem = q->elem;
				q->elem = pTmp;
			}
		}
		q = q->next;
	}
	if (p != pL)
	{
		pTmp = p->elem;
		p->elem = pL->elem;
		pL->elem = pTmp;
	}

	if (pL != p)
		SortTrainList (pL, p);
	if (p->next != pR)
		SortTrainList (p->next, pR);
}
