#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#pragma pack(push)	// 保存当前内存对齐
#pragma pack(1)	// 改成1

// 包
class CPacket
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack)	// 复制构造函数
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(WORD  nCmd, const BYTE* pData, size_t nSize)	// 打包的重构
	{
		sHead = 0xFEFF;	// 包头
		nLength = nSize + 4;	// =数据加命令加校验
		sCmd = nCmd;
		if (nSize > 0)
		{
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else
			strData.clear();
		// 和校验 
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	CPacket(const BYTE* pData, size_t& nSize)	// 解包的重构
	{
		size_t i = 0;
		for (; i < nSize; i++)
		{
			if (*(WORD*)(pData + i) == 0xFEFF)
			{
				// 找到包头
				sHead = *(WORD*)(pData + i);
				i += 2;	// 因为包头是两个字节，要指向最新的位置·有疑问
				break;
			}
		}
		// 加4是包长度，加2是是命令，加2是和校验
		if (i + 4 + 2 + 2 > nSize)
		{
			// 包数据可能不全，或者包头没有完全接收
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);
		i += 4;	// 加上包长度
		if (nLength + i > nSize)
		{
			// 包数据不全，没有完全接收到
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);
		i += 2;	// 加上命令
		if (nLength > 4)
		{
			// -2是减去命令-2是减去和校验
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 2 - 2);
			i += nLength - 4;	// 加上包数据
		}
		sSum = *(WORD*)(pData + i);
		i += 2;	// 加上和校验
		// 和校验 
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum)
		{
			// 解包成功
			nSize = i;
			return;
		}
		nSize = 0;
	}
	~CPacket() {	}
	CPacket& operator=(const CPacket& pack)
	{
		if (this != &pack)
		{
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}
	// 包数据大小
	int Size()
	{
		// +6是加包头和本身
		return nLength + 6;
	}
	// 得到包数据的值
	const char* Data()
	{
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;
		pData += 2;
		*(DWORD*)pData = nLength;
		pData += 4;
		*(WORD*)pData = sCmd;
		pData += 2;
		memcpy(pData, strData.c_str(), strData.size());
		pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}
public:
	WORD sHead; // 包头 FE FF
	DWORD nLength;// 包长度，控制命令开始到和校验结束
	WORD sCmd;// 命令
	std::string strData;// 包数据
	WORD sSum;// 和校验
	std::string strOut;	// 整个包的数据
};
#pragma pack(pop)	// 还原
typedef struct MouseEvent
{
	MouseEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;	// 点击、移动、双击
	WORD nButton;	// 左键、右键、中键
	POINT ptXY;	// 坐标
}MOUSEEV, * PMOUSEEV;
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
// 报错函数
std::string GetErrorInfo(int wsaErrCode);
class CClientSocket
{
public:
	// 静态函数
	static CClientSocket* getInstance()
	{
		if (m_instance == NULL)
			m_instance = new CClientSocket();
		return m_instance;
	}
	// 初始化操作函数
	bool InitSocket(int nIP,int nPort)
	{
		if (m_sock != INVALID_SOCKET)
			CloseSocket();
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sock == -1)
			return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;	// 地址族
		serv_adr.sin_addr.s_addr = htonl(nIP);	// 监听strIPAddressIP
		serv_adr.sin_port = htons(nPort);	// 端口
		if (serv_adr.sin_addr.s_addr == INADDR_NONE)
		{
			// 不存在
			AfxMessageBox("指定的IP地址不存在！");
			return false;
		}
		int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)); // 绑定
		if (ret == -1)
		{
			AfxMessageBox("连接失败！");
			TRACE("连接失败，%d %s\r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
			return false;
		}
		return true;
	}
#define BUFFER_SIZE  4096
	// 接收数据
	int DealCommand()
	{
		if (m_sock == -1)
			return -1;
		char* buffer = m_buffer.data();
		memset(buffer, 0, BUFFER_SIZE);	// 初始化
		size_t index = 0;
		while (true)
		{
			// 收数据
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0)
				return -1;
			// 解包
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			// 因为往构造函数里面传的长度是引用，会被修改，如果放回的长度 =0 则是解析失败
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}
	// 发送数据
	bool Send(const char* pData, int nSize)
	{
		if (m_sock == -1)
			return false;
		return send(m_sock, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack)
	{
		TRACE("m_sock %d\r\n", m_sock);
		if (m_sock == -1)
			return false;
		return send(m_sock, pack.Data(), pack.Size(), 0) > 0;

	}
	//	文件查找功能的获取包数据的内容
	bool GetFilePath(std::string& strPath)
	{
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4))
		{
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	// 鼠标操作功能的获取包数据的内容
	bool GetMouseEvent(MOUSEEV& mouse)
	{
		if (m_packet.sCmd == 5)
		{
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}
	CPacket& GetPacket()
	{
		return m_packet;
	}
	void CloseSocket()
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
private:
	std::vector<char>m_buffer;
	SOCKET m_sock;
	CPacket m_packet;
	CClientSocket& operator=(const CClientSocket& ss)	// 赋值重载函数
	{

	}
	CClientSocket(const CClientSocket& ss)	// 复制构造函数
	{
		// 初始化
		m_sock = ss.m_sock;
	}
	CClientSocket()
	{
		if (InitSockEnv() == FALSE)
		{
			// 套接字初始化失败
			MessageBox(NULL, _T("无法初始化套接字环境！\n请检查网络设置！"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);	// 结束
		}
		m_buffer.resize(BUFFER_SIZE);
	}
	~CClientSocket()
	{
		// 关闭连接
		closesocket(m_sock);
		// 关闭套接字
		WSACleanup();
	}
	BOOL InitSockEnv()
	{
		// 套接字初始化
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0)	// 1.1的版本
		{
			return FALSE;
		}
		return TRUE;
	}
	static void releaseInstance()	// 这个函数是为了把 m_instance 设置为空
	{
		if (m_instance != NULL)
		{
			CClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	static CClientSocket* m_instance;

	class CHelper	// 这个类是为了能让 CServerSocket 可以析构
	{
	public:
		CHelper()
		{
			CClientSocket::getInstance();
		}
		~CHelper()
		{
			CClientSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};

//extern CServerSocket server;	// 外部的全局变量


