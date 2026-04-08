#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "ViewCorneaCamera.h"
#include "ViewRetinaCamera.h"

#include "ViewScanProfile.h"

#include "DlgFocusControl.h"
#include "DlgCameraControl.h"
#include "DlgMotorControl.h"
#include "DlgLightControl.h"

#include <memory>
#include <string>

#include "OctSystem2.h"
#include "OctConfig2.h"
#include "SigProc2.h"
#include "SigChain2.h"
#include "CppUtil2.h"

using namespace OctSystem;
using namespace OctConfig;
using namespace SigProc;
using namespace SigChain;
using namespace CppUtil;
using namespace std;



// DlgSpectrometerCalibration dialog

class DlgSpectrometerCalibration : public CDialogEx
{
	DECLARE_DYNAMIC(DlgSpectrometerCalibration)

public:
	DlgSpectrometerCalibration(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgSpectrometerCalibration();

protected:
	ViewCorneaCamera corneaView;
	ViewRetinaCamera retinaView;
	ViewScanProfile profileView;
	ViewScanProfile profileHeadView;
	ViewScanProfile profileTailView;
	SpectroCalibrator spectCalib;

	std::unique_ptr<DlgFocusControl> pDlgFocus;
	std::unique_ptr<DlgCameraControl> pDlgCamera;
	std::unique_ptr<DlgMotorControl> pDlgMotor;
	std::unique_ptr<DlgLightControl> pDlgLight;

	Resampler resampler;

	ScanSpeed m_scanSpeed = ScanSpeed::Fastest;
	int m_blackOffset = 1;
	int m_coeffsOrder = 4;

	bool runReferenceLoop = false;
	bool isEndOfReference = false;
	bool isMovingToUpper = false;

	bool runPolarizationLoop = false;
	bool isEndOfPolarization = false;
	bool isMovingToUpperPolar = false;

	void initControls(void);	
	void alignControls(void); 
	void controlCameras(bool play);

	void updateThreshPeak(bool redraw);
	void updateThreshSubs(bool redraw);
	void updateProfiles(void);
	void updateExposureTime(void);

	void updateSpectroParams(double* params);
	void clearSpectroParams(void);

	void updateGraphDisplay(bool reset);
	void updateGraphEndsDisplay(bool reset);
	void updateRadioCameraSpeed(UINT value);
	void updateRadioBlackOffset(UINT value);

	void callbackProfileImage(unsigned short* data, unsigned int width, unsigned int height);
	SpectrumDataCallback callback;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGSPECTROMETERCALIBRATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton btnCalibrate;
	CButton btnPeakAdd;
	CButton btnPeakDel;
	CButton btnStartScan;
	CButton btnWaveAdd;
	CButton btnWaveDel;
	CButton btnWaveReset;
	CComboBox cmbClass;
//	CEdit editParam;
	CEdit editThresh;
	CEdit editThresh2;
	CListBox lstFbgs;
	CListBox lstPeaks;
	CListBox lstWaves;
	CSliderCtrl sldThresh;
	CSliderCtrl sldThresh2;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnReleasedcaptureSliderThresh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderThresh2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSelchangeComboClass();
	afx_msg void OnClickedButtonCalibrate();
	afx_msg void OnClickedButtonPeakAdd();
	afx_msg void OnClickedButtonPeakDel();
	afx_msg void OnClickedButtonStartScan();
	afx_msg void OnClickedButtonWaveAdd();
	afx_msg void OnClickedButtonWaveDel();
	afx_msg void OnClickedButtonWaveReset();
	CButton btnProfileAdd;
	CButton btnProfileDel;
	CButton btnProfileReset;
	CComboBox cmbProfile;
	CListBox lstProfiles;
	afx_msg void OnClickedButtonProfileAdd();
	afx_msg void OnClickedButtonProfileDel();
	afx_msg void OnClickedButtonProfileReset();
	afx_msg void OnBnClickedButtonFocus();
	afx_msg void OnBnClickedButtonCamera2();
	afx_msg void OnBnClickedButtonMotor2();
	afx_msg void OnBnClickedButtonLeds2();
	afx_msg void OnBnClickedButtonSaveImage();
	afx_msg void OnBnClickedButtonExit2();
	CEdit editPeak;
	CEdit editWave;
	afx_msg void OnBnClickedButtonFindPeaks();
	CEdit editParam1;
	CEdit editParam2;
	CEdit editParam3;
	CEdit editParam4;
	afx_msg void OnBnClickedButtonLoadParams();
	afx_msg void OnBnClickedButtonSaveParams();
	afx_msg void OnBnClickedButtonDrawLines();
	afx_msg void OnBnClickedButtonClearLines();
	CEdit editExposure;
	afx_msg void OnBnClickedButtonExposureApply();
	CEdit editRangeX1;
	CEdit editRangeX2;
	CEdit editRangeY1;
	CEdit editRangeY2;
	afx_msg void OnBnClickedCheckShowText();
	afx_msg void OnBnClickedCheckRecordValues();
	afx_msg void OnBnClickedButtonRangeReset();
	afx_msg void OnBnClickedButtonRangeSize();
	CEdit editBlackOffset;
	afx_msg void OnBnClickedButtonOffsetWrite();
	afx_msg void OnBnClickedButtonOffsetSave();
	afx_msg void OnBnClickedButtonOffsetRead();
	afx_msg void OnBnClickedButtonSld();
	afx_msg void OnBnClickedButtonReferLoop();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CEdit editCenterX;
	afx_msg void OnBnClickedRadioOrder1();
	afx_msg void OnBnClickedRadioOrder2();
	afx_msg void OnBnClickedButtonPolarLoop();
	afx_msg void OnBnClickedButtonCalReset();
	afx_msg void OnBnClickedButtonEndsReset();
	CEdit editHeadX1;
	CEdit editHeadX2;
	CEdit editHeadY1;
	CEdit editHeadY2;
	CEdit editTailX1;
	CEdit editTailX2;
	CEdit editTailY1;
	CEdit editTailY2;
	CComboBox cmbLineCamAgain;
	afx_msg void OnSelchangeComboLineCamAgain();
	afx_msg void OnBnClickedButtonHighCode();
};
