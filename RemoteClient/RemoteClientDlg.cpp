
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nport(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nport);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
}

int CRemoteClientDlg::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nLendth)
{
	// 把控件的值更新到成员变量
	UpdateData();
	// 获取套接字的实例
	CClientSocket* pClient = CClientSocket::getInstance();
	// 网络初始化
	bool ret = pClient->InitSocket(m_server_address, atoi((LPCTSTR)m_nport));
	if (!ret)
	{
		AfxMessageBox("网络初始化失败！");
		return -1;
	}
	// 打包
	CPacket pack(nCmd, pData, nLendth);
	// 发送数据
	ret = pClient->Send(pack);
	TRACE("Send ret %d\r\n", ret);
	// 接收数据并解析
	int cmd = pClient->DealCommand();
	TRACE("ack：\r\n", cmd);
	if (bAutoClose)
		// 关闭套接字
		pClient->CloseSocket();
	return cmd;
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteClientDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	m_server_address = 0x7F000001;
	m_nport = _T("9527");
	UpdateData(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 连接测试按钮
void CRemoteClientDlg::OnBnClickedBtnTest()
{
	SendCommandPacket(1981);
}
// 查看文件信息按钮
void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{
	// 发送查看目录命令
	int ret = SendCommandPacket(1);
	if (ret == -1)
	{
		AfxMessageBox(_T("命令处理失败"));
		return;
	}
	// 获取实例
	CClientSocket* pClient = CClientSocket::getInstance();
	// 拿到包数据
	std::string drivers = pClient->GetPacket().strData;
	std::string dr;
	// 清空m_Tree里面的数据
	m_Tree.DeleteAllItems();
	for (size_t i = 0; i < drivers.size(); i++)
	{
		// 遍历字符串
		if (drivers[i] == ',')
		{
			dr += ":";
			// 添加节点
			HTREEITEM hTemp = m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);	// 以追加的方式加到根目录下
			m_Tree.InsertItem("", hTemp, TVI_LAST);
			// 清空
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
}
// 拿到文件路径
CString  CRemoteClientDlg::GetPath(HTREEITEM hTree)
{
	CString strRet, strTmp;
	do
	{
		// 拿到数据
		strTmp = m_Tree.GetItemText(hTree);
		strRet = strTmp + '\\' + strRet;
		// 拿到副节点
		hTree = m_Tree.GetParentItem(hTree);
	} while (hTree != NULL);
	return strRet;
}
// 删除所有子节点
void CRemoteClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do
	{
		// 获取一个子节点
		hSub = m_Tree.GetChildItem(hTree);
		if (hSub != NULL)
			// 如果子节点不为空就删除子节点
			m_Tree.DeleteItem(hSub);
	} while (hSub != NULL);
}
void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	// 获取鼠标当前位置
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	// 坐标转换
	m_Tree.ScreenToClient(&ptMouse);
	// 获取当前路径
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	// 判断鼠标有没有点到节点
	if (hTreeSelected == NULL)
		return;
	// 判断选中的目录有没有子节点,没有就返回
	if (m_Tree.GetChildItem(hTreeSelected) == NULL)
		return;
	// 删除节点
	DeleteTreeChildrenItem(hTreeSelected);
	// 拿到文件路径
	CString strPath = GetPath(hTreeSelected);
	// 发送命令
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCSTR)strPath, strPath.GetLength());
	// 拿到数据
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	// 获取套接字的实例
	CClientSocket* pClient = CClientSocket::getInstance();
	while (pInfo->HasNext)
	{
		// 如果还有后续信息则继续循环
		if (pInfo->IsDirectory)
		{
			// 是目录
			if ((CString(pInfo->szFileName) == ".") || (CString(pInfo->szFileName) == ".."))
			{
				// 是文件夹
				// 接收数据并解析
				int cmd = pClient->DealCommand();
				TRACE("ack：\r\n", cmd);
				if (cmd < 0)
					break;
				// 再次拿数据
				pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
		}
		// 插入数据
		HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
		if (pInfo->IsDirectory)
		{
			// 是目录
			m_Tree.InsertItem("", hTemp, TVI_LAST);
		}
		// 接收数据并解析
		int cmd = pClient->DealCommand();
		TRACE("ack：\r\n", cmd);
		if (cmd < 0)
			break;
		// 再次拿数据
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}
	// 关闭套接字
	pClient->CloseSocket();
}
