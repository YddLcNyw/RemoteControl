#pragma once
#include "pch.h"
#include "framework.h"
class CServerSocket
{
public:
	// ��̬����
	static CServerSocket* getInstance() 
	{
		if (m_instance == NULL)
			m_instance = new CServerSocket();
		return m_instance;
	}
	// ��ʼ����������
	bool InitSocket()	
	{
		if (m_sock == -1)
			return false;

		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;	// ��ַ��
		serv_adr.sin_addr.s_addr = INADDR_ANY;	// ��������IP
		serv_adr.sin_port = htons(9527);	// �˿�
		// ��
		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
			return false;

		// ����
		if (listen(m_sock, 1) == -1)
			return false;

		return true;
	}
	// �������������û�
	bool AcceptClient()	
	{
		sockaddr_in client_adr;
		// ��������
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		if (m_client == -1)
			return false;
		return true;
	}
	// ��������
	int DealCommand()	
	{
		if (m_client == -1)
			return false;
		char buffer[1024] = "";
		while (true)
		{
			// ������
			int len= recv(m_client, buffer, sizeof(buffer), 0);
			if (len <= 0)
				return -1;
			// TODO:��������
		}
	}
	// ��������
	bool Send(const char* pData, int nSize)	
	{
		if (m_client == -1)
			return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
private:
	SOCKET m_sock, m_client;
	CServerSocket& operator=(const CServerSocket& ss)	// ��ֵ���غ���
	{

	}
	CServerSocket(const CServerSocket& ss)	// ���ƹ��캯��
	{
		// ��ʼ��
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}
	CServerSocket()
	{
		m_client = INVALID_SOCKET;	// ��Ч��
		if (InitSockEnv() == FALSE)
		{
			// �׽��ֳ�ʼ��ʧ��
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ�����\n�����������ã�"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);	// ����
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}
	~CServerSocket()
	{
		// �ر�����
		closesocket(m_sock);
		// �ر��׽���
		WSACleanup();
	}
	BOOL InitSockEnv()
	{
		// �׽��ֳ�ʼ��
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0)	// 1.1�İ汾
		{
			return FALSE;
		}
		return TRUE;
	}
	static void releaseInstance()	// ���������Ϊ�˰� m_instance ����Ϊ��
	{
		if (m_instance != NULL)
		{
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	static CServerSocket* m_instance;

	class CHelper	// �������Ϊ������ CServerSocket ��������
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

//extern CServerSocket server;	// �ⲿ��ȫ�ֱ���

