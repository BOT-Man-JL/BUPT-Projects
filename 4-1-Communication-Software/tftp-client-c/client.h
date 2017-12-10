
#if !defined(AFX_CLIENT_H__C0B708B3_BC63_4935_97BA_8DC8845648DF__INCLUDED_)
#define AFX_CLIENT_H__C0B708B3_BC63_4935_97BA_8DC8845648DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

using namespace std;

#include <winsock2.h>
#pragma comment(lib, "WS2_32")	// 链接到WS2_32.lib

#define IDLE          0
#define WAIT_FIRST_P  1
#define WAIT_NEXT_P   2
#define WAIT_ACK      3
#define WAIT_LAST_ACK 4

#define RRQ   0x0001
#define WRQ   0x0002
#define DATA  0x0003
#define ACK   0x0004
#define ERR   0x0005

#define ReadFile_REQ   0x0005
#define ReadFile_CONF  0x0006
#define WriteFile_REQ  0x0007
#define WriteFile_CONF 0x0008

#define DATALEN 512
#define MAX 5  //最大重发次数为5
#define TID 69  //端口号

#define WRITE 9
#define READ  10

extern int T1; 
extern int T2; 
extern int T3;
extern int T4; 

extern int T1ELAPSE; 
extern int T2ELAPSE; 
extern int T3ELAPSE;
extern int T4ELAPSE; 

extern int g_nState;
extern int g_nEvent; //ReadFile_REQ,WriteFile_REQ
extern unsigned short int Type;     //ACK,DATA,RRQ,WRQ,ERR,ReadFile_CONF,WriteFile_CONF

extern int  g_nport;  //记录与服务器通信的目的端口号

extern int g_nSendTimes;
extern unsigned short int g_nSendSeqNo;
extern unsigned short int g_nRecvSeqNo;

extern char g_ptr[512];
extern char g_sFileName[50];
extern int  g_nLength;

extern char g_TXBuf[517];
extern char g_RXBuf[517];
extern int  g_nTXBufLength;
extern int  g_nRXBufLength;

extern string errmsg;
extern unsigned short int errcode;
extern string Mode;
extern BOOL g_bNeedSendPacket;
extern BOOL g_bPacketRecv;

extern BOOL err;     // FALSE说明没有出错, TRUE说明出错了

extern char *mode1_down;
extern char *mode1_up;
extern char *mode2_down;
extern char *mode2_up;


typedef struct _RRQPacket
{
    unsigned short int opcode;    /* Code 01							*/
    string  filename;             /* file name lenght,filename[50]=0	*/
	BYTE    end_filename;
    string  mode;                 /* mode：netascii or octet,mode[8]=0	*/ 
	BYTE    end_mode;
} RRQPacket;

typedef struct _WRQPacket
{
    unsigned short int opcode;    /* Code 02							*/
    string  filename;             /* File name length					*/
	BYTE    end_fileName;
    string  mode;                 /* Mode：Ascii or Binary				*/
	BYTE    end_mode;
} WRQPacket;

typedef struct _dataPacket
{
    unsigned short int  opcode;  /* Code 03								*/
    unsigned short int  seq;     /* Sequence Number						*/
    char     data[DATALEN];      /* Buffer								*/
} DataPacket;

typedef struct _ACKPacket
{
    unsigned short int opcode;  /* Code 04								*/
    unsigned short int seq;     /* Sequence Number						*/
} ACKPacket;

typedef struct _ERRORPacket
{
    unsigned short int opcode;   /* Code 05								*/
    unsigned short int err_code; /* Error code							*/
    string  errstring;           /* Error message						*/
	BYTE    end_err;
} ERRORPacket;


BOOL TFTP_IDLE();
BOOL TFTP_WAIT_FIRST_P();
BOOL TFTP_WAIT_NEXT_P();
BOOL TFTP_WAIT_ACK();
BOOL TFTP_WAIT_LAST_ACK();

BOOL isFileExist(char *filename);
BOOL FileOpen(char * sFileName, unsigned short int nType,char *mode);
BOOL FileClose(char * sFileName);
int  FileRead(char * sFileName, int & nLength,char * ptr);
BOOL FileWrite(char * sFileName, int nLength,char * ptr);

BOOL SendPacket(unsigned short int nType);
BOOL RecvPacket(unsigned short int nType);
int recvd(char * recvBuf,int timeout,SOCKET & commSock,sockaddr_in & t_from);

BOOL GetFromUser();

#endif // !defined(AFX_CLIENT_H__C0B708B3_BC63_4935_97BA_8DC8845648DF__INCLUDED_)
