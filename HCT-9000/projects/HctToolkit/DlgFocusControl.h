#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "OctSystem2.h"


// DlgFocusControl dialog

class DlgFocusControl : public CDialogEx
{
	DECLARE_DYNAMIC(DlgFocusControl)

public:
	DlgFocusControl(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgFocusControl();

protected:
	void initControls(void);
	void resetControls(void);

protected:
	OctDevice::OctFocusMotor* focusMotor;
	OctDevice::PolarizationMotor* polarMotor;
	OctDevice::ReferenceMotor* referMotor;
	OctDevice::FundusFocusMotor* fundusMotor;

	bool useRefCornea;
	bool useFocusTopo;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGFOCUSCONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void MoveReferenceToCorneaOrigin();
	void initControlsWithoutShowWindow();
	void isTopography(bool isTopo);

	afx_msg void OnClose();
	afx_msg void OnReleasedcaptureSliderRefer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderPolar(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderFundus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderFocus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonFocusStep1();
	afx_msg void OnBnClickedButtonFocusStep2();
	afx_msg void OnBnClickedButtonFocusStep3();
	afx_msg void OnBnClickedButtonFocusStep4();
	afx_msg void OnBnClickedButtonFocusZero();
	afx_msg void OnBnClickedButtonFocusOrigin();
	afx_msg void OnBnClickedButtonPolarStep1();
	afx_msg void OnBnClickedButtonPolarStep2();
	afx_msg void OnBnClickedButtonPolarStep3();
	afx_msg void OnBnClickedButtonPolarStep4();
	afx_msg void OnBnClickedButtonPolarZero();
	afx_msg void OnBnClickedButtonPolarOrigin();
	afx_msg void OnBnClickedButtonFundusStep1();
	afx_msg void OnBnClickedButtonFundusStep2();
	afx_msg void OnBnClickedButtonFundusStep3();
	afx_msg void OnBnClickedButtonFundusStep4();
	afx_msg void OnBnClickedButtonFundusZero();
	afx_msg void OnBnClickedButtonFundusOrigin();
	CSliderCtrl sldFocus;
	CSliderCtrl sldFundus;
	CSliderCtrl sldPolar;
	CSliderCtrl sldRefer;
	CEdit editFocus;
	CEdit editFocusDiopt;
	CEdit editFundus;
	CEdit editPolar;
	CEdit editPolarDegree;
	CEdit editFundusDiopt;
	CEdit editRefer;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedButtonReferOrigin();
	afx_msg void OnBnClickedCheckRefCornea();
	afx_msg void OnBnClickedButtonReferZero();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnEnChangeEditRefer();
	afx_msg void OnBnClickedButtonReferStep1();
	afx_msg void OnBnClickedButtonReferStep2();
	afx_msg void OnBnClickedCheckFocusTopo();
	afx_msg void OnBnClickedButtonSaveBoard();
};
