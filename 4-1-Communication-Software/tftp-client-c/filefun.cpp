#undef UNICODE

#include "client.h"
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

FILE *file;

BOOL isFileExist(char *filename) {
	//判断文件是否存在
	HANDLE  hFindFile;
	WIN32_FIND_DATA  FileData;

	hFindFile = FindFirstFile("resource\\*", &FileData);   //处理"."这个文件

	if (hFindFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	else {
		//处理打印出的文件名的前两行，第一行是".",第二行是".."。
		//"."表示当前目录,".."表示上层目录.DOS的文件系统中把这两个都算作文件,不过大小为0
		FindNextFile(hFindFile, &FileData);  //这是为了处理".."那个文件
		char br[2] = { '\n','\0' };
		while (FindNextFile(hFindFile, &FileData)) {
			if (strstr(FileData.cFileName, filename) == NULL)
				continue;
			else {
				FindClose(hFindFile); //说明文件存在
				return TRUE;
			}
		}
		FindClose(hFindFile);
		return FALSE;
	}
}

BOOL FileOpen(char * sFileName, unsigned short int nType, char * mode) {
	int flag;
	string path = "resource\\";
	string filePath = path + sFileName;

	if (nType == READ) {
		// 读文件，上传
		if (strcmp(mode, mode1_down) == 0 || strcmp(mode, mode1_up) == 0) {
			flag = fopen_s(&file, filePath.c_str(), "r");
			if (flag == 0)
				return true;
			else
				return false;
		}
		else if (strcmp(mode, mode2_down) == 0 || strcmp(mode, mode2_up) == 0) {
			flag = fopen_s (&file, filePath.c_str (), "br");
			if (flag == 0)
				return true;
			else
				return false;
		}
	}
	else {
		//写文件,下载
		if (strcmp(mode, mode1_down) == 0 || strcmp(mode, mode1_up) == 0) {
			// 文本
			flag = fopen_s (&file, filePath.c_str (), "w");
			if (flag == 0)
				return true;
			else
				return false;
		}
		else if (strcmp(mode, mode2_down) == 0 || strcmp(mode, mode2_up) == 0) {
			flag = fopen_s (&file, filePath.c_str (), "wb");
			if (flag == 0)
				return true;
			else
				return false;
		}
		else return false;
	}
	return false;
}

BOOL FileClose(char * sFileName) {
    fclose (file);
	return true;
}

//ptr是输出参数，大小为512字节,返回值是实际读到的字符数
int FileRead(char * sFileName, int & nLength, char * ptr) {
	int len = 0;
	memset(ptr, 0, 512);
    len = fread (ptr, 1, nLength, file);
	return len;
}

//prt是输入参数
BOOL FileWrite(char * sFileName, int nLength, char * ptr) {
    fwrite (ptr, 1, nLength, file);
	return true;
}
