#include "server.h"
using namespace std;

unsigned short int getOpCode(char *p) {
	unsigned short int opCode;
	memcpy(&opCode, p, 2);
	//opCode=ntohs(opCode);  //转化为本地字节序
	return opCode;
}

void code_REQPacket(REQPacket packet, char*buf) {
	//buf是输出参数
	//packet.opCode = htons(packet.opCode); //转化为网络字节序
	memcpy(buf, &packet, 2); //opcode
	string filename = packet.fileName;
	memcpy(buf + 2, filename.data(), filename.length()); //filename
	string mode = packet.mode;
	memcpy(buf + 2 + filename.length() + 1, mode.data(), mode.length()); //mode
}

void code_DATAPacket(DATAPacket packet, char*buf) {
	//packet.opCode = htons(packet.opCode);//转化为网络字节序
	//packet.seq = htons(packet.seq);
	memcpy(buf, &packet, 4 + DATA_LEN); //opCode+seq+data
}

void code_ACKPacket(ACKPacket packet, char*buf) {
	//packet.opCode = htons(packet.opCode); //转化为网络字节序
	//packet.seq = htons(packet.seq);
	memcpy(buf, &packet, 4); //opCode+seq
}

void code_ERRPacket(ERRPacket packet, char*buf) {
	//packet.opCode = htons(packet.opCode); //转化为网络字节序
	//packet.errCode = htons(packet.errCode);
	memcpy(buf, &packet, 4); //opCode+errCode
	string errMsg = packet.errMsg;
	memcpy(buf + 4, errMsg.data(), errMsg.length());//errMsg
}

REQPacket decode_REQPacket(char * buf) {
	REQPacket packet;
	memcpy(&packet.opCode, buf, 2); //opcode
	string filename(buf + 2);
	packet.fileName = filename;  //filename
	string mode(buf + 2 + filename.length() + 1); //mode
	packet.mode = mode;
	packet.end_fileName = '\0'; //end_fileName
	packet.end_mode = '\0'; //end_mode
							//packet.opCode = ntohs(packet.opCode); //转化为本机字节序
	return packet;
}

DATAPacket decode_DATAPacket(char * buf) {
	DATAPacket packet;
	memcpy(&packet.opCode, buf, 2); //opCode
	memcpy(&packet.seq, buf + 2, 2); //seq
	clear(packet.data, DATA_LEN);//注意先清零
	memcpy(packet.data, buf + 4, DATA_LEN);//data
										   //packet.opCode = ntohs(packet.opCode);
										   //packet.seq = ntohs(packet.seq);
	return packet;
}

ACKPacket decode_ACKPacket(char * buf) {
	ACKPacket packet;
	memcpy(&packet.opCode, buf, 2); //opCode
	memcpy(&packet.seq, buf + 2, 2); //seq
									 //packet.opCode = ntohs(packet.opCode);
									 //packet.seq = ntohs(packet.seq);
	return packet;
}

ERRPacket decode_ERRPacket(char * buf) {
	ERRPacket packet;
	memcpy(&packet.opCode, buf, 2); //opCode
	memcpy(&packet.errCode, buf + 2, 2); //errCode
	string errMsg(buf + 4);
	packet.errMsg = errMsg; //errMsg
	packet.end_errMsg = '\0'; //end_err_Msg
							  //packet.opCode = ntohs(packet.opCode);
							  //packet.errCode = ntohs(packet.errCode);
	return packet;
}
