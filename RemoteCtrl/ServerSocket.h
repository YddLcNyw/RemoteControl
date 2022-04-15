#pragma once
#include "pch.h"
#include "framework.h"
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
	// 接收数据
	int DealCommand()	
	{
		if (m_client == -1)
			return false;
		char buffer[1024] = "";
		while (true)
		{
			// 收数据
			int len= recv(m_client, buffer, sizeof(buffer), 0);
			if (len <= 0)
				return -1;
			// TODO:处理命令
		}
	}
	// 发送数据
	bool Send(const char* pData, int nSize)	
	{
		if (m_client == -1)
			return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
private:
	SOCKET m_sock, m_client;
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

