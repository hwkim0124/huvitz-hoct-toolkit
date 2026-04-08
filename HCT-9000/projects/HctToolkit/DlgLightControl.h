#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "OctSystem2.h"

// DlgLightControl dialog

class DlgLightControl : public CDialogEx
{
	DECLARE_DYNAMIC(DlgLightControl)

public:
	DlgLightControl(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgLightControl();

protected:
	void initControls(void);
	void linkFlashToPannel(void);

protected:
	OctDevice::FlashLed* flash;
	OctDevice::LightLed* pannel;
	OctDevice::LightLed* wDots;
	OctDevice::LightLed* wDot2;
	OctDevice::LightLed* split;
	OctDevice::OctSldLed* octSld;
	OctDevice::LightLed* extFix;
	OctDevice::LightLed* intFix;


// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGLIGHTCONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	CComboBox cmbColumn;
	CComboBox cmbRow;
	CEdit editExtFix;
	CEdit editFlash;
	// CEdit editPannel;
	CEdit editSld;
	CEdit editSplit;
	CEdit editWdots;
	CSliderCtrl sldExtFix;
	CSliderCtrl sldFlash;
	//CSliderCtrl sldPannel;
	CSliderCtrl sldSld;
	CSliderCtrl sldSplit;
	CSliderCtrl sldWdots;
	afx_msg void OnReleasedcaptureSliderFlash(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnReleasedcaptureSliderPannel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderWdots(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderSplit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderSld(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderExtfix(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonFlashStep1();
	afx_msg void OnBnClickedButtonFlashStep2();
	//afx_msg void OnBnClickedButtonPannelStep1();
	//afx_msg void OnBnClickedButtonPannelStep2();
	afx_msg void OnBnClickedButtonWdotsStep1();
	afx_msg void OnBnClickedButtonWdotsStep2();
	afx_msg void OnBnClickedButtonSplitStep1();
	afx_msg void OnBnClickedButtonSplitStep2();
	afx_msg void OnBnClickedButtonSldOn();
	afx_msg void OnBnClickedButtonSldOff();
	afx_msg void OnBnClickedButtonExtfixStep1();
	afx_msg void OnBnClickedButtonExtfixStep2();
	afx_msg void OnBnClickedButtonIntfixOn();
	afx_msg void OnBnClickedButtonIntfixOff();
	CEdit editHighCode;
	CEdit editLowCode1;
	CEdit editLowCode2;
	CEdit editRsiCode;
	afx_msg void OnBnClickedButtonSldUpdate();
	afx_msg void OnBnClickedButtonSldLoad();
	afx_msg void OnBnClickedCheckStrobeCont();
	afx_msg void OnBnClickedButtonLcdOn();
	afx_msg void OnBnClickedButtonLcdOff();
	afx_msg void OnBnClickedButtonLcdParam();
	afx_msg void OnEnChangeEditImon();
	afx_msg void OnBnClickedButtonWdots2Step1();
	afx_msg void OnBnClickedButtonWdots2Step2();
	CEdit editWdots2;
	CSliderCtrl sldWdots2;
	afx_msg void OnReleasedcaptureSliderWdots2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonSldCalc();
	void blackSpotAlign();
	void blackSpotCapture();
	void ledAlign();
	void setHighCode100();
	void resetHighCode();
	afx_msg void OnBnClickedButtonLcdfixRowStep1();
	afx_msg void OnBnClickedButtonLcdfixRowStep2();
	afx_msg void OnBnClickedButtonLcdfixColStep1();
	afx_msg void OnBnClickedButtonLcdfixColStep2();
	afx_msg void OnBnClickedButtonFlashUpdate();
	CEdit editLcdRow;
	CEdit editLcdCol;
};
