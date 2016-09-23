//
// File I/O definition for Crazy Train Simulation
// By LJN-BOT Man, 2015
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Scales.h"
#include "Element.h"
#include "File.h"

//========================================================================================
//Config I/O Function
//========================================================================================

int ReadConf (const char *szConf, RouteList **plrtHead)
{
	FILE *pFile;
	Route *pRoute;
	char ch;
	int a, b, c;

	pFile = fopen (szConf, "r");
	if (!pFile) return 0;

	fscanf (pFile, "%d%d", &a, &b);
	if (a != WORLDWIDTH || b != WORLDHEIGHT)  //not in the same scale
	{
		fclose (pFile);
		return 0;
	}

	ch = fgetc (pFile);
	while (ch != 'E')
	{
		switch (ch)
		{
		case 'R':
			fscanf (pFile, "%d%d", &a, &b);
			pRoute = NewRoute (a, b);
			if (!AddRoute (plrtHead, pRoute))
			{
				if (pRoute) free (pRoute);
				fclose (pFile);
				return 0;
			}
			fscanf (pFile, "%d%d%d", &a, &b, &c);
			while (a != -1)
			{
				if (AddNewRailway (pRoute, a, b, c) != 1) return 0;
				fscanf (pFile, "%d%d%d", &a, &b, &c);
			}
			break;
		case 'T':
			fscanf (pFile, "%d%d", &a, &b);
			if (!AddNewTrain (pRoute, a, b)) break;
			pRoute = NULL;  //One Route for one Train in this version...
			break;
		}
		ch = fgetc (pFile);
		while (ch == '\n' || ch == ' ' || ch == '\t') ch = fgetc (pFile);
	}

	fclose (pFile);
	return 1;
}

int WriteConf (RouteList *lrtHead)
{
	FILE *pFile;
	RouteList *lrtCur;
	RailwayList *lwyCur;

	pFile = fopen ("CTS_Map.dat", "w");
	if (!pFile) return 0;

	fprintf (pFile, "%d\t%d\n", WORLDWIDTH, WORLDHEIGHT);

	for (lrtCur = lrtHead; lrtCur != NULL; lrtCur = lrtCur->next)
	{
		fprintf (pFile, "R\t%d\t%d\n", lrtCur->elem->xBeg, lrtCur->elem->yBeg);
		for (lwyCur = lrtCur->elem->lwyHead; lwyCur != NULL; lwyCur = lwyCur->next)
			fprintf (pFile, "%d\t%d\t%d\n",
						lwyCur->elem->x, lwyCur->elem->y, lwyCur->elem->diTo);
		fprintf (pFile, "-1\t-1\t-1\n");
		if (lrtCur->elem->pTrain)
			fprintf (pFile, "T\t%d\t%d\n",
						lrtCur->elem->pTrain->tDep, lrtCur->elem->pTrain->cSpeed);
	}

	fprintf (pFile, "E\n");
	fclose (pFile);
	return 1;
}

//========================================================================================
//Log Output Function
//========================================================================================

int WriteLog (int cTime, int iStrategy, RouteList *lrtHead)
{
	FILE		*pFile;
	RouteList	*lrtCur;
	int			i;
	static char	*szState[5] = {"TS_NDP", "TS_ARR", "TS_SUS", "TS_RUN", "TS_BRK"};

	pFile = fopen ("CTS_Result.log", "a+");
	if (!pFile) return 0;

	fprintf (pFile, "Time = %d\t Strategy = %d\n", cTime, iStrategy);
	for (i = 1, lrtCur = lrtHead; lrtCur != NULL; i++, lrtCur = lrtCur->next)
	{
		if (lrtCur->elem->pTrain == NULL) continue;
		fprintf (pFile, "Train %d = (%d, %d)\t State = %s\n", i,
				lrtCur->elem->pTrain->x, lrtCur->elem->pTrain->y,
				szState[lrtCur->elem->pTrain->state]);
	}
	fprintf (pFile, "\n");

	fclose (pFile);
	return 1;
}
