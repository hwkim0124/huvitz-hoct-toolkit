#pragma once

#include "DlgFocusControl.h"
#include "DlgCameraControl.h"
#include "DlgMotorControl.h"
#include "DlgLightControl.h"

#include "ViewCorneaCamera.h"
#include "ViewRetinaCamera.h"
#include "ViewScanPreview.h"

#include "afxwin.h"
#include "afxcmn.h"

#include <memory>

// DlgOctScanTest dialog

class DlgOctScanTest : public CDialogEx
{
	DECLARE_DYNAMIC(DlgOctScanTest)

public:
	DlgOctScanTest(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgOctScanTest();

protected:
	ViewCorneaCamera corneaView;
	ViewRetinaCamera retinaView;
	ViewScanPreview previewView;

	std::unique_ptr<DlgFocusControl> pDlgFocus;
	std::unique_ptr<DlgCameraControl> pDlgCamera;
	std::unique_ptr<DlgMotorControl> pDlgMotor;
	std::unique_ptr<DlgLightControl> pDlgLight;

	void initControls(void);
	void controlCameras(bool play);


// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGOCTSCANTEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	CComboBox cmbPattern;
	CComboBox cmbPoints;
	CComboBox cmbRange;
	afx_msg void OnClickedButtonStartScan();
	CButton btnStartScan;
	afx_msg void OnBnClickedButtonFocus();
	afx_msg void OnBnClickedButtonCamera();
	afx_msg void OnBnClickedButtonMotor();
	afx_msg void OnBnClickedButtonLight();
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnBnClickedButtonSaveImage();
	afx_msg void OnBnClickedButtonMeasure();
};
