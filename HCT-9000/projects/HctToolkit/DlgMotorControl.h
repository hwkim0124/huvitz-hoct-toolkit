#pragma once
#include "afxcmn.h"
#include "afxwin.h"


#include "OctSystem2.h"


// DlgMotorControl dialog

class DlgMotorControl : public CDialogEx
{
	DECLARE_DYNAMIC(DlgMotorControl)

public:
	DlgMotorControl(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgMotorControl();

protected:
	void initControls(void);

protected:
	OctDevice::OctSampleMotor* sample;
	OctDevice::FundusDiopterMotor* funDiopt;
	OctDevice::SplitFocusMotor* split;
	OctDevice::PupilMaskMotor* pupil;
	OctDevice::OctDiopterMotor* octDiopt;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGMOTORCONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	CSliderCtrl sldCompen;
	CSliderCtrl sldDiopt;
	CSliderCtrl sldMirror;
	CSliderCtrl sldPupil;
	CSliderCtrl sldSplit;
	CEdit editCompen;
	CEdit editDiopt;
	CEdit editMirror;
	CEdit editPupil;
	CEdit editSplit;
	CComboBox cmbDiopt;
	CComboBox cmbMirror;
	CComboBox cmbPupil;
	afx_msg void OnReleasedcaptureSliderMirror(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderSplit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderPupil(NMHDR *pNMHDR, LRESULT *pResult);
	// afx_msg void OnReleasedcaptureSliderCompen(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderDiopt(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonMirrorIn();
	afx_msg void OnBnClickedButtonMirrorOut();
	afx_msg void OnBnClickedButtonMirrorSet();
	afx_msg void OnBnClickedButtonDioptMinus();
	afx_msg void OnBnClickedButtonDioptZero();
	afx_msg void OnBnClickedButtonDioptPlus();
	afx_msg void OnBnClickedButtonDioptSet();
	afx_msg void OnBnClickedButtonSplitStep1();
	afx_msg void OnBnClickedButtonSplitStep2();
	// afx_msg void OnBnClickedButtonSplitZero();
	// afx_msg void OnBnClickedButtonSplitOrigin();
	afx_msg void OnBnClickedButtonPupilNormal();
	afx_msg void OnBnClickedButtonPupilSmall();
	afx_msg void OnBnClickedButtonPupilSet();
	CComboBox cmbSplit;
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonSplitSet();
	CComboBox cmbDiopt2;
	CEdit editDiopt2;
	CSliderCtrl sldDiopt2;
	afx_msg void OnBnClickedButtonDioptSet2();
	afx_msg void OnBnClickedButtonDioptMinus2();
	afx_msg void OnBnClickedButtonDioptZero2();
	afx_msg void OnBnClickedButtonDioptPlus2();
	afx_msg void OnReleasedcaptureSliderDiopt2(NMHDR *pNMHDR, LRESULT *pResult);
	void blackSpotAlign();
};
