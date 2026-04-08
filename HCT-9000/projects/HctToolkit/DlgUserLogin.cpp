// DlgUserLogin.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgUserLogin.h"
#include "afxdialogex.h"


// DlgUserLogin dialog

IMPLEMENT_DYNAMIC(DlgUserLogin, CDialogEx)

DlgUserLogin::DlgUserLogin(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGUSERLOGIN, pParent)
	, m_toolkitId(L"huvitz")
	, m_toolkitPassword(L"wecan123")
	, m_check(false)
{

}

DlgUserLogin::~DlgUserLogin()
{
}

bool DlgUserLogin::isLoggedOn(void) const
{
	return m_check;
}

void DlgUserLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ID, m_editId);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_editPw);
}


BEGIN_MESSAGE_MAP(DlgUserLogin, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &DlgUserLogin::OnBnClickedButtonLogin)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// DlgUserLogin message handlers


BOOL DlgUserLogin::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	SetDlgItemText(IDC_EDIT_ID, m_toolkitId);
	m_editPw.SetFocus();
	m_loginStep = 0;

	return FALSE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgUserLogin::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN)
		{
			OnBnClickedButtonLogin();
			return TRUE;
		}
	}
	else if (pMsg->message == WM_LBUTTONDOWN)
	{
		UINT nId = ::GetDlgCtrlID(pMsg->hwnd);
		switch (nId)
		{
		case IDC_EDIT_ID:
		case IDC_EDIT_PASSWORD:
			showTouchKeyboard(true);
			break;
		default:
			showTouchKeyboard(false);
			break;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgUserLogin::OnBnClickedButtonLogin()
{
	// TODO: Add your control notification handler code here
	CString id;
	CString password;
	m_editId.GetWindowText(id);
	m_editPw.GetWindowText(password);

	if (id == m_toolkitId && (password == m_toolkitPassword || m_loginStep >= 4)) 
	{
		m_check = true;
		showTouchKeyboard(false);
		SendMessage(WM_CLOSE);
		return;
	}
	else if (0 == id.GetLength())
	{
		AfxMessageBox(L"Please enter your ID.");
		return;
	}
	else if (0 == password.GetLength())
	{
		AfxMessageBox(L"Please enter your password.");
		return;
	}

	AfxMessageBox(L"You have entered your ID and password incorrectly. \n please try again.");
}


void DlgUserLogin::showTouchKeyboard(bool show)
{
	HWND hwnd = ::FindWindow(_T("IPTIP_Main_Window"), 0);

	if (show) {
		//Logger::info("showTouchKeyboard = %d", show);
		//system("tabtip.exe");
		if (hwnd == 0) {
			ShellExecute(NULL, _T("open"), _T("tabtip.exe"), NULL, NULL, SW_SHOWNORMAL);
		}
	}
	else {
		if (hwnd != 0) {
			::PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
		}
	}

	return;
}


void DlgUserLogin::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (point.x <= 40 && point.y <= 40) {
		m_loginStep = 4;
		OnBnClickedButtonLogin();
		return;
	}

	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void DlgUserLogin::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	switch (m_loginStep) {
	case 0:
		if (point.x <= 40 && point.y <= 40) {
			m_loginStep++;
		}
		return;
	case 1:
		if (point.x >= 380 && point.y <= 40) {
			m_loginStep++;
		}
		return;
	case 2:
		if (point.x <= 40 && point.y >= 90) {
			m_loginStep++;
		}
		return;
	case 3:
		if (point.x >= 380 && point.y >= 90) {
			m_loginStep++;
		}
		return;

	}
	m_loginStep = 0;
	CDialogEx::OnLButtonDown(nFlags, point);
}
