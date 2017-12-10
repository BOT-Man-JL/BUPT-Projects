#include <iostream>

#include "client.h"
#include "conio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL bStart = TRUE;
string errmsg;    //错误信息,只用于发送时


BOOL SendUser(unsigned short int nType, BOOL nerr) {
	switch (nType) {
		case ReadFile_CONF:
			if (!nerr) { 
				printf("提示：文件下载已完成。\n\n");
			}
			else {
				printf("提示：文件下载失败。\n出现错误：");
				cout << errmsg << endl << endl;
			}
			memset(g_sFileName, '\0', 50);
			bStart = FALSE;
			break;
		case WriteFile_CONF:
			//printf("WRITE FILE SEND CONFIRM %s \n", g_sFileName);
			if (!nerr) {
				printf("提示：文件上传已完成。\n\n");
			}
			else {
				printf("提示：文件上传失败。\n出现错误：");
				cout << errmsg << endl << endl;
			}
			memset(g_sFileName, '\0', 50);
			bStart = FALSE;
			break;
	}
	return TRUE;
}

BOOL GetFromUser() {
	char ch;
	if (bStart) {
		cout << "选项：\n1. 下载文件。\n2. 上传文件。\n";
		cout << "n. 退出\n\n";
		cout << "请输入选项：";
		bStart = FALSE;
	}

	cin >> ch;
	if (ch == 'n' || ch == 'N') {
		cout << "感谢您的使用。" << endl << endl;
		exit(0);
	}
	else if (ch == '1') {
		g_nEvent = ReadFile_REQ;
	}
	else if (ch == '2') {
		g_nEvent = WriteFile_REQ;
	}
	cout << "\n输入文件名：";
	cin >> g_sFileName;
	cout << endl;
	cout << "提示：文件为 " << g_sFileName << " 。" << endl << endl;

	return TRUE;
}