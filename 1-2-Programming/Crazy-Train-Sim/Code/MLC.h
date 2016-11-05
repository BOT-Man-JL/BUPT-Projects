//
// Memory Leaks Checker
// By LJN-BOT Man, 2015
//

#ifndef LJN_MLC
#define LJN_MLC

//Avaliable in Debug Mode
#define _CRTDBG_MAP_ALLOC  //For malloc ()
#include <crtdbg.h>

//Get the result in Output Window - Debug after the main () returns
//Initialization
#define InitMLTracer() (_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF))
//Get the result immediately
#define DumpML() (_CrtDumpMemoryLeaks ())

#endif  //LJN_MLC
