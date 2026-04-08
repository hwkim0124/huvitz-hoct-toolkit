#pragma once

#include "ViewCorneaCamera.h"

#include "ViewScanPreview.h"
#include "ViewScanSpectrum.h"
#include "ViewScanSignal.h"

#include "DlgFocusControl.h"
#include "DlgCameraControl.h"
#include "DlgMotorControl.h"
#include "DlgLightControl.h"
#include "DlgStageControl.h"

#include <memory>
#include <string>

#include "OctSystem2.h"
#include "SigProc2.h"
#include "SigChain2.h"
#include "CppUtil2.h"
#include "afxwin.h"

using namespace OctSystem;
using namespace SigProc;
using namespace SigChain;
using namespace CppUtil;
using namespace std;

// DlgSignalTester dialog


class DlgSignalTester : public CDialogEx
{
	DECLARE_DYNAMIC(DlgSignalTester)

public:
	DlgSignalTester(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgSignalTester();

protected:
	ViewCorneaCamera corneaView;
	ViewScanPreview previewView;
	ViewScanSpectrum spectrumView;
	ViewScanSignal signalView;

	std::unique_ptr<DlgFocusControl> pDlgFocus;
	std::unique_ptr<DlgMotorControl> pDlgMotor;
	std::unique_ptr<DlgStageControl> pDlgStage;
	std::unique_ptr<DlgLightControl> pDlgLight;
	std::unique_ptr<DlgCameraControl> pDlgCamera;

	OctDevice::ReferenceMotor* referMotor;
	OctDevice::OctFocusMotor* focusMotor;
	OctDevice::PolarizationMotor* polarMotor;

	PatternType scanPatternType;
	PatternName _patternName;
	ScanSpeed m_scanSpeed = ScanSpeed::Fastest;

	clock_t startTime, endTime;

	int _direction;
	int _radioCameraSpeed = 0;
	int scanLines;
	int scanPoints;
	int peakIndex;
	float peakMax;
	float scanRange;
	float scanOffsetX;
	float scanOffsetY;
	float scanAngle;
	double dispParams[3] = { 0 };
	double specParams[4] = { 0 };
	
	bool runReferenceLoop = false;
	bool runAutoTestLoop = false;
	bool isMovingToUpper = false;

	bool runPolarizationLoop = false;
	bool isEndOfPolarization = false;
	bool isMovingToUpperPolar = false;

	bool runFindPeakPolar = false;

	bool useNoImageGrab = false;
	bool useCorneaDisp = false;
	bool isZeroPadding;
	int multipleOfZeros;
	int autoTestStep;

	bool runFindPeakRef = false;
	int autoFindRefStep;

	void initControls(void);
	void controlCameras(bool play);
	void updateGraphDisplay(bool init);
	void updateCaptureControl(bool init, bool reset=false);
	void updateScanControl(bool init);
	void updateDispersionParams(bool init, bool zero=false);
	void updateSpectrometerParams(bool init, bool zero=false);

	void updateCapturePosition(void);
	void updateRadioCameraSpeed(UINT value);
	void updateExposureTime(void);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGSIGNALTESTER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	CComboBox cmbPeakMax;
	CComboBox cmbPeakMin;
	afx_msg void OnSelchangeComboPeakMax();
	afx_msg void OnSelchangeComboPeakMin();
	CComboBox cmbControl;
	CEdit editPosition;
	CEdit editStart;
//	CEdit editStep;
	CComboBox cmbCount;
	CComboBox cmbCStep;
	CComboBox cmbPattern;
	CComboBox cmbRange;
	CComboBox cmbPoints;
	//CEdit editOffsetX;
	//CEdit editOffsetY;
	CEdit editAngle;
	CEdit editAlpha2;
	CEdit editAlpha3;
	CEdit editParam1;
	CEdit editParam3;
	CEdit editParam2;
	CEdit editParam4;

	afx_msg void OnBnClickedCheckIntensity();
	afx_msg void OnBnClickedCheckFwhm();
	afx_msg void OnBnClickedCheckSnr();
	afx_msg void OnCbnSelchangeComboControl();
	afx_msg void OnBnClickedButtonDispInit();
	afx_msg void OnBnClickedButtonDispZero();
	afx_msg void OnBnClickedButtonParamInit();
	afx_msg void OnBnClickedButtonParamZero();
	afx_msg void OnBnClickedCheckBackground2();
	afx_msg void OnBnClickedCheckReduction2();
	afx_msg void OnBnClickedCheckDispersion2();
	afx_msg void OnBnClickedCheckGrayscale2();
	afx_msg void OnBnClickedCheckAlignGuide();
	afx_msg void OnBnClickedButtonBackground();
	afx_msg void OnBnClickedButtonStartScan2();
	afx_msg void OnBnClickedButtonExit2();
	afx_msg void OnBnClickedButtonCapture();

//	CEdit editRange1;
//	CEdit editRange2;
//	CEdit editPeakRange1;
//	CEdit editPeakRange2;
	CEdit editDataRange1;
	CEdit editDataRange2;
	CEdit editPeakOffset1;
	CEdit editPeakOffset2;
	afx_msg void OnBnClickedButtonFocus();
	afx_msg void OnBnClickedButtonMotor();
	afx_msg void OnBnClickedButtonStage3();
	afx_msg void OnBnClickedButtonAutoFocus();
	afx_msg void OnBnClickedButtonAutoRefer3();
	afx_msg void OnBnClickedButtonAutoPolar2();
	CComboBox cmbZero;
	afx_msg void OnSelchangeComboZero();
	CEdit editAlpha4;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButtonCamera();
	afx_msg void OnBnClickedButtonLeds();
	CEdit editScaleX;
	CEdit editScaleY;
	afx_msg void OnBnClickedCheckCorneaDisp2();
	afx_msg void OnBnClickedCheckNoImageGrab();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CEdit editAverageSize;
	CEdit editNoiseLevel;
	afx_msg void OnBnClickedButtonAutoTest();
	afx_msg void OnBnClickedCheckFftWindow();
	afx_msg void OnBnClickedButtonPolarLoop();
	void OnBnClickedButtonPolarPeak();
	afx_msg void OnBnClickedCheckResample();
	CComboBox cmbOffsetX;
	CComboBox cmbOffsetY;
	afx_msg void OnBnClickedButtonAutoRef26();
	afx_msg void OnBnClickedButtonAutoRef51();
	afx_msg void OnBnClickedButtonAutoRef381();
	afx_msg void OnBnClickedButtonAutoRef751();
//	CComboBox £ã£í£â£Í£ï£ä£å£ì;
	CComboBox cmbModel;
	CEdit editExposTime;
	afx_msg void OnSelchangeComboModel();
	afx_msg void OnBnClickedButtonSetExposTime();
	afx_msg void OnBnClickedCheckWindowing();
	CEdit editWindowAlpha;
};
