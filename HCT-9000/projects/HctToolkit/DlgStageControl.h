#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "OctSystem2.h"

// DlgStageControl dialog

class DlgStageControl : public CDialogEx
{
	DECLARE_DYNAMIC(DlgStageControl)

public:
	DlgStageControl(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgStageControl();

protected:
	void initControls(void);

protected:
	OctDevice::StageXMotor* xMotor;
	OctDevice::StageYMotor* yMotor;
	OctDevice::StageZMotor* zMotor;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGSTAGECONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit editStageX;
	CEdit editStageY;
	CEdit editStageZ;
	CSliderCtrl sldStageX;
	CSliderCtrl sldStageY;
	CSliderCtrl sldStageZ;
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnReleasedcaptureSliderStageX(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderStageY(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderStageZ(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonXStep1();
	afx_msg void OnBnClickedButtonXStep2();
	afx_msg void OnBnClickedButtonXStep3();
	afx_msg void OnBnClickedButtonXStep4();
	afx_msg void OnBnClickedButtonYStep1();
	afx_msg void OnBnClickedButtonYStep2();
	afx_msg void OnBnClickedButtonYStep3();
	afx_msg void OnBnClickedButtonYStep4();
	afx_msg void OnBnClickedButtonYOrigin();
	afx_msg void OnBnClickedButtonZStep1();
	afx_msg void OnBnClickedButtonZStep2();
	afx_msg void OnBnClickedButtonZStep3();
	afx_msg void OnBnClickedButtonZStep4();
	afx_msg void OnBnClickedButtonZOrigin();
	afx_msg void OnBnClickedButtonXOrigin();
};
