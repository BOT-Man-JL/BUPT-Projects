#include <iostream>

#include "client.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#pragma pack( push, _normal_pack_ )
#pragma pack(1)

char *mode1_down = "netascii";
char *mode1_up = "netascii";
char *mode2_down = "octet";
char *mode2_up = "OCTET";

/////////////////////////////////////////////////////////////////
#pragma pack( pop, _normal_pack_ )

unsigned short int errcode;
string Mode;

char g_TXBuf[517];   //用于发送的缓冲区
char g_RXBuf[517];   //用于接收的缓冲区
int  g_nTXBufLength = 0;
int  g_nRXBufLength = 0;

BOOL g_bNeedSendPacket = FALSE;
BOOL g_bPacketRecv = FALSE;

void packeting(unsigned short int ntype, char *filename, string mode, char *sendbuf) {//封装RRQ和WRQ包，根据ntype来确定
	int lengf = strlen(filename);
	int lengm = mode.size();
	if (ntype == RRQ) {
		RRQPacket pack;
		RRQPacket *p = &pack;
		p->opcode = RRQ;
		memcpy(sendbuf, &ntype, 2);
	}
	else if (ntype == WRQ) {
		WRQPacket pack;
		WRQPacket *p = &pack;
		p->opcode = WRQ;
		p->filename = filename;
		memcpy(sendbuf, &ntype, 2);
	}

	memcpy(sendbuf + 2, filename, lengf);
	sendbuf[2 + lengf] = '\0';         //end_filename='\0'
	memcpy(sendbuf + 3 + lengf, mode.data(), lengm);
	sendbuf[3 + lengf + lengm] = '\0';   //end_mode='0'
	sendbuf[3 + lengf + lengm + 1] = 0;   //结束!
	g_nTXBufLength = 4 + lengf + lengm;
}

BOOL SendPacket(unsigned short int nType) {
	//cout<<"SendPacket()"<<endl;
	g_bNeedSendPacket = TRUE;
	memset(g_TXBuf, '\0', 516);    //清零

	switch (nType) {
		case DATA:
		{
			//cout<<"g_nSendSeqNo="<<g_nSendSeqNo<<endl;
			memcpy(g_TXBuf, &nType, 2);
			memcpy(g_TXBuf + 2, &g_nSendSeqNo, 2);

			printf("提示：发送数据包 %d。\n\n", g_nSendSeqNo);

			//int leng=strlen(g_ptr);
			printf("长度：%d\n\n", g_nLength);
			memcpy(g_TXBuf + 4, g_ptr, g_nLength);
			g_TXBuf[4 + g_nLength] = '\0';
			g_nTXBufLength = g_nLength + 4;
		}
		break;

		case ACK:
		{
			memcpy(g_TXBuf, &nType, 2);
			memcpy(g_TXBuf + 2, &g_nRecvSeqNo, 2);
			g_TXBuf[4] = 0;
			g_nTXBufLength = 4;
		}
		break;

		case ERR:
		{

			memcpy(g_TXBuf, &nType, 2);
			memcpy(g_TXBuf + 2, &errcode, 2);

			cout << "错误：" << errmsg << endl << endl;

			int size = errmsg.size();
			memcpy(g_TXBuf + 4, errmsg.data(), size);
			g_TXBuf[4 + size] = '\0';
			g_TXBuf[4 + size + 1] = 0;
			g_nTXBufLength = size + 5;
		}
		break;
		case RRQ:
		{
			packeting(nType, g_sFileName, Mode, g_TXBuf);
		}
		break;
		case WRQ:
		{
			packeting(nType, g_sFileName, Mode, g_TXBuf);
		}
		default:
			g_bNeedSendPacket = FALSE;
			break;
	}

	return TRUE;
}


BOOL RecvPacket(unsigned short int nType)   //leng=strlen(g_RXBuf);
{
	//cout<<"RecvPacket()"<<endl;
	g_bPacketRecv = TRUE;

	//printf("Receive packet,state = %d ", g_nState);
	unsigned short int seqno;    //用于接收包序号
	int leng = 512;

	switch (nType) {
		case DATA:
		{
			memset(g_ptr, 0, 512);   //清零
			memcpy(&seqno, g_RXBuf + 2, 2);
			if (g_nRecvSeqNo == seqno) {
				//cout<<"收到 DATA, SeqNo="<<seqno<<endl;
				//如果是接收第一包数据，则要先创建一个文件
				if (g_nState == WAIT_FIRST_P) {
					BOOL state = FileOpen(g_sFileName, WRITE, (char *)Mode.data());
					if (!state) {
						err = true;
						errcode = 0;
						errmsg = "Cannot create the file.";
						Type = ERR;
						SendPacket(Type);
						return true;
					}

				}
				//否则可以直接向该文件写入数据      
				memcpy(g_ptr, g_RXBuf + 4, leng);
				if (FileWrite(g_sFileName, leng, g_ptr)) {
					//cout<<" Write file success!"<<endl;
					Type = ACK;
					SendPacket(Type);
					return true;
				}
				else {
					FileClose(g_sFileName);
					err = true;
					errcode = 0;
					errmsg = "Load file failed.";
					Type = ERR;
					SendPacket(Type);
					g_nRXBufLength = 0;
					return true;
				}
			}

			else   //包序号不对,重发ACK 
			{
				memset(g_RXBuf, '\0', 516);
				Type = 0;
				return false;
			}
		}
		break;

		case ACK:
		{
			memcpy(&seqno, g_RXBuf + 2, 2);
			memset(g_RXBuf, '\0', 516);
			//cout<<"g_nSendSeqNo="<<g_nSendSeqNo<<endl;
			//cout<<"ACK seqno="<<seqno<<endl;
			if (g_nSendSeqNo == seqno) {
				if (g_nSendSeqNo == 0) {
					g_nSendSeqNo = 1;  //1
									   //当g_nSendSeqNo==0,说明收到ACK0，先打开文件再读取数据到g_ptr中
									   //cout<<" Receive ACK0 !"<<endl;
					BOOL state = FileOpen(g_sFileName, READ, (char *)Mode.data());
					if (!state) {
						err = true;
						errcode = 0;
						errmsg = "Cannot open the file.";
						Type = ERR;
						SendPacket(Type);
						return true;
					}
					else {
						g_nLength = 512;
						g_nLength = FileRead(g_sFileName, g_nLength, g_ptr);
						if (g_nLength == 512) {
							//cout<<" Read file success !"<<endl;
							Type = DATA; //准备传送下一包数据
							SendPacket(Type);
							g_nSendSeqNo = 0; //恢复为0,等待返回Type==DATA时把序号加1 
							return true;
						}
						else if ((g_nLength<512) && (g_nLength >= 0)) {
							//cout<<"读文件，所读得数据长度:"<<g_nLength<<endl;
							//最后一包数据
							//cout<<" Read file success ,the last packet !"<<endl;
							err = false;
							g_nSendSeqNo++;
							Type = DATA;
							SendPacket(Type);
							return true;
						}
					}
				}

				else {
					//cout<<"收到 ACK, SeqNo="<<seqno<<endl;
					if (g_nState == WAIT_LAST_ACK) {   //已经收到最后一个ACK，且序号正确
													   //g_nState=IDLE;
						Type = WriteFile_CONF;
						memset(g_TXBuf, '\0', 516); //可以将在g_prt中的最后一包数据清楚
						return true;
					}
					else {
						g_nLength = 512;
						g_nLength = FileRead(g_sFileName, g_nLength, g_ptr);
						if (g_nLength == 512) {
							//cout<<" Read file success !"<<endl;
							g_nSendSeqNo++;
							Type = DATA; //准备传送下一包数据
							SendPacket(Type);
							g_nSendSeqNo--;
							return true;
						}
						else if ((g_nLength<512) && (g_nLength >= 0)) {
							//最后一包数据
							//cout<<"读文件，所读得数据长度:"<<g_nLength<<endl;
							//cout<<" Read file success ,the last packet !"<<endl;
							err = false;
							g_nSendSeqNo++;
							Type = DATA;
							SendPacket(Type);
							return true;
						}

						else {
							//出现其他异常错误，终止 
							Type = WriteFile_CONF;
							FileClose(g_sFileName);
							err = true;
							errcode = 0;
							errmsg = "Load file failed.";
							Type = ERR;
							SendPacket(Type);
							return true;
						}
					}
				}
			}
			else return false;  //ACK序号不对，不作任何处理，采取超时重传机制

		}
		break;

		case ERR:
		{
			Type = 0;
			unsigned short int errtype;
			memcpy(&errtype, g_RXBuf + 2, 2);

			cout << "提示：收到错误代码：" << errtype << "的数据包。" << endl << endl;
			errmsg = g_RXBuf + 4;
			cout << "提示：错误信息为 " << errmsg << endl << endl;
			return true;
		}
		break;

		default:
			g_bPacketRecv = FALSE;
			break;
	}
	return false;
}

