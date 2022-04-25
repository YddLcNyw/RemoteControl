#pragma once
#include "pch.h"
#include "framework.h"

#pragma pack(push)	// ���浱ǰ�ڴ����
#pragma pack(1)	// �ĳ�1

// ��
class CPacket
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack)	// ���ƹ��캯��
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(WORD  nCmd, const BYTE* pData, size_t nSize)	// ������ع�
	{
		sHead = 0xFEFF;	// ��ͷ
		nLength = nSize + 4;	// =���ݼ������У��
		sCmd = nCmd;
		if (nSize > 0)
		{
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else
			strData.clear();
		// ��У�� 
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	CPacket(const BYTE* pData, size_t& nSize)	// ������ع�
	{
		size_t i = 0;
		for (; i < nSize; i++)
		{
			if (*(WORD*)(pData + i) == 0xFEFF)
			{
				// �ҵ���ͷ
				sHead = *(WORD*)(pData + i);
				i += 2;	// ��Ϊ��ͷ�������ֽڣ�Ҫָ�����µ�λ�á�������
				break;
			}
		}
		// ��4�ǰ����ȣ���2���������2�Ǻ�У��
		if (i + 4 + 2 + 2 > nSize)
		{
			// �����ݿ��ܲ�ȫ�����߰�ͷû����ȫ����
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);
		i += 4;	// ���ϰ�����
		if (nLength + i > nSize)
		{
			// �����ݲ�ȫ��û����ȫ���յ�
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);
		i += 2;	// ��������
		if (nLength > 4)
		{
			// -2�Ǽ�ȥ����-2�Ǽ�ȥ��У��
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 2 - 2);
			i += nLength - 4;	// ���ϰ�����
		}
		sSum = *(WORD*)(pData + i);
		i += 2;	// ���Ϻ�У��
		// ��У�� 
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum)
		{
			// ����ɹ�
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
	// �����ݴ�С
	int Size()
	{
		// +6�ǼӰ�ͷ�ͱ���
		return nLength + 6;
	}
	// �õ������ݵ�ֵ
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
	WORD sHead; // ��ͷ FE FF
	DWORD nLength;// �����ȣ��������ʼ����У�����
	WORD sCmd;// ����
	std::string strData;// ������
	WORD sSum;// ��У��
	std::string strOut;	// ������������
};
#pragma pack(pop)	// ��ԭ
typedef struct MouseEvent
{
	MouseEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;	// ������ƶ���˫��
	WORD nButton;	// ������Ҽ����м�
	POINT ptXY;	// ����
}MOUSEEV, * PMOUSEEV;

// �鿴ָ��Ŀ¼�µ��ļ����ܵĽṹ��
typedef struct file_info
{
	// �ṹ��Ĺ��캯��
	file_info()
	{
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;	// �Ƿ�Ϊ��Ч�ļ�
	BOOL IsDirectory;	// �Ƿ�ΪĿ¼ 0�� 1��
	BOOL HasNext;	// �Ƿ��к��� 0û�� 1��
	char szFileName[256];	// �ļ���
}FILEINFO, * PFILEINFO;
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
		TRACE("m_client = %d\r\n", m_client);
		if (m_client == -1)
			return false;
		return true;
	}
#define BUFFER_SIZE  409600
	// ��������
	int DealCommand()
	{
		if (m_client == -1)
			return -1;
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == NULL)
		{
			TRACE("�ڴ治�㣡\r\n");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);	// ��ʼ��
		size_t index = 0;
		while (true)
		{
			// ������
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0)
			{
				delete[]buffer;
				return -1;
			}TRACE("recv %d\r\n", len);
			// ���
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			// ��Ϊ�����캯�����洫�ĳ��������ã��ᱻ�޸ģ�����Żصĳ��� =0 ���ǽ���ʧ��
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				delete[]buffer;
				return m_packet.sCmd;
			}
		}
		delete[]buffer;
		return -1;
	}
	// ��������
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
	//	�ļ����ҹ��ܵĻ�ȡ�����ݵ�����
	bool GetFilePath(std::string& strPath)
	{
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4) || (m_packet.sCmd == 9))
		{
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	// ���������ܵĻ�ȡ�����ݵ�����
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
	void CliseCilent()
	{
		// �Ͽ�����
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}
private:
	SOCKET m_sock, m_client;
	CPacket m_packet;
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

