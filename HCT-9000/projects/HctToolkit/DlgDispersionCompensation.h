#pragma once

#include "ViewCorneaCamera.h"
#include "ViewRetinaCamera.h"

#include "ViewScanPreview.h"
#include "ViewScanSpectrum.h"
#include "ViewScanIntensity.h"

#include "DlgFocusControl.h"
#include "DlgCameraControl.h"
#include "DlgMotorControl.h"
#include "DlgLightControl.h"
#include "DlgStageControl.h"

#include <memory>
#include <string>

#include "OctSystem2.h"
#include "OctConfig2.h"
#include "SigProc2.h"
#include "SigChain2.h"
#include "CppUtil2.h"
#include "afxwin.h"


using namespace OctSystem;
using namespace OctConfig;
using namespace SigProc;
using namespace SigChain;
using namespace CppUtil;
using namespace std;


// DlgDispersionCompensation dialog

class DlgDispersionCompensation : public CDialogEx
{
	DECLARE_DYNAMIC(DlgDispersionCompensation)

public:
	DlgDispersionCompensation(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgDispersionCompensation();

protected:
	ViewCorneaCamera corneaView;
	ViewRetinaCamera retinaView;
	ViewScanPreview previewView;
	ViewScanSpectrum spectrumView;
	ViewScanIntensity intensityView;

	std::unique_ptr<DlgFocusControl> pDlgFocus;
	std::unique_ptr<DlgCameraControl> pDlgCamera;
	std::unique_ptr<DlgMotorControl> pDlgMotor;
	std::unique_ptr<DlgLightControl> pDlgLight;
	std::unique_ptr<DlgStageControl> pDlgStage;

	bool isVertScan = false;
	bool useCorneaDisp = false;
	float scanRange = SPECTRO_CALIB_BSCAN_RANGE;
	int numPoints = SPECTRO_CALIB_BSCAN_WIDTH;

	PatternType patternType;
	PatternDomain patternDomain;
	PatternName _patternName;

	int _direction;
	float offsetX;
	float offsetY;
	float angle;
	float scaleX;
	float scaleY;

	void initControls(void);
	void controlCameras(bool play);
	void updateDispersionParams(bool init);
	void updateScanControls(bool init);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGDISPERSIONCOMPENSATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CButton btnStartScan;
	afx_msg void OnClickedButtonStartScan();
	afx_msg void OnBnClickedButtonBackground();
	CEdit editAlpha2;
	CEdit editAlpha3;
	afx_msg void OnBnClickedCheckBackground();
	afx_msg void OnBnClickedCheckDispersion();
	afx_msg void OnBnClickedCheckReduction();
	afx_msg void OnBnClickedButtonParamDefault();
	afx_msg void OnBnClickedButtonParamZero();
	afx_msg void OnBnClickedButtonFocus();
	afx_msg void OnBnClickedButtonMotor2();
	afx_msg void OnBnClickedButtonCamera2();
	afx_msg void OnBnClickedButtonLight2();
	afx_msg void OnBnClickedButtonExit2();
	afx_msg void OnBnClickedButtonSaveImage();
	afx_msg void OnBnClickedCheckAlignGuide();
	afx_msg void OnBnClickedButtonAutoFocus();
	afx_msg void OnBnClickedCheckGrayscale();
	afx_msg void OnBnClickedCheckVscan();
	CComboBox cmbRange;
	CComboBox cmbPoints;
	afx_msg void OnSelchangeComboRange();
	afx_msg void OnSelchangeComboPoints();
	afx_msg void OnBnClickedButtonAutoPolar();
	afx_msg void OnBnClickedButtonAutoRefer();
	CComboBox cmbPattern;
	CEdit editAngle;
	CEdit editOffsetX;
	CEdit editOffsetY;
	CEdit editScaleX;
	CEdit editScaleY;
	afx_msg void OnSelchangeComboPattern();
	afx_msg void OnBnClickedButtonStage();
	afx_msg void OnBnClickedCheckSnrRatios();
	afx_msg void OnBnClickedCheckAutoAlign();
	afx_msg void OnBnClickedCheckCorneaDisp();
};
