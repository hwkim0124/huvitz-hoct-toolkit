#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "OctSystem2.h"

// DlgCameraControl dialog

class DlgCameraControl : public CDialogEx
{
	DECLARE_DYNAMIC(DlgCameraControl)

public:
	DlgCameraControl(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgCameraControl();

protected:
	void initControls(void);
	void updateCorneaAgain(bool reset);
	void updateCorneaDgain(bool reset);
	void updateRetinaAgain(bool reset);
	void updateRetinaDgain(bool reset);


protected:
	OctDevice::LightLed* retinaIr;
	OctDevice::LightLed* corneaIr;
	OctDevice::LightLed* kerRing;
	OctDevice::LightLed* kerFocus;

	OctDevice::CorneaCamera* corneaCam;
	OctDevice::RetinaCamera* retinaCam;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGCAMERACONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	CEdit editCornea;
	CEdit editCorneaDgain;
	CEdit editFocus;
	CEdit editRetina;
	CEdit editRetinaDgain;
	CEdit editRing;
	CSliderCtrl sldCornea;
	CSliderCtrl sldFocus;
	CSliderCtrl sldRetina;
	CSliderCtrl sldRing;
	afx_msg void OnReleasedcaptureSliderCornea(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderFocus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderRetina(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderRing(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonCorneaOn();
	afx_msg void OnBnClickedButtonCorneaOff();
	afx_msg void OnBnClickedButtonRetinaOn();
	afx_msg void OnBnClickedButtonRetinaOff();
	afx_msg void OnBnClickedButtonRingStep1();
	afx_msg void OnBnClickedButtonRingStep2();
	afx_msg void OnBnClickedButtonFocusStep1();
	afx_msg void OnBnClickedButtonFocusStep2();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	CComboBox cmbCornea;
	CComboBox cmbRetina;
	afx_msg void OnSelchangeComboCorneaAgain();
	afx_msg void OnSelchangeComboRetinaAgain();
	afx_msg void OnBnClickedButtonCorneaChOn();
	afx_msg void OnBnClickedButtonCorneaChOff();
	afx_msg void OnBnClickedButtonRetinaChOn();
	afx_msg void OnBnClickedButtonRetinaChOff();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonSave();
};
