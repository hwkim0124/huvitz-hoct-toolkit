#pragma once

#include "ViewCorneaCamera.h"
#include "ViewRetinaCamera.h"
#include "ViewScanPreview.h"
#include "ViewEnfacePreview.h"
#include "ViewColorCamera.h"

#include "DlgFocusControl.h"
#include "DlgCameraControl.h"
#include "DlgMotorControl.h"
#include "DlgLightControl.h"
#include "DlgStageControl.h"

#include <memory>
#include <string>
#include <vector>

#include "OctSystem2.h"
#include "afxwin.h"
#include "afxcmn.h"

using namespace OctSystem;


// DlgLongRunTest dialog

class DlgLongRunTest : public CDialogEx
{
	DECLARE_DYNAMIC(DlgLongRunTest)

public:
	DlgLongRunTest(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgLongRunTest();

protected:
	ViewCorneaCamera corneaView;
	ViewRetinaCamera retinaView;
	ViewScanPreview previewView;
	ViewEnfacePreview enfaceView;
	ViewColorCamera colorView;

	std::unique_ptr<DlgFocusControl> pDlgFocus;
	std::unique_ptr<DlgCameraControl> pDlgCamera;
	std::unique_ptr<DlgMotorControl> pDlgMotor;
	std::unique_ptr<DlgLightControl> pDlgLight;
	std::unique_ptr<DlgStageControl> pDlgStage;

	int m_startPosX;
	int m_startPosY;
	int m_startPosZ;

	int m_longRunStep;
	int m_stopAfter;
	int m_currCount;
	CTime m_startTime;
	bool m_longRunTest;

	bool m_useScanning = true;
	bool m_useOptimizing = true;
	bool m_useColorFundus = true;
	bool m_useStageX = true;
	bool m_useStageY = true;
	bool m_useStageZ = true;
	bool m_useChinrest = true;

protected:
	void initControls(void);
	void showIrCameras(bool play);
	void prepareSystem(void);
	void pauseSystem(void);
	void showStatus(CString text);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGLONGRUNTEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
//	CEdit editCurrCount;
	CEdit editStopAfter;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CEdit editStatus;
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonPause();
	CEdit editDelayTime;
	afx_msg void OnBnClickedCheckStageX();
	afx_msg void OnBnClickedCheckStageY();
	afx_msg void OnBnClickedCheckStageZ();
	afx_msg void OnBnClickedCheckScanning();
	afx_msg void OnBnClickedCheckOptimizing();
	afx_msg void OnBnClickedCheckColorFundus();
	afx_msg void OnBnClickedButtonFocus();
	afx_msg void OnBnClickedButtonMotor();
	afx_msg void OnBnClickedButtonStage2();
	afx_msg void OnBnClickedButtonLight();
	afx_msg void OnBnClickedButtonCamera();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedCheckChinrest();
};
