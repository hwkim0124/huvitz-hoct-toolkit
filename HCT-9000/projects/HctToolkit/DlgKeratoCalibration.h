#pragma once

#include "DlgFocusControl.h"
#include "DlgCameraControl.h"
#include "DlgMotorControl.h"
#include "DlgLightControl.h"
#include "DlgStageControl.h"

#include "ViewCorneaCamera.h"
#include "ViewRetinaCamera.h"

#include <memory>
#include "afxwin.h"

// DlgKeratoCalibration dialog

using namespace OctConfig;

class DlgKeratoCalibration : public CDialogEx
{
	DECLARE_DYNAMIC(DlgKeratoCalibration)

public:
	DlgKeratoCalibration(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgKeratoCalibration();

protected:
	void initControls(void);
	void controlCameras(bool play);
	void loadSetupData(bool init);
	void saveSetupData(void);
	void importSetupData(void);
	void exportSetupData(void);
	void updateFocusStepData(bool init = false);
	void setRadioIrExposureTime(UINT value);
	void setInternalFixation(UINT value);

protected:
	ViewCorneaCamera corneaView;
	ViewRetinaCamera retinaView;
	bool isPlaying = false;
	bool isDisc = false;
	EyeSide eyeSide = EyeSide::OD;

	std::unique_ptr<DlgFocusControl> pDlgFocus;
	std::unique_ptr<DlgCameraControl> pDlgCamera;
	std::unique_ptr<DlgMotorControl> pDlgMotor;
	std::unique_ptr<DlgLightControl> pDlgLight;
	std::unique_ptr<DlgStageControl> pDlgStage;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGKERATOCALIBRATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonFocus();
	afx_msg void OnBnClickedButtonCamera();
	afx_msg void OnBnClickedButtonMotor();
	afx_msg void OnBnClickedButtonLight();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedCheckCamera();
	afx_msg void OnBnClickedCheckFocus();
	afx_msg void OnBnClickedCheckGuide();
	afx_msg void OnBnClickedCheckKerato();
	afx_msg void OnBnClickedCheckSpots();
	afx_msg void OnBnClickedButtonSaveCornea();
	afx_msg void OnBnClickedButtonSaveRetina();
	afx_msg void OnBnClickedButtonKeratoExit();
	CListBox lstData;
	CComboBox cmbDiopt;
	afx_msg void OnSelchangeComboDiopt();
	afx_msg void OnSelchangeListData();
	afx_msg void OnBnClickedButtonDelData();
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnBnClickedButtonSetData();
	afx_msg void OnBnClickedButtonStage2();
	afx_msg void OnBnClickedButtonLoadData();
	afx_msg void OnBnClickedButtonSaveData();
	afx_msg void OnBnClickedCheckSplitFocus();
	afx_msg void OnBnClickedCheckSplitWindows();
	afx_msg void OnBnClickedCheckSplitPoints();
	afx_msg void OnBnClickedButtonAutoFundus();
	afx_msg void OnBnClickedButtonAutoTrack();
	afx_msg void OnBnClickedCheckRetinaRoiMask();
	afx_msg void OnBnClickedCheckStageX();
	afx_msg void OnBnClickedCheckStageY();
	afx_msg void OnBnClickedCheckStageZ();
	afx_msg void OnBnClickedButtonMoveX();
	CEdit editDeltaX;
	afx_msg void OnBnClickedButtonStopX();
	int retinaCameraSpeed;
	CEdit editDeltaY;
	CEdit editDeltaZ;
	afx_msg void OnBnClickedCheckStageInfo();
	CEdit editRoiX1;
	CEdit editRoiY1;
	CEdit editRoiRadius;
	afx_msg void OnBnClickedButtonRoiApply();
	afx_msg void OnBnClickedCheckStageOverride();
	CEdit editSplitCenterX;
	CEdit editSplitCenterY;
	afx_msg void OnBnClickedButtonSplitApply();
	afx_msg void OnBnClickedCheckWorkingDots();
	afx_msg void OnBnClickedButtonImportData();
	afx_msg void OnBnClickedButtonExportData2();
	CEdit editRetinaMask;
	CEdit editMaskX;
	CEdit editMaskY;
	afx_msg void OnBnClickedCheckEnableRetinaEq();
	CEdit editRetinaClipLimit;
	CEdit editRetinaClipScalar;
	afx_msg void OnBnClickedButtonApplyRetinaEq();
	afx_msg void OnBnClickedRadioIrFps2();
	afx_msg void OnBnClickedRadioIrFps1();
	afx_msg void OnBnClickedButtonResetRetinaEq();
	afx_msg void OnBnClickedButtonSplitUp();
	afx_msg void OnBnClickedButtonSplitDown();
	afx_msg void OnBnClickedButtonCameraAlignOn();
	afx_msg void OnBnClickedButtonCameraAlignOff();
	afx_msg void OnBnClickedButtonRetinaRoiXUp();
	afx_msg void OnBnClickedButtonRetinaRoiXDown();
	afx_msg void OnBnClickedButtonRetinaRoiYUp();
	afx_msg void OnBnClickedButtonRetinaRoiYDown();
	afx_msg void OnBnClickedButtonSaveRoi();
	afx_msg void OnEnChangeEditRoiX1();
	afx_msg void OnEnChangeEditRoiY1();
	afx_msg void OnBnClickedButtonRetinaAutoAdjust();
	afx_msg void OnBnClickedButtonTrackRegist();
	afx_msg void OnBnClickedButtonTrackPrepare();
	afx_msg void OnBnClickedButtonTrackRetina();
};
