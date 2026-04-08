#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "ViewCorneaCamera.h"
#include "ViewRetinaCamera.h"
#include "ViewScanPreview.h"
#include "ViewEnfacePreview.h"
#include "ViewScanIntensity.h"
#include "ViewAngioPreview.h"

#include "DlgFocusControl.h"
#include "DlgCameraControl.h"
#include "DlgMotorControl.h"
#include "DlgLightControl.h"
#include "DlgStageControl.h"

#include <memory>
#include <string>
#include <vector>

#include "OctSystem2.h"
#include "SigChain2.h"
#include "SigProc2.h"
#include "afxwin.h"
#include "afxcmn.h"

using namespace OctSystem;
using namespace SigChain;
using namespace SigProc;


// DlgAngioPattern dialog

class DlgAngioPattern : public CDialogEx
{
	DECLARE_DYNAMIC(DlgAngioPattern)

public:
	DlgAngioPattern(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgAngioPattern();

protected:
	ViewCorneaCamera corneaView;
	ViewRetinaCamera retinaView;
	ViewRetinaCamera registView;
	ViewScanPreview previewView;
	ViewEnfacePreview enfaceView;
	ViewAngioPreview angioView;

	std::unique_ptr<DlgFocusControl> pDlgFocus;
	std::unique_ptr<DlgMotorControl> pDlgMotor;
	std::unique_ptr<DlgLightControl> pDlgLight;
	std::unique_ptr<DlgStageControl> pDlgStage;
	std::unique_ptr<DlgCameraControl> pDlgCamera;

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

	bool m_segmented = false;
	bool m_prepared = false;

	int m_lineSelectedIdx = -1;
	bool m_viewAmplitudes = true;
	bool m_openAngioData2 = false;
	bool m_dataAligned = false;

	ScanSpeed m_speed = ScanSpeed::Fastest;
	OctScanMeasure m_measure;
	EyeSide m_eyeSide = EyeSide::OD;
	bool m_isDisc = false;

	ScanPatternCompletedCallback m_clbScanPatternCompleted;
	JoystickEventCallback cbJoystick;

	bool m_angioVertical;
	bool m_angioLayerSel;
	bool m_angioAlignAxial;
	bool m_angioAlignLateral;
	bool m_angioEnhance;
	bool m_useVascularLayers;
	bool m_usePostProcessing;
	bool m_useMotionCorrect;
	bool m_useNormProjection;
	bool m_useDecorrProjection;
	bool m_useDifferProjection;
	bool m_useProjectRemoval;
	bool m_useReflectCorrect;
	bool m_useBiasFieldCorrect;

	float m_angioDecorThresh;
	float m_angioDecorThresh2;
	float m_angioDifferThresh;
	float m_angioDifferThresh2;
	float m_angioContrastEq;

	float m_angioFoveaCx;
	float m_angioFoveaCy;
	float m_angioLayerOffset1;
	float m_angioLayerOffset2;

	int m_testNumAvgLines;
	int m_testNumTopLines;
	float m_testDecorrThresh;

	int m_angioLines;
	int m_angioPoints;
	int m_angioOverlaps;
	std::string m_angioLayerUpper;
	std::string m_angioLayerLower;
	OcularLayerType m_upperLayerType;
	OcularLayerType m_lowerLayerType;

	OcularLayerType _vascUpperLayer;
	OcularLayerType _vascLowerLayer;
	OcularLayerType _maskUpperLayer;
	OcularLayerType _maskLowerLayer;

	int _avgOffsetInnFlows = 0;
	int _avgOffsetOutFlows = 0;
	float _thresholdInnFlows = 1.0f;
	float _thresholdOutFlows = 1.0f;

	int _vascAvgOffset = 9;
	int _maskAvgOffset = 9;
	float _maskWeight = 1.0f;
	int _projAvgOffset1 = 9;
	int _projAvgOffset2 = 9;

	BOOL m_paramNoiseReduction = FALSE;
	BOOL m_paramRetinaTracking = FALSE;
	BOOL m_paramPostProcessing = FALSE;
	BOOL m_paramNormProjection = TRUE;
	BOOL m_paramDecorrProjection = TRUE;
	BOOL m_paramLayersSelection = TRUE;
	BOOL m_paramMotionCorrection = FALSE;
	BOOL m_paramProjectionRemoval = FALSE;
	BOOL m_paramVascularLayers = FALSE;
	BOOL m_paramReflectCorrect = FALSE;

	int m_paramPattern = 5;
	int m_paramRangeX = 8;
	int m_paramRangeY = 8;
	int m_paramOverlaps = 1;
	int m_paramPoints = 1;
	int m_paramLines = 1;
	int m_paramUpperLayer = 0;
	int m_paramLowerLayer = 6;
	int m_paramTestAvgLines = 5;
	int m_paramTestTopLines = 5;
	float m_paramTestDecorrThresh = 0.15f;

	bool _isScanStarting = false;
	bool _isScanClosing = false;

protected:
	void initControls(void);
	void showIrCameras(bool play);
	void prepareSystem(void);
	void updateScanControls(void);

	void setRadioCameraSpeed(UINT value);
	void setInternalFixation(UINT value);
	void updateScannerResult(bool enable=true);
	void updatePreviewImagesSlider(int size);
	void updatePatternImagesSlider(int size);
	void updateOverlapImagesSlider(int size);
	void updateAngioControls(bool fetch=false);
	void updateAngioSettings(bool clear = true);
	void updateAngioTestControls(void);
	void refreshAngioControls(void);

	void showPreviewImageSelected(void);
	void showPatternImageSelected(void);
	void showOverlapImageSelected(void);

	void callbackScanPatternCompleted(bool result);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGANGIOPATTERN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CComboBox cmbDirection;
	CComboBox cmbLines;
	CComboBox cmbOverlaps;
	CComboBox cmbPattern;
	CComboBox cmbPoints;
	CComboBox cmbRangeX;
	CComboBox cmbRangeY;
	CComboBox cmbSpace;
	CEdit editOffsetX;
	CEdit editOffsetY;
	CEdit editPatternNo;
	CEdit editPreviewNo;
	CEdit editScaleX;
	CEdit editScaleY;
	CSliderCtrl sldPattern;
	CSliderCtrl sldPreview;
	CEdit editAngle;
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	afx_msg void OnReleasedcaptureSliderPattern(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderPreview(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStartScan();
	afx_msg void OnBnClickedButtonExit2();
	afx_msg void OnBnClickedButtonAutoFocus();
	afx_msg void OnBnClickedButtonAutoPolar2();
	afx_msg void OnBnClickedButtonAutoRefer3();
	afx_msg void OnBnClickedButtonOptimize();
	afx_msg void OnBnClickedButtonFocus();
	afx_msg void OnBnClickedButtonCamera();
	afx_msg void OnBnClickedButtonMotor();
	afx_msg void OnBnClickedButtonLeds();
	afx_msg void OnBnClickedButtonResultExport();
	afx_msg void OnBnClickedButtonSegment();
	int m_cameraSpeed;
	afx_msg void OnCbnSelchangeComboDirection();
	afx_msg void OnCbnSelchangeComboPattern();
	afx_msg void OnCbnSelchangeComboPoints();
	afx_msg void OnCbnSelchangeComboRangex();
	afx_msg void OnCbnSelchangeComboRangey();

	CComboBox cmbAngioLower;
	CComboBox cmbAngioUpper;
	CEdit editAngioLines;
	CEdit editAngioOverlaps;
	CEdit editAngioPoints;
	CEdit editDecorThresh;
	afx_msg void OnCbnSelchangeComboOverlaps();
	afx_msg void OnCbnSelchangeComboLines();
	CEdit editAngioContrast;
	afx_msg void OnBnClickedButtonAngioReload();
	afx_msg void OnBnClickedButtonAngioRedraw();
	afx_msg void OnBnClickedButtonAngioExport();
	afx_msg void OnNMCustomdrawSliderPattern(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditPatternNo();
	afx_msg void OnBnClickedCheckExportBuffers();
	afx_msg void OnBnClickedButtonAngioExportData();
	afx_msg void OnBnClickedButtonAngioImportData();
	afx_msg void OnBnClickedButtonAngioImportBins();
	afx_msg void OnBnClickedCheckAngioEnhance();
	afx_msg void OnBnClickedButtonRetinaRegist();
	CEdit editLowerThresh;
	CEdit editUpperThresh;
	afx_msg void OnBnClickedCheckOverlayDecorr();
	afx_msg void OnBnClickedCheckFitToHeight();
	afx_msg void OnBnClickedCheckViewAmplitudes();
	CEdit editOverlapNo;
	CSliderCtrl sldOverlap;
	afx_msg void OnCustomdrawSliderOverlap(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderOverlap(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckPreviewDecorrs();
	CEdit editDecorThresh2;
	CEdit editDifferThresh;
	CEdit editDifferThresh2;
	afx_msg void OnBnClickedButtonAngioImportData2();
	afx_msg void OnBnClickedRadioCamera3();
	afx_msg void OnBnClickedCheckRetinaTrack();
	afx_msg void OnBnClickedRadioIntfixOdFundus();
	CEdit editFoveaCx;
	CEdit editFoveaCy;
	CEdit editLayerOffset1;
	CEdit editLayerOffset2;
	afx_msg void OnBnClickedButtonTestSetParams();
	afx_msg void OnBnClickedCheckFixedNoiseReduction();
	CEdit editTestAvgLines;
	CEdit editTestDecorrThresh;
	CEdit editTestTopLines;
	afx_msg void OnBnClickedCheckTestHorzLines();
	afx_msg void OnBnClickedCheckTestVertLines();
	afx_msg void OnBnClickedButtonTestResult();
	afx_msg void OnBnClickedButtonTestSaveParams();
	afx_msg void OnBnClickedButtonTestInitParams();
	CEdit editCameraGain;
	afx_msg void OnBnClickedButtonCameraSet();
	afx_msg void OnBnClickedCheckAngioProjectRemoval();
	afx_msg void OnBnClickedButtonAngioExportData2();
	afx_msg void OnBnClickedButtonAngioImportData3();
	afx_msg void OnBnClickedButtonAngioOpenData2();
	afx_msg void OnBnClickedRadioIntfixOdDisc();
	afx_msg void OnBnClickedButtonAngioOpenBins();
	afx_msg void OnBnClickedCheckExportCameraBuffer();
	afx_msg void OnBnClickedButtonAutoAdjust();
	afx_msg void OnBnClickedButtonRetinaPrepare();
	afx_msg void OnBnClickedButtonRetinaStart();
	CEdit editFlowAvgOffs1;
	CEdit editFlowAvgOffs2;
	CEdit editFlowThresh1;
	CEdit editFlowThresh2;
	CComboBox cmbProjLower;
	CComboBox cmbProjUpper;
	CComboBox cmbVascLower;
	CComboBox cmbVascUpper;
	CEdit editVascAvgOffs;
	CEdit editMaskAvgOffs;
	CEdit editMaskWeight;
	CEdit editProjAvgOffs1;
	CEdit editProjAvgOffs2;
};
