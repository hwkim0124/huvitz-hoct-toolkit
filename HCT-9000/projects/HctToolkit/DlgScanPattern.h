#pragma once

#include "ViewCorneaCamera.h"
#include "ViewRetinaCamera.h"
#include "ViewScanPreview.h"
#include "ViewEnfacePreview.h"

#include "DlgFocusControl.h"
#include "DlgCameraControl.h"
#include "DlgMotorControl.h"
#include "DlgLightControl.h"
#include "DlgStageControl.h"

#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <string>

#include "OctSystem2.h"
#include "CppUtil2.h"
#include "CorTopo2.h"
#include "SigProc2.h"
#include "SigChain2.h"

using namespace OctSystem;
using namespace SigProc;
using namespace SigChain;
using namespace CppUtil;
using namespace std;

#include <afxwin.h>
#include <afxcmn.h>

// DlgScanPattern dialog

class DlgScanPattern : public CDialogEx
{
	DECLARE_DYNAMIC(DlgScanPattern)

public:
	DlgScanPattern(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgScanPattern();

	enum AutoMeasureMode {
		None,
		AXIAL_LENGTH, 
		LENS_THICKNESS,
		AL_TEST_1,
		AL_TEST_2,
		LT_TEST_1, 
		LT_TEST_2,
		CCT_TEST_1, 
		CCT_TEST_2,
		ACD_TEST_1,
		ACD_TEST_2,
	};

	float const AL_VALUE1 = 13.9639f; // 14.01747f;
	float const AL_VALUE2 = 39.9901f; // 40.14368f;
	float const CCT_VALUE1 = 0.2281f; // 0.227189f;
	float const CCT_VALUE2 = 1.24717f; // 1.246793f;
	float const ACD_VALUE1 = 1.47629f; // 1.427885f;
	float const ACD_VALUE2 = 6.99448f;// 6.967719f;
	float const LT_VALUE1 = 0.43992f; // 0.438237f;
	float const LT_VALUE2 = 6.6462f;

	struct AutoFocusResult {
		AutoFocusResult(bool result, int refPos, float diopt, wstring path)
			: result_(result), refPos_(refPos), diopt_(diopt), path_(path) {};

		bool result_;
		int refPos_;
		float diopt_;
		wstring path_;
	};

	static constexpr float getRefractiveIndexForBK7() {
		return 1.5102f;
	}

	static constexpr float getRefractiveIndexForB270() {
		return 1.5160f;
	}
	
	static constexpr float getRefractiveIndexForCornea() {
		return 1.3375f;
	}

	static constexpr float getRefractiveIndexForLens() {
		return 1.406f;
	}

	static constexpr float getRefractiveIndexForRetina() {
		return 1.375f;
	}

protected:
	ViewCorneaCamera corneaView;
	ViewRetinaCamera retinaView;
	ViewScanPreview previewView;
	ViewEnfacePreview enfaceView;

	std::unique_ptr<DlgFocusControl> pDlgFocus;
	std::unique_ptr<DlgMotorControl> pDlgMotor;
	std::unique_ptr<DlgLightControl> pDlgLight;
	std::unique_ptr<DlgStageControl> pDlgStage;
	std::unique_ptr<DlgCameraControl> pDlgCamera;

	std::deque<AutoFocusResult> m_corneaResult;
	std::deque<AutoFocusResult> m_retinaResult;
	std::deque<AutoFocusResult> m_lensFrontResult;
	
	std::vector<float> m_axialLengths;
	std::vector<float> m_lensThicknesses;
	std::vector<float> m_acds;
	std::vector<float> m_ccts;
	AutoMeasureMode m_autoMeasureMode = AutoMeasureMode::None;

	float m_offsetX;
	float m_offsetY;
	float m_scaleX;
	float m_scaleY;
	float m_angle;

	int m_patternCode;
	PatternName m_patternName;
	float m_rangeX;
	float m_rangeY;
	int m_points;
	int m_direction;
	int m_overlaps;
	int m_lines;
	float m_space;

	bool m_usePattern;
	bool m_useFaster;
	bool m_useEnface;
	bool m_useNoImageGrab;
	int m_enfacePoints;
	int m_enfaceLines;
	float m_enfaceRangeX;
	float m_enfaceRangeY;

	bool _isScanStarting = false;
	bool _isScanClosing = false;
	bool isMeMode = false;

	ScanSpeed m_speed = ScanSpeed::Fastest;
	OctScanMeasure m_measure;

	ScanPatternCompletedCallback m_clbScanPatternCompleted;
	AutoCorneaFocusCompletedCallback m_clbAutoCorneaFocusCompleted;
	AutoRetinaFocusCompletedCallback m_clbAutoRetinaFocusCompleted;
	AutoLensFrontFocusCompletedCallback m_clbAutoLensFrontFocusCompleted;
	AutoLensBackFocusCompletedCallback m_clbAutoLensBackFocusCompleted;

	JoystickEventCallback cbJoystick;

protected:
	void initControls(void);
	void showIrCameras(bool play);
	void prepareSystem(void);
	void updateScanControls(void);
	void updateEnfaceControls(void);
	void updatePreviewControls(void);
	void setRadioCameraSpeed(UINT value);
	void updateInternalFixation(void);
	void updateScannerResult(bool enable);
	void updateExposureTime(void);

	void showPreviewImageSelected(void);
	void showPatternImageSelected(void);

	void callbackScanPatternCompleted(bool result);
	void callbackCorneaFocusCompleted(bool result, int refPos, float diopt);
	void callbackRetinaFocusCompleted(bool result, int refPos, float diopt);
	void callbackLensFrontFocusCompleted(bool result, int refPos, float diopt);
	void callbackLensBackFocusCompleted(bool result, int refPos, float diopt);

	bool updateAutoMeasureMode(AutoMeasureMode mode);
	bool checkAutoMeasureCompleted(void);
	bool buildAutoMeasureResult(void);
	int getAutoMeasureNumber(void) { return 1; };

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGSCANPATTERN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	CComboBox cmbDirection;
	CComboBox cmbPattern;
	CComboBox cmbPoints;
//	CComboBox cmbRange;
	afx_msg void OnSelchangeComboDirection();
	afx_msg void OnSelchangeComboPattern();
	afx_msg void OnSelchangeComboPoints();
	CComboBox cmbOverlaps;
	CEdit editAngle;
	CEdit editOffsetX;
	CEdit editOffsetY;
	CEdit editScaleX;
	CEdit editScaleY;
	afx_msg void OnBnClickedButtonExit2();
	afx_msg void OnBnClickedButtonStartScan();
//	CComboBox cmbEnfaceLines;
//	CComboBox cmbEnfacePoints;
	CComboBox cmbLines;
	CComboBox cmbSpace;
	CComboBox cmbEnfaceRangeX;
	CComboBox cmbEnfaceRangeY;
	CComboBox cmbRangeX;
	CComboBox cmbRangeY;
	afx_msg void OnBnClickedButtonSaveEnface();
	afx_msg void OnBnClickedCheckEqualization();
	afx_msg void OnBnClickedButtonFocus();
	afx_msg void OnBnClickedButtonMotor();
	afx_msg void OnBnClickedButtonCamera();
	afx_msg void OnBnClickedButtonLeds();
	afx_msg void OnBnClickedButtonAutoFocus();
	afx_msg void OnBnClickedButtonAutoRefer3();
	afx_msg void OnBnClickedButtonAutoPolar2();
	afx_msg void OnBnClickedButtonExposure();
	int m_cameraSpeed;
	afx_msg void OnBnClickedButtonGalvApply();
	afx_msg void OnBnClickedButtonIntfixOn();
	afx_msg void OnCbnSelchangeComboIntfixCol();
	CComboBox cmbInfixCol;
	CComboBox cmbInfixRow;
	afx_msg void OnBnClickedButtonIntfixOff();
	afx_msg void OnBnClickedButtonIntfixUp();
	afx_msg void OnBnClickedButtonIntfixDown();
	afx_msg void OnBnClickedButtonIntfixLeft();
	afx_msg void OnBnClickedButtonIntfixRight();
	afx_msg void OnBnClickedButtonIntfixCenter();
	afx_msg void OnBnClickedRadioCamera2();
	afx_msg void OnBnClickedRadioOd();
	afx_msg void OnBnClickedRadioOs();
	afx_msg void OnBnClickedRadioMacular();
	afx_msg void OnBnClickedRadioDisc();
	CEdit editEnfaceNo;
//	CEdit editPatientName;
	CEdit editPatternNo;
	CEdit editPreviewNo;
	CSliderCtrl sldEnface;
	CSliderCtrl sldPattern;
	CSliderCtrl sldPreview;
//	CEdit editPatientId;
	afx_msg void OnReleasedcaptureSliderPattern(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderPreview(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderEnface(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonSaveImage();
	afx_msg void OnBnClickedButtonPreviewShow();
	afx_msg void OnBnClickedButtonPatternShow();
	afx_msg void OnBnClickedButtonResultExport();
	afx_msg void OnBnClickedButtonOptimize();
	afx_msg void OnCbnSelchangeComboRangex();
	afx_msg void OnBnClickedButtonResultNew();
	afx_msg void OnBnClickedButtonResultSave();
	afx_msg void OnNMCustomdrawSliderEnface(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnBnClickedButtonGalvExport();
	afx_msg void OnBnClickedCheckEnfaceGrid();
	afx_msg void OnBnClickedRadioDioptZero();
	afx_msg void OnBnClickedRadioDioptMinus();
	afx_msg void OnBnClickedRadioDioptPlus();
//	CEdit editEnfaceRage1;
	CEdit editEnfaceRange2;
	CEdit editEnfaceRange1;
	afx_msg void OnBnClickedButtonEnfceApply();
	afx_msg void OnBnClickedButtonEnfceReset();
	afx_msg void OnCbnSelchangeComboEnfacePoints();
	afx_msg void OnCbnSelchangeComboEnfaceRangex();
	afx_msg void OnCbnSelchangeComboEnfaceRangey();
	afx_msg void OnCbnSelchangeComboRangey();
//	CComboBox cmbEnfaceLines;
//	CComboBox cmbEnfacePoints;
	CComboBox cmbEnfaceLines;
	CComboBox cmbEnfacePoints;
	afx_msg void OnCbnSelchangeComboEnfacePoints2();
	afx_msg void OnBnClickedButtonAutoTrack();
	afx_msg void OnBnClickedButtonAutoSplit();
	CEdit editEnfaceClipLimit;
	CEdit editEnfaceScalar;
	afx_msg void OnBnClickedCheckPreviewPattern();
	CEdit editGalvOffsetX;
	CEdit editGalvOffsetY;
	afx_msg void OnBnClickedButtonGalvDynApply();
	afx_msg void OnBnClickedButtonSegment();
	CEdit editRefraction;
	afx_msg void OnBnClickedButtonRetinaRegist();
	afx_msg void OnBnClickedButtonRetinaLeds();
	afx_msg void OnBnClickedCheckAverageSnrs();
	afx_msg void OnBnClickedButtonResultClear();
	CEdit editAverageSize;
	afx_msg void OnBnClickedButtonLensTest1();
	afx_msg void OnBnClickedButtonLensTest2();
	afx_msg void OnBnClickedButtonCctTest1();
	afx_msg void OnBnClickedButtonCctTest2();
	afx_msg void OnBnClickedButtonAcdTest1();
	afx_msg void OnBnClickedButtonAcdTest2();
	afx_msg void OnBnClickedButtonAlTest1();
	afx_msg void OnBnClickedButtonAlTest2();
	afx_msg void OnBnClickedButtonRetinaOrigin();
	afx_msg void OnBnClickedButtonCorneaOrigin();
	afx_msg void OnCbnSelchangeComboIntfixRow();
	afx_msg void OnBnClickedCheckStretch();
	afx_msg void OnBnClickedCheckNoImageGrab();
	afx_msg void OnBnClickedCheckDynamicDispersion();
	CComboBox combLineCamAgain;
	afx_msg void OnSelchangeComboLineCamAgain();
	CEdit editExposTime;
};
