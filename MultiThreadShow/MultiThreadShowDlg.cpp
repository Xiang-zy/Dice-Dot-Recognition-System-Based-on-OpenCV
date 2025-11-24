
// MultiThreadShowDlg.cpp : 实现文件
//多线程采集摄像头视频，抓取图像并处理显示

#include "stdafx.h"
#include "MultiThreadShow.h"
#include "MultiThreadShowDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
Mat preprocessImage(const Mat& img);
vector<vector<Point>> findDiceContours(const Mat& mask);
int diceRecognize(Mat& originalImage, const Rect& roiRect);

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


// CMultiThreadShowDlg 对话框



CMultiThreadShowDlg::CMultiThreadShowDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MULTITHREADSHOW_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMultiThreadShowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_DICE_RESULTS, m_listDiceResults);
}

BEGIN_MESSAGE_MAP(CMultiThreadShowDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_VIDEO, &CMultiThreadShowDlg::OnBnClickedButtonVideo)
	ON_BN_CLICKED(IDC_BUTTON_GETIMG, &CMultiThreadShowDlg::OnBnClickedButtonGetimg)
	ON_BN_CLICKED(IDC_BUTTON_PROCESS, &CMultiThreadShowDlg::OnBnClickedButtonProcess)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CMultiThreadShowDlg::OnBnClickedButtonExit)

	ON_MESSAGE(WM_DRAWPIC, OnDrawPic)
	ON_WM_CLOSE()
	ON_LBN_SELCHANGE(IDC_LIST_DICE_RESULTS, &CMultiThreadShowDlg::OnLbnSelchangeListDiceResults)
END_MESSAGE_MAP()


// CMultiThreadShowDlg 消息处理程序

BOOL CMultiThreadShowDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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
	bOpenCap = false;
	bStopVideo = false;
	m_bProcessing = false;

	((CComboBox*)GetDlgItem(IDC_COMBO_CAMINDEX))->SetCurSel(0);//设置第n行内容为显示的内容

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMultiThreadShowDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMultiThreadShowDlg::OnPaint()
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
HCURSOR CMultiThreadShowDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//开启/关闭相机
void CMultiThreadShowDlg::OnBnClickedButtonVideo()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!bOpenCap)//开启相机
	{
		int iIndex = ((CComboBox*)GetDlgItem(IDC_COMBO_CAMINDEX))->GetCurSel();
		cap.open(iIndex);
		if (cap.isOpened())
		{
			bOpenCap = true;
			GetDlgItem(IDC_BUTTON_VIDEO)->SetWindowTextW(_T("关闭"));
			bStopVideo = false;

			// 重置处理状态
			m_bProcessing = false;
			GetDlgItem(IDC_BUTTON_PROCESS)->SetWindowTextW(_T("开始处理"));
			GetDlgItem(IDC_BUTTON_PROCESS)->EnableWindow(TRUE);

			//启动线程
			pGrabbingThread = AfxBeginThread(ThreadFuncGrab, this);
			pGrabbingThread->m_bAutoDelete = FALSE;
		}
		else
		{
			MessageBox(_T("相机未能正确打开！"), _T("警告"));
		}
	}
	else //关闭相机
	{
		cap.release();
		bOpenCap = false;
		GetDlgItem(IDC_BUTTON_VIDEO)->SetWindowTextW(_T("开启"));
		bStopVideo = true;
		GetDlgItem(IDC_BUTTON_PROCESS)->EnableWindow(FALSE);
	}

}
Mat processImage(const Mat& img, int& nTotalPoints)
{
	Mat mask = preprocessImage(img);
	vector<vector<Point>> diceContours = findDiceContours(mask);

	nTotalPoints = 0; // 重置总点数
	Mat resultImg = img.clone();

	for (const auto& contour : diceContours) {
		RotatedRect rect = minAreaRect(contour);
		Point center = rect.center;
		Rect roiRect(center.x - 50, center.y - 50, 100, 100);
		roiRect &= Rect(0, 0, img.cols, img.rows);

		// 修改这里：直接传递 roiRect 而不是提取的 Mat
		int num = diceRecognize(resultImg, roiRect);
		nTotalPoints += num; // 累加点数

		putText(resultImg, to_string(num), Point(center.x - 30, center.y - 30),
			FONT_HERSHEY_COMPLEX_SMALL, 3, Scalar(0, 255,255), 2);
	}
	return resultImg;
}

#pragma region 线程函数
//视频采集线程函数
UINT ThreadFuncGrab(LPVOID lpParam)
{
	CMultiThreadShowDlg* pMyDlg = (CMultiThreadShowDlg*)lpParam;
	Mat img;
	while (1)
	{
		if (pMyDlg->bStopVideo)
			break;

		pMyDlg->cap >> img;
		if (img.empty()) continue;

		Mat processedImg;
		if (pMyDlg->m_bProcessing)
		{
			int nTotalPoints = 0;
			processedImg = processImage(img, nTotalPoints);

			// 更新List Box显示
			pMyDlg->m_listDiceResults.ResetContent();
			CString strTotal;
			strTotal.Format(_T("骰子总数: %d"), nTotalPoints);
			pMyDlg->m_listDiceResults.AddString(strTotal);
		}
		else
		{
			processedImg = img.clone();
			pMyDlg->m_listDiceResults.ResetContent();
		}

		SendMessage(pMyDlg->m_hWnd, WM_DRAWPIC, (WPARAM)&img, (LPARAM)&processedImg);
		Sleep(30);
	}
	return 0;
}
#pragma endregion

#pragma region 自定义消息映射函数
//自定义消息WM_DRAWPIC的映射函数
LRESULT CMultiThreadShowDlg::OnDrawPic(WPARAM wParam, LPARAM lParam)
{
	Mat* pSrc = (Mat*)wParam;
    Mat* pProcessed = (Mat*)lParam;
    
    if(pSrc) DrawcvMat(*pSrc, IDC_STATIC_VIDEO);
    if(pProcessed) DrawcvMat(*pProcessed, IDC_STATIC_IMG);

    return 0;
}
#pragma endregion


//采集图像
void CMultiThreadShowDlg::OnBnClickedButtonGetimg()
{   
	// TODO: 在此添加控件通知处理程序代码
	src.copyTo(showImg);
	if (showImg.empty())
	{
		return;
	}

	DrawcvMat(showImg, IDC_STATIC_IMG);

	//处理按钮可用
	GetDlgItem(IDC_BUTTON_PROCESS)->EnableWindow(TRUE);

}

Mat preprocessImage(const Mat& img) {
	Mat hsvImg;
	cvtColor(img, hsvImg, COLOR_BGR2HSV);

	Mat mask;
	Scalar hsvMin(0, 0, 200);
	Scalar hsvMax(180, 80, 255);
	inRange(hsvImg, hsvMin, hsvMax, mask);

	medianBlur(mask, mask, 5);
	return mask;
}

// 查找骰子轮廓的函数
vector<vector<Point>> findDiceContours(const Mat& mask) {
	vector<vector<Point>> contours;
	findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	Mat filledMask = mask.clone();
	drawContours(filledMask, contours, -1, Scalar(255), FILLED);

	Mat distTransform;
	distanceTransform(filledMask, distTransform, DIST_L2, 5);
	normalize(distTransform, distTransform, 0, 1.0, NORM_MINMAX);

	double minVal, maxVal;
	Point minLoc, maxLoc;
	minMaxLoc(distTransform, &minVal, &maxVal, &minLoc, &maxLoc);

	Mat sureFg;
	threshold(distTransform, sureFg, maxVal * 0.7, 255, THRESH_BINARY);
	sureFg.convertTo(sureFg, CV_8U);

	vector<vector<Point>> diceContours;
	findContours(sureFg, diceContours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	return diceContours;
}

// 识别骰子点数的函数
int diceRecognize(Mat& originalImage, const Rect& roiRect) {
	// 提取ROI区域
	Mat roi = originalImage(roiRect);

	Mat gray, binary;
	cvtColor(roi, gray, COLOR_BGR2GRAY);

	GaussianBlur(gray, gray, Size(11, 11), 0);
	adaptiveThreshold(gray, binary, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 35, 9);

	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(2, 2));
	morphologyEx(binary, binary, MORPH_OPEN, kernel);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(binary, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	vector<vector<Point>> validContours;
	int minSide = min(roi.rows, roi.cols);
	double minArea = pow(minSide * 0.05, 2);
	double maxArea = pow(minSide * 0.5, 2);
	for (size_t i = 0; i < contours.size(); ++i) {
		// 更严格的层级关系判断：必须是子轮廓（有父轮廓）且没有子轮廓
		if (hierarchy[i][3] != -1 && hierarchy[i][2] == -1) {
			double area = contourArea(contours[i]);
			if (area >= minArea && area <= maxArea) {
				// 额外检查圆形度
				double perimeter = arcLength(contours[i], true);
				double circularity = (4 * CV_PI * area) / (perimeter * perimeter);

				if (circularity > 0.6) {  // 只保留较圆的轮廓（骰子点通常是圆形）
					validContours.push_back(contours[i]);
				}
			}
		}
	}
	// 在原始图像上绘制圆圈（考虑ROI偏移）
	for (const auto& contour : validContours) {
		Moments M = moments(contour);
		Point2f center(M.m10 / M.m00, M.m01 / M.m00);
		float radius = sqrt(M.m00 / CV_PI);

		// 将ROI坐标转换为原始图像坐标
		Point centerInOriginal(cvRound(center.x) + roiRect.x,
			cvRound(center.y) + roiRect.y);
		circle(originalImage, centerInOriginal, static_cast<int>(radius), Scalar(0, 255, 0), 2);
	}

	return static_cast<int>(validContours.size());
}

//处理
void CMultiThreadShowDlg::OnBnClickedButtonProcess()
{
	m_bProcessing = !m_bProcessing; // 切换处理状态
	GetDlgItem(IDC_BUTTON_PROCESS)->SetWindowTextW(
		m_bProcessing ? _T("停止处理") : _T("开始处理"));
}

//退出
void CMultiThreadShowDlg::OnBnClickedButtonExit()
{
	// TODO: 在此添加控件通知处理程序代码
	SendMessage(WM_CLOSE, NULL, NULL);
}

//在指定控件中显示图像
void  CMultiThreadShowDlg::DrawcvMat(cv::Mat mat, UINT ID)
{
	int width = mat.cols;
	int height = mat.rows;
	int channels = mat.channels();
	int depth = mat.depth();
	int dims = mat.dims;
	if ((width <= 0) || (height <= 0))
	{
		return;
	}
	CWnd* m_pMyWnd = GetDlgItem(ID);
	CDC *m_pDC = m_pMyWnd->GetDC();
	CImage cImage;
	cImage.Destroy();
	cImage.Create(width, height, 8 * channels);
	if (1 == channels)
	{
		RGBQUAD* ColorTable;
		int MaxColors = 256;
		ColorTable = new RGBQUAD[MaxColors];
		cImage.GetColorTable(0, MaxColors, ColorTable);
		for (int i = 0; i < MaxColors; i++)
		{
			ColorTable[i].rgbBlue = (BYTE)i;
			ColorTable[i].rgbGreen = (BYTE)i;
			ColorTable[i].rgbRed = (BYTE)i;
		}
		cImage.SetColorTable(0, MaxColors, ColorTable);
		delete[]ColorTable;
	}
	if (depth == 0)
	{
	}
	else if (depth == 1)
	{
		mat.convertTo(mat, CV_8U, 255.0, 0);
	}
	else if (depth == 2)
	{
		mat.convertTo(mat, CV_8U, 255.0, 0);
	}
	else if (depth == 3)
	{
		mat.convertTo(mat, CV_8U, 255.0, 0);
	}
	else if (depth == 4)
	{
		mat.convertTo(mat, CV_8U, 255.0, 0);
	}
	else if (depth == 5)
	{
		mat.convertTo(mat, CV_8U, 255.0, 0);
	}
	else if (depth == 6)
	{
		mat.convertTo(mat, CV_8U, 255.0, 0);
	}
	else if (depth == 7)
	{
		AfxMessageBox(_T("Format Error    depth==7    Type = CV_USRTYPE1"));
		return;
	}
	else
	{
		AfxMessageBox(_T("Format Error. Type = Unknown"));
		return;
	}
	uchar* ps;
	uchar* pimg = (uchar*)cImage.GetBits();
	int step = cImage.GetPitch();
	for (int i = 0; i < height; i++)
	{
		ps = mat.ptr<uchar>(i);
		for (int j = 0; j < width; j++)
		{
			if (1 == channels)
			{
				*(pimg + i * step + j) = ps[j];
			}
			else if (3 == channels)
			{
				*(pimg + i * step + j * 3) = ps[j * 3];
				*(pimg + i * step + j * 3 + 1) = ps[j * 3 + 1];
				*(pimg + i * step + j * 3 + 2) = ps[j * 3 + 2];
			}
		}
	}
	CRect rc;
	m_pMyWnd->GetWindowRect(&rc);
	int nwidth = rc.Width();
	int nheight = rc.Height();
	int fixed_width = min(cImage.GetWidth(), nwidth);
	int fixed_height = min(cImage.GetHeight(), nheight);
	double ratio_w = fixed_width / (double)cImage.GetWidth();
	double ratio_h = fixed_height / (double)cImage.GetHeight();
	double ratio = min(ratio_w, ratio_h);
	int show_width = rc.Width();
	int show_height = rc.Height();
	int offsetx = (nwidth - show_width) / 2;
	int offsety = (nheight - show_height) / 2;
	::SetStretchBltMode(m_pDC->GetSafeHdc(), COLORONCOLOR);
	cImage.StretchBlt(m_pDC->GetSafeHdc(), offsetx, offsety, show_width, show_height,
		0, 0, cImage.GetWidth(), cImage.GetHeight(), SRCCOPY);
	m_pMyWnd->ReleaseDC(m_pDC);
}


void CMultiThreadShowDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//结束视频采集线程
	bStopVideo = true;
	::WaitForSingleObject(pGrabbingThread, INFINITE);

	CDialogEx::OnClose();
}


void CMultiThreadShowDlg::OnLbnSelchangeListDiceResults()
{
	// TODO: 在此添加控件通知处理程序代码
}
