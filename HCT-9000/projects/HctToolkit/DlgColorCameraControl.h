#pragma once
#include "afxwin.h"

#include "OctSystem2.h"

// DlgColorCameraControl dialog

class DlgColorCameraControl : public CDialogEx
{
	DECLARE_DYNAMIC(DlgColorCameraControl)

public:
	DlgColorCameraControl(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgColorCameraControl();

protected:
	OctDevice::ColorCamera* camera;

protected:
	void initControls(void);
	void readControls(void);


// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGCOLORCAMERACONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CComboBox cmbAgain;
	CComboBox cmbCapture;
	CComboBox cmbFlip;
	CComboBox cmbFrameSize;
	CComboBox cmbRate;
	CComboBox cmbShutter;
	CEdit editDgain;
	CEdit editExposCount;
	CEdit editExposOffCount;
	CEdit editStrobeAct;
	CEdit editStrobeDelay;
	CEdit editStartX;
	CEdit editWidth;
	CEdit editHeight;
	CEdit editStartY;
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	afx_msg void OnSelchangeComboFramesize();
	afx_msg void OnSelchangeComboRate();
	afx_msg void OnSelchangeComboShutter();
	afx_msg void OnSelchangeComboFlip();
	afx_msg void OnSelchangeComboAgain();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonDefault();
	afx_msg void OnCbnSelchangeComboCapture();
};
