// DlgTopographyPattern.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgTopographyPattern.h"
#include "afxdialogex.h"
#include "CppUtil2.h"
#include "OctConfig2.h"

#include <numeric>
#include <json\json.hpp>
#include <boost\format.hpp>
#include <fstream>

// DlgTopographyPattern dialog
using namespace std;
using namespace CppUtil;
using namespace cv;
using namespace OctConfig;

#define kInvalidValue -999.f
#define CX(PMAP) ((float)(*PMAP)[0].size() / 2.0f)
#define CY(PMAP) ((float)(*PMAP).size() / 2.0f)

typedef struct _MapDataLine {
	int index;
	double theta;
	vector<double> points;
} MapDataLine;

typedef struct _InvalidMapArea {
	int r_begin;
	int r_end;
	double theta_begin;
	double theta_end;
} InvalidMapArea;

static const auto kTopoCenterRange = 1.0f;
static const auto kRangeForSimK = 1.5f;
static const auto kSimKRangeFrom = 1.25f;
static const auto kSimKRangeTo = 1.75f;

static auto s_makeRadialLineList(vector<vector<float>> rawData, int nLines, int nPoints)->vector<MapDataLine>;
static bool s_getInvalidAreaList(vector<InvalidMapArea>& out_list, vector<MapDataLine>* pLineList);
static bool s_makeMapImageData(float* out_imageData, vector<MapDataLine> lineList, vector<InvalidMapArea> invalidAreaList, int nPoints);
static void s_makeSplineList(vector<CubicSpline*>& out_splineList, vector<MapDataLine> lineList);
static auto s_getValueAt(int x, int y, vector<CubicSpline*>& splineList, vector<InvalidMapArea>* pInvalidAreaList)->float;

IMPLEMENT_DYNAMIC(DlgTopographyPattern, CDialogEx)

DlgTopographyPattern::DlgTopographyPattern(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGTOPOGRAPHYPATTERN, pParent)
	, m_cameraSpeed(0)
{
	m_callbackPreviewImage = std::bind(&DlgTopographyPattern::callbackPreviewAcquired, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7);
	m_clbScanPatternCompleted = std::bind(&DlgTopographyPattern::callbackScanPatternCompleted, this, std::placeholders::_1);
	cbJoystick = std::bind(&DlgTopographyPattern::OnBnClickedBtnStartScan, this);
	Controller::setJoystickEventCallback(&cbJoystick);
}

DlgTopographyPattern::~DlgTopographyPattern()
{
}

void DlgTopographyPattern::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATTERN_NO, editPatternNo);
	DDX_Control(pDX, IDC_SLIDER_PATTERN, sldPattern);
	DDX_Control(pDX, IDC_SPIN_BSCAN, spinPattern);
	DDX_Control(pDX, IDC_EDIT_CALIBRATION_RADIUS, editCalibrationRadius);
	DDX_Control(pDX, IDC_EDIT_OFFSET_X, editOffsetX);
	DDX_Control(pDX, IDC_EDIT_OFFSET_Y, editOffsetY);
	DDX_Control(pDX, IDC_EDIT_SCALE_X, editScaleX);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y, editScaleY);
	DDX_Control(pDX, IDC_EDIT_ME1, editModeleyeR1);
	DDX_Control(pDX, IDC_EDIT_ME2, editModeleyeR2);
	DDX_Control(pDX, IDC_EDIT_ME3, editModeleyeR3);
	DDX_Control(pDX, IDC_EDIT_ME4, editModeleyeR4);
	DDX_Control(pDX, IDC_EDIT_ME_MEASURED1, editMeasuredModeleyeR1);
	DDX_Control(pDX, IDC_EDIT_ME_MEASURED2, editMeasuredModeleyeR2);
	DDX_Control(pDX, IDC_EDIT_ME_MEASURED3, editMeasuredModeleyeR3);
	DDX_Control(pDX, IDC_EDIT_ME_MEASURED4, editMeasuredModeleyeR4);

	DDX_Radio(pDX, IDC_RADIO_CAMERA1, m_cameraSpeed);
}

void DlgTopographyPattern::initControls(void)
{
	CString text;

	enfaceView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 0, 320, 320), this);
	enfaceView.setDisplaySize(320, 320);
	enfaceView.ModifyStyle(0, SS_NOTIFY);
	
	retinaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 320, 320, 320 + 240), this);
	retinaView.setDisplaySize(320, 240);

	corneaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 320 + 240, 320, 320 + 240 + 240), this);
	corneaView.setDisplaySize(320, 240);
	corneaView.showCameraInfo(true);

	horzPreviewView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(320, 0, 320 + 768, 360), this);
	horzPreviewView.setDisplaySize(768, 360);
	horzPreviewView.ModifyStyle(0, SS_NOTIFY);
	horzPreviewView.setDisplayStretched(true);
	horzPreviewView.clearResultData();

	vertPreviewView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(320, 360, 320 + 768, 360 + 360), this);
	vertPreviewView.setDisplaySize(768, 360);
	vertPreviewView.ModifyStyle(0, SS_NOTIFY);
	vertPreviewView.setDisplayStretched(true);
	vertPreviewView.clearResultData();

	PatternSettings* pset = SystemProfile::getPatternSettings();

	text.Format(_T("%.3f"), pset->topographyPatternScale(1).first);
	editScaleX.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternScale(1).second);
	editScaleY.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternOffset(1).first);
	editOffsetX.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternOffset(1).second);
	editOffsetY.SetWindowTextW(text);

	editCalibrationRadius.SetWindowTextW(_T("8.000"));

	editModeleyeR1.SetWindowTextW(_T("6.5"));
	editModeleyeR2.SetWindowTextW(_T("8.0"));
	editModeleyeR3.SetWindowTextW(_T("9.5"));
	editModeleyeR4.SetWindowTextW(_T("13.0"));
	editMeasuredModeleyeR1.SetWindowTextW(_T("6.5"));
	editMeasuredModeleyeR2.SetWindowTextW(_T("8.0"));
	editMeasuredModeleyeR3.SetWindowTextW(_T("9.5"));
	editMeasuredModeleyeR4.SetWindowTextW(_T("13.0"));

	text.Format(_T("%.2f"), Scanner::getTriggerTimeStep());
	SetDlgItemText(IDC_EDIT_TIME_STEP, text);
	text.Format(_T("%d"), Scanner::getTriggerTimeDelay());
	SetDlgItemText(IDC_EDIT_TIME_DELAY, text);
	text.Format(_T("%d"), Scanner::getTriggerForePaddings(m_speed));
	SetDlgItemText(IDC_EDIT_FORE_PADDS, text);
	text.Format(_T("%d"), Scanner::getTriggerPostPaddings(m_speed));
	SetDlgItemText(IDC_EDIT_POST_PADDS, text);

	CheckDlgButton(IDC_RADIO_CAMERA1, FALSE);
	CheckDlgButton(IDC_RADIO_CAMERA2, TRUE);
	CheckDlgButton(IDC_RADIO_CAMERA3, FALSE);
	setRadioCameraSpeed(1);

	CheckDlgButton(IDC_CHECK_STRETCH, TRUE);

	pDlgFocus = make_unique<DlgFocusControl>();
	pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	pDlgFocus->initControlsWithoutShowWindow();
	pDlgFocus->MoveReferenceToCorneaOrigin();

	updateScannerResult(true);

	/*editOffsetX.SetWindowTextW(_T("0.0"));
	editOffsetY.SetWindowTextW(_T("0.0"));
	editScaleX.SetWindowTextW(_T("1.0"));
	editScaleY.SetWindowTextW(_T("1.0"));
	CheckDlgButton(IDC_CHECK_STRETCH, true);

	cmbEnfaceRangeX.SetCurSel(7);
	cmbEnfaceRangeY.SetCurSel(7);
	cmbEnfaceLines.SetCurSel(2);
	cmbEnfacePoints.SetCurSel(0);

	CheckDlgButton(IDC_CHECK_EQUALIZATION, (GlobalSettings::useEnfaceImageCorrection() ? FALSE : TRUE));
	editEnfaceClipLimit.SetWindowTextW(_T("16.0"));
	editEnfaceScalar.SetWindowTextW(_T("0.0"));

	auto range = GlobalSettings::enfacePreviewRange();
	text.Format(_T("%d"), range.first);
	editEnfaceRange1.SetWindowTextW(text);
	text.Format(_T("%d"), range.second);
	editEnfaceRange2.SetWindowTextW(text);

	CheckDlgButton(IDC_RADIO_OD, TRUE);
	CheckDlgButton(IDC_RADIO_MACULAR, TRUE);
	CheckDlgButton(IDC_RADIO_DIOPT_ZERO, TRUE);
	updateInternalFixation();

	updateScannerResult(true);

	CheckDlgButton(IDC_RADIO_CAMERA1, FALSE);
	CheckDlgButton(IDC_RADIO_CAMERA2, FALSE);
	CheckDlgButton(IDC_RADIO_CAMERA3, TRUE);
	setRadioCameraSpeed(2);

	float offsetX, offsetY;
	Controller::getScanCenterPosition(offsetX, offsetY);
	text.Format(_T("%.2f"), offsetX);
	editGalvOffsetX.SetWindowTextW(text);
	text.Format(_T("%.2f"), offsetY);
	editGalvOffsetY.SetWindowTextW(text);

	editAverageSize.SetWindowTextW(_T("5"));

	editCalibrationRadius.SetWindowTextW(_T("8.00"));*/

	return;
}


BEGIN_MESSAGE_MAP(DlgTopographyPattern, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &DlgTopographyPattern::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_BTN_FOCUS, &DlgTopographyPattern::OnBnClickedBtnFocus)
	ON_BN_CLICKED(IDC_BUTTON_MAKE_TOPO_CALIBRATION, &DlgTopographyPattern::OnBnClickedButtonMakeTopoCalibration)
	ON_BN_CLICKED(IDC_CHECK_ENFACE_GRID, &DlgTopographyPattern::OnBnClickedCheckEnfaceGrid)
	ON_BN_CLICKED(IDC_BTN_START_SCAN, &DlgTopographyPattern::OnBnClickedBtnStartScan)
	ON_BN_CLICKED(IDC_CHECK_GUIDE_RANGE, &DlgTopographyPattern::OnBnClickedCheckGuideRange)
	ON_BN_CLICKED(IDC_CHECK_GUIDE_CURVE, &DlgTopographyPattern::OnBnClickedCheckGuideCurve)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_PATTERN, &DlgTopographyPattern::OnReleasedcaptureSliderPattern)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_CAMERA1, IDC_RADIO_CAMERA3, DlgTopographyPattern::setRadioCameraSpeed)
	ON_BN_CLICKED(IDC_BUTTON_RESULT_EXPORT, &DlgTopographyPattern::OnBnClickedButtonResultExport)
	ON_BN_CLICKED(IDC_BUTTON_GALV_APPLY, &DlgTopographyPattern::OnBnClickedButtonGalvApply)
	ON_BN_CLICKED(IDC_BUTTON_GALV_EXPORT, &DlgTopographyPattern::OnBnClickedButtonGalvExport)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_AUTOCENTER, &DlgTopographyPattern::OnBnClickedBtnAutocenter)
	ON_BN_CLICKED(IDC_BTN_OFF_SCL_SAVE, &DlgTopographyPattern::OnBnClickedBtnOffSclSave)
	ON_BN_CLICKED(IDC_BTN_REFRESH, &DlgTopographyPattern::OnBnClickedBtnRefresh)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BSCAN, &DlgTopographyPattern::OnDeltaposSpinBscan)
	ON_BN_CLICKED(IDC_BTN_AUTO_FOCUS, &DlgTopographyPattern::OnBnClickedBtnAutoFocus)
	ON_BN_CLICKED(IDC_BTN_MODELEYESAVE, &DlgTopographyPattern::OnBnClickedBtnModeleyesave)
END_MESSAGE_MAP()

BOOL DlgTopographyPattern::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	initControls();
	showIrCameras(true);
	prepareSystem();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


// DlgTopographyPattern message handlers

void DlgTopographyPattern::OnBnClickedBtnClose()
{
	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);
}

void DlgTopographyPattern::prepareSystem(void)
{
	Controller::turnOnInternalFixationAtCenter();
	return;
}


void DlgTopographyPattern::callbackScanPatternCompleted(bool result)
{
	updateScannerResult(true);

	float acqTime = Controller::getScanner()->getAcquisitionTime();
	horzPreviewView.setAcquisitionTime(acqTime);
	horzPreviewView.setQualityIndex(Measure::getPatternQualityIndex());

	vertPreviewView.setAcquisitionTime(acqTime);
	vertPreviewView.setQualityIndex(Measure::getPatternQualityIndex());

	float asize = 5.0f;
	vector<float> stat;
	Measure::getPatternQualityStat(stat, asize);
	horzPreviewView.setPatternQualities(m_direction, stat);
	vertPreviewView.setPatternQualities(m_direction, stat);

	m_vFlip = false;
	if (IsDlgButtonChecked(IDC_CHECK_CURVATURE) || IsDlgButtonChecked(IDC_CHECK_LAYER)) {
		getCorneaLayerPoints();
		calculateCurvature();
		m_vFlip = true;

		if (IsDlgButtonChecked(IDC_CHECK_CURVATURE)) {
			vertPreviewView.setAverageCurvature(m_valueAvgK);
			vertPreviewView.setVerticalCurvature(m_vertR);
			vertPreviewView.setMajorCurvature(m_major);
			vertPreviewView.setMajorAxis(m_degreeMajor);
			vertPreviewView.setMinorCurvature(m_minor);
			vertPreviewView.setMinorAxis(m_degreeMinor);

			horzPreviewView.setHorizontalCurvature(m_horzR);
		}
	}

	horzPreviewView.updateWindow();
	vertPreviewView.updateWindow();

	EndWaitCursor();
	return;
}

void DlgTopographyPattern::updateScannerResult(bool enable)
{
	int size, cpos;
	CString text;

	sldPattern.EnableWindow(FALSE);
	spinPattern.EnableWindow(FALSE);

	if (Measure::isScannerResult() && enable) {
		size = Measure::getCountOfPatternSections();
		if (size > 0) {
			sldPattern.SetRange(1, size);
			sldPattern.SetLineSize(1);
			sldPattern.SetPageSize(1);
			cpos = (size <= 1 ? 1 : size / 2);
			sldPattern.SetPos(cpos);

			text.Format(_T("%d"), cpos);
			editPatternNo.SetWindowTextW(text);
			sldPattern.EnableWindow(TRUE);

			spinPattern.SetRange(1, size);
			spinPattern.SetPos(cpos);
			spinPattern.EnableWindow(TRUE);
		}

		//const OctEnfaceImage* enface = Measure::getEnfaceImageDescript();
		//if (enface) {
		//	// enfaceView.setEqualization(false);
		//	//enfaceView.callbackEnfaceImage(enface->getBuffer(), enface->getWidth(), wecenface->getHeight());
		//}
	}
	return;
}


void DlgTopographyPattern::showIrCameras(bool play)
{
	BeginWaitCursor();
	if (play) {
		Controller::startCorneaCamera(corneaView.getCallbackFunction());
		Controller::startRetinaCamera(retinaView.getCallbackFunction());
	}
	else {
		Controller::closeCorneaCamera();
		Controller::closeRetinaCamera();
	}
	EndWaitCursor();
	return;
}

void DlgTopographyPattern::updateScanControls(void)
{
	CString text;
	editOffsetX.GetWindowText(text);
	m_offsetX = (float)(text.IsEmpty() ? 0.0f : _ttof(text));
	editOffsetY.GetWindowText(text);
	m_offsetY = (float)(text.IsEmpty() ? 0.0f : _ttof(text));
	editScaleX.GetWindowText(text);
	m_scaleX = (float)(text.IsEmpty() ? 1.0f : _ttof(text));
	editScaleY.GetWindowText(text);
	m_scaleY = (float)(text.IsEmpty() ? 1.0f : _ttof(text));

	m_patternName = PatternName::Topography;
	m_rangeX = 8.0f;
	m_rangeY = 8.0f;
	m_points = 1024;
	m_direction = 0;
	m_overlaps = 1;
	m_lines = 16;
	m_space = 0.15f;
	m_angle = 0.0f;

	m_resolutionX = m_rangeX / m_points;
	m_resolutionY = (float)(GlobalSettings::getCorneaScanAxialResolution() * CORNEA_REFLECTIVE_INDEX) * 0.001;

	return;
}


void DlgTopographyPattern::setRadioCameraSpeed(UINT value)
{
	UpdateData(TRUE);
	CString text;
	PatternSettings* pset = SystemProfile::getPatternSettings();
	updatePreviewControls();

	bool ret = true;
	float exposure = 0.0f;

	switch (m_cameraSpeed) {
	case 0:
		ret = Scanner::changeScanSpeedToNormal(m_useNoImageGrab);
		exposure = Scanner::getDefaultTriggerTimeStep(ScanSpeed::Normal);
		text.Format(_T("%.1f"), exposure);
		SetDlgItemText(IDC_EDIT_TIME_STEP, text);
		m_speed = ScanSpeed::Normal;

		text.Format(_T("%.3f"), pset->topographyPatternScale(0).first);
		editScaleX.SetWindowTextW(text);
		text.Format(_T("%.3f"), pset->topographyPatternScale(0).second);
		editScaleY.SetWindowTextW(text);
		text.Format(_T("%.3f"), pset->topographyPatternOffset(0).first);
		editOffsetX.SetWindowTextW(text);
		text.Format(_T("%.3f"), pset->topographyPatternOffset(0).second);
		editOffsetY.SetWindowTextW(text);

		break;
	case 1:
		ret = Scanner::changeScanSpeedToFaster(m_useNoImageGrab);
		exposure = Scanner::getDefaultTriggerTimeStep(ScanSpeed::Faster);
		text.Format(_T("%.1f"), exposure);
		SetDlgItemText(IDC_EDIT_TIME_STEP, text);
		m_speed = ScanSpeed::Faster;

		text.Format(_T("%.3f"), pset->topographyPatternScale(1).first);
		editScaleX.SetWindowTextW(text);
		text.Format(_T("%.3f"), pset->topographyPatternScale(1).second);
		editScaleY.SetWindowTextW(text);
		text.Format(_T("%.3f"), pset->topographyPatternOffset(1).first);
		editOffsetX.SetWindowTextW(text);
		text.Format(_T("%.3f"), pset->topographyPatternOffset(1).second);
		editOffsetY.SetWindowTextW(text);

		break;
	case 2:
		ret = Scanner::changeScanSpeedToFastest(m_useNoImageGrab);
		exposure = Scanner::getDefaultTriggerTimeStep(ScanSpeed::Fastest);
		text.Format(_T("%.1f"), exposure);
		SetDlgItemText(IDC_EDIT_TIME_STEP, text);
		m_speed = ScanSpeed::Fastest;

		text.Format(_T("%.3f"), pset->topographyPatternScale(2).first);
		editScaleX.SetWindowTextW(text);
		text.Format(_T("%.3f"), pset->topographyPatternScale(2).second);
		editScaleY.SetWindowTextW(text);
		text.Format(_T("%.3f"), pset->topographyPatternOffset(2).first);
		editOffsetX.SetWindowTextW(text);
		text.Format(_T("%.3f"), pset->topographyPatternOffset(2).second);
		editOffsetY.SetWindowTextW(text);

		break;
	}

	text.Format(_T("%d"), Scanner::getTriggerForePaddings(m_speed));
	SetDlgItemText(IDC_EDIT_FORE_PADDS, text);
	text.Format(_T("%d"), Scanner::getTriggerPostPaddings(m_speed));
	SetDlgItemText(IDC_EDIT_POST_PADDS, text);

	if (!ret) {
		AfxMessageBox(_T("Reset scan speed failed!"));
	}
	else {
		OnBnClickedButtonGalvApply();
	}
	return;
}



void DlgTopographyPattern::updateEnfaceControls(void)
{
	m_useEnface = (IsDlgButtonChecked(IDC_CHECK_ENFACE) ? true : false);

	m_enfaceLines = (int)256;
	m_enfacePoints = (int)256;
	m_enfaceRangeX = (float)8.f;
	m_enfaceRangeY = (float)8.f;

	enfaceView.setScanRange(m_enfaceRangeX, m_enfaceRangeY);
	return;
}


void DlgTopographyPattern::updatePreviewControls(void)
{
	horzPreviewView.setDisplayStretched(IsDlgButtonChecked(IDC_CHECK_STRETCH) ? true : false);
	vertPreviewView.setDisplayStretched(IsDlgButtonChecked(IDC_CHECK_STRETCH) ? true : false);

	m_usePattern = (IsDlgButtonChecked(IDC_CHECK_PREVIEW_PATTERN) ? true : false);
	m_useFaster = (IsDlgButtonChecked(IDC_CHECK_FAST_RASTERS) ? true : false);
	m_useNoImageGrab = false;//(IsDlgButtonChecked(IDC_CHECK_NO_IMAGE_GRAB) ? true : false);

	return;
}

void DlgTopographyPattern::OnBnClickedCheckEqualization()
{
	bool useEqual = (IsDlgButtonChecked(IDC_CHECK_EQUALIZATION) ? true : false);
	GlobalSettings::useEnfaceImageCorrection(true, !useEqual);

	CString text;
	float limit = 16.0f;
	float scalar = 0.0f;

	enfaceView.setEqualization(useEqual, limit, scalar);
	return;
}


void DlgTopographyPattern::OnBnClickedBtnFocus()
{
	// TODO: Add your control notification handler code here
	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);

	return;
}

void DlgTopographyPattern::OnBnClickedCheckEnfaceGrid()
{
	// TODO: Add your control notification handler code here
	bool flag = (IsDlgButtonChecked(IDC_CHECK_ENFACE_GRID) ? true : false);
	if (flag) {
		updateEnfaceControls();
		enfaceView.setScanRange(m_enfaceRangeX, m_enfaceRangeY);
	}
	enfaceView.showTopoGridLines(flag);
	return;
}


void DlgTopographyPattern::OnBnClickedBtnStartScan()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Controller::isScanProcessing()) {
		EndWaitCursor();
		return;
	}

	if (Controller::isScanning())
	{
		updateScannerResult(false);
		Controller::closeScan(true, nullptr, &m_clbScanPatternCompleted);
		SetDlgItemText(IDC_BTN_START_SCAN, L"Start Scan");

		showIrCameras(false);
		return;
	}
	else {
		showIrCameras(true);

		updateScanControls();
		updateEnfaceControls();
		updatePreviewControls();

		ScanSpeed speed;
		if (Scanner::isScanSpeedNormal()) {
			speed = ScanSpeed::Normal;
		}
		else if (Scanner::isScanSpeedFaster()) {
			speed = ScanSpeed::Faster;
		}
		else {
			speed = ScanSpeed::Fastest;
		}

		m_measure.getPattern().setPreviewType(PatternType::Cross);

		m_measure.setEyeSide(EyeSide::OD);
		m_measure.getEnface().setup(m_patternName, m_enfacePoints, m_enfaceLines, m_enfaceRangeX, m_enfaceRangeY,
			m_direction, m_overlaps, m_space, speed);
		m_measure.getEnface().setScanOffset(m_offsetX, m_offsetY, m_angle, m_scaleX, m_scaleY);

		Controller::presetScanEnface(m_measure.getEnface());

		m_measure.getPattern().setup(m_patternName, m_points, m_lines, m_rangeX, m_rangeY,
			m_direction, m_overlaps, m_space, speed);
		m_measure.getPattern().setScanOffset(m_offsetX, m_offsetY, m_angle, m_scaleX, m_scaleY);

		m_measure.useFastRaster(true, m_useFaster);
		m_measure.usePhaseEnface(true, m_useEnface);
		m_measure.usePreviewPattern(true, m_usePattern);
		m_measure.useNoImageGrab(true, m_useNoImageGrab);

		ChainSetup::useRefreshDispersionParams(true, false);

		bool clearPreset = true;
		if (Controller::startScan2(m_measure,
									&m_callbackPreviewImage,
									enfaceView.getCallbackFunction(), clearPreset)) {
			SetDlgItemText(IDC_BTN_START_SCAN, L"Capture");
			sldPattern.EnableWindow(FALSE);
			spinPattern.EnableWindow(FALSE);

			OnBnClickedCheckEqualization();

			vertPreviewView.clearLayerPoints();
			vertPreviewView.clearAverageCurvature();
			vertPreviewView.clearMajorCurvature();
			vertPreviewView.clearMajorAxis();
			vertPreviewView.clearMinorCurvature();
			vertPreviewView.clearMinorAxis();
			vertPreviewView.clearVerticalCurvature();

			horzPreviewView.clearHorizontalCurvature();
			horzPreviewView.clearResultData(true);
			vertPreviewView.clearResultData(true);
		}
	}
	EndWaitCursor();

	return;
}


void DlgTopographyPattern::showPatternImageSelected(void)
{
	CString text;
	int pos = sldPattern.GetPos();
	text.Format(_T("%d"), pos);
	editPatternNo.SetWindowText(text);

	int index = pos - 1;
	const OctScanImage* image = Measure::getPatternImageDescript(index);
	vertPreviewView.setQualityIndex(image->getQuality());
	if (IsDlgButtonChecked(IDC_CHECK_LAYER) && m_vFlip) {
		vertPreviewView.setLayerPoints(m_layerPoints[index]);
	}
	else {
		vertPreviewView.clearLayerPoints();
	}

	/*if (IsDlgButtonChecked(IDC_CHECK_CURVATURE) && m_vFlip) {
		vertPreviewView.setCurvatureInfo(m_curvatureInfo[index]);
	}*/
	
	vertPreviewView.drawScanImage(image, m_vFlip);

	return;
}

void DlgTopographyPattern::OnReleasedcaptureSliderPattern(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	int pos = sldPattern.GetPos();
	spinPattern.SetPos(pos);

	showPatternImageSelected();
	*pResult = 0;

	return;
}

void DlgTopographyPattern::callbackPreviewAcquired(unsigned char* data, unsigned int width,
	unsigned int height, float quality, float snrRatio, unsigned int sigCenter, unsigned int image)
{
	ViewTopoPreview* pWndBScanPreview;
	if (image == 0) {
		pWndBScanPreview = &horzPreviewView;
	}
	else {
		pWndBScanPreview = &vertPreviewView;
	}

	pWndBScanPreview->setImage(data, width, height, sigCenter);

	float offsetX;
	if (OctSystem::CorneaTrack::getCorneaCenterOffsetX(offsetX, image)) {
		CRect rcWndBScanPreview;
		pWndBScanPreview->GetClientRect(&rcWndBScanPreview);

		auto imgWidthRatio = (float)rcWndBScanPreview.Width() / (float)width;
		pWndBScanPreview->setSubGuidePos((int)(imgWidthRatio * offsetX));
		// �ɼ�: Ŀ�� �׸� �� ����.
		//vector<int> antes;
		//GlobalRecord::readCorneaTrackAnteriorCurve(antes);
	}
	else {
		//pWndBScanPreview->setSubGuidePos(-1);
	}

	//pWndBScanPreview->clearResultData(true);

	return;
}


void DlgTopographyPattern::setSubGuidePos(int value)
{
	m_subGuidePos = value;

	return;
}


void DlgTopographyPattern::OnBnClickedCheckGuideRange()
{
	horzPreviewView.setRangeGuideline(IsDlgButtonChecked(IDC_CHECK_GUIDE_RANGE) ? true : false);
	vertPreviewView.setRangeGuideline(IsDlgButtonChecked(IDC_CHECK_GUIDE_RANGE) ? true : false);

	if (IsDlgButtonChecked(IDC_CHECK_GUIDE_RANGE)) {
		CheckDlgButton(IDC_CHECK_GUIDE_CURVE, false);
		horzPreviewView.setCurveGuideline(IsDlgButtonChecked(IDC_CHECK_GUIDE_CURVE) ? true : false);
		vertPreviewView.setCurveGuideline(IsDlgButtonChecked(IDC_CHECK_GUIDE_CURVE) ? true : false);
	}

	return;
}


void DlgTopographyPattern::OnBnClickedCheckGuideCurve()
{
	if (IsDlgButtonChecked(IDC_CHECK_GUIDE_CURVE)) {
		pDlgFocus->isTopography(true);
		pDlgFocus->MoveReferenceToCorneaOrigin();
		pDlgFocus->OnBnClickedButtonFocusOrigin();
	}

	horzPreviewView.setCurveGuideline(IsDlgButtonChecked(IDC_CHECK_GUIDE_CURVE) ? true : false);
	vertPreviewView.setCurveGuideline(IsDlgButtonChecked(IDC_CHECK_GUIDE_CURVE) ? true : false);

	if (IsDlgButtonChecked(IDC_CHECK_GUIDE_CURVE)) {
		CheckDlgButton(IDC_CHECK_GUIDE_RANGE, false);
		horzPreviewView.setRangeGuideline(IsDlgButtonChecked(IDC_CHECK_GUIDE_RANGE) ? true : false);
		vertPreviewView.setRangeGuideline(IsDlgButtonChecked(IDC_CHECK_GUIDE_RANGE) ? true : false);
	}

	return;
}


void DlgTopographyPattern::OnBnClickedButtonMakeTopoCalibration()
{
	// TODO: Add your control notification handler code here
	if (Measure::isScannerResult()) {
		BeginWaitCursor();
		CString text;

		float horzK = horzPreviewView.getHorizontalCurvature();
		float vertK = vertPreviewView.getVerticalCurvature();

		if (horzK > 7.98f && horzK < 8.02f && vertK > 7.98f && vertK < 8.02f) {
			
		}
		else {
			text.Format(_T("Invalid value!! check horzK or vertK value"));
			AfxMessageBox(text);
		}
	
		float modelEyeR = 0.0f;
		editCalibrationRadius.GetWindowTextW(text);

		if (!text.IsEmpty())
			modelEyeR = _ttof(text);

		wstring path, name;

		if ((_waccess(_T(".\\Calibration"), 0)) == -1)
			CreateDirectory(_T(".\\Calibration"), NULL);	

		for (int i = 0; i < m_lines; i++) {
			auto layerEPI = m_layerPoints[i];
			if (layerEPI.size() > 0) {
				name = (boost::wformat(L"%03d") % i).str();
				path = (boost::wformat(L"./Calibration//%s.json") % name).str();

				using json = nlohmann::json;
				json j;
				// j["name"] = wtoa(getImageName());
				j["model_eye R"] = (float)modelEyeR;

				int size = m_points;
				auto dataX = std::vector<double>(size, -1);
				auto dataEPI = std::vector<double>(size, -1);

				for (int i = 0; i < size; i++) {
					dataX[i] = i * m_resolutionX;
					dataEPI[i] = layerEPI[i] * m_resolutionY;
				}

				vector<double> coeffs;
				auto fittY = vector<double>(size, -1);
				auto circleY = vector<double>(size, -1);
				auto offsets = vector<float>(size, -1);

				CppUtil::Regressor::polyFit(dataX, dataEPI, 6, coeffs);
				double fx1, fx2, num;
				for (int i = 0; i < size; i++) {
					fittY[i] = coeffs[6] * pow(dataX[i], 6) + coeffs[5] * pow(dataX[i], 5) + coeffs[4] * pow(dataX[i], 4) + coeffs[3] * pow(dataX[i], 3) + coeffs[2] * pow(dataX[i], 2) + coeffs[1] * dataX[i] + coeffs[0];
				}

				int offset = 16;
				float fdx = (dataX[size - offset] - dataX[offset]);
				float fdy = (fittY[size - offset] - fittY[offset]);
				float fxc = (dataX[size - offset] + dataX[offset]) / 2.0f;
				float fyc = (fittY[size - offset] + fittY[offset]) / 2.0f;

				float fd = sqrt(fdx * fdx + fdy * fdy);
				float fd2 = fd / 2;
				float fOffset = sqrt(modelEyeR * modelEyeR - fd2 * fd2);

				float fplusx = fOffset * fdy / fd;
				float fplusy = fOffset * fdx / fd;

				float centerX = fxc - fplusx;
				float centerY = fyc + fplusy;

				/*auto padding = fittY[std::upper_bound(fittY.begin(), fittY.end(), 0) - fittY.begin()];
				float scanRange = 8.f;
				float theta = 2 * asin(scanRange / (2 * modelEyeR));
				float centerX = scanRange / 2.f;
				float centerY = modelEyeR - modelEyeR * (1 - cos(theta / 2.f)) + padding;*/

				for (int i = 0; i < size; i++) {
					circleY[i] = -sqrt((modelEyeR + dataX[i] - centerX) * (modelEyeR - dataX[i] + centerX)) + centerY;
					//circleY[i] = -sqrt(pow(modelEyeR, 2) - pow((dataX[i] - centerX), 2)) + centerY;
					offsets[i] = (float)(fittY[i] - circleY[i]);
				}
				//j["padding"] = (float)padding;
				//j["theta"] = (float)theta;
				j["centerX"] = (float)centerX;
				j["centerY"] = (float)centerY;

				j["horzR"] = (float)m_horzR;
				j["vertR"] = (float)m_vertR;
				j["Calibration_data"] = offsets;


				auto mmToPoint = m_points / m_rangeX;
				float meanCurves = 0;
				m_curvatureInfo.resize(m_lines);

				for (int i = 0; i < m_curveRadius.size(); i++) {
					auto curve = m_curveRadius[i];

					for (int j = mmToPoint * 2.25; j < mmToPoint * 2.75; j++) {
						meanCurves += curve[j];
					}
					for (int j = mmToPoint * 5.25; j < mmToPoint * 5.75; j++) {
						meanCurves += curve[j];
					}

					meanCurves /= mmToPoint * 3;

					m_curvatureInfo[i] = meanCurves;
				}
				j["Mean_cur"] = m_curvatureInfo;

				/*if (isLayerPoints(OcularLayerType::BOW) || true) {
				layer = getLayerPoints(OcularLayerType::BOW);
				j["layers"]["BOW"] = layer;
				}
				if (isLayerPoints(OcularLayerType::END) || true) {
				layer = getLayerPoints(OcularLayerType::END);
				j["layers"]["END"] = layer;
				}

				std::vector<float> curvs;
				curvs = getCurvatureRadius();
				j["curvature"] = curvs;*/

				try {
					std::ofstream f(path);
					f << j.dump();
					f.close();
				}
				catch (...) {
					return;
				}
			}
		}
		text.Format(_T("Topography Calibration files has been saved!"));
		AfxMessageBox(text);
	}

	return;
}

void DlgTopographyPattern::getCorneaLayerPoints()
{
	if (Measure::isScannerResult()) {
		BeginWaitCursor();

		auto result = Measure::fetchScannerResult();
		auto data = Analysis::insertProtocolData(result, false);
		vector<vector<int>> points(m_lines, vector<int>(m_points, -1));
		vector<vector<float>> curveR(m_lines, vector<float>(m_points, -1));

		Analysis::getCorneaLayerAndCurvature(data, OcularLayerType::EPI, points, curveR);
		m_layerPoints = points;
		m_curveRadius = curveR;

		EndWaitCursor();
	}
	
	return;
}

auto s_makeRadialLineList(vector<vector<float>> rawData, int nBScanLines,
	int nBScanPoints)->vector<MapDataLine>
{
	vector<MapDataLine> mapData;

	auto stepSizeOfAngle = -CV_PI / (double)nBScanLines;
	auto nPointForLine = nBScanPoints / 2;
	auto dataCnt = nBScanLines * nBScanPoints;

	for (int rawDataIdx = 0; rawDataIdx < (int)rawData.size(); rawDataIdx++) {
		auto lineData = rawData[rawDataIdx];

		// from the center
		{
			int pointIdxOffset = nBScanPoints / 2;

			MapDataLine lineItem;
			lineItem.theta = (double)rawDataIdx * stepSizeOfAngle;
			if (lineItem.theta < 0.0) {
				lineItem.theta += 2.0 * CV_PI;
			}

			vector<double> pointList;
			for (int pointIdx = 0; pointIdx < nPointForLine; pointIdx++) {
				pointList.push_back(lineData[pointIdx + pointIdxOffset]);
			}
			lineItem.points = pointList;

			//
			mapData.push_back(lineItem);
		}

		// to the center
		{
			int pointIdxOffset = 0;

			MapDataLine lineItem;
			lineItem.theta = (double)rawDataIdx * stepSizeOfAngle + CV_PI;

			vector<double> pointList;
			for (int pointIdx = nPointForLine - 1; pointIdx >= 0; pointIdx--)
			{
				pointList.push_back(lineData[pointIdx + pointIdxOffset]);
			}
			lineItem.points = pointList;

			//
			mapData.push_back(lineItem);
		}
	}

	// sort
	sort(mapData.begin(), mapData.end(), [](auto x1, auto x2) { return x1.theta < x2.theta; });

	// set index
	for (size_t i = 0; i < mapData.size(); i++)
	{
		mapData[i].index = (int)i;
	}

	return mapData;
}


bool s_getInvalidAreaList(vector<InvalidMapArea>& out_list, vector<MapDataLine>* pLineList)
{
	const auto kAngleGapBetweenLines = CV_PI / (double)pLineList->size();

	auto lastLine = *(pLineList->end() - 1);
	auto firstLine = (*pLineList->begin());

	vector<MapDataLine> lineListLocal;
	lineListLocal.push_back(
		MapDataLine{ (int)lineListLocal.size() - 1, lastLine.theta + CV_PI * 2.0, lastLine.points });
	lineListLocal.insert(lineListLocal.end(), pLineList->begin(), pLineList->end());
	lineListLocal.push_back(
		MapDataLine{ (int)lineListLocal.size(), firstLine.theta + CV_PI * 2.0, firstLine.points });

	for (int lineIdx = 1; lineIdx < (int)lineListLocal.size() - 2; lineIdx++) {
		auto line = lineListLocal[lineIdx];

		const int nPts = (int)line.points.size();
		for (int idx = 0; idx < nPts; idx++) {
			if (line.points[idx] == kInvalidValue) {
				InvalidMapArea area;
				area.r_begin = idx;
				area.r_end = nPts - 1;
				area.theta_begin = lineListLocal[lineIdx - 1].theta;
				area.theta_end = lineListLocal[lineIdx + 1].theta;

				out_list.push_back(area);
				break;
			}
		}
	}

	return true;
}

bool s_makeMapImageData(float* out_imageData, vector<MapDataLine> lineList,
	vector<InvalidMapArea> invalidAreaList, int nPoints)
{
	if (out_imageData == nullptr) {
		return false;
	}

	auto offset = nPoints / 2;

	// make splines
	vector<CubicSpline*> splineList;
	s_makeSplineList(splineList, lineList);

	// fill the map
	for (int y = 0; y < nPoints; y++) {
		for (int x = 0; x < nPoints; x++) {
			auto x_in_data = x - offset;
			auto y_in_data = y - offset;

			out_imageData[y * nPoints + x] = s_getValueAt(x_in_data, y_in_data, splineList,
				&invalidAreaList);
		}
	}

	// clear splines
	for (auto item : splineList) {
		delete item;
	}

	return true;
}

void s_makeSplineList(vector<CubicSpline*>& out_splineList, vector<MapDataLine> lineList)
{
	auto nPts = lineList[0].points.size();

	vector<MapDataLine> lineListInterp;
	lineListInterp.insert(lineListInterp.end(), lineList.begin(), lineList.end());
	lineListInterp.push_back(
		MapDataLine{ (int)lineListInterp.size(), lineList[0].theta + CV_PI * 2.0, lineList[0].points });

	auto lastValue = 0.0;
	for (int ptIdx = 0; ptIdx < nPts; ptIdx++)
	{
		vector<double> xList;
		vector<double> yList;

		for (int lineIdx = 0; lineIdx < lineListInterp.size(); lineIdx++)
		{
			xList.push_back(lineListInterp[lineIdx].theta);

			auto value = lineListInterp[lineIdx].points[ptIdx];
			if (value <= kInvalidValue) {
				value = lastValue;
			}
			yList.push_back(value);

			lastValue = value;
		}
		CubicSpline* pSpline = new CubicSpline;
		pSpline->setPoints(xList, yList);
		out_splineList.push_back(pSpline);
	}
}

auto s_getValueAt(int x, int y, vector<CubicSpline*>& splineList, vector<InvalidMapArea>* pInvalidAreaList)->float
{
	// check out of range
	auto r = (int)sqrt(x * x + y * y);
	if (r >= splineList.size()) {
		return kInvalidValue;
	}

	// theta
	auto theta = atan2(y, x);
	if (theta < 0.0) {
		theta += CV_PI * 2.0;
	}

	// check if the scan value is valid
	for (auto area : *pInvalidAreaList) {
		// check theta
		if (area.theta_begin < area.theta_end) {
			if (theta < area.theta_begin || theta > area.theta_end) {
				continue;
			}
		}
		else {
			if (theta > area.theta_begin || theta < area.theta_end) {
				continue;
			}
		}

		// check radius
		if (r < area.r_begin || r > area.r_end) {
			continue;
		}

		// invalid
		return kInvalidValue;
	}

	// spline
	vector<double> thetaList = { theta };
	auto spline = splineList[r];
	auto valueList = spline->interpolate(thetaList);

	return (float)valueList[0];
}

auto DlgTopographyPattern::getValueAtVecPx(const vector<vector<float>>* pMapData, float radian,
	int radiusPx)->float
{
	auto x_on_data = radiusPx * cos(radian) + CX(pMapData);
	auto y_on_data = radiusPx * sin(radian) + CY(pMapData);

	return (*pMapData)[(int)y_on_data][(int)x_on_data];
}

auto DlgTopographyPattern::getMNAAtDistMeridian(const vector<vector<float>>* pMapData)->tuple<float, float, float, float, float>
{
	auto ptPerMM = (float)pMapData->size() / 8.f;

	auto radiusPxFrom = (int)(ptPerMM * kSimKRangeFrom);
	auto radiusPxTo = (int)(ptPerMM * kSimKRangeTo);

	// ellipsis fitting
	vector<cv::Point2f> ptList;
	for (auto degree = 0; degree < 360; degree++) {
		auto radian = (float)(CV_PI / 180.0 * (double)degree);

		vector<float> valueList;
		for (auto radius = radiusPxFrom; radius < radiusPxTo; radius++) {
			auto value = getValueAtVecPx(pMapData, radian, radius);
			valueList.push_back(value);
		}

		auto r = accumulate(valueList.begin(), valueList.end(), 0.0f) / (float)valueList.size();
		auto x = (float)(r * cos(radian));
		auto y = (float)(r * sin(radian));

		ptList.push_back(cv::Point2f(x, y));
	}

	auto retRect = cv::fitEllipse(ptList);
	auto retAngle = retRect.angle + 90;
	if (retAngle >= 180) {
		retAngle -= 180;
	}
	if (retAngle < 0) {
		retAngle += 180;
	}
	auto radianForR1 = (float)(CV_PI / 180.0f * (double)retAngle);
	auto radianForR2 = radianForR1 + (float)CV_PI / 2.0f;
	if (radianForR2 >(float)CV_PI) {
		radianForR2 -= (float)CV_PI;
	}

	auto radianForR1_o = (float)CV_PI;
	auto radianForR2_o = (float)CV_PI / 2.0f + (float)CV_PI;

	auto mmFrom = kSimKRangeFrom;
	auto mmTo = kSimKRangeTo;

	auto horzR = (getAvgByRange(pMapData, 0, 0, mmFrom, mmTo) + getAvgByRange(pMapData, radianForR1_o, radianForR1_o, mmFrom, mmTo)) / 2.0f;
	auto vertR = (getAvgByRange(pMapData, (float)CV_PI / 2.0f, (float)CV_PI / 2.0f, mmFrom, mmTo) + getAvgByRange(pMapData, radianForR2_o, radianForR2_o, mmFrom, mmTo)) / 2.0f;

	auto valueForR1 = retRect.size.height / 2.0f;
	auto valueForR2 = retRect.size.width / 2.0f;

	return make_tuple(valueForR1, valueForR2, radianForR1, horzR, vertR);
}

auto DlgTopographyPattern::getAvgByRange(const vector<vector<float>>* pMapData,
	float radianBegin, float radianEnd, float radiusMMBegin, float radiusMMEnd)->float
{
	vector<float> valueList;

	if (!getAllPtInRange(valueList, pMapData, radianBegin, radianEnd, radiusMMBegin, radiusMMEnd)) {
		return 0.0;
	}

	valueList = getInvalidDataRemovedValueList(valueList);
	if (valueList.size() == 0) {
		return 0.0f;
	}

	return accumulate(valueList.begin(), valueList.end(), 0.0f) / (float)valueList.size();
}

bool DlgTopographyPattern::getAllPtInRange(vector<float>& out_data,
	const vector<vector<float>>* pMapData, float radianBegin, float radianEnd,
	float radiusMMBegin, float radiusMMEnd)
{
	function<void(vector<float>&, const vector<vector<float>>*, int, int, float)> fnValuesForALine =
		[](vector<float>& out_data, const vector<vector<float>>* pMapData, int radiusPxFrom,
			int radiusPxTo, float radian) {
		out_data.clear();
		if (radiusPxFrom == radiusPxTo) {
			out_data.push_back(getValueAtVecPx(pMapData, radian, radiusPxFrom));
		}
		else {
			for (int r = radiusPxFrom; r < radiusPxTo; r++) {
				out_data.push_back(getValueAtVecPx(pMapData, radian, r));
			}
		}
	};

	auto ptPerMM = (float)pMapData->size() / 8.f;
	auto radiusPxBegin = (int)(ptPerMM * radiusMMBegin);
	auto radiusPxEnd = (int)(ptPerMM * radiusMMEnd);
	auto degBegin = (int)(radianBegin * 180.0 / CV_PI);
	auto degEnd = (int)(radianEnd * 180.0 / CV_PI);

	out_data.clear();
	if (degBegin == degEnd) {
		auto radian = (float)(degBegin * CV_PI / 180.0);

		vector<float> valueList;
		fnValuesForALine(valueList, pMapData, radiusPxBegin, radiusPxEnd, radian);

		out_data.insert(out_data.end(), valueList.begin(), valueList.end());
	}
	else {
		for (int deg = degBegin; deg < degEnd; deg++) {
			auto radian = (float)(deg * CV_PI / 180.0);

			vector<float> valueList;
			fnValuesForALine(valueList, pMapData, radiusPxBegin, radiusPxEnd, radian);

			out_data.insert(out_data.end(), valueList.begin(), valueList.end());
		}
	}

	return true;
}

auto DlgTopographyPattern::getInvalidDataRemovedValueList(vector<float> valueListRaw)->vector<float>
{
	vector<float> valueList;
	for (auto value : valueListRaw) {
		if (value > kInvalidValue) {
			valueList.push_back(value);
		}
	}

	return valueList;
}

bool DlgTopographyPattern::getAvgValueListInRangeMeridian(
	vector<pair<float, float>>& out_data, const vector<vector<float>>* pMapData,
	float radiusMMBegin, float radiusMMEnd)
{
	for (int deg = 0; deg < 180; deg++) {
		auto radian0 = (float)(CV_PI / 180.0 * (double)deg);
		auto radian1 = (float)(CV_PI / 180.0 * (double)(deg + 180));

		auto avgValue0 = 0.0f;
		{
			vector<float> valueList;
			if (getAllPtInRange(valueList, pMapData, radian0, radian0, radiusMMBegin, radiusMMEnd)) {
				valueList = getInvalidDataRemovedValueList(valueList);
				if (valueList.size() > 0) {
					avgValue0 = accumulate(valueList.begin(), valueList.end(), 0.0f) / (float)valueList.size();
				}
			}
		}

		auto avgValue1 = 0.0f;
		{
			vector<float> valueList;
			if (getAllPtInRange(valueList, pMapData, radian1, radian1, radiusMMBegin, radiusMMEnd)) {
				valueList = getInvalidDataRemovedValueList(valueList);
				if (valueList.size() > 0) {
					avgValue1 = accumulate(valueList.begin(), valueList.end(), 0.0f) / (float)valueList.size();
				}
			}
		}

		out_data.push_back(make_pair(radian0, (avgValue0 + avgValue1) / 2.0f));
	}

	return true;
}

void DlgTopographyPattern::calculateCurvature()
{
	if (Measure::isScannerResult()) {
		BeginWaitCursor();
		
		auto radialLineList = s_makeRadialLineList(m_curveRadius, m_lines, m_points);

		// check invalid areas
		vector<InvalidMapArea> invalidAreaList;
		s_getInvalidAreaList(invalidAreaList, &radialLineList);

		// make splines
		vector<CubicSpline*> splineList;
		s_makeSplineList(splineList, radialLineList);

		// fill the map
		auto margin = m_points / 2;
		vector<vector<float>> pMapData;

		for (int y = 0; y < m_points; y++) {
			vector<float> line;
			for (int x = 0; x < m_points; x++) {
				auto x_in_data = x - margin;
				auto y_in_data = y - margin;

				auto value = s_getValueAt(x_in_data, y_in_data, splineList,	&invalidAreaList);
				line.push_back(value);
			}
			pMapData.push_back(line);
		}

		// clear splines
		for (auto item : splineList) {
			delete item;
		}

		auto valueItem = getMNAAtDistMeridian(&pMapData);
		m_major = get<0>(valueItem);
		m_minor = get<1>(valueItem);
		m_degreeMajor = get<2>(valueItem) * (180.0f / (float)CV_PI);
		m_degreeMinor = m_degreeMajor + 90.0f;
		m_horzR = get<3>(valueItem);
		m_vertR = get<4>(valueItem);

		if (m_degreeMinor >= 180.0f) {
			m_degreeMinor -= 180.0f;
		}
		
		m_valueAvgK = 0.0f;
		{
			vector<pair<float, float>> itemList;
			if (getAvgValueListInRangeMeridian(itemList, &pMapData, kTopoCenterRange / 2.0, kRangeForSimK)) {
				list<float> valueList;
				for (auto item : itemList) {
					valueList.push_back(get<1>(item));
				}

				m_valueAvgK = std::accumulate(valueList.begin(), valueList.end(), 0.0f) / valueList.size();
			}
			else {
				m_valueAvgK = 0.0f;
			}
		}

		auto value = vector<double>(3, 0);
		auto offsets = vector<double>(4, 0);

		vector<double> modelEyeR = GlobalSettings::topoModelData();
		vector<double> measuredR = GlobalSettings::topoMeasuredData();

		offsets[0] = measuredR[0] - modelEyeR[0];
		offsets[1] = measuredR[1] - modelEyeR[1];
		offsets[2] = measuredR[2] - modelEyeR[2];
		offsets[3] = measuredR[3] - modelEyeR[3];

		value[0] = m_major;
		value[1] = m_minor;
		value[2] = m_valueAvgK;

		CubicSpline spline;
		spline.setPoints(modelEyeR, offsets);

		auto offset = spline.interpolate(value);

		if (!isnan(offset[0]))
			m_major -= offset[0];
		if (!isnan(offset[1]))
			m_minor -= offset[1];
		if (!isnan(offset[2]))
			m_valueAvgK -= offset[2]; 

		//auto mmToPoint = m_points / m_rangeX;
		//float meanCurves = 0;
		//m_curvatureInfo.resize(m_lines);

		//for (int i = 0; i < m_curveRadius.size(); i++) {
		//	auto curve = m_curveRadius[i];

		//	for (int j = mmToPoint * 1.5; j < mmToPoint * 6.5; j++) {
		//		meanCurves += curve[j];
		//	}
		//	/*for (int j = mmToPoint * 5.5; j < mmToPoint * 7; j++) {
		//		meanCurves += curve[j];
		//	}*/

		//	meanCurves /= mmToPoint * 5;

		//	m_curvatureInfo[i] = meanCurves;
		//}
	 
		EndWaitCursor();
	}

	return;
}


void DlgTopographyPattern::OnBnClickedButtonResultExport()
{
	BeginWaitCursor();
	int count = Measure::exportScannerResults();
	CWnd* pWndDesktop = GetDesktopWindow();
	CWindowDC ScrDC(pWndDesktop);
	CClientDC dc(this);

	CRect Rect;
	GetClientRect(&Rect);
	CWnd::GetWindowRect(&Rect);

	CImage Image;
	int sx = Rect.left + 8;
	int sy = Rect.top;
	int cx = Rect.Width() - 15;
	int cy = Rect.Height() - 7;

	Image.Create(cx, cy, ScrDC.GetDeviceCaps(BITSPIXEL));
	CDC* pDC = CDC::FromHandle(Image.GetDC());
	pDC->BitBlt(0, 0, cx, cy, &ScrDC, sx, sy, SRCCOPY);
	Image.ReleaseDC();

	CString filePath;
	CTime data = CTime::GetCurrentTime();
	filePath.Format(_T("Screen_%d-%02d-%d.jpg"),
		data.GetYear(), data.GetMonth(), data.GetDay());

	Image.Save(filePath, Gdiplus::ImageFormatJPEG);

	EndWaitCursor();

	CString text;
	text.Format(_T("Screen Capture Succeed!"));
	AfxMessageBox(text);

	return;
}


void DlgTopographyPattern::OnBnClickedButtonGalvApply()
{
	// TODO: Add your control notification handler code here
	CString text;
	float timeStep;
	uint32_t timeDelay;
	uint32_t forePadds;
	uint32_t postPadds;

	GetDlgItemText(IDC_EDIT_TIME_STEP, text);
	if (!text.IsEmpty()) {
		timeStep = (float)_ttof(text);
		Scanner::setTriggerTimeStep(timeStep);
	}
	GetDlgItemText(IDC_EDIT_TIME_DELAY, text);
	if (!text.IsEmpty()) {
		timeDelay = _ttoi(text);
		Scanner::setTriggerTimeDelay(timeDelay);
	}
	GetDlgItemText(IDC_EDIT_FORE_PADDS, text);
	if (!text.IsEmpty()) {
		forePadds = _ttoi(text);
		Scanner::setTriggerForePaddings(m_speed, forePadds);
	}
	GetDlgItemText(IDC_EDIT_POST_PADDS, text);
	if (!text.IsEmpty()) {
		postPadds = _ttoi(text);
		Scanner::setTriggerPostPaddings(m_speed, postPadds);
	}

	return;
}


void DlgTopographyPattern::OnBnClickedButtonGalvExport()
{
	auto scanner = Controller::getScanner();
	scanner->exportGalvanoPositions();

	return;
}


void DlgTopographyPattern::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	Controller::cancelAutoControls();
	Controller::closeScan(false);

	showIrCameras(false);
	CDialogEx::OnClose();

	return;
}


void DlgTopographyPattern::OnBnClickedBtnAutocenter()
{
	// TODO: Add your control notification handler code here
	if (enfaceView.IsEmptyImage()) {
		AfxMessageBox(_T("Enface Image does not exist."));
		return;
	}

	auto pBits = enfaceView.getImageBitData();
	auto height = enfaceView.getImageHeight();
	auto width = enfaceView.getImageWidth();
	auto padding = 0;
	float pixelPerMM = m_rangeX / (float)m_points;

	Mat src, dst;
	src = Mat(height, width, CV_8UC1, (void*)pBits, (size_t)(padding == 0 ? Mat::AUTO_STEP : padding));
	cv::flip(src, src, 0);

	cv::resize(src, src, Size(m_points, m_points), INTER_CUBIC);

	equalizeHist(src, dst);
	cv::threshold(dst, dst, cv::mean(dst)[0] * 0.5, 255, CV_THRESH_BINARY_INV);
	distanceTransform(dst, dst, DIST_L2, 3);

	double radius;
	int maxIdx[2];
	minMaxIdx(dst, NULL, &radius, NULL, maxIdx);

	auto distX = (dst.cols / 2 - maxIdx[1]) * pixelPerMM;
	auto distY = (dst.cols / 2 - maxIdx[0]) * pixelPerMM;

	distX = distX * -1;
	distY = distY * -1;

	CString text;

	// Offset
	float offsetX, offsetY;
	editOffsetX.GetWindowText(text);
	offsetX = (float)(text.IsEmpty() ? 0.0f : _ttof(text));
	editOffsetY.GetWindowText(text);
	offsetY = (float)(text.IsEmpty() ? 0.0f : _ttof(text));

	distX = offsetX + distX;
	distY = offsetY + distY;

	text.Format(_T("%.2f"), distX);
	editOffsetX.SetWindowTextW(text);
	text.Format(_T("%.2f"), distY);
	editOffsetY.SetWindowTextW(text);

	// Scale
	float scaleX, scaleY;
	editScaleX.GetWindowText(text);
	scaleX = (float)(text.IsEmpty() ? 0.0f : _ttof(text));
	editScaleY.GetWindowText(text);
	scaleY = (float)(text.IsEmpty() ? 0.0f : _ttof(text));

	pixelPerMM = (m_rangeX * scaleX) / (float)m_points;
	auto ratio = (radius + 8) * pixelPerMM / 0.5f;

	text.Format(_T("%.2f"), ratio);
	editScaleX.SetWindowTextW(text);
	text.Format(_T("%.2f"), ratio);
	editScaleY.SetWindowTextW(text);

	return;
}


void DlgTopographyPattern::OnBnClickedBtnOffSclSave()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();

	float x, y;
	CString text;
	PatternSettings* pset = SystemProfile::getPatternSettings();

	/*	int index;
	UpdateData(TRUE);

	switch (m_cameraSpeed) {
	case 0:
		index = 0;

		break;
	case 1:
		index = 1;

		break;
	case 2:
		index = 2;

		break;
	}*/

	for (int i = 0; i < 3; i++) {
		editScaleX.GetWindowText(text);
		x = min(max((float)_ttof(text), 0.1f), 2.0f);
		editScaleY.GetWindowText(text);
		y = min(max((float)_ttof(text), 0.1f), 2.0f);
		pset->topographyPatternScale(i) = std::pair<float, float>(x, y);

		editOffsetX.GetWindowText(text);
		x = min(max((float)_ttof(text), -10.0f), +10.0f);
		editOffsetY.GetWindowText(text);
		y = min(max((float)_ttof(text), -10.0f), +10.0f);
		pset->topographyPatternOffset(i) = std::pair<float, float>(x, y);
	}
	
	if (Loader::saveSetupProfile(false, false)) {
		Loader::applySystemProfile(true);
		AfxMessageBox(_T("Topo settings saved to mainboard!"));
	}
	else {
		AfxMessageBox(_T("Failed to save Topo settings to mainboard!"));
	}
	EndWaitCursor();

	return;
}


void DlgTopographyPattern::OnBnClickedBtnRefresh()
{
	// TODO: Add your control notification handler code here
	OctSystem::Loader::loadTopoCalibration();

	return;
}

BOOL DlgTopographyPattern::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL) {
		}
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void DlgTopographyPattern::OnDeltaposSpinBscan(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int pos = pNMUpDown->iPos + pNMUpDown->iDelta;
	sldPattern.SetPos(pos);

	showPatternImageSelected();
	*pResult = 0;
}


void DlgTopographyPattern::OnBnClickedBtnAutoFocus()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		if (Controller::isAutoOptimizing()) {
			Controller::cancelAutoOptimize();
		}
		else {
			Controller::startAutoOptimize(true);
			//Controller::startAutoPolarization();
		}
	}
	return;
}


void DlgTopographyPattern::OnBnClickedBtnModeleyesave()
{
	if (Measure::isScannerResult()) {
		BeginWaitCursor();

		CString text;
		auto modelEyeR = vector<double>(4, 0);
		auto measuredR = vector<double>(4, 0);

		editModeleyeR1.GetWindowTextW(text);
		modelEyeR[0] = _ttof(text);
		
		editModeleyeR2.GetWindowTextW(text);
		modelEyeR[1] = _ttof(text);

		editModeleyeR3.GetWindowTextW(text);
		modelEyeR[2] = _ttof(text);

		editModeleyeR4.GetWindowTextW(text);
		modelEyeR[3] = _ttof(text);

		editMeasuredModeleyeR1.GetWindowTextW(text);
		measuredR[0] = _ttof(text);

		editMeasuredModeleyeR2.GetWindowTextW(text);
		measuredR[1] = _ttof(text);

		editMeasuredModeleyeR3.GetWindowTextW(text);
		measuredR[2] = _ttof(text);

		editMeasuredModeleyeR4.GetWindowTextW(text);
		measuredR[3] = _ttof(text);

		wstring path, name;

		if ((_waccess(_T(".\\Calibration"), 0)) == -1)
			CreateDirectory(_T(".\\Calibration"), NULL);

		name = (boost::wformat(L"setting")).str();
		path = (boost::wformat(L"./Calibration//%s.json") % name).str();
		
		using json = nlohmann::json;
		json j;

		j["model"] = modelEyeR;
		j["measured"] = measuredR;

		try {
			std::ofstream f(path);
			f << j.dump();
			f.close();
		}
		catch (...) {
			return;
		}
		text.Format(_T("Topography offset saved!"));
		AfxMessageBox(text);
	}
	return;
}
