#include <iostream>
#include <string>
using namespace std;

#include <winsock2.h>
#pragma comment(lib, "WS2_32")	// 链接到WS2_32.lib

class CInitSock	   //封装CInitSock类	
{
public:
	CInitSock(BYTE minorVer = 2, BYTE majorVer = 2)
	{
		// 初始化WS2_32.dll
		WSADATA wsaData;
		WORD sockVersion = MAKEWORD(minorVer, majorVer);  //指定要加载的Winsock库的版本 
		if(::WSAStartup(sockVersion, &wsaData) != 0)    //WSAStartup用来加载Winsock库
		{
			exit(0);
		}
	}
	~CInitSock()
	{	
		::WSACleanup();	  //释放Winsock库
	}
};