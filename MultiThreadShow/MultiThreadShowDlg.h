
// MultiThreadShowDlg.h : 头文件
//

#pragma once
#include<opencv2\opencv.hpp>


#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

#define WM_DRAWPIC  (WM_USER+1001)		//自定义消息


// CMultiThreadShowDlg 对话框
class CMultiThreadShowDlg : public CDialogEx
{
	Mat src;
	Mat showImg;
	Mat dst;
	VideoCapture cap;
	
	CListBox m_listDiceResults;
	//相机打开标识
	bool bOpenCap;
	//
	bool bStopVideo;

	bool m_bProcessing;

	CWinThread *pGrabbingThread;	//视频采集线程
	friend UINT ThreadFuncGrab(LPVOID lpParam);	//视频采集线程函数


	void  DrawcvMat(cv::Mat mat, UINT ID);

	afx_msg LRESULT OnDrawPic(WPARAM wParam, LPARAM lParam);//自定义消息WM_DRAWPIC的映射函数
// 构造
public:
	CMultiThreadShowDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MULTITHREADSHOW_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonVideo();
	afx_msg void OnBnClickedButtonGetimg();
	afx_msg void OnBnClickedButtonProcess();
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnClose();
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnLbnSelchangeList3();
	afx_msg void OnLbnSelchangeListDiceResults();
};
