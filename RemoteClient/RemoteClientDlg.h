﻿
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "ClientSocket.h"
#include "StatusDlg.h"

#define WM_SEND_PACKET (WM_USER + 1)	// ①·发送数据包的消息
// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
	// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif
public:
	bool isFull() const
	{
		return m_isFull;
	}
	CImage& GetImage()
	{
		return m_image;
	}
	void SetImageStatus(bool isFull = false)
	{
		m_isFull = isFull;
	}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	CImage m_image;	// 图片缓存
	bool m_isFull;	// 缓存是否有数据 true有数据 false没有数据
private:
	// 监控数据线程函数调用的函数
	void threadWatchData();
	// 监控数据线程函数
	static void threadEntryForWatchData(void* arg);
	// 线程函数调用的文件下载功能
	void threadDownFile();
	// 文件下载线程函数
	static void threadEntryForDownFile(void* arg);
	// 更新文件信息
	void LoadFileCurrent();
	// 显示文件信息
	void LoadFileInfo();
	// 拿到文件路径
	CString GetPath(HTREEITEM hTree);
	// 删除所有子节点
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	// 1、查看磁盘分区
	// 2、查看指定目录下的文件
	// 3、打开文件
	// 4、下载文件
	// 5、鼠标操作
	// 6、发送屏幕内容
	// 7、锁机
	// 8、解锁
	// 9、删除文件
	// 1981、连接测试
	// 返回值是命令号，小于0则是错误
	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t nLendth = 0);
	// 实现
protected:
	HICON m_hIcon;
	CStatusDlg m_dlgStatus;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	// IP输入框
	DWORD m_server_address;
	// 端口
	CString m_nport;
	afx_msg void OnBnClickedBtnFileinfo();
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件列表
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
	// ②·定义自定义消息响应函数
	afx_msg LRESULT OnSendPacket(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
