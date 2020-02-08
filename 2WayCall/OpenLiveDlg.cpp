
// OpenLiveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OpenLive.h"
#include "OpenLiveDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// static members
const char* COpenLiveDlg::EventsStrings[] = { "Connection", "Message", "Disconnection" };
const char* COpenLiveDlg::ProfileStrings[] = { "2WayCall" };
const char* COpenLiveDlg::ActionStrings[] = { "join", "leave" };
const char* COpenLiveDlg::MessageStrings[] = { "profile", "type", "wsID", "channel", "ClassroomName", "ClassroomID", "action", "Failure", "CenterName" };
const char* COpenLiveDlg::WebApiStrings[] = { "ClassRoomName", "ClassRoomId", "LastUsedCommand", "CenterName" };
std::string COpenLiveDlg::m_sBaseUrl = std::string("http://portal.2waylive.com/api/");
std::string COpenLiveDlg::m_sAuthPath = std::string("auth.pem");
std::string TESTING_URI = "ws://localhost:2000";
std::string PROD_URI = "ws://52.15.186.193:2000";

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
	DDX_Control(pDX, IDC_STATUSCONNECT, m_statusConnect);
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
	ON_MESSAGE(WM_STARTSOCKET, &COpenLiveDlg::WebSocketHandler)
	ON_MESSAGE(WM_SETCONNECTED, &COpenLiveDlg::SetConnected)
	ON_MESSAGE(WM_SETDISCONNECTED, &COpenLiveDlg::SetDisconnected)

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
	m_ftTxt.CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));

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

	m_statusConnect.SetWindowTextW(L"Disconnected");

	atexit([]() {
		std::terminate();
	});
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

	//m_statusConnect->SetWindowText(_T("Disconnected"));
	m_statusConnect.MoveWindow(ClientRect.Width()/2-ClientRect.Width()/8, 40, ClientRect.Width()/4, 40);
	m_statusConnect.SetFont(&m_ftTxt);
	
	m_btnMin.MoveWindow(ClientRect.Width() - 46, 1, 22, 22, TRUE);
	m_btnClose.MoveWindow(ClientRect.Width() - 23, 1, 22, 22, TRUE);
	m_linkAgora.MoveWindow(ClientRect.Width() / 2 - 30, ClientRect.Height()-55, 80, 20, TRUE);

	m_btnMin.SetBackImage(IDB_BTNMIN, RGB(0xFF, 0, 0xFF));
	m_btnClose.SetBackImage(IDB_BTNCLOSE, RGB(0xFF, 0, 0xFF));

	m_linkAgora.SetFont(&m_ftLink);
	m_linkAgora.SetURL(_T("http://www.2WayLive.com"));
	m_linkAgora.SetWindowText(LANG_STR("IDS_LOGO_AGORAWEB"));
	CMFCButton::EnableWindowsTheming(FALSE);
	SetClassroomDetails();
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
	ShowWindow(SW_MINIMIZE);
}


void COpenLiveDlg::OnBnClickedBtnclose()
{
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
	RtcEngineParameters *lpRtcParameters = new RtcEngineParameters(*lpRtcEngine);
	lpRtcParameters->setLocalVideoMirrorMode(agora::rtc::VIDEO_MIRROR_MODE_DISABLED); //Set Video mirror for local video 

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


void COpenLiveDlg::COutputLogger(const char* txt)
{
	std::ofstream log("output.txt", std::ios_base::app | std::ios_base::out);
	log << txt << std::endl;
}

std::wstring ConvertToWString(std::string str)
{
	std::wstring tempUrl;
	tempUrl.assign(str.begin(), str.end());
	return tempUrl;
}

LRESULT COpenLiveDlg::SetConnected(WPARAM wParam, LPARAM lParam)
{
	COutputLogger("SETTING CONNECTED");
	m_statusConnect.SetWindowTextW(L"Connected");
	CRect rect;
	m_statusConnect.GetWindowRect(&rect);
	ScreenToClient(&rect);
	InvalidateRect(&rect);
	UpdateWindow();

	CString s;
	m_statusConnect.GetWindowTextW(s);
	std::wstring st(s);
	std::string str(st.begin(), st.end());
	COutputLogger(str.c_str());

	return 0;
}

LRESULT COpenLiveDlg::SetDisconnected(WPARAM wParam, LPARAM lParam)
{
	COutputLogger("SETTING DISCONNECTED");
	m_statusConnect.SetWindowTextW(L"Disconnected");
	CRect rect;
	m_statusConnect.GetWindowRect(&rect);
	ScreenToClient(&rect);
	InvalidateRect(&rect);
	UpdateWindow();

	return 0;
}

LRESULT COpenLiveDlg::WebSocketHandler(WPARAM wParam, LPARAM lParam)
{
	COutputLogger("Inside Websocket Handler");
	auto t = concurrency::create_task([&]()
	{
		COutputLogger("GETTING HERE");
		StartWebSockets(this);
	});

	return 0;
}


const char* COpenLiveDlg::GetTextForEvent(int enumVal)
{
	return COpenLiveDlg::EventsStrings[enumVal];
}

const char* COpenLiveDlg::GetTextForMessage(int enumVal)
{
	return COpenLiveDlg::MessageStrings[enumVal];
}

const char* COpenLiveDlg::GetTextForProfile(int enumVal)
{
	return COpenLiveDlg::ProfileStrings[enumVal];
}

const char* COpenLiveDlg::GetTextForAction(int enumVal)
{
	return COpenLiveDlg::ActionStrings[enumVal];
}

const char* COpenLiveDlg::GetTextForWebApi(int enumVal)
{
	return COpenLiveDlg::WebApiStrings[enumVal];
}

void COpenLiveDlg::StartWebSockets(CWnd *m_statusConnect)
{
	using namespace std;
	using json = nlohmann::json;

	connection = COpenLiveDlg::GetConnectionDetails(COpenLiveDlg::m_nClassroomID);
	if (strcmp(connection.c_str(), "Failure") == 0) {
		MessageBoxA(0, "An Error Occurred Finding Server. Please contact support", "Notice", MB_ICONINFORMATION);
	}
	else
	{
		h.connect(connection.c_str());
	}

	h.onConnection([&](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) 
	{
		COutputLogger("Connection to server successful");

		m_statusConnect->SendMessage(WM_SETCONNECTED, 0, 0);

		auto channel = m_dlgEnterChannel.GetChannelName().GetBuffer();
		char buffer[500];
		wcstombs(buffer, channel, 500);		
		
		json jsConnectionConfirmation;
		jsConnectionConfirmation[GetTextForMessage(Message::PROFILE)] = GetTextForProfile(Profile::TWOWAYCALL);
		jsConnectionConfirmation[GetTextForMessage(Message::TYPE)] = GetTextForEvent(Events::CONNECTION);
		jsConnectionConfirmation[GetTextForMessage(Message::CLASSROOMNAME)] = COpenLiveDlg::m_sClassroomName;
		COutputLogger(m_sCenterName.c_str());
		jsConnectionConfirmation[GetTextForMessage(Message::CENTERNAME)] = COpenLiveDlg::m_sCenterName;
		jsConnectionConfirmation[GetTextForMessage(Message::CLASSROOMID)] = COpenLiveDlg::m_nClassroomID;
		jsConnectionConfirmation[GetTextForMessage(Message::CHANNEL)] = buffer;

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
			if (jsParseData.find(GetTextForMessage(Message::PROFILE)) != jsParseData.end()) {
				auto profile = jsParseData[GetTextForMessage(Message::PROFILE)].get<string>();

				if (profile.compare(string(GetTextForProfile(Profile::TWOWAYCALL))) == 0) {
					auto action = jsParseData[GetTextForMessage(Message::ACTION)].get<string>();

					if (action.compare(string(GetTextForAction(Action::JOIN))) == 0) {
						std::ofstream log("output.txt", std::ios_base::app | std::ios_base::out);
						log << "Join call on channel " << m_dlgEnterChannel.GetChannelName() << endl;
						COpenLiveDlg::OnJoinChannel(0, 0);
						CVideoDlg::m_bInitialFullScreenCheck = TRUE;

						Sleep(1000);
						system("stop.exe");
					}
					else if (action.compare(string(GetTextForAction(Action::LEAVE))) == 0) {
						// Leave video
						COutputLogger("Leaving Call Channel");
						COutputLogger("Starting VLC");
						lpAgoraObject->SetMsgHandlerWnd(m_dlgVideo.GetSafeHwnd());
						m_dlgVideo.SendMessage(WM_LEAVEHANDLER, 0, 0);
						m_dlgVideo.SetForegroundWindow();

						// Get VLC Last Used Command
						StartVlc();
					}
				}
			}
		}
		else {
			COutputLogger("Message was not json");
		}
	});

	h.onDisconnection([&](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
		COutputLogger("CLIENT CLOSE: ");
		m_statusConnect->SendMessage(WM_SETDISCONNECTED, 0, 0);
		MessageBox(_T("Disconnected. Enter channel again."));
	});
	
	h.onError([&](void *user) {
		ErrorCheck(user);
		h.connect(connection.c_str());
	});

	//h.connect(TESTING_URI); // connect to server Change to prod server for release
	
	h.run();

	COutputLogger("Exiting WebSocket thread");
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
		COutputLogger("Client emitted error on invalid URI");
		break;
	case 2:
		COutputLogger("Client emitted error on resolve failure");
		break;
	case 3:
		COutputLogger("Client emitted error on connection timeout (non-SSL)");
		break;
	case 5:
		COutputLogger("Client emitted error on connection timeout (SSL)");
		break;
	case 6:
		COutputLogger("Client emitted error on HTTP response without upgrade (non-SSL)");
		break;
	case 7:
		COutputLogger("Client emitted error on HTTP response without upgrade (SSL)");
		break;
	case 10:
		COutputLogger("Client emitted error on poll error");
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
		COutputLogger("Could not connect to websocket");
		//MessageBox(_T("Unable to Reach Server. Please contact administrator"), _T("Notice"), MB_ICONSTOP);
		break;
	}
}

void COpenLiveDlg::StartVlc()
{
	using namespace std;
	using web::uri;
	using json = nlohmann::json;

	string classroomUrl = COpenLiveDlg::m_sBaseUrl + std::string("GetClassroom/") + m_sAuthKey;
	COutputLogger(classroomUrl.c_str());

	uri *url = new uri(ConvertToWString(classroomUrl).c_str());
	
	string result = "";
	try
	{
		string result = HTTPStreamingAsync(url).get();

		nlohmann::json classroom = nlohmann::json::parse(result);

		m_sLastUsedCommand = classroom[GetTextForWebApi(WebApi::LASTUSEDCOMMAND)].get<string>();

		COutputLogger("Result of last used command");
		
		COutputLogger("No errors retrieving VLC Command");
		string str = string("START \"\" ") + m_sLastUsedCommand;
		COutputLogger(str.c_str());
		system(str.c_str());
	}
	catch (...)
	{
		string str = string("START \"\" ") + m_sLastUsedCommand;
		system(str.c_str());
	}
}

std::string COpenLiveDlg::GetConnectionDetails(int m_nClassroomID)
{
	using web::uri;
	std::string connectUrl = COpenLiveDlg::m_sBaseUrl + "GetCallServer/" + std::to_string(m_nClassroomID);
	COutputLogger("URL Is: ");
	COutputLogger(connectUrl.c_str());

	uri* url = new uri(ConvertToWString(connectUrl).c_str());
	std::string val = std::string(HTTPStreamingAsync(url).get());

	if (!IsJson(val))
		throw std::exception("Call to server unsuccessful!");

	val = val.substr(1, val.length() - 2);

	COutputLogger("Server Url Retrieved");
	COutputLogger(val.c_str());

	return val;
}

void COpenLiveDlg::SetClassroomDetails()
{
	using namespace std;
	using web::uri;
	COutputLogger("About to get baseboard details!");
	string baseboard = Exec("wmic path win32_computersystemproduct get uuid");

	// Vector of string to save tokens 
	vector <string> tokens;

	// stringstream class check1 
	stringstream check1(baseboard);
	string intermediate;

	// Tokenize
	while (getline(check1, intermediate, '\n'))
	{
		tokens.push_back(intermediate);
	}

	// Baseboard number is the second one
	baseboard = tokens[1].substr(0, tokens[1].size() - 3);
	COutputLogger(baseboard.c_str());

	COutputLogger("Setting Classroom details");

	// Read permissions
	m_sAuthKey = baseboard;//ReadAuthPermissions();

	if (m_sAuthKey.empty()) {
		MessageBox(_T("NO Permissions!"), _T("Notice"), MB_ICONINFORMATION);
		//PostQuitMessage(0);
		COutputLogger("Unable to read permissions");
	}

	string classroomUrl = COpenLiveDlg::m_sBaseUrl + std::string("GetClassroom/") + m_sAuthKey;
	COutputLogger(classroomUrl.c_str());

	uri *url = new uri(ConvertToWString(classroomUrl).c_str());
	string result = HTTPStreamingAsync(url).get();
	COutputLogger(result.c_str());
	
	nlohmann::json classroom = nlohmann::json::parse(result);

	if (int(classroom[GetTextForWebApi(WebApi::CLASSROOMIDS)]) == -1) {
		MessageBox(_T("Something went wrong!"), _T("Notice"), MB_ICONINFORMATION);
	}
	else {
		m_sClassroomName = classroom[GetTextForWebApi(WebApi::CLASSROOMNAMES)].get<string>();
		m_sCenterName = classroom[GetTextForWebApi(WebApi::CENTERNAMES)].get<string>();
		m_nClassroomID = int(classroom[GetTextForWebApi(WebApi::CLASSROOMIDS)]);
		m_sLastUsedCommand = classroom[GetTextForWebApi(WebApi::LASTUSEDCOMMAND)].get<string>();
		COutputLogger(m_sLastUsedCommand.c_str());
	}
	COutputLogger(m_sClassroomName.c_str());
	COutputLogger(m_sCenterName.c_str());
}

std::string COpenLiveDlg::Exec(const char* cmd) {
	
	char buffer[128];
	std::string result = "";
	
	FILE* pipe = _popen(cmd, "r");
	if (!pipe) throw std::runtime_error("popen() failed!");
	try {
		while (fgets(buffer, sizeof buffer, pipe) != NULL) {
			result += buffer;
		}
	}
	catch (...) {
		_pclose(pipe);
		throw;
	}
	_pclose(pipe);
	
	return result;
}

std::string COpenLiveDlg::ReadAuthPermissions()
{
	using namespace std;
	ifstream fileStream(m_sAuthPath.c_str());
	ASSERT(fileStream.good());
	string line;	string result;
	while (std::getline(fileStream, line)) {
		result = line;
	}

	COutputLogger(result.c_str());
	return result;
}

// Creates an HTTP request and returns the response body.
pplx::task<std::string> COpenLiveDlg::HTTPStreamingAsync(web::uri* url)
{
	using namespace web::http;
	using namespace web::http::client;

	http_client client(*url);

	// Make the request and asynchronously process the response.
	try {
		return client.request(methods::GET).then([&](http_response response) {
			if (response.status_code() == status_codes::OK) {
				try {
					auto body = response.extract_utf8string();
					return body;
				}
				catch (...) {
					COutputLogger("Error extracting string. Return \" - 1 \" Error: ");
					MessageBox(_T("Unable to connect to 2WayLive Server. Contact administrator!"), _T("Notice"), MB_ICONSTOP);
					std::string a = std::string("-1");
					return concurrency::create_task(
						[a]()
					{
						return a;
					});
				}
			}
			else {
				COutputLogger("Unable to connect to 2WayLive Server.");
				MessageBox(_T("Unable to connect to 2WayLive Server. Contact administrator!"), _T("Notice"), MB_ICONSTOP);
			}
		});
	}
	catch (...) {
		COutputLogger("Unable to connect to 2WayLive Server.");
		MessageBox(_T("Unable to connect to 2WayLive Server. Contact administrator!"), _T("Notice"), MB_ICONSTOP);
		std::string a = std::string("-1");
		return concurrency::create_task(
			[a]()
		{
			return a;
		});
	}

	COutputLogger("Reaching end of the url function. Error");
}

