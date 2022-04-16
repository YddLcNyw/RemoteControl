#pragma once
#include "pch.h"
#include "framework.h"

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
		strData.resize(nSize);
		memcpy((void*)strData.c_str(), pData, nSize);
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
		if (i + 4 + 2 + 2 >= nSize)
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
		BYTE* pDaata = (BYTE*)strOut.c_str();
		*(WORD*)pDaata = sHead;
		pDaata += 2;
		*(DWORD*)pDaata = nLength;
		pDaata += 4;
		*(WORD*)pDaata = sCmd;
		pDaata += 2;
		memcpy(pDaata, strData.c_str(), strData.size());
		pDaata += strData.size();
		*(WORD*)pDaata = sSum;
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
class CServerSocket
{
public:
	// 静态函数
	static CServerSocket* getInstance()
	{
		if (m_instance == NULL)
			m_instance = new CServerSocket();
		return m_instance;
	}
	// 初始化操作函数
	bool InitSocket()
	{
		if (m_sock == -1)
			return false;

		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;	// 地址族
		serv_adr.sin_addr.s_addr = INADDR_ANY;	// 监听所有IP
		serv_adr.sin_port = htons(9527);	// 端口
		// 绑定
		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
			return false;

		// 监听
		if (listen(m_sock, 1) == -1)
			return false;

		return true;
	}
	// 填充参数，接入用户
	bool AcceptClient()
	{
		sockaddr_in client_adr;
		// 接受连接
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		if (m_client == -1)
			return false;
		return true;
	}
#define BUFFER_SIZE  4096
	// 接收数据
	int DealCommand()
	{
		if (m_client == -1)
			return -1;
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);	// 初始化
		size_t index = 0;
		while (true)
		{
			// 收数据
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
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
		if (m_client == -1)
			return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack)
	{
		if (m_client == -1)
			return false;
		return send(m_client, pack.Data(), pack.Size(), 0) > 0;

	}
private:
	SOCKET m_sock, m_client;
	CPacket m_packet;
	CServerSocket& operator=(const CServerSocket& ss)	// 赋值重载函数
	{

	}
	CServerSocket(const CServerSocket& ss)	// 复制构造函数
	{
		// 初始化
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}
	CServerSocket()
	{
		m_client = INVALID_SOCKET;	// 无效的
		if (InitSockEnv() == FALSE)
		{
			// 套接字初始化失败
			MessageBox(NULL, _T("无法初始化套接字环境！\n请检查网络设置！"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);	// 结束
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}
	~CServerSocket()
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
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	static CServerSocket* m_instance;

	class CHelper	// 这个类是为了能让 CServerSocket 可以析构
	{
	public:
		CHelper()
		{
			CServerSocket::getInstance();
		}
		~CHelper()
		{
			CServerSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};

//extern CServerSocket server;	// 外部的全局变量

