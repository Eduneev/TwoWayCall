
// OpenLiveDlg.h : header file
//

#pragma once

#include "AGHyperlink.h"
#include "EnterChannelDlg.h"
#include "SetupDlg.h"
#include "VideoDlg.h"
#include <uWS/uWS.h>
#include <nlohmann/json.hpp>
#include <ppltasks.h>
#include <cpprest/http_client.h>


// COpenLiveDlg dialog
class COpenLiveDlg : public CDialogEx
{
// Construction
public:
    COpenLiveDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_AGORAVIDEOCALL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnNcHitTest(CPoint point);

	afx_msg void OnBnClickedBtnmin();
	afx_msg void OnBnClickedBtnclose();

	afx_msg LRESULT OnBackPage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNextPage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnJoinChannel(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLeaveChannel(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT WebSocketHandler(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT SetConnected(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT SetDisconnected(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnNetworkQuality(WPARAM wParam, LPARAM lParam);


	DECLARE_MESSAGE_MAP()

protected:
	void InitCtrls();
	void DrawClient(CDC *lpDC);
	void InitChildDialog();

private:
	CAGButton		m_btnMin;
	CAGButton		m_btnClose;

	CAGHyperLink	m_linkAgora;

	CDialogEx			*m_lpCurDialog;
	CEnterChannelDlg	m_dlgEnterChannel;
	CSetupDlg			m_dlgSetup;

	CFont		m_ftTitle;
	CFont		m_ftLink;
	CFont		m_ftVer;
	CFont		m_ftTxt;
	CImageList	m_imgNetQuality;
	//CWnd		*m_statusConnect;
	//CStatic		m_statusConnect;

private:
	CVideoDlg		m_dlgVideo;
	CAgoraObject	*m_lpAgoraObject;
	IRtcEngine		*m_lpRtcEngine;
	uWS::Hub h;


	enum Events { CONNECTION, MESSAGE, DISCONNECTION };

	enum Profile { TWOWAYCALL };

	enum Action { JOIN, LEAVE };

	enum Message { PROFILE, TYPE, WSID, CHANNEL, CLASSROOMNAME, CLASSROOMID, ACTION, FAILURE, CENTERNAME };

	enum WebApi { CLASSROOMNAMES, CLASSROOMIDS, LASTUSEDCOMMAND, CENTERNAMES };
	
private:	// data
    int m_nVideoProfile;
	int m_nNetworkQuality;
public:
    afx_msg void OnClose();
	afx_msg void OnStnClickedLinkagora();

	//place in different file maybe
	void StartWebSockets(CWnd *m_statusConnect);
	bool IsJson(std::string str);
	std::string Exec(const char* cmd);
	void ErrorCheck(void* user);
	Concurrency::task<std::string> COpenLiveDlg::HTTPStreamingAsync(web::uri* url);
	void SetClassroomDetails();
	void StartVlc();
	void COutputLogger(const char* txt);
	std::string COpenLiveDlg::ReadAuthPermissions();
	std::string GetConnectionDetails(int m_nClassroomID);


	static const char* EventsStrings[];
	static const char* ProfileStrings[];
	static const char* ActionStrings[];
	static const char* MessageStrings[];
	static const char* WebApiStrings[];
	static std::string m_sBaseUrl;
	static std::string m_sAuthPath;

	const char* GetTextForEvent(int enumVal);
	const char* GetTextForProfile(int enumVal);
	const char* GetTextForAction(int enumVal);
	const char* GetTextForMessage(int enumVal);
	const char* GetTextForWebApi(int enumval);
	int m_nClassroomID;
	std::string m_sClassroomName;
	std::string m_sCenterName;
	std::string m_sLastUsedCommand;
	std::string m_sAuthKey;
	std::string connection;
};
