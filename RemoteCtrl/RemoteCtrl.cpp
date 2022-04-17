// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include <direct.h>	// 读取磁盘分区
#include <io.h>	// 查找文件
//#include <list>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

void Dump(BYTE* pData, size_t nSize)
{
	string strOut;
	for (size_t i = 0; i < nSize; i++)
	{
		char buf[8] = "";
		if (i > 0 && (i % 16) == 0)strOut += "\n";
		snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
		strOut += buf;
	}
	strOut += "\n";
	OutputDebugStringA(strOut.c_str());
}
// 查看磁盘分区
int MakeDriverInfo()
{
	string result;
	for (int i = 1; i <= 26; i++)
	{
		if (_chdrive(i) == 0)
		{
			// 找到一个分区
			if (result.size() > 0)
				result += ',';
			result += 'A' + i - 1;
		}
	}
	CPacket pack(1, (BYTE*)result.c_str(), result.size());	// 打包
	Dump((BYTE*)pack.Data(), pack.Size());
	//CServerSocket::getInstance()->Send(pack);
	return 0;
}
// 查看指定目录下的文件功能的结构体
typedef struct file_info
{
	// 结构体的构造函数
	file_info()
	{
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(IsDirectory));
	}
	BOOL IsInvalid;	// 是否为有效文件
	BOOL IsDirectory;	// 是否为目录 0否 1是
	BOOL HasNext;	// 是否还有后续 0没有 1有
	char szFileName[256];	// 文件名
}FILEINFO, * PFILEINFO;
// 查看指定目录下的文件
int MakeDirectorInfo()
{
	string strPath;
	//std::list<FILEINFO> lstFileInfo;
	if (CServerSocket::getInstance()->GetFilePath(strPath) == false)
	{
		OutputDebugString(_T("当前的命令不是获取文件列表，命令解析错误！"));
		return -1;
	}
	// 切换当前目录
	if (_chdir(strPath.c_str()) != 0)
	{
		FILEINFO finfo;
		finfo.IsInvalid = TRUE;
		finfo.IsDirectory = TRUE;
		finfo.HasNext = FALSE;
		memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
		//lstFileInfo.push_back(finfo);
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));	// 打包
		CServerSocket::getInstance()->Send(pack);	// 发送
		OutputDebugString(_T("没有权限访问目录！"));
		return -2;
	}
	_finddata_t fdata;
	int hfind = 0;
	// 查找文件
	if ((hfind = _findfirst("*", &fdata)) == -1)
	{
		OutputDebugString(_T("没有找到任何文件！"));
		return -3;
	}
	// 遍历查找文件
	do
	{
		FILEINFO finfo;
		finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
		memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
		//lstFileInfo.push_back(finfo);
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));	// 打包
		CServerSocket::getInstance()->Send(pack);	// 发送
	} while (!_findnext(hfind, &fdata));
	FILEINFO finfo;
	finfo.HasNext = FALSE;
	CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));	// 打包
	CServerSocket::getInstance()->Send(pack);	// 发送
	return 0;
}
// 打开文件
int RunFile()
{
	string strPath;
	CServerSocket::getInstance()->GetFilePath(strPath);
	// 对指定文件执行操作
	ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
	CPacket pack(3, NULL, 0);	// 打包
	CServerSocket::getInstance()->Send(pack);	// 发送
	return 0;
}
// 下载文件
int DownloadFile()
{
	string strPath;
	CServerSocket::getInstance()->GetFilePath(strPath);
	long long data = 0;
	// 以读的方式打开文件
	FILE* pFile = NULL;
	errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
	if (err != 0 || (pFile == NULL))
	{
		// 文件打开失败
		CPacket pack(4, (BYTE*)&data, 8);	// 打包
		CServerSocket::getInstance()->Send(pack);	// 发送
		return -1;
	}
	// 移动文件流的读写指针位置到尾部
	fseek(pFile, 0, SEEK_END);
	// 拿取文件的长度
	data = _ftelli64(pFile);
	CPacket head(4, (BYTE*)&data, 8);
	// 移动文件流的读写指针位置到头部
	fseek(pFile, 0, SEEK_SET);
	char buffer[1024] = "";
	size_t rlen = 0;
	do
	{
		// 从文件流中读取数据
		rlen = fread(buffer, 1, 1024, pFile);
		CPacket pack(4, (BYTE*)buffer, rlen);	// 打包
		CServerSocket::getInstance()->Send(pack);	// 发送
	} while (rlen >= 1024);
	CPacket pack(4, NULL, 0);	// 打包
	CServerSocket::getInstance()->Send(pack);	// 发送
	fclose(pFile);	// 关闭文件
	return 0;
}
// 鼠标操作
int MouseEvent()
{
	MOUSEEV mouse;
	if (CServerSocket::getInstance()->GetMouseEvent(mouse))
	{
		DWORD nFlags = 0;
		switch (mouse.nButton)
		{
		case 0:	// 左键
			nFlags = 1;
			break;
		case 1:	// 右键
			nFlags = 2;
			break;
		case 2:	// 中键
			nFlags = 4;
			break;
		case 4: // 鼠标移动
			nFlags = 8;
			break;
		}
		// 设置鼠标的位置
		if (nFlags != 8)
			SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
		switch (mouse.nAction)
		{
		case 0:	// 单击
			nFlags |= 0x10;
			break;
		case 1:	// 双击
			nFlags |= 0x20;
			break;
		case 2:	// 按下
			nFlags |= 0x40;
			break;
		case 3:	// 放开
			nFlags |= 0x80;
			break;
		default:
			break;
		}
		switch (nFlags)
		{
		case 0x21:	// 左键双击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11:	// 左键单击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41:	// 左键按下
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81:	// 左键放开
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22:	// 右键双击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12:	// 右键单击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42:	// 右键按下
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82:	// 右键放开
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24:	// 中键双击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x14:	// 中键单击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44:	// 中键按下
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84:	// 中键放开
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x08:	// 鼠标移动
			mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
			break;
		}
		CPacket pack(5, NULL, 0);	// 打包
		CServerSocket::getInstance()->Send(pack);	// 发包
	}
	else
	{
		OutputDebugString(_T("获取鼠标操作参数失败！"));
		return -1;
	}
	return 0;
}
int main()
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(nullptr);

	if (hModule != nullptr)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: 在此处为应用程序的行为编写代码。
			wprintf(L"错误: MFC 初始化失败\n");
			nRetCode = 1;
		}
		else
		{
			// TODO: 在此处为应用程序的行为编写代码。
			// 套接字初始化
			//CServerSocket* pserver = CServerSocket::getInstance();
			//int count = 0;
			//if (pserver->InitSocket() == false)
			//{
			//	MessageBox(NULL, _T("网络初始化异常，未能成功初始化，请检查网络状态！"), _T("网络初始化失败！"), MB_OK | MB_ICONERROR);
			//	exit(0);
			//}
			//while (CServerSocket::getInstance() != NULL)
			//{
			//	if (pserver->AcceptClient() == false)
			//	{
			//		if (count >= 3)
			//		{
			//			MessageBox(NULL, _T("多次无法正常接入用户，结束程序！"), _T("多次接入用户失败！"), MB_OK | MB_ICONERROR);
			//			exit(0);
			//		}
			//		MessageBox(NULL, _T("无法正常接入用户，自动重试！"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
			//		count++;
			//	}
			//	int ret = pserver->DealCommand();
			//	// TODO:
			//}

			int nCmd = 1;
			switch (nCmd)
			{
			case 1:
				// 查看磁盘分区
				MakeDriverInfo();
				break;
			case 2:
				// 查看指定目录下的文件
				MakeDirectorInfo();
				break;
			case 3:
				// 打开文件
				RunFile();
				break;
			case 4:
				// 下载文件
				DownloadFile();
				break;
			case 5:
				// 鼠标操作
				MouseEvent();
				break;
			}
		}
	}
	else
	{
		// TODO: 更改错误代码以符合需要
		wprintf(L"错误: GetModuleHandle 失败\n");
		nRetCode = 1;
	}

	return nRetCode;
}
