// client.cpp : 整个程序的入口

#include "client.h"
#include "comm.h"
#include <stdlib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int g_nState = IDLE;
int g_nEvent = 0;
int g_nSendTimes = 0;  //数据包重发次数，最大为3次
unsigned short int g_nSendSeqNo = 0;
unsigned short int g_nRecvSeqNo = 0;

char g_ptr[512];
char g_sFileName[50];
int  g_nLength = 0;
BOOL err = FALSE;

int T1 = 2;
int T2 = 1;
int T3 = 1;
int T4 = 1;

int T1ELAPSE = 2000;
int T2ELAPSE = 1000;
int T3ELAPSE = 1000;
int T4ELAPSE = 1000;

unsigned short int Type = 0;  //ACK,DATA,RRQ,WRQ,ERR,ReadFile_CONF,WriteFile_CONF

BOOL g_nconnect = TRUE;   //表示与服务器连通情况

int  g_nport = 69;  //记录与服务器通信的目的端口号,最初使用端口号69

BOOL SendUser (unsigned short int nType, BOOL nerr);
BOOL GetFromUser ();

CInitSock initSock;	// 初始化Winsock库
SOCKET clientSock;
struct sockaddr_in local, from;
int fromLen = sizeof (from);

// main 函数

int main (int argc, TCHAR* argv[], TCHAR* envp[]) {
    int nRetCode = 0;

    // 清零
    memset (g_ptr, '\0', 512);
    memset (g_sFileName, '\0', 50);
    memset (g_TXBuf, '\0', 516);
    memset (g_RXBuf, '\0', 516);

    // 创建套节字
    //SOCKET clientSock;
    //struct sockaddr_in local,from;
    WSADATA wsadata;

    unsigned short port = 3500;
    char ip[16];
    char ch;         //mode type

    memset (ip, 0, sizeof (ip));
    cout << "============================" << endl;
    cout << "=                          =" << endl;
    cout << "=    TFTP 协议 - 客户端    =" << endl;
    cout << "=                          =" << endl;
    cout << "============================" << endl << endl;
    cout << "请输入服务器的ip地址: ";
    cin >> ip;
    cout << endl << "服务器的端口号: 69" << endl << endl;
    //cout << "请选择传输模式: 1. netascii | NETASCII    2 .binary | BINARY \n";
    //cin >> ch;

    //if (ch == '1')
    Mode = "netascii";
    //if (ch == '2')
    //	Mode = "octet";

    if (WSAStartup (MAKEWORD (2, 2), &wsadata) != 0) {
        cout << "错误：无法启动 Winsock 服务。" << endl << endl;
        return 0;
    }

    cout << "提示：Winsock 服务已启动。" << endl << endl;

    from.sin_family = AF_INET;
    from.sin_port = htons (69);
    from.sin_addr.S_un.S_addr = inet_addr (ip);  //inet_addr直接返回网络字节序

    clientSock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSock == INVALID_SOCKET) {
        printf ("错误：初始化 Socket 失败 %d。\n\n", ::WSAGetLastError ());
        return 0;
    }

    cout << "提示：端口 69 可以使用。" << endl << endl;

    local.sin_family = AF_INET;
    local.sin_port = htons (port);
    local.sin_addr.S_un.S_addr = htonl (INADDR_ANY);

    if (bind (clientSock, (struct sockaddr*)&local, sizeof (local)) != 0) {
        cout << "错误：端口 69 无法使用。" << endl << endl;
        closesocket (clientSock);
        WSACleanup ();
        return 0;
    }

    unsigned long argp = 65535;
    int ioc = ioctlsocket (clientSock, FIONBIO, &argp);
    if (ioc == SOCKET_ERROR) {
        cout << "错误：套接口设置为非阻塞模式失败。" << endl << endl;
        closesocket (clientSock);
        WSACleanup ();
        return 0;
    }

    GetFromUser ();      //来自用户传输文件的请求
    while (g_nconnect) {
        switch (g_nState) {
        case IDLE:
            TFTP_IDLE ();
            break;
        case WAIT_FIRST_P:
            TFTP_WAIT_FIRST_P ();
            break;
        case WAIT_NEXT_P:
            TFTP_WAIT_NEXT_P ();
            break;
        case WAIT_ACK:
            TFTP_WAIT_ACK ();
            break;
        case WAIT_LAST_ACK:
            TFTP_WAIT_LAST_ACK ();
            break;
        default:
            g_nconnect = false;
        }
    }
    // 关闭Socket
    closesocket (clientSock);
    system ("PAUSE");
    return nRetCode;
}


BOOL TFTP_IDLE () {
    //由Type判断是发RRQ还是WRQ包，封包并且发送后进入下一个状态
    cout << "----------------------------" << endl;
    cout << "客户端状态：空闲。" << endl << endl;
    int send = 0;
    if (g_nEvent == 0) {
        g_nconnect = false;
        return FALSE;
    }
    switch (g_nEvent) {
    case ReadFile_REQ:
        if (isFileExist (g_sFileName)) {
            // 当本地有同名文件时不再下载
            cout << "提示：在本地存在同名文件，已停止下载。" << endl << endl;
            g_nState = IDLE;
            g_nconnect = false;
            return FALSE;
        }
        else {
            g_nRecvSeqNo = 1;
            Type = RRQ;
            SendPacket (Type);
            send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, sizeof (from));
            g_nState = WAIT_FIRST_P;
            g_nconnect = true;
        }
        break;

    case WriteFile_REQ:
        if (isFileExist (g_sFileName)) {
            // 当本地有这个文件时，上传文件
            g_nSendSeqNo = 0;
            Type = WRQ;
            SendPacket (Type);
            send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, sizeof (from));
            g_nState = WAIT_ACK;
            g_nconnect = true;
        }
        else {
            cout << "错误：上传文件不存在。" << endl << endl;
            g_nState = IDLE;
            g_nconnect = false;
            return FALSE;
        }
        break;

    default:
        break;
    }
    g_nEvent = 0;
    return TRUE;
}

BOOL TFTP_WAIT_FIRST_P () {
    //发出RRQ后等待第一包的到达
    cout << "----------------------------" << endl;
    cout << "客户端状态：等待第一个数据包到达。" << endl << endl;
    int send = 0;
    int ret = 0;
    BOOL recv = false;

    // 启动定时器，等待第一个DATA的到达
    // setsockopt(clientSock,SOL_SOCKET,SO_RCVTIMEO,(char *)&T1ELAPSE,sizeof(int));
    for (g_nSendTimes = 0; g_nSendTimes < MAX; g_nSendTimes++) {
        // ret=recvfrom(clientSock,g_RXBuf,516,0,(struct sockaddr*)&from,&fromLen);
        ret = recvd (g_RXBuf, T1, clientSock, from);
        g_nport = ntohs (from.sin_port);
        //连接后服务器和该客户端通信的端口号
        if (ret > 0) {
            unsigned short int operate;
            memcpy (&operate, g_RXBuf, 2);
            switch (operate) {
            case DATA:
                Type = DATA;
                recv = RecvPacket (Type);
                break;
            case ERR:
                Type = ERR;
                recv = RecvPacket (Type);
                err = true;
                Type = ReadFile_CONF;
                SendUser (Type, err);
                g_nState = IDLE;
                g_nconnect = false;
                break;
            default:
                Type = 0;
                break;
            }
            if (recv && (Type == ACK || Type == ERR || Type == ReadFile_CONF))
                break;
        }

        send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);
        cout << "提示：第 " << g_nSendTimes + 1 << " 次重传。" << endl << endl;
    }
    // 如果超时且重传次数>5则异常终止,或者收到的数据出现错误，异常终止
    // if(g_nSendTimes==MAX||Type==ERR)
    if (ret <= 0 || g_nSendTimes == MAX) {
        Type = ERR;
        err = TRUE;
        errcode = 0;
        errmsg = "错误：超时，传输失败。";
        g_nSendTimes = 0;

        Type = ReadFile_CONF;
        SendUser (Type, err);
        send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);

        memset (g_RXBuf, '\0', 516);
        g_nState = IDLE;
        g_nconnect = false;
        return FALSE;
    }

    if (Type == ERR) {
        errcode = 0;
        errmsg = "错误：文件操作失败。";
        g_nSendTimes = 0;

        Type = ReadFile_CONF;
        SendUser (Type, err);
        send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);

        memset (g_RXBuf, '\0', 516);
        g_nState = IDLE;
        g_nconnect = false;
        return FALSE;
    }

    if (Type == ACK) {
        // T2 = 1s：用于发ACK后，监视下一包数据包的到达
        // cout << "已经回最后一个数据包的ACK!" << endl;
        // setsockopt(clientSock,SOL_SOCKET,SO_RCVTIMEO,(char *)&T2ELAPSE,sizeof(int));
        sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);
        // ret=recvd(g_RXBuf,T2,clientSock,from);
        if ((ret - 4) < 512) {
            //说明已经回最后一个数据包的ACK,发出ACK后就退出
            FileClose (g_sFileName);
            memset (g_RXBuf, '\0', 516);
            Type = ReadFile_CONF;
            err = false;
            SendUser (Type, err);
            g_nState = IDLE;
            g_nconnect = false;
        }
        else {
            g_nRecvSeqNo++;
            g_nSendTimes = 0;
            memset (g_RXBuf, '\0', 516);
            g_nconnect = true;
            g_nState = WAIT_NEXT_P;
        }
    }
    return false;
}


BOOL TFTP_WAIT_NEXT_P () {
    //发出ACK后等待下一包的到达,接收时限 T2 = 1000ms
    cout << "----------------------------" << endl;
    cout << "客户端状态：等待下一个数据包到达。" << endl << endl;
    int send = 0;
    int ret;
    BOOL recv = false;
    int nport;
    // 目的端的端口号
    // 启动定时期T2，等待下一包DATA的到达
    // setsockopt(clientSock,SOL_SOCKET,SO_RCVTIMEO,(char *)&T2ELAPSE,sizeof(int));
    for (g_nSendTimes = 0; g_nSendTimes < MAX; g_nSendTimes++) {
        //#define MAX 5
        //ret=recvfrom(clientSock,g_RXBuf,516,0,(struct sockaddr*)&from,&fromLen);
        ret = recvd (g_RXBuf, T2, clientSock, from);
        nport = ntohs (from.sin_port);  // 此时收到的包的目的端口号
        if (g_nport != nport) {  // 此时不会断开连接
            errcode = 5;   // Unknown port
            errmsg = "错误：未知端口。";
            SendPacket (ERR);
            send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);
            continue;
        }

        if (ret > 0) {
            unsigned short int operate;
            memcpy (&operate, g_RXBuf, 2);
            switch (operate) {
            case DATA:
                Type = DATA;
                recv = RecvPacket (Type);
                break;
            case ERR:
                Type = ERR;
                recv = RecvPacket (Type);
                err = true;
                Type = ReadFile_CONF;
                SendUser (Type, err);
                g_nState = IDLE;

                FileClose (g_sFileName);
                g_nconnect = false;
                break;
            default:
                Type = 0;
                break;
            }
            if (recv && (Type == ACK || Type == ERR || Type == ReadFile_CONF))
                break;
        }

        send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);
        cout << "提示：第 " << g_nSendTimes + 1 << "次重传 ACK。" << endl << endl;
    }
    // 如果超时且重传次数 >= 5 或者收到的数据出现错误，异常终止
    if (ret <= 0 || g_nSendTimes == MAX) {
        Type = ERR;
        err = TRUE;
        errcode = 0;
        errmsg = "错误：超时，传输失败。";
        g_nSendTimes = 0;

        Type = ReadFile_CONF;
        SendUser (Type, err);
        send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);

        memset (g_RXBuf, '\0', 516);
        g_nState = IDLE;

        FileClose (g_sFileName);
        g_nconnect = false;
        return FALSE;
    }

    if (Type == ERR) {
        errcode = 0;
        errmsg = "错误：文件操作失败。";
        g_nSendTimes = 0;

        Type = ReadFile_CONF;
        SendUser (Type, err);
        send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);

        memset (g_RXBuf, '\0', 516);
        g_nState = IDLE;
        g_nconnect = false;
        return FALSE;
    }

    if (Type == ACK) {
        // T2 = 1s：用于发ACK后，监视下一包数据包
        // setsockopt(clientSock,SOL_SOCKET,SO_SNDTIMEO,(char *)&T2ELAPSE,sizeof(int));
        sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);
        // ret=recvd(g_RXBuf,T2,clientSock,from);
        if (ret < 512) {
            // 说明已经回复最后一个数据包的ACK
            // cout << "提示：已经回复最后一个数据包的ACK。" << endl;
            FileClose (g_sFileName);
            memset (g_RXBuf, '\0', 516);
            Type = ReadFile_CONF;
            err = false;
            SendUser (Type, err);
            g_nState = IDLE;
            g_nconnect = false;
        }
        else {
            g_nRecvSeqNo++;
            g_nSendTimes = 0;
            memset (g_RXBuf, '\0', 516);
            g_nconnect = true;
            g_nState = WAIT_NEXT_P;
        }
    }
    return true;
}


BOOL TFTP_WAIT_ACK () {   //发出WRQ或者DATA后等待ACK的到达， T3 = 1000ms
    cout << "----------------------------" << endl;
    cout << "客户端状态：等待ACK数据包。" << endl << endl;
    int send = 0;
    int ret;
    BOOL recv = false;
    // 启动定时器 T3 = 1000ms，等待ACK的到达
    // setsockopt(clientSock,SOL_SOCKET,SO_RCVTIMEO,(char *)&T3ELAPSE,sizeof(int));
    for (g_nSendTimes = 0; g_nSendTimes < MAX; g_nSendTimes++) {
        // MAX = 5
        // ret=recvfrom(clientSock,g_RXBuf,516,0,(struct sockaddr*)&from,&fromLen);
        ret = recvd (g_RXBuf, T3, clientSock, from);

        if (ret > 0) {
            unsigned short int operate;
            memcpy (&operate, g_RXBuf, 2);
            switch (operate) {
            case ACK:
                Type = ACK;
                recv = RecvPacket (Type);
                break;
            case ERR:
                Type = ERR;
                recv = RecvPacket (Type);
                err = true;
                Type = WriteFile_CONF;
                SendUser (Type, err);
                g_nState = IDLE;
                g_nconnect = false;
                break;
            default:
                Type = 0;
                break;
            }
            //当收到正确的ACK且能够正确封装下一包数据时Type=DATA
            if (recv && (Type == DATA || Type == ERR || Type == WriteFile_CONF))
                break;
        }

        send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);
        // cout << "端口号：" << ntohs(from.sin_port) << endl;
        if (g_nSendSeqNo == 0)
            cout << "提示：第 " << g_nSendTimes + 1 << " 次重传 WRQ。" << endl << endl;
        else
            cout << "提示：第 " << g_nSendTimes + 1 << " 次重传 DATA。" << endl << endl;
    }
    // 如果超时且重传次数>5则异常终止,或者收到的数据出现错误，异常终止
    if (ret <= 0 || g_nSendTimes == MAX) {
        Type = ERR;
        err = TRUE;
        errcode = 0;
        errmsg = "错误：超时，传输失败。";
        g_nSendTimes = 0;

        Type = ReadFile_CONF;
        SendUser (Type, err);
        send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);

        memset (g_RXBuf, '\0', 516);
        g_nState = IDLE;
        g_nconnect = false;
        return FALSE;
    }

    if (Type == ERR) {
        errcode = 0;
        errmsg = "错误：文件操作失败。";
        g_nSendTimes = 0;

        Type = WriteFile_CONF;
        SendUser (Type, err);
        send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);

        memset (g_RXBuf, '\0', 516);
        g_nState = IDLE;
        g_nconnect = false;
        return FALSE;
    }

    if (Type == DATA)   //当收到正确的ACK且能够正确封装下一包数据时Type=DATA;
    {
        //T3=1s：发送DATA，并且监视确认包ACK的到达
        sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);
        if (g_nLength < 512) { //说明已经回最后一个数据包DATA
            g_nState = WAIT_LAST_ACK;
            g_nconnect = true;
        }
        else {
            g_nSendSeqNo++;
            g_nSendTimes = 0;
            memset (g_RXBuf, '\0', 516);
            g_nconnect = true;
            g_nState = WAIT_ACK;
        }
    }
    return true;
}

BOOL TFTP_WAIT_LAST_ACK () {
    cout << "客户端状态：等待最后一个 ACK 数据包。" << endl << endl;
    int send = 0;
    int ret;
    BOOL recv = false;
    //启动定时期T3=1000ms，等待Last_ACK的到达
    //setsockopt(clientSock,SOL_SOCKET,SO_RCVTIMEO,(char *)&T3ELAPSE,sizeof(int));
    for (g_nSendTimes = 0; g_nSendTimes < MAX; g_nSendTimes++)      //#define MAX 5
    {
        int fromlen = sizeof (from);
        //ret=recvfrom(clientSock,g_RXBuf,516,0,(struct sockaddr*)&from,&fromLen);

        ret = recvd (g_RXBuf, T3, clientSock, from);
        if (ret > 0) {
            unsigned short int operate;
            memcpy (&operate, g_RXBuf, 2);
            switch (operate) {
            case ACK:
                Type = ACK;
                recv = RecvPacket (Type);
                break;
            case ERR:
                Type = ERR;
                recv = RecvPacket (Type);
                err = true;
                Type = ReadFile_CONF;
                SendUser (Type, err);
                g_nState = IDLE;
                g_nconnect = false;
                break;
            default:
                Type = 0;
                break;
            }
            if (recv || Type == WriteFile_CONF)
                break;
        }

        send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);
        cout << "提示：第 " << g_nSendTimes + 1 << " 次重传最后一个 DATA 包。" << endl << endl;
    }
    //如果超时且重传次数>5则异常终止,或者收到的数据出现错误，异常终止
    if ((ret <= 0 || g_nSendTimes == MAX) && (Type != WriteFile_CONF)) {
        Type = ERR;
        err = TRUE;
        errcode = 0;
        errmsg = "提示：超时，传输错误。";
        g_nSendTimes = 0;

        Type = WriteFile_CONF;
        SendUser (Type, err);
        send = sendto (clientSock, (const char *) g_TXBuf, g_nTXBufLength, 0, (struct sockaddr*)&from, fromLen);

        FileClose (g_sFileName);  //关闭文件
        memset (g_RXBuf, '\0', 516);
        memset (g_TXBuf, '\0', 516);
        g_nState = IDLE;
        g_nconnect = false;
        return FALSE;
    }

    else if (Type == WriteFile_CONF) {   //说明已经正确接收最后一个ACK
        FileClose (g_sFileName);
        err = FALSE;
        SendUser (Type, err);
        memset (g_TXBuf, '\0', 516);
        memset (g_RXBuf, '\0', 516);

        g_nState = IDLE;
        g_nconnect = false;
    }
    return false;
}

int recvd (char * recvBuf, int timeout, SOCKET & commSock, sockaddr_in & t_from) {
    int ret = 0;
    int t_fromLen = sizeof (t_from);
    int i = 0;
    while (i < timeout * 100) {
        memset (recvBuf, '\0', 516);  //每次使用前都要清零
        ret = recvfrom (commSock, recvBuf, 516, 0, (struct sockaddr*)&t_from, &t_fromLen);
        if (ret >= 0) {
            //表示真正接收到数据
            //cout<<"receive ret= "<<ret<<endl;
            return ret;
        }
        else {
            //返回SOCKET_ERROR
            Sleep (10);  //每隔10ms查询一次
            i++;
            continue;
        }
    }
    return -1;  //超时
}



