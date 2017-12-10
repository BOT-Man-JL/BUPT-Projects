#include <iostream>

#include "server.h"

void sendERR (unsigned short int errCode, string errMsg, SOCKET & commSock, sockaddr_in & t_from);
int sendDATA (unsigned short int seq, char * dataBuf, FILE * * pfile, SOCKET & commSock, sockaddr_in & t_from);
void reSendDATA (char * dataBuf, SOCKET & commSock, sockaddr_in & t_from);
void sendACK (unsigned short int seq, SOCKET & commSock, sockaddr_in & t_from);
void reSendACK (unsigned short int seq, SOCKET & commSock, sockaddr_in & t_from);
void plusOne (unsigned short int & seq);
int recv (char * recvBuf, int timeout, SOCKET & commSock, sockaddr_in & t_from);

// 返回下一个状态,如果返回的是ERROR_OVER，说明出错了(发了错误包，或者收到错误包,或者超时5次)。均是输入参数
unsigned short int s_IDLE (REQPacket packet) {
    switch (packet.opCode) {
    case RRQ:
        cout << "提示：读取文件中。" << endl << endl; // 模拟指示原语
        return WAIT_R_RESP;
    case WRQ:
        cout << "提示：写入文件中。" << endl << endl;
        return WAIT_W_RESP;
    default:
        cout << "提示：状态未知。" << endl << endl;
        return ERROR_OVER;
    }
}

//前四个参数均是输出参数，后三个为输入参数
unsigned short int s_WAIT_R_RESP (char*dataBuf, int & reSendTimes, unsigned short int & sendSeqNo,
    FILE * *pfile, REQPacket packet, SOCKET & commSock, sockaddr_in & t_from) {
    cout << "提示：读取文件中。" << endl << endl;
    int t_fromLen = sizeof (t_from);
    if (!isFileExist (packet.fileName)) //出现文件不存在的错误,发ERR包
    {
        sendERR (1, "File not found", commSock, t_from);
        cout << "提示：下载失败，文件未找到。（WAIT_R_RESP）" << endl << endl;
        return ERROR_OVER;
    }

    //文件存在，但打不开
    if (!openFile (pfile, packet)) {
        sendERR (0, "File cannot be opened", commSock, t_from);
        cout << "提示：下载失败，文件无法打开。（WAIT_R_RESP）" << endl << endl;
        return ERROR_OVER;
    }

    //文件打开了，读取第一包数据(这也可能是最后一包数据)
    sendSeqNo = 1;
    clear (dataBuf, DATA_BUF_LEN);
    int len = sendDATA (sendSeqNo, dataBuf, pfile, commSock, t_from);
    cout << "提示：发送数据包 No." << sendSeqNo << " = " << len + 4 << "B" << endl << endl;
    reSendTimes = 0;
    //判断是不是最后一包数据
    if (len < DATA_LEN) //最后一包
    {
        return WAIT_LAST_ACK;
    }
    else  //不是最后一包
    {
        return WAIT_ACK;
    }
}

//前三个参数既是输入参数，又是给WAIT_LAST_ACK准备的输出参数，后三个为输入参数
unsigned short int s_WAIT_ACK (char*dataBuf, int & reSendTimes, unsigned short int & sendSeqNo,
    FILE * *pfile, SOCKET & commSock, sockaddr_in & t_from) {
    //上一次发送的数据在dataBuf里面，用于重传
    int t_fromLen = sizeof (t_from);
    unsigned short int sourcePort = ntohs (t_from.sin_port); //保留源端口号，用于后面检查
    char recvBuf[RECV_BUF_LEN];
    clear (recvBuf, RECV_BUF_LEN);

    //启动T3,等待ACK的到来
    int timeout = T3;
    int ret = 0;
    int len = 0;
    while (1) {
        clear (recvBuf, RECV_BUF_LEN);
        ret = recv (recvBuf, timeout, commSock, t_from);
        if (ret > 0) //收到packet
        {
            //先判断源端口号是否正确
            if (ntohs (t_from.sin_port) != sourcePort) //源端口号错，发err包，但是不中止传输
            {
                sendERR (0, "Source port is wrong", commSock, t_from);
                continue;
            }

            //源端口号正确
            switch (getOpCode (recvBuf)) {
            case ACK:
                if (decode_ACKPacket (recvBuf).seq != sendSeqNo) {
                    break; //不做处理，继续等待
                }
                else  //正确收到ACK
                {
                    plusOne (sendSeqNo);
                    clear (dataBuf, DATA_BUF_LEN);
                    len = sendDATA (sendSeqNo, dataBuf, pfile, commSock, t_from);
                    cout << "提示：发送数据包 No." << sendSeqNo << " = " << len + 4 << "B" << endl << endl;
                    reSendTimes = 0;
                    if (len < DATA_LEN) {
                        return WAIT_LAST_ACK;
                    }
                    else {
                        break;
                    }
                }
            case ERR:
                cout << "提示：下载失败。（WAIT_ACK  " << decode_ERRPacket (recvBuf).errMsg << "）" << endl << endl;
                closeFile (pfile);  //注意关闭文件
                return ERROR_OVER;
            default:
                break;
            }
        }
        else//超时 
        {
            if (reSendTimes < MAX) //重传
            {
                reSendDATA (dataBuf, commSock, t_from);
                reSendTimes++;
                cout << "提示：发送数据包 No." << sendSeqNo << "。" << endl << endl;
                continue;
            }
            else {
                //sendERR(0,"timeout",commSock,t_from);
                closeFile (pfile);
                cout << "提示：下载失败，无法重发 WAIT_ACK。" << endl << endl;
                return ERROR_OVER;
            }
        }
    }
}



//参数均是输入参数
unsigned short int s_WAIT_LAST_ACK (char*dataBuf, int & reSendTimes, unsigned short int & sendSeqNo,
    FILE * *pfile, SOCKET & commSock, sockaddr_in & t_from) {
    //上一次发送的数据在dataBuf里面，用于重传
    int t_fromLen = sizeof (t_from);
    unsigned short int sourcePort = ntohs (t_from.sin_port); //保留源端口号，用于后面检查
    char recvBuf[RECV_BUF_LEN];
    clear (recvBuf, RECV_BUF_LEN);

    //启动T3,等待ACK的到来
    int timeout = T3;
    int ret = 0;
    int len = 0;
    while (1) {
        clear (recvBuf, RECV_BUF_LEN);
        ret = recv (recvBuf, timeout, commSock, t_from);
        if (ret > 0) //收到packet
        {
            //先判断源端口号是否正确
            if (ntohs (t_from.sin_port) != sourcePort) //源端口号错，发err包，但是不中止传输
            {
                sendERR (0, "Source port is wrong", commSock, t_from);
                continue;
            }

            //源端口号正确
            switch (getOpCode (recvBuf)) {
            case ACK:
                if (decode_ACKPacket (recvBuf).seq != sendSeqNo) {
                    break; //不做处理，继续等待
                }
                else  //正确收到最后一包的ACK
                {
                    cout << "提示：下载成功。" << endl << endl;
                    closeFile (pfile);
                    return SUCCESS_OVER;
                }
            case ERR:
                cout << "提示：下载失败。（WAIT_LAST_ACK " << decode_ERRPacket (recvBuf).errMsg << "）" << endl << endl;
                closeFile (pfile);  //注意关闭文件
                return ERROR_OVER;
            default:
                break;
            }
        }
        else//超时 
        {
            if (reSendTimes < MAX) //重传
            {
                reSendDATA (dataBuf, commSock, t_from);
                reSendTimes++;
                cout << "提示：发送 No." << sendSeqNo << "。" << endl << endl;
                continue;
            }
            else {
                //sendERR(0,"timeout",commSock,t_from);
                closeFile (pfile);
                cout << "提示：下载失败，无法重发 WAIT_LAST_ACK。" << endl << endl;
                return ERROR_OVER;
            }
        }
    }
}

//前三个是输出参数，后两个是输入参数
unsigned short int s_WAIT_W_RESP (int & reSendTimes, unsigned short int & recvSeqNo,
    SOCKET & commSock, sockaddr_in & t_from) {
    cout << "提示：写入文件。（WriteFile_RESP）" << endl << endl;
    recvSeqNo = 1;
    sendACK (recvSeqNo - 1, commSock, t_from);
    reSendTimes = 0;
    return WAIT_FIRST_P;
}



//前三个是输入输出参数，后四个是输入参数
unsigned short int s_WAIT_FIRST_P (int & reSendTimes, unsigned short int & recvSeqNo, FILE * * pfile,
    REQPacket packet, SOCKET & commSock, sockaddr_in & t_from) {
    //等待第一个数据的到来
    int t_fromLen = sizeof (t_from);
    unsigned short int sourcePort = ntohs (t_from.sin_port); //保留源端口号，用于后面检查
    char recvBuf[RECV_BUF_LEN];
    clear (recvBuf, RECV_BUF_LEN);

    //启动T2,等待data的到来
    int timeout = T2;
    int ret = 0;
    int len = 0;
    while (1) {
        clear (recvBuf, RECV_BUF_LEN);
        ret = recv (recvBuf, timeout, commSock, t_from);
        cout << "提示：接收数据包 No." << decode_DATAPacket (recvBuf).seq << " = " << ret << "B" << endl << endl;
        if (ret > 0) //收到packet
        {
            //先判断源端口号是否正确
            if (ntohs (t_from.sin_port) != sourcePort) //源端口号错，发err包，但是不中止传输
            {
                sendERR (0, "Source port is wrong", commSock, t_from);
                continue;
            }

            //源端口号正确
            switch (getOpCode (recvBuf)) {
            case DATA:
                if (decode_DATAPacket (recvBuf).seq != recvSeqNo) {
                    //重传ACK
                    if (reSendTimes < MAX) {
                        reSendACK (recvSeqNo - 1, commSock, t_from);
                        reSendTimes++;
                        cout << "提示：重传 ACK。" << recvSeqNo - 1 << endl << endl;
                        break;
                    }
                    else {
                        cout << "提示：上传失败，无法重传 WAIT_FIRST_P。" << endl << endl;
                        return ERROR_OVER;
                    }
                }
                else  //正确收到第一包数据
                {
                    if (isFileExist (packet.fileName)) //出现文件已存在的错误,发ERR包
                    {
                        sendERR (6, "File already exists", commSock, t_from);
                        cout << "提示：上传失败，文件已存在。" << endl << endl;
                        return ERROR_OVER;
                    }
                    //同名文件不存在,但无法创建新文件
                    if (!openFile (pfile, packet)) {
                        sendERR (0, "File cannot be created", commSock, t_from);
                        cout << "提示：上传失败，文件无法被创建。" << endl << endl;
                        return ERROR_OVER;
                    }
                    //创建了新文件，并往新文件中写入第一包数据
                    writeFile (pfile, decode_DATAPacket (recvBuf).data);
                    sendACK (decode_DATAPacket (recvBuf).seq, commSock, t_from);
                    plusOne (recvSeqNo);
                    reSendTimes = 0;
                    //判断这是不是最后一包数据，是的话，发完ACK即退出
                    len = ret - 4; //得到数据部分的长度
                    if (len == DATA_LEN) //说明不是最后一包
                    {
                        return WAIT_NEXT_P;
                    }
                    else {
                        closeFile (pfile);
                        cout << "提示：上传成功。" << endl << endl;
                        return SUCCESS_OVER;
                    }
                }
            case ERR:
                cout << "提示：上传失败，在 WAIT_FIRST_P 时，" << decode_ERRPacket (recvBuf).errMsg << endl << endl;
                return ERROR_OVER;
            default:
                break;
            }
        }
        else//超时 
        {
            if (reSendTimes < MAX) //重传
            {
                reSendACK (recvSeqNo - 1, commSock, t_from);
                reSendTimes++;
                cout << "提示：重传 ACK。" << recvSeqNo - 1 << endl << endl;
                continue;
            }
            else {
                //sendERR(0,"timeout",commSock,t_from);
                cout << "提示：上传失败，无法重传 WAIT_FIRST_P。" << endl << endl;
                return ERROR_OVER;
            }
        }
    }
}



//均是输入参数
unsigned short int s_WAIT_NEXT_P (int & reSendTimes, unsigned short int & recvSeqNo,
    FILE * * pfile, SOCKET & commSock, sockaddr_in & t_from) {
    int t_fromLen = sizeof (t_from);
    unsigned short int sourcePort = ntohs (t_from.sin_port); //保留源端口号，用于后面检查
    char recvBuf[RECV_BUF_LEN];
    clear (recvBuf, RECV_BUF_LEN);

    //启动T2,等待data的到来
    int timeout = T2;
    int ret = 0;
    int len = 0;
    while (1) {
        clear (recvBuf, RECV_BUF_LEN);
        ret = recv (recvBuf, timeout, commSock, t_from);
        cout << "提示：接收数据包 No." << decode_DATAPacket (recvBuf).seq << " = " << ret << "B" << endl << endl;
        if (ret > 0) //收到数据
        {
            //先判断源端口号是否正确
            if (ntohs (t_from.sin_port) != sourcePort) //源端口号错，发err包，但是不中止传输
            {
                sendERR (0, "Source port is wrong", commSock, t_from);
                continue;
            }

            //源端口号正确
            switch (getOpCode (recvBuf)) {
            case DATA:
                if (decode_DATAPacket (recvBuf).seq != recvSeqNo) {
                    //重传ACK
                    if (reSendTimes < MAX) {
                        reSendACK (recvSeqNo - 1, commSock, t_from);
                        reSendTimes++;
                        cout << "提示：重发 ACK：" << recvSeqNo - 1 << endl << endl;
                        break;
                    }
                    else {
                        closeFile (pfile);
                        cout << "提示：上传失败，无法重传。（WAIT_NEXT_P）" << endl << endl;
                        return ERROR_OVER;
                    }
                }
                else  //正确收到数据
                {
                    //写数据
                    writeFile (pfile, decode_DATAPacket (recvBuf).data);
                    sendACK (decode_DATAPacket (recvBuf).seq, commSock, t_from);
                    plusOne (recvSeqNo);
                    reSendTimes = 0;
                    //判断这是不是最后一包数据，是的话，发完ACK即退出
                    len = ret - 4; //得到数据部分的长度
                    if (len == DATA_LEN) //说明不是最后一包
                    {
                        break;
                    }
                    else {
                        closeFile (pfile);
                        cout << "提示：上传成功。" << endl << endl;
                        return SUCCESS_OVER;
                    }
                }
            case ERR:
                cout << "提示：上传失败，在 WAIT_NEXT_P 中，" << decode_ERRPacket (recvBuf).errMsg << endl << endl;
                closeFile (pfile);
                return ERROR_OVER;
            default:
                break;
            }
        }
        else//超时 
        {
            if (reSendTimes < MAX) //重传
            {
                reSendACK (recvSeqNo - 1, commSock, t_from);
                reSendTimes++;
                cout << "提示：重传 ACK：" << recvSeqNo - 1 << endl << endl;
                continue;
            }
            else {
                //sendERR(0,"timeout",commSock,t_from);
                cout << "提示：上传失败，无法重发。（WAIT_NEXT_P）" << endl << endl;
                closeFile (pfile);
                return ERROR_OVER;
            }
        }
    }
}



void sendERR (unsigned short int errCode, string errMsg, SOCKET & commSock, sockaddr_in & t_from) {
    int t_fromLen = sizeof (t_from);
    ERRPacket ERRpacket;
    ERRpacket.opCode = ERR;
    ERRpacket.errCode = errCode;
    ERRpacket.errMsg = errMsg;
    ERRpacket.end_errMsg = '\0';
    char errBuf[ERR_BUF_LEN];
    clear (errBuf, ERR_BUF_LEN);
    code_ERRPacket (ERRpacket, errBuf);
    sendto (commSock, errBuf, ERR_BUF_LEN, 0, (struct sockaddr*)&t_from, t_fromLen);
}



//dataBuf是输出参数，存放刚刚发出的包，以待重传,返回值是data的长度
int sendDATA (unsigned short int seq, char * dataBuf, FILE * * pfile, SOCKET & commSock, sockaddr_in & t_from) {
    int t_fromLen = sizeof (t_from);
    DATAPacket DATApacket;
    DATApacket.opCode = DATA;
    DATApacket.seq = seq;
    clear (DATApacket.data, DATA_LEN);  //注意!!!
    int len = readFile (pfile, DATApacket.data);

    clear (dataBuf, DATA_BUF_LEN);
    code_DATAPacket (DATApacket, dataBuf);
    //判断是否是最后一包
    if (len == DATA_LEN)  //不是最后一包
    {
        sendto (commSock, dataBuf, DATA_BUF_LEN, 0, (struct sockaddr*)&t_from, t_fromLen);
    }
    else  //是最后一包，这时不能直接传dataBuf
    {
        char *p_lastDataBuf = new char[4 + len];
        clear (p_lastDataBuf, 4 + len);
        memcpy (p_lastDataBuf, dataBuf, 4 + len);
        sendto (commSock, p_lastDataBuf, 4 + len, 0, (struct sockaddr*)&t_from, t_fromLen);
    }
    return len;
}



void reSendDATA (char * dataBuf, SOCKET & commSock, sockaddr_in & t_from) {
    int t_fromLen = sizeof (t_from);
    sendto (commSock, dataBuf, DATA_BUF_LEN, 0, (struct sockaddr*)&t_from, t_fromLen);
}

void sendACK (unsigned short int seq, SOCKET & commSock, sockaddr_in & t_from) {
    int t_fromLen = sizeof (t_from);
    ACKPacket ACKpacket;
    ACKpacket.opCode = ACK;
    ACKpacket.seq = seq;
    char ackBuf[ACK_BUF_LEN];
    clear (ackBuf, ACK_BUF_LEN);
    code_ACKPacket (ACKpacket, ackBuf);
    sendto (commSock, ackBuf, ACK_BUF_LEN, 0, (struct sockaddr*)&t_from, t_fromLen);
}



void reSendACK (unsigned short int seq, SOCKET & commSock, sockaddr_in & t_from) {
    int t_fromLen = sizeof (t_from);
    ACKPacket ACKpacket;
    ACKpacket.opCode = ACK;
    ACKpacket.seq = seq;
    char ackBuf[ACK_BUF_LEN];
    clear (ackBuf, ACK_BUF_LEN);
    code_ACKPacket (ACKpacket, ackBuf);
    sendto (commSock, ackBuf, ACK_BUF_LEN, 0, (struct sockaddr*)&t_from, t_fromLen);
}



void plusOne (unsigned short int & seq) {
    if (seq == 65535) {
        seq = 1;
    }
    else {
        seq++;
    }
}

int recv (char * recvBuf, int timeout, SOCKET & commSock, sockaddr_in & t_from) {
    int ret = 0;
    int t_fromLen = sizeof (t_from);
    int i = 0;
    while (i < timeout * 100) {
        clear (recvBuf, RECV_BUF_LEN);
        ret = recvfrom (commSock, recvBuf, RECV_BUF_LEN, 0, (struct sockaddr*)&t_from, &t_fromLen);
        if (ret >= 0)  //表示真正接收到packet
        {
            return ret;
        }
        else  //返回SOCKET_ERROR
        {
            Sleep (10);  //每隔10ms查询一次
            i++;
            continue;
        }
    }
    return -1;  //超时
}

