#pragma once
#include "afxwin.h"


// DlgUserLogin dialog

class DlgUserLogin : public CDialogEx
{
	DECLARE_DYNAMIC(DlgUserLogin)

public:
	DlgUserLogin(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgUserLogin();

public:
	bool isLoggedOn(void) const;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGUSERLOGIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	bool m_check;
	CString m_toolkitId;
	CString m_toolkitPassword;
	int m_loginStep;

	CEdit m_editId;
	CEdit m_editPw;
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonLogin();

	static void showTouchKeyboard(bool show);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
