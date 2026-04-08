#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// DlgLedsControl dialog

class DlgLedsControl : public CDialogEx
{
	DECLARE_DYNAMIC(DlgLedsControl)

public:
	DlgLedsControl(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgLedsControl();

protected:
	void initControls(void);

	void updateCorneaIr(void);
	void updateKerFocus(void);
	void updateKerRing(void);
	void updateRetinaIr(void);
	void updateSplitFocus(void);
	void updateWorkingDots(void);
	void updateOctSld(void);
	void updateExtFixation(void);
	void updateFundusFlash(void);


// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGLEDSCONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();

	virtual BOOL OnInitDialog();
	CSliderCtrl sldExtFix;
	CSliderCtrl sldFundusFlash;
	CSliderCtrl sldKerFocus;
	CSliderCtrl sldKerRing;
	CSliderCtrl sldOctSld;
	CSliderCtrl sldRetinaIR;
	CSliderCtrl sldSplitFocus;
	CSliderCtrl sldWdots;
	CSliderCtrl sldCorneaIR;
	CEdit editCorneaIR;
	CEdit editExtFix;
	CEdit editFundusFlash;
	CEdit editKerFocus;
	CEdit editKerRing;
	CEdit editOctSld;
	CEdit editRetinaIR;
	CEdit editSplitFocus;
	CEdit editWdots;
	afx_msg void OnReleasedcaptureSliderCorneaIr(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderExtFix(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderFundusFlash(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderKerFocus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderKerRing(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderOctSld(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderRetinaIr(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderSplitFocus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderWdots(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
