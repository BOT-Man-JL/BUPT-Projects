
#if !defined(AFX_TFTP_SERVER_H__41FD7E46_297D_42AB_98C5_81DED2604D38__INCLUDED_)
#define AFX_TFTP_SERVER_H__41FD7E46_297D_42AB_98C5_81DED2604D38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<string>
#include<winsock2.h>
#pragma comment( lib, "Ws2_32.lib" )
using namespace std;

#define IDLE          0
#define WAIT_FIRST_P  1
#define WAIT_NEXT_P   2
#define WAIT_ACK      3
#define WAIT_LAST_ACK 4
#define WAIT_R_RESP   5
#define WAIT_W_RESP   6
#define ERROR_OVER    7
#define SUCCESS_OVER  8

#define RRQ   0x0001
#define WRQ   0x0002
#define DATA  0x0003
#define ACK   0x0004
#define ERR   0x0005
#define ReadFile_IND   0x0006
#define WriteFile_IND  0x0007
#define ReadFile_RESP  0x0008
#define WriteFile_RESP 0x0009
#define TIMER          0x000a
#define NOTHING        0x000b

#define DATA_LEN       512
#define DATA_BUF_LEN   516
#define ACK_BUF_LEN    4
#define ERR_BUF_LEN    100	
#define RECV_BUF_LEN   516

#define T1  2//2000
#define T2  1//1000
#define T3  1//1000
#define MAX 5

typedef struct _REQPacket
{
    unsigned short int opCode;
    string  fileName;
    BYTE    end_fileName;
    string  mode;
    BYTE    end_mode;
} REQPacket;

typedef struct _DATAPacket
{
    unsigned short int opCode;
    unsigned short int seq; /*block#*/
    char data[DATA_LEN];
} DATAPacket;

typedef struct _ACKPacket
{
    unsigned short int opCode;
    unsigned short int seq; /*block#*/
} ACKPacket;

typedef struct _ERRPacket
{
    unsigned short int opCode;
    unsigned short int errCode;
    string errMsg;
    BYTE  end_errMsg;
} ERRPacket;



extern SOCKET serverSock;
extern struct sockaddr_in local, from;
extern WSADATA wsadata;
extern bool flag;
extern char listenBuf[RECV_BUF_LEN];

void clear (char*a, int size);
void startUp (void);

DWORD WINAPI handleRRQThread (LPVOID p);
DWORD WINAPI handleWRQThread (LPVOID p);

unsigned short int getOpCode (char *p);
void code_REQPacket (REQPacket packet, char*buf);
void code_DATAPacket (DATAPacket packet, char*buf);
void code_ACKPacket (ACKPacket packet, char*buf);
void code_ERRPacket (ERRPacket packet, char*buf);
REQPacket decode_REQPacket (char*buf);
DATAPacket decode_DATAPacket (char*buf);
ACKPacket decode_ACKPacket (char*buf);
ERRPacket decode_ERRPacket (char*buf);

bool isFileExist (string filename);
bool openFile (FILE * * pfile, REQPacket packet);
void closeFile (FILE * * pfile);
int readFile (FILE * * pfile, char * buf);
void writeFile (FILE * * pfile, char * buf);

unsigned short int s_IDLE (REQPacket packet);
unsigned short int s_WAIT_R_RESP (char*dataBuf, int & reSendTimes, unsigned short int & sendSeqNo, FILE * *pfile,
    REQPacket packet, SOCKET & commSock, sockaddr_in & t_from);
unsigned short int s_WAIT_ACK (char*dataBuf, int & reSendTimes, unsigned short int & sendSeqNo, FILE * *pfile,
    SOCKET & commSock, sockaddr_in & t_from);
unsigned short int s_WAIT_LAST_ACK (char*dataBuf, int & reSendTimes, unsigned short int & sendSeqNo,
    FILE * *pfile, SOCKET & commSock, sockaddr_in & t_from);
unsigned short int s_WAIT_W_RESP (int & reSendTimes, unsigned short int & recvSeqNo,
    SOCKET & commSock, sockaddr_in & t_from);
unsigned short int s_WAIT_FIRST_P (int & reSendTimes, unsigned short int & recvSeqNo, FILE * * pfile,
    REQPacket packet, SOCKET & commSock, sockaddr_in & t_from);
unsigned short int s_WAIT_NEXT_P (int & reSendTimes, unsigned short int & recvSeqNo,
    FILE * * pfile, SOCKET & commSock, sockaddr_in & t_from);

#endif // !defined(AFX_TFTP_SERVER_H__41FD7E46_297D_42AB_98C5_81DED2604D38__INCLUDED_)
