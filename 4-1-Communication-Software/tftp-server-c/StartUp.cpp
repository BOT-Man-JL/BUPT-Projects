#include<winsock2.h>
#pragma comment( lib, "Ws2_32.lib" )
#include <iostream>

#include "server.h"

bool flag = true;
char listenBuf[RECV_BUF_LEN];

void startUp(void) {
	SOCKET listenSock = INVALID_SOCKET;
	struct sockaddr_in local, from;
	WSADATA wsadata;

	cout << "=============================" << endl;
	cout << "=                           =" << endl;
	cout << "=   TFTP 协议 - 服务器端    =" << endl;
	cout << "=                           =" << endl;
	cout << "=============================" << endl << endl;
	cout << endl;

	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		cout << "错误：无法启动 Winsock 服务。" << endl << endl;
		exit(0);
	}

	cout << "提示：Winsock 已启动。" << endl << endl;

	listenSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (listenSock == INVALID_SOCKET) {
		cout << "错误：无法为监听端口创建 Socket 连接。" << endl << endl;
		exit(0);
	}

	local.sin_family = AF_INET;
	local.sin_port = htons(69);
	local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(listenSock, (struct sockaddr*)&local, sizeof(local)) != 0) {
		cout << "错误：无法监听端口。" << endl << endl;
		closesocket(listenSock);
		exit(0);
	}

	cout << "提示：正在监听端口 69。" << endl << endl;

	int fromLen = sizeof(from);

	while (1) {
		flag = true;
		clear(listenBuf, RECV_BUF_LEN);
		recvfrom(listenSock, listenBuf, RECV_BUF_LEN, 0, (struct sockaddr*)&from, &fromLen);
		switch (getOpCode(listenBuf)) {
			case RRQ:
				cout << "提示：操作码为 RRQ。" << endl << endl;
				CreateThread(0, 0, handleRRQThread, &from, THREAD_PRIORITY_NORMAL, NULL);
				break;
			case WRQ:
				cout << "提示：操作码为 WRQ。" << endl << endl;
                CreateThread (0, 0, handleWRQThread, &from, THREAD_PRIORITY_NORMAL, NULL);
				break;
			default:
				flag = false;
				break;
		}
		while (flag) {
		}
	}
}



