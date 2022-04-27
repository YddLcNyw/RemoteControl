﻿// CWatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "CWatchDialog.h"
#include "afxdialogex.h"
#include "RemoteClientDlg.h"

// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{

}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point)
{
	// 屏幕坐标到客户端区域坐标
	ScreenToClient(&point);
	// 本地坐标到远程坐标
	CRect clietRect;
	m_picture.GetWindowRect(clietRect);
	return CPoint(point.x * 2560 / clietRect.Width(), point.y * 1440 / clietRect.Height());
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	//定时器
	SetTimer(0, 45, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0)
	{
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		if (pParent->isFull())
		{
			// 获取宽高
			CRect rect;
			m_picture.GetWindowRect(rect);
			// 拿到DC
		//z	pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			// 进行图片缩放大小
			pParent->GetImage().StretchBlt(
				m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
			// 进行重新绘制
			m_picture.InvalidateRect(NULL);
			// 销毁
			pParent->GetImage().Destroy();
			// 设置false
			pParent->SetImageStatus();
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// 坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	// 封装
	MOUSEEV event;
	event.ptXY = remote;	// 坐标
	event.nButton = 0;	// 左键
	event.nAction = 2;	// 双击
	// 发送
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{

	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	//// 坐标转换
	//CPoint remote = UserPoint2RemoteScreenPoint(point);
	//// 封装
	//MOUSEEV event;
	//event.ptXY = remote;	// 坐标
	//event.nButton = 0;	// 左键
	//event.nAction = 4;	// 弹起
	//// 发送
	//CClientSocket* pClient = CClientSocket::getInstance();
	//CPacket pack(5, (BYTE*)&event, sizeof(event));
	//pClient->Send(pack);
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// 坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	// 封装
	MOUSEEV event;
	event.ptXY = remote;	// 坐标
	event.nButton = 2;	// 右键
	event.nAction = 2;	// 双击
	// 发送
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// 坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	// 封装
	MOUSEEV event;
	event.ptXY = remote;	// 坐标
	event.nButton = 0;	// 左键
	event.nAction = 3;	// 按下
	// 发送
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	// 坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	// 封装
	MOUSEEV event;
	event.ptXY = remote;	// 坐标
	event.nButton = 0;	// 
	event.nAction = 4;	// 弹起
	// 发送
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// 坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	// 封装
	MOUSEEV event;
	event.ptXY = remote;	// 坐标
	event.nButton = 0;	// 左键
	event.nAction = 1;	// 移动
	// 发送
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()
{
	// 拿到鼠标坐标
	CPoint point;
	GetCursorPos(&point);
	// 坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	// 封装
	MOUSEEV event;
	event.ptXY = remote;	// 坐标
	event.nButton = 0;	// 左键
	event.nAction = 3;	// 按下
	// 发送
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
}
