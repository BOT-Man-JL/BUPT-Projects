//
// Elements definition for Crazy Train Simulation
// By LJN-BOT Man, 2015
//

#include <stdlib.h>  //malloc & free

#include "Scales.h"
#include "Element.h"

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
//Golbal Resource - Signal
//========================================================================================

SignalList *g_lsgHead;  //Resource Control

//========================================================================================
//Route Function
//========================================================================================

Route *NewRoute (int xBeg, int yBeg)
{
	Route *pNew;
	pNew = (Route *) malloc (sizeof (Route));
	if (pNew == NULL) return NULL;

	pNew->xBeg = pNew->xEnd = xBeg;
	pNew->yBeg = pNew->yEnd = yBeg;
	pNew->lwyHead = NULL;  //Updated in AddNewRailway ()
	pNew->pTrain = NULL;  //Updated in AddNewTrain ()
	pNew->lsgHead = NULL;  //Updated in AddSignal ()

	return pNew;
}

int AddRoute (RouteList **plHead, Route *pNew)
{
	RouteList *pCur, *pPre;

	if (pNew == NULL) return 0;

	for (pCur = pPre = (*plHead); pCur != NULL; pCur = pCur->next)
	{
		pPre = pCur;
		if (pCur->elem == pNew) return 1;  //Find if the Route has been added
	}

	pCur = (RouteList *) malloc (sizeof (RouteList));
	if (pCur == NULL) return 0;

	pCur->elem = pNew;
	pCur->next = NULL;

	if ((*plHead) != NULL) pPre->next = pCur;
	else (*plHead) = pCur;

	return 1;
}

//========================================================================================
//Railway Function
//========================================================================================

int AddNewRailway (Route *pRoute, int x, int y, Direction diTo)
{
	RailwayList *pCur, *pPre;
	Railway *pNew;

	if (pRoute == NULL) return 0;

	for (pCur = pPre = pRoute->lwyHead; pCur != NULL; pCur = pCur->next)
		pPre = pCur;  //Find tail of the list

	if (pPre != NULL)  //There is Railway in it
	{
		//1.the same direction
		if (pPre->elem->diTo == diTo)
		{
			pPre->elem->x = x;
			pPre->elem->y = y;

			pRoute->xEnd = x;
			pRoute->yEnd = y;
			return 1;
		}

		//2.opposite direction
		if ((pPre->elem->diTo == NORTH && diTo == SOUTH) ||
			(pPre->elem->diTo == SOUTH && diTo == NORTH) ||
			(pPre->elem->diTo == EAST && diTo == WEST) ||
			(pPre->elem->diTo == WEST && diTo == EAST))
			return -1;
	}

	//3.turning
	//4.No Railway in it
	pNew = (Railway *) malloc (sizeof (Railway));
	if (pNew == NULL) return 0;

	pNew->x = x;
	pNew->y = y;
	pNew->diTo = diTo;

	pCur = (RailwayList *) malloc (sizeof (RailwayList));
	if (pCur == NULL)
	{
		free (pNew);
		return 0;
	}

	pCur->elem = pNew;
	pCur->next = NULL;

	if (pRoute->lwyHead != NULL) pPre->next = pCur;
	else pRoute->lwyHead = pCur;

	pRoute->xEnd = x;
	pRoute->yEnd = y;

	return 1;
}

//========================================================================================
//Train Function
//========================================================================================

int AddNewTrain (Route *pRoute, int tDep, int cSpeed)
{
	Train *pNew;

	if (pRoute == NULL) return 0;
	if (pRoute->pTrain) return 0;

	pNew = (Train *) malloc (sizeof (Train));
	if (pNew == NULL) return 0;

	pNew->tDep = tDep;
	pNew->cSpeed = cSpeed;
	pNew->pRoute = pRoute;
	pRoute->pTrain = pNew;

	pNew->state = TS_NDP;
	pNew->x = pRoute->xBeg;
	pNew->y = pRoute->yBeg;
	pNew->cDist = 0;  //distance from the signal or the corner
	pNew->lwyCur = NULL;  //Current Railway
	//pNew->lstHead = NULL;  //Stop List  //Unable now

	return 1;
}

int AddTrain (TrainList **plHead, Train *pNew)
{
	TrainList *pCur, *pPre;

	if (pNew == NULL) return 0;

	for (pCur = pPre = (*plHead); pCur != NULL; pCur = pCur->next)
	{
		pPre = pCur;
		if (pCur->elem == pNew) return 1;  //Find if the Train has been added
	}

	pCur = (TrainList *) malloc (sizeof (TrainList));
	if (pCur == NULL) return 0;

	pCur->elem = pNew;
	pCur->next = NULL;

	if ((*plHead) != NULL) pPre->next = pCur;
	else (*plHead) = pCur;

	return 1;
}

//========================================================================================
//Stop Function
//========================================================================================

/*
Stop *NewStop (int x, int y, int time)  //Unable now
{
	Stop *pNew;
	pNew = (Stop *) malloc (sizeof (Stop));
	if (pNew == NULL) return NULL;

	pNew->x = x;
	pNew->y = y;
	pNew->time = time;

	return pNew;
}

int AddStop (Train *pTrain, Stop *pNew)  //Unable now
{
	StopList *plHead = pTrain->lstHead, *pCur, *pPre;

	for (pCur = pPre = plHead; pCur != NULL; pCur = pCur->next)
		pPre = pCur;
	pCur = (StopList *) malloc (sizeof (StopList));
	if (pCur == NULL) return 0;

	pCur->elem = pNew;
	pCur->next = NULL;

	if (pTrain->lstHead != NULL) pPre->next = pCur;
	else pTrain->lstHead = pCur;
	return 1;
}
*/

//========================================================================================
//Signal Function
//========================================================================================

Signal *NewSignal (int xBeg, int yBeg, int xEnd, int yEnd)
{
	Signal *pNew;
	pNew = (Signal *) malloc (sizeof (Signal));
	if (pNew == NULL) return NULL;

	pNew->xBeg = xBeg;
	pNew->yBeg = yBeg;
	pNew->xEnd = xEnd;
	pNew->yEnd = yEnd;
	pNew->ptrUsing = NULL;
	pNew->ltrWaiting = NULL;

	if (!AddSignal_Inner (&g_lsgHead, pNew))  //For Resource Control
	{
		free (pNew);
		return NULL;
	}

	return pNew;
}

int AddSignal (Route *pRoute, Signal *pNew)
{
	if (!AddSignal_Inner (&pRoute->lsgHead, pNew)) return 0;
	return 1;
}

int AddSignal_Inner (SignalList **plHead, Signal *pNew)
{
	SignalList *pCur, *pPre;

	if (pNew == NULL) return 0;

	for (pCur = pPre = (*plHead); pCur != NULL; pCur = pCur->next)
	{
		pPre = pCur;
		if (pCur->elem == pNew) return 1;  //Find if the Signal has been added
	}

	pCur = (SignalList *) malloc (sizeof (SignalList));
	if (pCur == NULL) return 0;

	pCur->elem = pNew;
	pCur->next = NULL;

	if ((*plHead) != NULL) pPre->next = pCur;
	else (*plHead) = pCur;

	return 1;
}

Signal *FindSignal (int xBeg, int yBeg, int xEnd, int yEnd)
{
	SignalList *pCur;

	for (pCur = g_lsgHead; pCur != NULL; pCur = pCur->next)
		if (pCur->elem->xBeg == xBeg && pCur->elem->yBeg == yBeg &&
			pCur->elem->xEnd == xEnd && pCur->elem->yEnd == yEnd)
			return pCur->elem;

	return NULL;
}

//========================================================================================
//FreeAllResource Function
//========================================================================================

static void FreeRailwayList (RailwayList *plHead);  //Private
//static void FreeStopList (StopList *plHead);  //Private  //Unable now
static void FreeSignalResource ();  //Private

void FreeAllResource (RouteList *plHead)
{
	RouteList *pCur, *pTmp;
	SignalList *lsgCur, *lsgTmp;

	for (pCur = plHead; pCur != NULL; )
	{
		pTmp = pCur;

		//RailwayList
		FreeRailwayList (pCur->elem->lwyHead);

		//Train
		//FreeStopList (pCur->elem->pTrain->lstHead);  //Unable now
		free (pCur->elem->pTrain);

		//SignalList
		for (lsgCur = pCur->elem->lsgHead; lsgCur != NULL; )  //just Container
		{
			lsgTmp = lsgCur;
			lsgCur = lsgCur->next;
			free (lsgTmp);
		}

		//.this
		free (pCur->elem);
		pCur = pCur->next;
		free (pTmp);
	}
	FreeSignalResource ();
}

static void FreeRailwayList (RailwayList *plHead)  //Private
{
	RailwayList *pCur, *pTmp;

	for (pCur = plHead; pCur != NULL; )
	{
		pTmp = pCur;
		free (pCur->elem);
		pCur = pCur->next;
		free (pTmp);
	}
}

/*
static void FreeStopList (StopList *plHead)  //Private  //Unable now
{
	StopList *pCur, *pTmp;

	for (pCur = plHead; pCur != NULL; )
	{
		pTmp = pCur;
		free (pCur->elem);
		pCur = pCur->next;
		free (pTmp);
	}
}
*/

static void FreeSignalResource ()  //Private
{
	SignalList *pCur, *pTmp;
	TrainList *ltrCur, *ltrTmp;

	for (pCur = g_lsgHead; pCur != NULL; )
	{
		for (ltrCur = pCur->elem->ltrWaiting; ltrCur != NULL; )  //just Container
		{
			ltrTmp = ltrCur;
			ltrCur = ltrCur->next;
			free (ltrTmp);
		}

		pTmp = pCur;
		free (pCur->elem);
		pCur = pCur->next;
		free (pTmp);
	}
}
