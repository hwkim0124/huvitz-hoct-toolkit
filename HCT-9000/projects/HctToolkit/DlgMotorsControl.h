#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// DlgMotorsControl dialog

class DlgMotorsControl : public CDialogEx
{
	DECLARE_DYNAMIC(DlgMotorsControl)

public:
	DlgMotorsControl(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgMotorsControl();

protected:
	void initControls(void);

	void updateOctFocus(bool edit=true);
	void updateRefer(bool edit = true);
	void updatePolar(bool edit = true);
	void updateFundusFocus(bool edit = true);
	void updateSplitFocus(bool edit = true);
	void updateSample(bool edit = true);
	void updateDioptSel(bool edit = true);
	void updateOctCompen(bool edit = true);
	void updatePupilMask(bool edit = true);


// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGMOTORSCONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	CEdit editDioptSel;
	CEdit editFundusFocus;
//	CEdit editOctCompens;
	CEdit editOctCompen;
	CEdit editOctFocus;
	CEdit editPolar;
	CEdit editRefer;
	CEdit editSample;
	CEdit editSplitFocus;
	CSliderCtrl sldDioptSel;
	CSliderCtrl sldFundusFocus;
	CSliderCtrl sldOctCompen;
	CSliderCtrl sldOctFocus;
	CSliderCtrl sldPolar;
	CSliderCtrl sldRefer;
	CSliderCtrl sldSample;
	CSliderCtrl sldSplitFocus;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnReleasedcaptureSliderDioptSel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderFundusFocus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderOctCompen(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderOctFocus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderPolar(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderRefer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderSample(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderSplitFocus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderPupilMask(NMHDR *pNMHDR, LRESULT *pResult);
	CSliderCtrl sldPupilMask;
	CEdit editPupilMask;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
