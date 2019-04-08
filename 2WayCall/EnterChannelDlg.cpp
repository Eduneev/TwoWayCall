// EnterChannelDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "OpenLive.h"
#include "EnterChannelDlg.h"
#include "afxdialogex.h"


// CEnterChannelDlg

IMPLEMENT_DYNAMIC(CEnterChannelDlg, CDialogEx)

CEnterChannelDlg::CEnterChannelDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEnterChannelDlg::IDD, pParent)
{   
}

CEnterChannelDlg::~CEnterChannelDlg()
{
}

void CEnterChannelDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDCHNAME_CHANNEL, m_ctrChannel);
    DDX_Control(pDX, IDC_BTNJOIN_CHANNEL, m_btnJoin);
    DDX_Control(pDX, IDC_BTNSET_CHANNEL, m_btnSetup);
	DDX_Control(pDX, IDC_BTNGET_CHANNEL, m_btnSetChannel);
}


BEGIN_MESSAGE_MAP(CEnterChannelDlg, CDialogEx)
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTNJOIN_CHANNEL, &CEnterChannelDlg::OnBnClickedBtnjoinChannel)
	ON_BN_CLICKED(IDC_BTNSET_CHANNEL, &CEnterChannelDlg::OnBnClickedBtnsetChannel)
	ON_BN_CLICKED(IDC_BTNGET_CHANNEL, &CEnterChannelDlg::OnBnClickedBtngetChannel)
END_MESSAGE_MAP()


// CEnterChannelDlg
BOOL CEnterChannelDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN){
		switch (pMsg->wParam){
		case VK_ESCAPE:
			return FALSE;
		case VK_RETURN:
			OnBnClickedBtnjoinChannel();
			return FALSE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CEnterChannelDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_ftHead.CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	m_ftDesc.CreateFont(25, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	m_ftBtn.CreateFont(19, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	m_penFrame.CreatePen(PS_SOLID, 1, RGB(0xD8, 0xD8, 0xD8));

	m_dlgDevice.Create(CDeviceDlg::IDD, this);
	m_dlgDevice.EnableDeviceTest(TRUE);

	SetBackgroundColor(RGB(0xFF, 0xFF, 0xFF));
	InitCtrls();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void COutputLogger2(const char* txt)
{
	std::ofstream log("output.txt", std::ios_base::app | std::ios_base::out);
	log << txt << std::endl;
}


void CEnterChannelDlg::InitCtrls()
{
	CRect ClientRect;

	GetClientRect(&ClientRect);

	m_ctrChannel.MoveWindow(80, 180, 100, 22, TRUE);
    m_ctrChannel.SetFont(&m_ftDesc);
	m_ctrChannel.SetCaretPos(CPoint(24, 148));
	//m_ctrChannel.ShowCaret();
	m_ctrChannel.SetTip(LANG_STR("IDS_CHN_CHANNELNAME"));

	//m_btnSetChannel.MoveWindow(ClientRect.Width()/2, ClientRect.Height()/2.9, ClientRect.Width()/5, ClientRect.Height()/8.5, TRUE);
		
	m_btnSetChannel.MoveWindow(360, 162, 150, 50, TRUE);
	char a[10];
	COutputLogger2(_itoa(ClientRect.Width(), a, 10));
	COutputLogger2(_itoa(ClientRect.Height(), a, 10));

	m_btnJoin.MoveWindow(120, 310, 360, 36, TRUE);
	m_btnSetup.MoveWindow(180, 355, 240, 36, TRUE);

	m_btnJoin.SetBackColor(RGB(0x00, 0xA0, 0xE9), RGB(0x05, 0x78, 0xAA), RGB(0x05, 0x78, 0xAA), RGB(0xE6, 0xE6, 0xE6));
	m_btnJoin.SetFont(&m_ftBtn);
	m_btnJoin.SetTextColor(RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xCC, 0xCC, 0xCC));
	m_btnJoin.SetWindowText(LANG_STR("IDS_CHN_BTJOIN"));
	m_btnSetup.SetBackColor(RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF), RGB(0xFF, 0xFF, 0xFF));
	m_btnSetup.SetFont(&m_ftBtn);
	m_btnSetup.SetTextColor(RGB(0x55, 0x58, 0x5A), RGB(0x00, 0xA0, 0xE9), RGB(0x00, 0xA0, 0xE9), RGB(0xCC, 0xCC, 0xCC));
	m_btnSetup.SetWindowText(_T("1920*1080,15fps, 3mbps"));

	CMFCButton::EnableWindowsTheming(FALSE);
}

void CEnterChannelDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	DrawClient(&dc);
}


void CEnterChannelDlg::DrawClient(CDC *lpDC)
{
	CRect	rcText;
	CRect	rcClient;
	LPCTSTR lpString = NULL;

	GetClientRect(&rcClient);

    CFont* defFont = lpDC->SelectObject(&m_ftDesc);
	lpDC->SetBkColor(RGB(0xFF, 0xFF, 0xFF));
	lpDC->SetTextColor(RGB(0x44, 0x45, 0x46));

//	lpDC->SelectObject(&m_penFrame);
	rcText.SetRect(rcClient.Width() / 2 - 180, rcClient.Height() / 2 - 60, rcClient.Width() / 2 + 270, rcClient.Height() / 2 - 10);
	lpDC->RoundRect(&rcText, CPoint(42, 42));

	lpDC->SelectObject(defFont);
}

void CEnterChannelDlg::OnBnClickedBtnjoinChannel()
{
	CString     strKey;
    CString     strChannelName;
    CString     strInfo;
    CString     strOperation;
    BOOL        bFound = FALSE;
    BOOL        bSuccess = FALSE;

    m_ctrChannel.GetWindowText(strChannelName);

	GetParent()->SendMessage(WM_JOINCHANNEL, 0, 0);
}


void CEnterChannelDlg::OnBnClickedBtnsetChannel()
{
	GetParent()->SendMessage(WM_GONEXT, 0, 0);
}

void CEnterChannelDlg::OnBnClickedBtngetChannel()
{
	GetParent()->SendMessage(WM_STARTSOCKET, 0, 0);
}

CString CEnterChannelDlg::GetChannelName()
{
	CString strChannelName;

	m_ctrChannel.GetWindowText(strChannelName);

	return strChannelName;
}

void CEnterChannelDlg::SetVideoString(LPCTSTR lpVideoString)
{
	m_btnSetup.SetWindowText(lpVideoString);
}
