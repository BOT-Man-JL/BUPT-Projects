//
// Elements declaration for Crazy Train Simulation
// By LJN-BOT Man, 2015
//

#ifndef CTS_ELEMENT
#define CTS_ELEMENT

//Train State----------------------------------------------------------------------

#define TS_NDP 0  //Not ready departure
#define TS_ARR 1  //Arrived
#define TS_SUS 2  //Suspend
#define TS_RUN 3  //Running
#define TS_BRK 4  //Braking

//Basic Elements----------------------------------------------------------------------

typedef enum _Direction { NORTH = 1, SOUTH = 2, EAST = 4, WEST = 8 } Direction;

typedef struct _Route  //denoted as rt
{
	int xBeg, yBeg, xEnd, yEnd;
	struct _RailwayList *lwyHead;
	struct _Train *pTrain;
	struct _SignalList *lsgHead;
	//struct _TrainList *ltrHead;  //Unable now (One Route for one train)
} Route;

typedef struct _Railway  //denoted as wy
{
	int x, y;
	Direction diTo;
} Railway;

typedef struct _Train  //denoted as tr
{
	//Constant
	int tDep, cSpeed;
	struct _Route *pRoute;
	//struct _StopList *lstHead;  //Unable now

	//Variable
	int x, y;
	int state;  //Finite State Automachine
	int cDist;  //Actual Speed
	struct _RailwayList *lwyCur;
} Train;

/*
typedef struct _Stop  //denoted as st  //Unable now
{
	int x, y;
	int time;
} Stop;
*/

typedef struct _Signal  //denoted as sg
{
	int xBeg, yBeg, xEnd, yEnd;  //if beg == end then it is a point
	struct _Train *ptrUsing;
	struct _TrainList *ltrWaiting;
} Signal;

//List of Elements----------------------------------------------------------------------

typedef struct _RouteList  //denoted as lrt
{
	struct _Route *elem;
	struct _RouteList *next;
} RouteList;

typedef struct _RailwayList  //denoted as lwy
{
	struct _Railway *elem;
	struct _RailwayList *next;
} RailwayList;

typedef struct _TrainList  //denoted as ltr
{
	struct _Train *elem;
	struct _TrainList *next;
} TrainList;

/*
typedef struct _StopList  //denoted as lst  //Unable now
{
	struct _Stop *elem;
	struct _StopList *next;
} StopList;
*/

typedef struct _SignalList  //denoted as lsg
{
	struct _Signal *elem;
	struct _SignalList *next;
} SignalList;

//Functions----------------------------------------------------------------------

Route *NewRoute (int xBeg, int yBeg);
int AddRoute (RouteList **plHead, Route *pNew);

int AddNewRailway (Route *pRoute, int x, int y, Direction diTo);  //1 successful, 0 err, -1 opposite direction
int AddNewTrain (Route *pRoute, int tDep, int cSpeed);
int AddTrain (TrainList **plHead, Train *pNew);

//Stop *NewStop (int x, int y, int time);  //Unable now
//int AddStop (Train *pTrain, Stop *pNew);  //Unable now

Signal *NewSignal (int xBeg, int yBeg, int xEnd, int yEnd);
int AddSignal (Route *pRoute, Signal *pNew);
int AddSignal_Inner (SignalList **plHead, Signal *pNew);
Signal *FindSignal (int xBeg, int yBeg, int xEnd, int yEnd);

void FreeAllResource (RouteList *plHead);

#endif  //CTS_ELEMENT
