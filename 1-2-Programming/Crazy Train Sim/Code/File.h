//
// File I/O declaration for Crazy Train Simulation
// By LJN-BOT Man, 2015
//

#ifndef CTS_FILE
#define CTS_FILE

#include "Element.h"

int ReadConf (const char *szConf, RouteList **plrtHead);
int WriteConf (RouteList *lrtHead);
int WriteLog (int cTime, int iStrategy, RouteList *lrtHead);

#endif  //CTS_FILE
