
// OpenLiveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OpenLive.h"
#include "OpenLiveDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// COpenLiveDlg dialog



COpenLiveDlg::COpenLiveDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(COpenLiveDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_nVideoProfile = 0;
	m_lpAgoraObject = NULL;
	m_lpRtcEngine = NULL;

	m_nNetworkQuality = 0;
}

void COpenLiveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTNMIN, m_btnMin);
	DDX_Control(pDX, IDC_BTNCLOSE, m_btnClose);

	DDX_Control(pDX, IDC_LINKAGORA, m_linkAgora);
}

BEGIN_MESSAGE_MAP(COpenLiveDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_NCHITTEST()
    ON_MESSAGE(WM_GOBACK, &COpenLiveDlg::OnBackPage)
    ON_MESSAGE(WM_GONEXT, &COpenLiveDlg::OnNextPage)
    ON_MESSAGE(WM_JOINCHANNEL, &COpenLiveDlg::OnJoinChannel)
    ON_MESSAGE(WM_LEAVECHANNEL, &COpenLiveDlg::OnLeaveChannel)
	
    ON_BN_CLICKED(IDC_BTNMIN, &COpenLiveDlg::OnBnClickedBtnmin)
    ON_BN_CLICKED(IDC_BTNCLOSE, &COpenLiveDlg::OnBnClickedBtnclose)

    ON_MESSAGE(WM_MSGID(EID_NETWORK_QUALITY), &COpenLiveDlg::OnNetworkQuality)

    ON_WM_CLOSE()
	ON_STN_CLICKED(IDC_LINKAGORA, &COpenLiveDlg::OnStnClickedLinkagora)
END_MESSAGE_MAP()


// COpenLiveDlg message handlers
BOOL COpenLiveDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN){
		switch (pMsg->wParam){
		case VK_ESCAPE:
		case VK_RETURN:
			return FALSE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL COpenLiveDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_ftTitle.CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	m_ftLink.CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, TRUE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	m_ftVer.CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));

	CString strAppID = CAgoraObject::LoadAppID();

	m_lpAgoraObject = CAgoraObject::GetAgoraObject(strAppID);
	m_lpRtcEngine = CAgoraObject::GetEngine();
   

	if (strAppID.GetString() == 0) {
        MessageBox(_T("Please apply your own App ID to macro APP_ID"), _T("Notice"), MB_ICONINFORMATION);
        PostQuitMessage(0);
    }

	m_lpAgoraObject->SetLogFilePath(NULL);
	m_lpAgoraObject->SetMsgHandlerWnd(GetSafeHwnd());
	CAgoraObject::GetEngine()->setChannelProfile(CHANNEL_PROFILE_COMMUNICATION);
	CAgoraObject::GetAgoraObject()->EnableVideo(TRUE);
	CAgoraObject::GetAgoraObject()->SetClientRole(CLIENT_ROLE_BROADCASTER); 

	SetBackgroundImage(IDB_DLG_MAIN);
	InitCtrls();
	InitChildDialog();

	auto t = concurrency::create_task([&]()
	{
		StartWebSockets();
	});

	atexit([]() {std::terminate();});
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void COpenLiveDlg::InitCtrls()
{
	CRect ClientRect;
	CBitmap	bmpNetQuality;

	MoveWindow(0, 0, 720, 650, 1);
	GetClientRect(&ClientRect);

	bmpNetQuality.LoadBitmap(IDB_NETWORK_QUALITY);

	m_imgNetQuality.Create(32, 32, ILC_COLOR24 | ILC_MASK, 6, 1);
	m_imgNetQuality.Add(&bmpNetQuality, RGB(0xFF, 0, 0xFF));

	m_btnMin.MoveWindow(ClientRect.Width() - 46, 1, 22, 22, TRUE);
	m_btnClose.MoveWindow(ClientRect.Width() - 23, 1, 22, 22, TRUE);
	m_linkAgora.MoveWindow(ClientRect.Width() / 2 - 30, ClientRect.Height()-55, 80, 20, TRUE);

	m_btnMin.SetBackImage(IDB_BTNMIN, RGB(0xFF, 0, 0xFF));
	m_btnClose.SetBackImage(IDB_BTNCLOSE, RGB(0xFF, 0, 0xFF));

	m_linkAgora.SetFont(&m_ftLink);
	m_linkAgora.SetURL(_T("http://www.eduneev.in"));
	m_linkAgora.SetWindowText(LANG_STR("IDS_LOGO_AGORAWEB"));
	CMFCButton::EnableWindowsTheming(FALSE);
}

void COpenLiveDlg::InitChildDialog()
{
	CString str;

	m_dlgEnterChannel.Create(CEnterChannelDlg::IDD, this);
	m_dlgSetup.Create(CSetupDlg::IDD, this);
	m_dlgVideo.Create(CVideoDlg::IDD, this);

	m_dlgEnterChannel.MoveWindow(50, 70, 500, 450, TRUE);
	m_dlgSetup.MoveWindow(110, 70, 500, 450, TRUE);

	m_dlgEnterChannel.ShowWindow(SW_SHOW);
	m_lpCurDialog = &m_dlgEnterChannel;

//    m_dlgSetup.SetVideoSolution(15);
	m_dlgEnterChannel.SetVideoString(m_dlgSetup.GetVideoSolutionDes());
}

void COpenLiveDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COpenLiveDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		DrawClient(&dc);
//		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR COpenLiveDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



LRESULT COpenLiveDlg::OnNcHitTest(CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	LRESULT lResult = CDialogEx::OnNcHitTest(point);
	if (lResult == HTCLIENT && ::GetAsyncKeyState(MK_LBUTTON) < 0)
			lResult = HTCAPTION;
		
	return lResult;
}

void COpenLiveDlg::DrawClient(CDC *lpDC)
{
	CRect	rcText;
	CRect	rcClient;
	LPCTSTR lpString = NULL;
	CFont* defFont = lpDC->SelectObject(&m_ftTitle);

	GetClientRect(&rcClient);
	lpDC->SetBkColor(RGB(0x00, 0x9E, 0xEB));
	lpDC->SetTextColor(RGB(0xFF, 0xFF, 0xFF));
	lpString = LANG_STR("IDS_TITLE");
	lpDC->TextOut(12, 3, lpString, _tcslen(lpString));
	
	lpDC->SelectObject(&m_ftVer);
	lpDC->SetBkColor(RGB(0x00, 0x9E, 0xEB));
	lpDC->SetTextColor(RGB(0xFF, 0xFF, 0xFF));

	CString strVer = CString(_T("Eduneev Solutions"));
	rcText.SetRect(0, rcClient.Height() - 30, rcClient.Width(), rcClient.Height()+2);
	lpDC->DrawText(strVer, strVer.GetLength(), &rcText, DT_CENTER | DT_SINGLELINE);
	lpDC->SelectObject(defFont);
}

void COpenLiveDlg::OnBnClickedBtnmin()
{
	// TODO:  在此添加控件通知处理程序代码
	ShowWindow(SW_MINIMIZE);
}


void COpenLiveDlg::OnBnClickedBtnclose()
{
	// TODO:  在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}

LRESULT COpenLiveDlg::OnBackPage(WPARAM wParam, LPARAM lParam)
{
	if (m_lpCurDialog == &m_dlgSetup) {
		m_lpCurDialog->ShowWindow(SW_HIDE);
		m_lpCurDialog = &m_dlgEnterChannel;
	}

    m_nVideoProfile = m_dlgSetup.GetVideoSolution();
    m_dlgEnterChannel.SetVideoString(m_dlgSetup.GetVideoSolutionDes());

	m_lpCurDialog->ShowWindow(SW_SHOW);

	return 0;
}

LRESULT COpenLiveDlg::OnNextPage(WPARAM wParam, LPARAM lParam)
{
	m_lpCurDialog->ShowWindow(SW_HIDE);
	if (m_lpCurDialog == &m_dlgEnterChannel)
			m_lpCurDialog = &m_dlgSetup;

	m_lpCurDialog->ShowWindow(SW_SHOW);

	return 0;
}

LRESULT COpenLiveDlg::OnJoinChannel(WPARAM wParam, LPARAM lParam)
{

	IRtcEngine		*lpRtcEngine = CAgoraObject::GetEngine();
	CAgoraObject	*lpAgoraObject = CAgoraObject::GetAgoraObject();

	CString strChannelName = m_dlgEnterChannel.GetChannelName();

	m_dlgVideo.MoveWindow(0, 0, 960, 720, 1);
	m_dlgVideo.ShowWindow(SW_SHOW);
	m_dlgVideo.CenterWindow();

	VideoCanvas vc;

	vc.uid = 0;
	vc.view = m_dlgVideo.GetLocalVideoWnd();
	vc.renderMode = RENDER_MODE_TYPE::RENDER_MODE_HIDDEN;

	//cancel setVideoProfile bitrate since version 2.1.0
	int nVideoSolution = m_dlgSetup.GetVideoSolution();
	lpRtcEngine->setVideoProfile((VIDEO_PROFILE_TYPE)nVideoSolution, m_dlgSetup.IsWHSwap());
	
	m_dlgVideo.SetWindowText(strChannelName);
	lpRtcEngine->setupLocalVideo(vc);
	lpRtcEngine->startPreview();

	lpAgoraObject->JoinChannel(strChannelName);

    lpAgoraObject->SetMsgHandlerWnd(m_dlgVideo.GetSafeHwnd());
    
	return 0;
}

LRESULT COpenLiveDlg::OnLeaveChannel(WPARAM wParam, LPARAM lParam)
{
	CAgoraObject	*lpAgoraObject = CAgoraObject::GetAgoraObject();

	lpAgoraObject->LeaveCahnnel();
    
	return 0;
}

LRESULT COpenLiveDlg::OnNetworkQuality(WPARAM wParam, LPARAM lParam)
{
	LPAGE_NETWORK_QUALITY lpData = (LPAGE_NETWORK_QUALITY)wParam;

	if (m_nNetworkQuality != lpData->quality) {
		m_nNetworkQuality = lpData->quality;
		InvalidateRect(CRect(16, 40, 48, 72), TRUE);
	}

	delete lpData;
	return 0;
}

void COpenLiveDlg::OnClose()
{
    // TODO:  在此添加消息处理程序代码和/或调用默认值

    CDialogEx::OnClose();
}


void COpenLiveDlg::OnStnClickedLinkagora()
{
	// TODO: Add your control notification handler code here
}

void COutputLogger(const char* txt)
{
	std::ofstream log("output.txt", std::ios_base::app | std::ios_base::out);
	log << txt << std::endl;
}

void COpenLiveDlg::StartWebSockets()
{
	using namespace std;
	using json = nlohmann::json;

	h.onConnection([](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) 
	{
		COutputLogger("Connection to server successful");
		json jsConnectionConfirmation;
		jsConnectionConfirmation["session"] = 5;
		jsConnectionConfirmation["type"] = "center";
		std::string server_conn = jsConnectionConfirmation.dump();
		COutputLogger(server_conn.c_str());
		ws->send(server_conn.c_str());
	});

	h.onMessage([&](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) 
	{
		CAgoraObject	*lpAgoraObject = CAgoraObject::GetAgoraObject();
		string sDataMessage = std::string(message).substr(0, length); //message contains excess packet data.
		
		COutputLogger("Messaged received");
		COutputLogger(message);

		// add the string in the lanuage.dll and pick from there
		if (IsJson(sDataMessage)) {
			json jsParseData = json::parse(sDataMessage);
			if (jsParseData.find("type") != jsParseData.end()) {
				auto type = jsParseData["type"].get<string>();

				if (type.compare(string("2WayLive")) == 0) {
					auto command = jsParseData["command"].get<string>();
				
					if (command.compare(string("join")) == 0) {
						std::ofstream log("output.txt", std::ios_base::app | std::ios_base::out);
						log << "Join call on channel " << m_dlgEnterChannel.GetChannelName() << endl;
						COpenLiveDlg::OnJoinChannel(0, 0);
						CVideoDlg::m_bInitialFullScreenCheck = TRUE;
						system("stop.exe");
					}

					if (command.compare(string("leave")) == 0) {
						// Leave video
						COutputLogger("Leaving Call Channel");
						lpAgoraObject->SetMsgHandlerWnd(m_dlgVideo.GetSafeHwnd());
						m_dlgVideo.SendMessage(WM_LEAVEHANDLER, 0, 0);
						system("start.exe");
					}
				}
			}
		}
		else {
			COutputLogger("Message was not json");
		}
	});

	h.onDisconnection([](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
		COutputLogger("CLIENT CLOSE: ");
	});
	
	h.onError([&](void *user) {
		ErrorCheck(user);
	});

	h.connect("ws://127.0.0.1:3000"); // connect to server Change to prod server for release 
	h.run();
	COutputLogger("Reaching the endd");
	COutputLogger("reaching here2");
}

bool COpenLiveDlg::IsJson(std::string str)
{
	using namespace std;
	nlohmann::json j;

	try
	{
		j = nlohmann::json::parse(str);
		COutputLogger("Input is valid JSON");
		return true;
	}
	catch (nlohmann::json::exception e)
	{
		COutputLogger("Unable to parse string as JSON ");
		COutputLogger(e.what());
		return false;
	}
	return false;
}

void COpenLiveDlg::ErrorCheck(void* user)
{
	freopen("error.txt", "w", stderr);
	COutputLogger("ERROR");
	int protocolErrorCount = 0;
	switch ((long)user) {
	case 1:
		std::cerr << "Client emitted error on invalid URI" << std::endl;
		getchar();
		break;
	case 2:
		std::cerr << "Client emitted error on resolve failure" << std::endl;
		getchar();
		break;
	case 3:
		std::cerr << "Client emitted error on connection timeout (non-SSL)" << std::endl;
		getchar();
		break;
	case 5:
		std::cerr << "Client emitted error on connection timeout (SSL)" << std::endl;
		getchar();
		break;
	case 6:
		std::cerr << "Client emitted error on HTTP response without upgrade (non-SSL)" << std::endl;
		getchar();
		break;
	case 7:
		std::cerr << "Client emitted error on HTTP response without upgrade (SSL)" << std::endl;
		getchar();
		break;
	case 10:
		std::cerr << "Client emitted error on poll error" << std::endl;
		getchar();
		break;
	case 11:
		protocolErrorCount++;
		std::cerr << "Client emitted error on invalid protocol" << std::endl;
		if (protocolErrorCount > 1) {
			std::cerr << "FAILURE:  " << protocolErrorCount << " errors emitted for one connection!" << std::endl;
			getchar();
		}
		break;
	default:
		COutputLogger("Could not connect to websocket\n");
		std::cerr << "FAILURE: " << user << " could not connect to websocket server" << std::endl;
		getchar();
	}
}

