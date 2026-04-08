#pragma once
#include "afxdialogex.h"


// DlgSystemSettings dialog

class DlgSystemSettings : public CDialogEx
{
	DECLARE_DYNAMIC(DlgSystemSettings)

public:
	DlgSystemSettings(CWnd* pParent = nullptr);   // standard constructor
	virtual ~DlgSystemSettings();

	void initControls(void);
	void loadControls(void);
	void saveControls(void);
	void changeDiscTestStauts(void);

protected:
	BOOL IsExist(CString path);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGSYSTEMSETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonExport();
	afx_msg void OnBnClickedButtonImport();
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonApplyChina();
	afx_msg void OnBnClickedButtonDeleteChina();
	afx_msg void OnBnClickedButtonApplyDisctest();
	afx_msg void OnBnClickedButtonDeleteDisctest();
	CEdit editSerialNo;
	CComboBox cmbModelType;
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();

	int _grabberType = 0;
	int _lineCameraMode = 0;
	int _faultDetection = 0;
	int _retinaTracking = 0;
	afx_msg void updateOctGrabberType(UINT value);
	afx_msg void updateOctLineCameraMode(UINT value);
	afx_msg void updateSldFaultDetection(UINT value);
	afx_msg void updateRetinaTrackingSpeed(UINT value);
	afx_msg void OnBnClickedButtonClose();
	CEdit editFaultThresh;
	CEdit editFaultTimer;
	CEdit editDiscTestStatus;
	CEdit editForePadd1;
	CEdit editForePadd2;
	CEdit editForePadd3;
	CEdit editPostPadd1;
	CEdit editPostPadd2;
	CEdit editPostPadd3;
};
