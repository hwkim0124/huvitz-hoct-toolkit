
// HctToolkitDlg.h : header file
//

#pragma once

#include "OctSystem2.h"

// CHctToolkitDlg dialog
class CHctToolkitDlg : public CDialogEx
{
// Construction
public:
	CHctToolkitDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HCTTOOLKIT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	HANDLE m_hMutex;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL DestroyWindow();
	afx_msg void OnDestroy();
	afx_msg void OnClickedButtonOctScanTest();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonSpectroCalib();
	afx_msg void OnBnClickedButtonDispersion();
	afx_msg void OnBnClickedButtonKerato();
	afx_msg void OnBnClickedButtonColor();
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnBnClickedButtonSaveSystem();
	afx_msg void OnBnClickedButtonLoadSystem();
	afx_msg void OnBnClickedButtonResetUsb();
	afx_msg void OnBnClickedButtonSignal();
	afx_msg void OnBnClickedButtonSysCalib();
	afx_msg void OnBnClickedButtonMainboard();
	afx_msg void OnBnClickedButtonResetSystem();
	afx_msg void OnBnClickedButtonLongrun();
	afx_msg void OnBnClickedButtonFactorySetup();
	afx_msg void OnBnClickedButtonPackageMode();
	afx_msg void OnBnClickedButtonAngioPattern();
private:
	void checkPermission();
public:
	afx_msg void OnBnClickedButtonResetUsbDriver();
	afx_msg void OnBnClickedButtonTopographyPattern();
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnBnClickedButtonSystemSettings();
};
