#pragma once

#include "ViewCorneaCamera.h"
#include "ViewRetinaCamera.h"
#include "ViewTopoPreview.h"
#include "ViewEnfacePreview.h"

#include "DlgFocusControl.h"
#include "DlgCameraControl.h"
#include "DlgMotorControl.h"
#include "DlgLightControl.h"
#include "DlgStageControl.h"

#include <memory>
#include <string>
#include <vector>

#include "OctSystem2.h"
#include "SigProc2.h"
#include "SigChain2.h"

#include <afxwin.h>
#include <afxcmn.h>

using namespace OctSystem;
using namespace SigProc;
using namespace SigChain;
using namespace std;

// DlgTopographyPattern dialog

class DlgTopographyPattern : public CDialogEx
{
	DECLARE_DYNAMIC(DlgTopographyPattern)

protected:
	ViewCorneaCamera corneaView;
	ViewRetinaCamera retinaView;
	ViewTopoPreview horzPreviewView;
	ViewTopoPreview vertPreviewView;
	ViewEnfacePreview enfaceView;
	PatternName m_patternName;

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
	float m_resolutionX;
	float m_resolutionY;

	int m_patternCode;
	float m_rangeX;
	float m_rangeY;
	int m_points;
	int m_direction;
	int m_overlaps;
	int m_lines;

	int m_width;
	int m_height;
	int m_srcWidth;
	int m_srcHeight;
	int m_quality;
	int m_cameraSpeed;

	float m_corneaOffsetX;
	bool m_isCorneaCenter;
	bool m_stretched;
	int m_subGuidePos{ -1 };

	float m_space;

	bool m_usePattern;
	bool m_useFaster;
	bool m_useEnface;
	bool m_useNoImageGrab;
	bool m_vFlip;
	int m_enfacePoints;
	int m_enfaceLines;
	float m_enfaceRangeX;
	float m_enfaceRangeY;

	float m_major;
	float m_minor;
	float m_degreeMajor;
	float m_degreeMinor;
	float m_valueAvgK;
	float m_horzR;
	float m_vertR;

	std::vector<int> m_upperLayer;
	std::vector<int> m_lowerLayer;
	std::vector<std::vector<int>> m_layerPoints;
	std::vector<std::vector<float>> m_curveRadius;
	std::vector<float> m_curvatureInfo;

	ScanSpeed m_speed = ScanSpeed::Fastest;
	OctScanMeasure m_measure;
	
	ScanPatternCompletedCallback m_clbScanPatternCompleted;
	JoystickEventCallback cbJoystick;
	PreviewImageCallback2 m_callbackPreviewImage;

public:
	DlgTopographyPattern(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgTopographyPattern();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGTOPOGRAPHYPATTERN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void initControls(void);

	DECLARE_MESSAGE_MAP()

public:
	CEdit editPatternNo;
	CSliderCtrl sldPattern;
	CSpinButtonCtrl spinPattern;
	CEdit editOffsetX;
	CEdit editOffsetY;
	CEdit editScaleX;
	CEdit editScaleY;
	CEdit editCalibrationRadius;
	CEdit editModeleyeR1;
	CEdit editModeleyeR2;
	CEdit editModeleyeR3;
	CEdit editModeleyeR4;
	CEdit editMeasuredModeleyeR1;
	CEdit editMeasuredModeleyeR2;
	CEdit editMeasuredModeleyeR3;
	CEdit editMeasuredModeleyeR4;

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	void prepareSystem(void);
	void showIrCameras(bool play);
	void updateScanControls(void);
	void setRadioCameraSpeed(UINT value);
	void callbackScanPatternCompleted(bool result);
	void updateScannerResult(bool enable);
	void updateEnfaceControls(void);
	void updatePreviewControls(void);
	void OnBnClickedCheckEqualization();
	void showPatternImageSelected(void);
	void setSubGuidePos(int value);
	void getCorneaLayerPoints();

private:
	static auto getValueAtVecPx(const std::vector<std::vector<float>>* pMapData, float radian,
		int radiusPx)->float;
	static auto getMNAAtDistMeridian(const std::vector<std::vector<float>>* pMapData)->std::tuple<float, float, float, float, float>;
	static auto getAvgByRange(const  std::vector< std::vector<float>>* pMapData, float radianBegin,
		float radianEnd, float radiusMMBegin, float radiusMMEnd)->float;
	static bool getAllPtInRange(std::vector<float>& out_data, const std::vector<std::vector<float>>* pMapData,
		float radianBegin, float radianEnd, float radiusMMBegin, float radiusMMEnd);
	static auto getInvalidDataRemovedValueList(std::vector<float> valueListRaw)->std::vector<float>;
	static bool getAvgValueListInRangeMeridian(std::vector<std::pair<float, float>>& out_data,
		const std::vector<std::vector<float>>* pMapData, float radiusMMBegin, float radiusMMEnd);
	void calculateCurvature(void);

protected:
	void callbackPreviewAcquired(unsigned char * data, unsigned int width, unsigned int height, float quality, float snrRatio, unsigned int sigCenter, unsigned int image);

	

public:
	afx_msg	void OnBnClickedBtnClose();
	afx_msg void OnReleasedcaptureSliderPattern(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnFocus();
	afx_msg void OnBnClickedButtonMakeTopoCalibration();
	afx_msg void OnBnClickedCheckEnfaceGrid();
	afx_msg void OnBnClickedBtnStartScan();
	afx_msg void OnBnClickedCheckGuideRange();
	afx_msg void OnBnClickedCheckGuideCurve();
	afx_msg void OnBnClickedButtonResultExport();
	afx_msg void OnBnClickedButtonGalvApply();
	afx_msg void OnBnClickedButtonGalvExport();
	afx_msg void OnClose();
	afx_msg void OnBnClickedBtnAutocenter();
	afx_msg void OnBnClickedBtnOffSclSave();
	afx_msg void OnBnClickedBtnRefresh();
	afx_msg void OnDeltaposSpinBscan(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnAutoFocus();
	afx_msg void OnBnClickedBtnModeleyesave();
};
