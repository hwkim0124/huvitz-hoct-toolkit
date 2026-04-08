#pragma once

#include "IrCameraView.h"
#include "afxwin.h"

// DlgIrCameraTest dialog
#include "DlgLedsControl.h"
#include "DlgMotorsControl.h"

#include <memory>


class DlgIrCameraTest : public CDialogEx
{
	DECLARE_DYNAMIC(DlgIrCameraTest)

public:
	DlgIrCameraTest(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgIrCameraTest();

protected:
	IrCameraView corneaView;
	IrCameraView retinaView;

	std::unique_ptr<DlgLedsControl> pDlgLedsCtrl;
	std::unique_ptr<DlgMotorsControl> pDlgMotorsCtrl;
	

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGIRCAMERATEST };
#endif


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
//	CEdit m_editStatus;
	CButton btnStartCornea;
	CButton btnStartRetina;
	CButton chkOverlayCornea;
	CButton chkOverlayRetina;
	afx_msg void OnClickedButtonStartCornea();
	afx_msg void OnClickedButtonStartRetina();
	afx_msg void OnClickedCheckOverlayCornea();
	afx_msg void OnClickedCheckOverlayRetina();
	afx_msg void OnClickedButtonLeds();
	afx_msg void OnClickedButtonMotors();
	afx_msg void OnClickedButtonLoadConfig();
	afx_msg void OnClickedButtonSaveConfig();
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
