#pragma once
#include "AGButton.h"
#include "AGEdit.h"
#include "AGComboBox.h"
#include "DeviceDlg.h"
#include "afxwin.h"
#include "afxlistctrl.h"
#include "VideoDlg.h"

// CEnterChannelDlg ¶Ô»°¿ò

class CEnterChannelDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEnterChannelDlg)

public:
	CEnterChannelDlg(CWnd* pParent = NULL);
	virtual ~CEnterChannelDlg();

	CString GetChannelName();

	void SetVideoString(LPCTSTR lpVideoString);
	enum { IDD = IDD_ENTERCHANNEL_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX); 
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnPaint();
	afx_msg void OnBnClickedBtnjoinChannel();
	afx_msg void OnBnClickedBtnsetChannel();
	DECLARE_MESSAGE_MAP()

protected:
	void InitCtrls();
	void DrawClient(CDC *lpDC);

private:
	CAGEdit			m_ctrChannel;
    CAGButton		m_btnJoin;
	CAGButton		m_btnSetup;

	CFont			m_ftChannel;
	CFont			m_ftHead;
	CFont			m_ftDesc;
	CFont			m_ftBtn;

	CPen            m_penFrame;
	CDeviceDlg		m_dlgDevice;
};
