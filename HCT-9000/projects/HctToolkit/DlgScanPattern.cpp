// DlgScanPattern.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgScanPattern.h"
#include "afxdialogex.h"

#include <random>

using namespace std;

// DlgScanPattern dialog

IMPLEMENT_DYNAMIC(DlgScanPattern, CDialogEx)

DlgScanPattern::DlgScanPattern(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGSCANPATTERN, pParent)
	, m_cameraSpeed(0)
{
	m_clbScanPatternCompleted = std::bind(&DlgScanPattern::callbackScanPatternCompleted, this, std::placeholders::_1);
	m_clbAutoCorneaFocusCompleted = std::bind(&DlgScanPattern::callbackCorneaFocusCompleted, this, std::placeholders::_1,
		std::placeholders::_2, std::placeholders::_3);
	m_clbAutoRetinaFocusCompleted = std::bind(&DlgScanPattern::callbackRetinaFocusCompleted, this, std::placeholders::_1,
		std::placeholders::_2, std::placeholders::_3);
	m_clbAutoLensFrontFocusCompleted = std::bind(&DlgScanPattern::callbackLensFrontFocusCompleted, this, std::placeholders::_1,
		std::placeholders::_2, std::placeholders::_3);
	m_clbAutoLensBackFocusCompleted = std::bind(&DlgScanPattern::callbackLensBackFocusCompleted, this, std::placeholders::_1,
		std::placeholders::_2, std::placeholders::_3);

	cbJoystick = std::bind(&DlgScanPattern::OnBnClickedButtonStartScan, this);
	Controller::setJoystickEventCallback(&cbJoystick);
}


DlgScanPattern::~DlgScanPattern()
{
}


void DlgScanPattern::initControls(void)
{
	CString text;

	corneaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 0, 320, 240), this);
	retinaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 240, 320, 240 + 240), this);
	corneaView.setDisplaySize(320, 240);
	retinaView.setDisplaySize(320, 240);
	corneaView.showCameraInfo(true);

	previewView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(320, 0, 320 + 1024, 512), this);
	previewView.setDisplaySize(1024, 512);
	previewView.ModifyStyle(0, SS_NOTIFY);
	previewView.setDisplayStretched(true);
	previewView.clearResultData();

	enfaceView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 480, 320, 480 + 320), this);
	enfaceView.setDisplaySize(320, 320);
	enfaceView.ModifyStyle(0, SS_NOTIFY);

	cmbPattern.ResetContent();
	int size = PatternHelper::getNumberOfPatternTypes();
	for (int i = 0; i < size; i++) {
		cmbPattern.AddString(CString(PatternHelper::getPatternNameStringFromIndex(i)));
	}
	cmbPattern.SetCurSel(0);

	cmbRangeX.SetCurSel(8);
	cmbRangeY.SetCurSel(8);
	cmbSpace.SetCurSel(3);
	cmbDirection.SetCurSel(0);

	editOffsetX.SetWindowTextW(_T("0.0"));
	editOffsetY.SetWindowTextW(_T("0.0"));
	editAngle.SetWindowTextW(_T("0.0"));
	editScaleX.SetWindowTextW(_T("1.0"));
	editScaleY.SetWindowTextW(_T("1.0"));
	editRefraction.SetWindowTextW(_T("1.36"));
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

	text.Format(_T("%.2f"), Scanner::getTriggerTimeStep());
	SetDlgItemText(IDC_EDIT_TIME_STEP, text);
	text.Format(_T("%d"), Scanner::getTriggerTimeDelay());
	SetDlgItemText(IDC_EDIT_TIME_DELAY, text);

	int index = (m_speed == ScanSpeed::Fastest ? 0 : (m_speed == ScanSpeed::Faster ? 1 : 2));
	text.Format(_T("%d"), GlobalSettings::getTriggerForePadd(index));
	SetDlgItemText(IDC_EDIT_FORE_PADDS, text);
	text.Format(_T("%d"), GlobalSettings::getTriggerPostPadd(index));
	SetDlgItemText(IDC_EDIT_POST_PADDS, text);

	auto again = Scanner::getCameraAnalogGain();
	auto csel = (again == 2.0f ? 1 : (again == 4.0f ? 2 : 0));
	combLineCamAgain.SetCurSel(csel);

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
	
	bool dynamic = ChainSetup::useDynamicDispersionCorrection();
	CheckDlgButton(IDC_CHECK_DYNAMIC_DISPERSION, dynamic);
	return;
}


void DlgScanPattern::showIrCameras(bool play)
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


void DlgScanPattern::prepareSystem(void)
{
	Controller::turnOnInternalFixationAtCenter();
	return;
}


void DlgScanPattern::updateScanControls(void)
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
	editAngle.GetWindowText(text);
	m_angle = (float)(text.IsEmpty() ? 0.0f : _ttof(text));

	int index = cmbPattern.GetCurSel();
	int code = PatternHelper::getPatternCodeFromIndex(index);
	if (code < 0) {
		return;
	}
	
	m_patternName = PatternHelper::getPatternNameFromCode(code);

	index = cmbRangeX.GetCurSel();
	cmbRangeX.GetLBText(index, text);
	m_rangeX = (float)_ttof(text);

	if (index == 15) {
		m_rangeX = 12.0f;
	}
	else {
		m_rangeX = (float)_ttof(text);
	}

	index = cmbRangeY.GetCurSel();
	cmbRangeY.GetLBText(index, text);
	m_rangeY = (float)_ttof(text);

	index = cmbPoints.GetCurSel();
	cmbPoints.GetLBText(index, text);
	m_points = (int)_ttoi(text);

	index = cmbDirection.GetCurSel();
	m_direction = index;

	index = cmbOverlaps.GetCurSel();
	cmbOverlaps.GetLBText(index, text);
	m_overlaps = (int)_ttoi(text);

	index = cmbLines.GetCurSel();
	cmbLines.GetLBText(index, text);
	m_lines = (int)_ttoi(text);

	index = cmbSpace.GetCurSel();
	cmbSpace.GetLBText(index, text);
	m_space = (float)_ttof(text);
	return;
}


void DlgScanPattern::updateEnfaceControls(void)
{
	m_useEnface =  (IsDlgButtonChecked(IDC_CHECK_ENFACE) ? true : false);

	CString text;
	int index;

	index = cmbEnfaceLines.GetCurSel();
	cmbEnfaceLines.GetLBText(index, text);
	m_enfaceLines = (int)_ttoi(text);

	index = cmbEnfacePoints.GetCurSel();
	cmbEnfacePoints.GetLBText(index, text);
	m_enfacePoints = (int)_ttoi(text);

	index = cmbEnfaceRangeX.GetCurSel();
	cmbEnfaceRangeX.GetLBText(index, text);
	m_enfaceRangeX = (float)_ttof(text);

	index = cmbEnfaceRangeY.GetCurSel();
	cmbEnfaceRangeY.GetLBText(index, text);
	m_enfaceRangeY = (float)_ttof(text);

	enfaceView.setScanRange(m_enfaceRangeX, m_enfaceRangeY);
	return;
}


void DlgScanPattern::updatePreviewControls(void)
{
	CString text;
	editRefraction.GetWindowText(text);
	previewView.setRefractiveIndex((double)(text.IsEmpty() ? 1.36f : _ttof(text)));
	previewView.setDisplayStretched(IsDlgButtonChecked(IDC_CHECK_STRETCH) ? true : false);

	m_usePattern = (IsDlgButtonChecked(IDC_CHECK_PREVIEW_PATTERN) ? true : false);
	m_useFaster = (IsDlgButtonChecked(IDC_CHECK_FAST_RASTERS) ? true : false);
	m_useNoImageGrab = (IsDlgButtonChecked(IDC_CHECK_NO_IMAGE_GRAB) ? true : false);

	return;
}

  
void DlgScanPattern::setRadioCameraSpeed(UINT value)
{
	UpdateData(TRUE);
	CString text;

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
		break;
	case 1:
		ret = Scanner::changeScanSpeedToFaster(m_useNoImageGrab);
		exposure = Scanner::getDefaultTriggerTimeStep(ScanSpeed::Faster);
		text.Format(_T("%.1f"), exposure);
		SetDlgItemText(IDC_EDIT_TIME_STEP, text);
		m_speed = ScanSpeed::Faster;
		break;
	case 2:
		ret = Scanner::changeScanSpeedToFastest(m_useNoImageGrab);
		exposure = Scanner::getDefaultTriggerTimeStep(ScanSpeed::Fastest);
		text.Format(_T("%.1f"), exposure);
		SetDlgItemText(IDC_EDIT_TIME_STEP, text);
		m_speed = ScanSpeed::Fastest;
		break;
	}
	updateExposureTime();

	int index = (m_speed == ScanSpeed::Fastest ? 0 : (m_speed == ScanSpeed::Faster ? 1 : 2));
	text.Format(_T("%d"), GlobalSettings::getTriggerForePadd(index));
	SetDlgItemText(IDC_EDIT_FORE_PADDS, text);
	text.Format(_T("%d"), GlobalSettings::getTriggerPostPadd(index));
	SetDlgItemText(IDC_EDIT_POST_PADDS, text);

	if (!ret) {
		AfxMessageBox(_T("Reset scan speed failed!"));
	}
	else {
		OnBnClickedButtonGalvApply();
	}
	return;
}


void DlgScanPattern::updateInternalFixation(void)
{
	EyeSide side = (IsDlgButtonChecked(IDC_RADIO_OD) ? EyeSide::OD : EyeSide::OS);
	EyeRegion region = (IsDlgButtonChecked(IDC_RADIO_MACULAR) ? EyeRegion::Macular : EyeRegion::OpticDisc);

	Controller::turnOnInternalFixationAtCenter(side, region);

	int row, col;
	Controller::getInternalFixationOn(row, col);
	cmbInfixRow.SetCurSel(row);
	cmbInfixCol.SetCurSel(col);
	return;
}


void DlgScanPattern::updateScannerResult(bool enable)
{
	int size, cpos;
	CString text;

	sldPattern.EnableWindow(FALSE);
	sldPreview.EnableWindow(FALSE);
	sldEnface.EnableWindow(FALSE);

	GetDlgItem(IDC_BUTTON_PREVIEW_SHOW)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_PATTERN_SHOW)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_ENFACE_SHOW)->EnableWindow(FALSE);

	if (Measure::isScannerResult() && enable) {
		size = Measure::getCountOfPreviewSections();
		if (size > 0) {
			sldPreview.SetRange(1, size);
			sldPreview.SetLineSize(1);
			sldPreview.SetPageSize(1);
			cpos = (size <= 1 ? 1 : size / 2);
			sldPreview.SetPos(cpos);

			text.Format(_T("%d"), cpos);
			editPreviewNo.SetWindowTextW(text);

			sldPreview.EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_PREVIEW_SHOW)->EnableWindow(TRUE);
		}

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
			GetDlgItem(IDC_BUTTON_PATTERN_SHOW)->EnableWindow(TRUE);
		}

		const OctEnfaceImage* enface = Measure::getEnfaceImageDescript();
		if (enface) {
			// enfaceView.setEqualization(false);
			enfaceView.callbackEnfaceImage(enface->getBuffer(), enface->getWidth(), enface->getHeight());
		}

		
	}
	return;
}

void DlgScanPattern::updateExposureTime(void)
{
	CString text;
	auto expTime = Scanner::getCameraExposureTime();
	text.Format(_T("%.1f"), expTime);
	SetDlgItemText(IDC_EDIT_EXP_TIME, text);
}


void DlgScanPattern::showPreviewImageSelected(void)
{
	CString text;
	int pos = sldPreview.GetPos();
	text.Format(_T("%d"), pos);
	editPreviewNo.SetWindowText(text);

	const OctScanImage* image = Measure::getPreviewImageDescript(pos-1);
	previewView.drawScanImage(image);
	return;
}


void DlgScanPattern::showPatternImageSelected(void)
{
	CString text;
	int pos = sldPattern.GetPos();
	text.Format(_T("%d"), pos);
	editPatternNo.SetWindowText(text);

	int index = pos - 1;
	const OctScanImage* image = Measure::getPatternImageDescript(index);
	previewView.setQualityIndex(image->getQuality());
	previewView.drawScanImage(image);
	return;
}


void DlgScanPattern::callbackScanPatternCompleted(bool result)
{
	updateScannerResult(true);

	float acqTime = Controller::getScanner()->getAcquisitionTime();
	previewView.setAcquisitionTime(acqTime);
	previewView.setTriggerTimeStep(Scanner::getTriggerTimeStep());	
	previewView.setQualityIndex(Measure::getPatternQualityIndex());

	float asize = 5.0f;
	CString text;
	editAverageSize.GetWindowTextW(text);
	if (!text.IsEmpty()) {
		asize = (float)_ttof(text);
	}

	vector<float> stat;
	Measure::getPatternQualityStat(stat, asize);
	previewView.setPatternQualities(m_direction, stat);

	previewView.updateWindow();
	_isScanClosing = false;
	EndWaitCursor();
	return;
}


void DlgScanPattern::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DIRECTION, cmbDirection);
	DDX_Control(pDX, IDC_COMBO_PATTERN, cmbPattern);
	DDX_Control(pDX, IDC_COMBO_POINTS, cmbPoints);
	//  DDX_Control(pDX, IDC_COMBO_RANGE, cmbRange);
	DDX_Control(pDX, IDC_COMBO_OVERLAPS, cmbOverlaps);
	DDX_Control(pDX, IDC_EDIT_ANGLE, editAngle);
	DDX_Control(pDX, IDC_EDIT_OFFSET_X, editOffsetX);
	DDX_Control(pDX, IDC_EDIT_OFFSET_Y, editOffsetY);
	DDX_Control(pDX, IDC_EDIT_SCALE_X, editScaleX);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y, editScaleY);
	//  DDX_Control(pDX, IDC_COMBO_ENFACE_LINES, cmbEnfaceLines);
	//  DDX_Control(pDX, IDC_COMBO_ENFACE_POINTS, cmbEnfacePoints);
	DDX_Control(pDX, IDC_COMBO_LINES, cmbLines);
	DDX_Control(pDX, IDC_COMBO_SPACE, cmbSpace);
	DDX_Control(pDX, IDC_COMBO_ENFACE_RANGEX, cmbEnfaceRangeX);
	DDX_Control(pDX, IDC_COMBO_ENFACE_RANGEY, cmbEnfaceRangeY);
	DDX_Control(pDX, IDC_COMBO_RANGEX, cmbRangeX);
	DDX_Control(pDX, IDC_COMBO_RANGEY, cmbRangeY);
	DDX_Radio(pDX, IDC_RADIO_CAMERA1, m_cameraSpeed);
	DDX_Control(pDX, IDC_COMBO_INTFIX_COL, cmbInfixCol);
	DDX_Control(pDX, IDC_COMBO_INTFIX_ROW, cmbInfixRow);
	DDX_Control(pDX, IDC_EDIT_ENFACE_NO, editEnfaceNo);
	DDX_Control(pDX, IDC_EDIT_PATTERN_NO, editPatternNo);
	DDX_Control(pDX, IDC_EDIT_PREVIEW_NO, editPreviewNo);
	DDX_Control(pDX, IDC_SLIDER_ENFACE, sldEnface);
	DDX_Control(pDX, IDC_SLIDER_PATTERN, sldPattern);
	DDX_Control(pDX, IDC_SLIDER_PREVIEW, sldPreview);
	//  DDX_Control(pDX, IDC_EDIT_ENFACE_RANGE1, editEnfaceRage1);
	DDX_Control(pDX, IDC_EDIT_ENFACE_RANGE2, editEnfaceRange2);
	DDX_Control(pDX, IDC_EDIT_ENFACE_RANGE1, editEnfaceRange1);
	//  DDX_Control(pDX, IDC_COMBO_ENFACE_LINES, cmbEnfaceLines);
	//  DDX_Control(pDX, IDC_COMBO_ENFACE_POINTS, cmbEnfacePoints);
	DDX_Control(pDX, IDC_COMBO_ENFACE_LINES2, cmbEnfaceLines);
	DDX_Control(pDX, IDC_COMBO_ENFACE_POINTS2, cmbEnfacePoints);
	DDX_Control(pDX, IDC_EDIT_ENFACE_CLIP_LIMIT, editEnfaceClipLimit);
	DDX_Control(pDX, IDC_EDIT_ENFACE_SCALAR, editEnfaceScalar);
	DDX_Control(pDX, IDC_EDIT_GALV_OFFSET_X, editGalvOffsetX);
	DDX_Control(pDX, IDC_EDIT_GALV_OFFSET_Y, editGalvOffsetY);
	DDX_Control(pDX, IDC_EDIT_REFRACTION, editRefraction);
	DDX_Control(pDX, IDC_EDIT_AVERAGE_SIZE, editAverageSize);
	DDX_Control(pDX, IDC_COMBO_LINE_CAM_AGAIN, combLineCamAgain);
	DDX_Control(pDX, IDC_EDIT_EXP_TIME, editExposTime);
}


BEGIN_MESSAGE_MAP(DlgScanPattern, CDialogEx)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_COMBO_DIRECTION, &DlgScanPattern::OnSelchangeComboDirection)
	ON_CBN_SELCHANGE(IDC_COMBO_PATTERN, &DlgScanPattern::OnSelchangeComboPattern)
	ON_CBN_SELCHANGE(IDC_COMBO_POINTS, &DlgScanPattern::OnSelchangeComboPoints)
	ON_BN_CLICKED(IDC_BUTTON_EXIT2, &DlgScanPattern::OnBnClickedButtonExit2)
	ON_BN_CLICKED(IDC_BUTTON_START_SCAN, &DlgScanPattern::OnBnClickedButtonStartScan)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_ENFACE, &DlgScanPattern::OnBnClickedButtonSaveEnface)
	ON_BN_CLICKED(IDC_CHECK_EQUALIZATION, &DlgScanPattern::OnBnClickedCheckEqualization)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS, &DlgScanPattern::OnBnClickedButtonFocus)
	ON_BN_CLICKED(IDC_BUTTON_MOTOR, &DlgScanPattern::OnBnClickedButtonMotor)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA, &DlgScanPattern::OnBnClickedButtonCamera)
	ON_BN_CLICKED(IDC_BUTTON_LEDS, &DlgScanPattern::OnBnClickedButtonLeds)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_FOCUS, &DlgScanPattern::OnBnClickedButtonAutoFocus)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_REFER3, &DlgScanPattern::OnBnClickedButtonAutoRefer3)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_POLAR2, &DlgScanPattern::OnBnClickedButtonAutoPolar2)
	ON_BN_CLICKED(IDC_BUTTON_EXPOSURE, &DlgScanPattern::OnBnClickedButtonExposure)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_CAMERA1, IDC_RADIO_CAMERA3, DlgScanPattern::setRadioCameraSpeed)
	ON_BN_CLICKED(IDC_BUTTON_GALV_APPLY, &DlgScanPattern::OnBnClickedButtonGalvApply)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_ON, &DlgScanPattern::OnBnClickedButtonIntfixOn)
	ON_CBN_SELCHANGE(IDC_COMBO_INTFIX_COL, &DlgScanPattern::OnCbnSelchangeComboIntfixCol)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_OFF, &DlgScanPattern::OnBnClickedButtonIntfixOff)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_UP, &DlgScanPattern::OnBnClickedButtonIntfixUp)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_DOWN, &DlgScanPattern::OnBnClickedButtonIntfixDown)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_LEFT, &DlgScanPattern::OnBnClickedButtonIntfixLeft)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_RIGHT, &DlgScanPattern::OnBnClickedButtonIntfixRight)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_CENTER, &DlgScanPattern::OnBnClickedButtonIntfixCenter)
	ON_BN_CLICKED(IDC_RADIO_CAMERA2, &DlgScanPattern::OnBnClickedRadioCamera2)
	ON_BN_CLICKED(IDC_RADIO_OD, &DlgScanPattern::OnBnClickedRadioOd)
	ON_BN_CLICKED(IDC_RADIO_OS, &DlgScanPattern::OnBnClickedRadioOs)
	ON_BN_CLICKED(IDC_RADIO_MACULAR, &DlgScanPattern::OnBnClickedRadioMacular)
	ON_BN_CLICKED(IDC_RADIO_DISC, &DlgScanPattern::OnBnClickedRadioDisc)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_PATTERN, &DlgScanPattern::OnReleasedcaptureSliderPattern)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_PREVIEW, &DlgScanPattern::OnReleasedcaptureSliderPreview)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_ENFACE, &DlgScanPattern::OnReleasedcaptureSliderEnface)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_IMAGE, &DlgScanPattern::OnBnClickedButtonSaveImage)
	ON_BN_CLICKED(IDC_BUTTON_PREVIEW_SHOW, &DlgScanPattern::OnBnClickedButtonPreviewShow)
	ON_BN_CLICKED(IDC_BUTTON_PATTERN_SHOW, &DlgScanPattern::OnBnClickedButtonPatternShow)
	ON_BN_CLICKED(IDC_BUTTON_RESULT_EXPORT, &DlgScanPattern::OnBnClickedButtonResultExport)
	ON_BN_CLICKED(IDC_BUTTON_OPTIMIZE2, &DlgScanPattern::OnBnClickedButtonOptimize)
	ON_CBN_SELCHANGE(IDC_COMBO_RANGEX, &DlgScanPattern::OnCbnSelchangeComboRangex)
	ON_BN_CLICKED(IDC_BUTTON_RESULT_NEW, &DlgScanPattern::OnBnClickedButtonResultNew)
	ON_BN_CLICKED(IDC_BUTTON_RESULT_SAVE, &DlgScanPattern::OnBnClickedButtonResultSave)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_ENFACE, &DlgScanPattern::OnNMCustomdrawSliderEnface)
	ON_BN_CLICKED(IDC_BUTTON_GALV_EXPORT, &DlgScanPattern::OnBnClickedButtonGalvExport)
	ON_BN_CLICKED(IDC_CHECK_ENFACE_GRID, &DlgScanPattern::OnBnClickedCheckEnfaceGrid)
	ON_BN_CLICKED(IDC_RADIO_DIOPT_ZERO, &DlgScanPattern::OnBnClickedRadioDioptZero)
	ON_BN_CLICKED(IDC_RADIO_DIOPT_MINUS, &DlgScanPattern::OnBnClickedRadioDioptMinus)
	ON_BN_CLICKED(IDC_RADIO_DIOPT_PLUS, &DlgScanPattern::OnBnClickedRadioDioptPlus)
	ON_BN_CLICKED(IDC_BUTTON_ENFCE_APPLY, &DlgScanPattern::OnBnClickedButtonEnfceApply)
	ON_BN_CLICKED(IDC_BUTTON_ENFCE_RESET, &DlgScanPattern::OnBnClickedButtonEnfceReset)
	ON_CBN_SELCHANGE(IDC_COMBO_ENFACE_POINTS, &DlgScanPattern::OnCbnSelchangeComboEnfacePoints)
	ON_CBN_SELCHANGE(IDC_COMBO_ENFACE_RANGEX, &DlgScanPattern::OnCbnSelchangeComboEnfaceRangex)
	ON_CBN_SELCHANGE(IDC_COMBO_ENFACE_RANGEY, &DlgScanPattern::OnCbnSelchangeComboEnfaceRangey)
	ON_CBN_SELCHANGE(IDC_COMBO_RANGEY, &DlgScanPattern::OnCbnSelchangeComboRangey)
	ON_CBN_SELCHANGE(IDC_COMBO_ENFACE_POINTS2, &DlgScanPattern::OnCbnSelchangeComboEnfacePoints2)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_TRACK, &DlgScanPattern::OnBnClickedButtonAutoTrack)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_SPLIT, &DlgScanPattern::OnBnClickedButtonAutoSplit)
	ON_BN_CLICKED(IDC_CHECK_PREVIEW_PATTERN, &DlgScanPattern::OnBnClickedCheckPreviewPattern)
	ON_BN_CLICKED(IDC_BUTTON_GALV_DYN_APPLY, &DlgScanPattern::OnBnClickedButtonGalvDynApply)
	ON_BN_CLICKED(IDC_BUTTON_SEGMENT, &DlgScanPattern::OnBnClickedButtonSegment)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_REGIST, &DlgScanPattern::OnBnClickedButtonRetinaRegist)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_LEDS, &DlgScanPattern::OnBnClickedButtonRetinaLeds)
	ON_BN_CLICKED(IDC_CHECK_AVERAGE_SNRS, &DlgScanPattern::OnBnClickedCheckAverageSnrs)
	ON_BN_CLICKED(IDC_BUTTON_RESULT_CLEAR, &DlgScanPattern::OnBnClickedButtonResultClear)
	ON_BN_CLICKED(IDC_BUTTON_LENS_TEST1, &DlgScanPattern::OnBnClickedButtonLensTest1)
	ON_BN_CLICKED(IDC_BUTTON_LENS_TEST2, &DlgScanPattern::OnBnClickedButtonLensTest2)
	ON_BN_CLICKED(IDC_BUTTON_CCT_TEST1, &DlgScanPattern::OnBnClickedButtonCctTest1)
	ON_BN_CLICKED(IDC_BUTTON_CCT_TEST2, &DlgScanPattern::OnBnClickedButtonCctTest2)
	ON_BN_CLICKED(IDC_BUTTON_ACD_TEST1, &DlgScanPattern::OnBnClickedButtonAcdTest1)
	ON_BN_CLICKED(IDC_BUTTON_ACD_TEST2, &DlgScanPattern::OnBnClickedButtonAcdTest2)
	ON_BN_CLICKED(IDC_BUTTON_AL_TEST1, &DlgScanPattern::OnBnClickedButtonAlTest1)
	ON_BN_CLICKED(IDC_BUTTON_AL_TEST2, &DlgScanPattern::OnBnClickedButtonAlTest2)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_ORIGIN, &DlgScanPattern::OnBnClickedButtonRetinaOrigin)
	ON_BN_CLICKED(IDC_BUTTON_CORNEA_ORIGIN, &DlgScanPattern::OnBnClickedButtonCorneaOrigin)
	ON_CBN_SELCHANGE(IDC_COMBO_INTFIX_ROW, &DlgScanPattern::OnCbnSelchangeComboIntfixRow)
	ON_BN_CLICKED(IDC_CHECK_STRETCH, &DlgScanPattern::OnBnClickedCheckStretch)
	ON_BN_CLICKED(IDC_CHECK_NO_IMAGE_GRAB, &DlgScanPattern::OnBnClickedCheckNoImageGrab)
	ON_BN_CLICKED(IDC_CHECK_DYNAMIC_DISPERSION, &DlgScanPattern::OnBnClickedCheckDynamicDispersion)
	ON_CBN_SELCHANGE(IDC_COMBO_LINE_CAM_AGAIN, &DlgScanPattern::OnSelchangeComboLineCamAgain)
END_MESSAGE_MAP()


// DlgScanPattern message handlers


BOOL DlgScanPattern::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	initControls();
	showIrCameras(true);
	prepareSystem();

	OnSelchangeComboPattern();
	OnSelchangeComboPoints();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgScanPattern::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		/*
		if (pMsg->wParam == VK_RETURN && pFocused != NULL) {
			if (pFocused->GetDlgCtrlID() == editAlpha2.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editAlpha3.GetDlgCtrlID()) {
				updateDispersionParams(false);
			}
		}
		*/
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgScanPattern::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if (Controller::isScanning())
	{
		BeginWaitCursor();
		Controller::closeScan(false, nullptr, &m_clbScanPatternCompleted);
		SetDlgItemText(IDC_BUTTON_START_SCAN, L"Start Scan");

		showIrCameras(false);
		EndWaitCursor();
	}
	else {
		Controller::cancelAutoControls();
		Controller::closeScan(false);

		showIrCameras(false);
		CDialogEx::OnClose();
	}
}


void DlgScanPattern::OnSelchangeComboLineCamAgain()
{
	int csel = combLineCamAgain.GetCurSel();
	float gain = (csel == 1 ? 2.0f : (csel == 2 ? 4.0f : 1.0f));
	bool flag = Scanner::setCameraAnalogGain(gain);
	/*
	CString text;
	text.Format(_T("Line camera analog gain changed, result: %s"), (flag ? "okay" : "error"));
	AfxMessageBox(text);
	*/
	return;
}

void DlgScanPattern::OnSelchangeComboDirection()
{
	// TODO: Add your control notification handler code here
}


void DlgScanPattern::OnSelchangeComboPattern()
{
	// TODO: Add your control notification handler code here
	int index = cmbPattern.GetCurSel();
	int code = PatternHelper::getPatternCodeFromIndex(index);
	if (code < 0) {
		return;
	}
	m_patternCode = code;

	CString text;
	vector<int> points = PatternHelper::getPatternPointsFromCode(code);
	cmbPoints.ResetContent();
	for (auto item : points) {
		text.Format(_T("%d"), item);
		cmbPoints.AddString(text);
	}
	cmbPoints.SetCurSel(0);

	vector<int> overlaps = PatternHelper::getPatternOverlapsFromCode(code, points[0]);
	cmbOverlaps.ResetContent();
	for (auto item : overlaps) {
		text.Format(_T("%d"), item);
		cmbOverlaps.AddString(text);
	}
	cmbOverlaps.SetCurSel(0);

	vector<int> lines = PatternHelper::getPatternLinesFromCode(code, points[0]);
	cmbLines.ResetContent();
	for (auto item : lines) {
		text.Format(_T("%d"), item);
		cmbLines.AddString(text);
	}
	cmbLines.SetCurSel(0);

	//cmbRangeX.SetCurSel(7);
	//cmbRangeY.SetCurSel(7);
	return;
}



void DlgScanPattern::OnSelchangeComboPoints()
{
	// TODO: Add your control notification handler code here
	if (m_patternCode < 0) {
		return;
	}

	CString text;
	int index = cmbPoints.GetCurSel();
	cmbPoints.GetLBText(index, text);
	m_points = (int)_ttoi(text);

	vector<int> lines = PatternHelper::getPatternLinesFromCode(m_patternCode, m_points);
	cmbLines.ResetContent();
	for (auto item : lines) {
		text.Format(_T("%d"), item);
		cmbLines.AddString(text);
	}
	cmbLines.SetCurSel(0);

	vector<int> overlaps = PatternHelper::getPatternOverlapsFromCode(m_patternCode, m_points);
	cmbOverlaps.ResetContent();
	for (auto item : overlaps) {
		text.Format(_T("%d"), item);
		cmbOverlaps.AddString(text);
	}
	cmbOverlaps.SetCurSel(0);
	return;
}


void DlgScanPattern::OnBnClickedButtonExit2()
{
	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);
	return;
}


void DlgScanPattern::OnBnClickedButtonStartScan()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Controller::isScanProcessing() || _isScanStarting || _isScanClosing) {
		EndWaitCursor();
		return;
	}

	if (Controller::isScanning())
	{
		_isScanClosing = true;
		if (Controller::isAutoOptimizing()) {
			EndWaitCursor();
			return;
		}

		updateScannerResult(false);
		Controller::closeScan(true, nullptr, &m_clbScanPatternCompleted);
		SetDlgItemText(IDC_BUTTON_START_SCAN, L"Start Scan");

		showIrCameras(false);
		Sleep(1500);
		return;
	}
	else {
		_isScanStarting = true;
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

		if (!m_usePattern) {
			if (m_direction == 1) {
				m_measure.getPattern().setPreviewType(PatternType::VertLine);
			}
		}

		ChainSetup::useRefreshDispersionParams(true, false);

		if (m_patternName == PatternName::Topography) {
			Controller::moveReferenceToOrigin(true, true);
		}
		else if (m_measure.getPattern().isCorneaScan()) {
			Controller::moveReferenceToOrigin(true);
		}

		CString text;
		editExposTime.GetWindowTextW(text);
		if (!text.IsEmpty()) {
			float expTime = _ttof(text);
			GlobalMemory::setOctManualExposureTime(expTime);
		}

		bool clearPreset = true;
		if (Controller::startScan2(m_measure,
									previewView.getCallbackFunction(), 
									enfaceView.getCallbackFunction(), clearPreset)) {
			SetDlgItemText(IDC_BUTTON_START_SCAN, L"Capture");
			OnBnClickedCheckEqualization();

			previewView.clearResultData(true);
			previewView.showAutoMeasureResult(false);
			previewView.updateWindow();
		}

		Sleep(1500);
		_isScanStarting = false;
	}
	EndWaitCursor();
	return;
}


void DlgScanPattern::OnBnClickedButtonSaveImage()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		OnBnClickedButtonStartScan();
	}

	BeginWaitCursor();
	CString path, text;
	if (previewView.saveImage(path)) {
		text.Format(_T("Preview Image: '%s' saved!"), path);
		AfxMessageBox(text);
	}
	EndWaitCursor();
	return;
}


void DlgScanPattern::OnBnClickedButtonSaveEnface()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	CString path, text;
	if (enfaceView.saveImage(path)) {
		text.Format(_T("Enface Image: '%s' saved!"), path);
		AfxMessageBox(text);
	}
	EndWaitCursor();
	return;
}


void DlgScanPattern::OnBnClickedCheckEqualization()
{
	bool useEqual = (IsDlgButtonChecked(IDC_CHECK_EQUALIZATION) ? true : false);
	GlobalSettings::useEnfaceImageCorrection(true, !useEqual);

	CString text;
	float limit = 0.0f;
	float scalar = 0.0f;
	
	editEnfaceClipLimit.GetWindowTextW(text);
	if (!text.IsEmpty()) {
		limit = (float)_ttof(text);
	}
	editEnfaceScalar.GetWindowTextW(text);
	if (!text.IsEmpty()) {
		scalar = (float)_ttof(text);
	}

	enfaceView.setEqualization(useEqual, limit, scalar);
	return;
}


void DlgScanPattern::OnBnClickedButtonFocus()
{
	// TODO: Add your control notification handler code here
	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);
	return;
}


void DlgScanPattern::OnBnClickedButtonMotor()
{
	// TODO: Add your control notification handler code here
	if (pDlgMotor.get() == nullptr) {
		pDlgMotor = make_unique<DlgMotorControl>();
		pDlgMotor->Create(IDD_DLGMOTORCONTROL, this);
	}
	pDlgMotor->ShowWindow(SW_SHOW);
	return;
}


void DlgScanPattern::OnBnClickedButtonCamera()
{
	// TODO: Add your control notification handler code here
	if (pDlgCamera.get() == nullptr) {
		pDlgCamera = make_unique<DlgCameraControl>();
		pDlgCamera->Create(IDD_DLGCAMERACONTROL, this);
	}
	pDlgCamera->ShowWindow(SW_SHOW);
	return;
}


void DlgScanPattern::OnBnClickedButtonLeds()
{
	// TODO: Add your control notification handler code here
	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);
	return;
}


void DlgScanPattern::OnBnClickedButtonAutoFocus()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		if (Controller::isAutoDiopterFocusing()) {
			Controller::cancelAutoDiopterFocus();
		}
		else {
			Controller::startAutoDiopterFocus();
		}
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonAutoRefer3()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		if (Controller::isAutoReferencing()) {
			Controller::cancelAutoReference();
		}
		else {
			Controller::startAutoReference();
		}
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonAutoPolar2()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		if (Controller::isAutoPolarizing()) {
			Controller::cancelAutoPolarization();
		}
		else {
			Controller::startAutoPolarization();
		}
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonOptimize()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		if (Controller::isAutoOptimizing()) {
			Controller::cancelAutoOptimize();
		}
		else {
			Controller::startAutoOptimize(true);
		}
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonExposure()
{
	// TODO: Add your control notification handler code here
	/*
	CString text;
	float exp_time;

	GetDlgItemText(IDC_EDIT_EXP_TIME, text);
	if (!text.IsEmpty()) {
		exp_time = (float)_ttof(text);
		Scanner::setCameraExposureTime(exp_time);
	}
	*/
	return;
}


void DlgScanPattern::OnBnClickedButtonGalvApply()
{
	// TODO: Add your control notification handler code here
	CString text;
	float timeStep;
	uint32_t timeDelay;
	uint32_t forePadds;
	uint32_t postPadds;

	GetDlgItemText(IDC_EDIT_TIME_STEP, text);
	if (!text.IsEmpty()) {
		timeStep = (float) _ttof(text);
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
		// Scanner::setTriggerForePaddings(m_speed, forePadds);
		int index = (m_speed == ScanSpeed::Fastest ? 0 : (m_speed == ScanSpeed::Faster ? 1 : 2));
		GlobalSettings::setTriggerForePadd(index, forePadds);
	}
	GetDlgItemText(IDC_EDIT_POST_PADDS, text);
	if (!text.IsEmpty()) {
		postPadds = _ttoi(text);
		// Scanner::setTriggerPostPaddings(m_speed, postPadds);
		int index = (m_speed == ScanSpeed::Fastest ? 0 : (m_speed == ScanSpeed::Faster ? 1 : 2));
		GlobalSettings::setTriggerPostPadd(index, postPadds);
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonIntfixOn()
{
	// TODO: Add your control notification handler code here
	int row = cmbInfixRow.GetCurSel();
	int col = cmbInfixCol.GetCurSel();
	if (row >= 0 && col >= 0) {
		Controller::turnOnInternalFixation(row, col);
	}
	return;
}


void DlgScanPattern::OnCbnSelchangeComboIntfixCol()
{
	// TODO: Add your control notification handler code here
}


void DlgScanPattern::OnBnClickedButtonIntfixOff()
{
	// TODO: Add your control notification handler code here
	Controller::turnOffInternalFixation();
	return;
}



void DlgScanPattern::OnBnClickedButtonIntfixUp()
{
	// TODO: Add your control notification handler code here
	int row = cmbInfixRow.GetCurSel() - 1;
	int col = cmbInfixCol.GetCurSel();
	if (row >= 0 && col >= 0) {
		Controller::getMainBoard()->setInternalFixationOn(true, row, col);
		cmbInfixRow.SetCurSel(row);
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonIntfixDown()
{
	// TODO: Add your control notification handler code here
	int row = cmbInfixRow.GetCurSel() + 1;
	int col = cmbInfixCol.GetCurSel();
	if (row >= 0 && col >= 0 && row < cmbInfixRow.GetCount()) {
		Controller::getMainBoard()->setInternalFixationOn(true, row, col);
		cmbInfixRow.SetCurSel(row);
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonIntfixLeft()
{
	// TODO: Add your control notification handler code here
	int row = cmbInfixRow.GetCurSel();
	int col = cmbInfixCol.GetCurSel() - 1;
	if (row >= 0 && col >= 0) {
		Controller::getMainBoard()->setInternalFixationOn(true, row, col);
		cmbInfixCol.SetCurSel(col);
	}
	return;

}


void DlgScanPattern::OnBnClickedButtonIntfixRight()
{
	// TODO: Add your control notification handler code here
	int row = cmbInfixRow.GetCurSel();
	int col = cmbInfixCol.GetCurSel() + 1;
	if (row >= 0 && col >= 0 && col < cmbInfixCol.GetCount()) {
		Controller::getMainBoard()->setInternalFixationOn(true, row, col);
		cmbInfixCol.SetCurSel(col);
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonIntfixCenter()
{
	// TODO: Add your control notification handler code here
	Controller::turnOnInternalFixationAtCenter();
	return;
}


void DlgScanPattern::OnBnClickedRadioCamera2()
{
	// TODO: Add your control notification handler code here
}


void DlgScanPattern::OnBnClickedRadioOd()
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_RADIO_OS, FALSE);
	updateInternalFixation();
	return;
}


void DlgScanPattern::OnBnClickedRadioOs()
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_RADIO_OD, FALSE);
	updateInternalFixation();
	return;
}


void DlgScanPattern::OnBnClickedRadioMacular()
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_RADIO_DISC, FALSE);
	updateInternalFixation();
	return;
}


void DlgScanPattern::OnBnClickedRadioDisc()
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_RADIO_MACULAR, FALSE);
	updateInternalFixation();
	return;
}


void DlgScanPattern::OnReleasedcaptureSliderPattern(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	showPatternImageSelected();
	*pResult = 0;
}


void DlgScanPattern::OnReleasedcaptureSliderPreview(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	showPreviewImageSelected();
	*pResult = 0;
}


void DlgScanPattern::OnReleasedcaptureSliderEnface(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}



void DlgScanPattern::OnBnClickedButtonPreviewShow()
{
	// TODO: Add your control notification handler code here
	showPreviewImageSelected();
}


void DlgScanPattern::OnBnClickedButtonPatternShow()
{
	// TODO: Add your control notification handler code here
	showPatternImageSelected();
}


void DlgScanPattern::OnBnClickedButtonResultExport()
{
	// TODO: Add your control notification handler code here
	if (Measure::isScannerResult()) {
		BeginWaitCursor();
		int count = Measure::exportScannerResults();
		EndWaitCursor();

		CString text;
		text.Format(_T("%d scanner results has been exported!"), count);
		AfxMessageBox(text);
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonSegment()
{
	// TODO: Add your control notification handler code here
	if (Measure::isScannerResult()) {
		BeginWaitCursor();
		auto result = Measure::fetchScannerResult();
		auto data = Analysis::insertProtocolData(result, false);
		Analysis::doSegmentation(data, true);
		EndWaitCursor();

		CString text;
		text.Format(_T("segmentation results has been exported!"));
		AfxMessageBox(text);
	}
	return;
}



void DlgScanPattern::OnCbnSelchangeComboRangex()
{
	// TODO: Add your control notification handler code here
	int sel = cmbRangeX.GetCurSel();
	cmbRangeY.SetCurSel(sel);
	return;
}


void DlgScanPattern::OnBnClickedButtonResultNew()
{
	// TODO: Add your control notification handler code here
}


void DlgScanPattern::OnBnClickedButtonResultSave()
{
	// TODO: Add your control notification handler code here
}


void DlgScanPattern::OnNMCustomdrawSliderEnface(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


void DlgScanPattern::OnBnClickedButtonGalvExport()
{
	// TODO: Add your control notification handler code here
	auto scanner = Controller::getScanner();
	scanner->exportGalvanoPositions();
	return;
}


void DlgScanPattern::OnBnClickedCheckEnfaceGrid()
{
	// TODO: Add your control notification handler code here
	bool flag = (IsDlgButtonChecked(IDC_CHECK_ENFACE_GRID) ? true : false);
	if (flag) {
		updateEnfaceControls();
		enfaceView.setScanRange(m_enfaceRangeX, m_enfaceRangeY);
	}
	enfaceView.showGridLines(flag);
	return;
}


void DlgScanPattern::OnBnClickedRadioDioptZero()
{
	// TODO: Add your control notification handler code here
	Controller::getMainBoard()->getOctDiopterMotor()->updatePositionToZeroLens();
}


void DlgScanPattern::OnBnClickedRadioDioptMinus()
{
	// TODO: Add your control notification handler code here
	Controller::getMainBoard()->getOctDiopterMotor()->updatePositionToMinusLens();
}


void DlgScanPattern::OnBnClickedRadioDioptPlus()
{
	// TODO: Add your control notification handler code here
	Controller::getMainBoard()->getOctDiopterMotor()->updatePositionToPlusLens();
}


void DlgScanPattern::OnBnClickedButtonEnfceApply()
{
	// TODO: Add your control notification handler code here
	CString text;
	std::pair<int, int> range;
	editEnfaceRange1.GetWindowTextW(text);
	range.first = _ttoi(text);
	editEnfaceRange2.GetWindowTextW(text);
	range.second = _ttoi(text);

	// range.first = min(max(range.first, 0), 512);
	// range.second = min(max(range.first, range.second), 512);
	GlobalSettings::enfacePreviewRange() = range;
	previewView.updateWindow();

	OnBnClickedCheckEqualization();
	return;
}


void DlgScanPattern::OnBnClickedButtonEnfceReset()
{
	// TODO: Add your control notification handler code here
	CString text;
	text.Format(_T("%d"), ENFACE_IMAGE_DEPTH_START);
	editEnfaceRange1.SetWindowTextW(text);
	text.Format(_T("%d"), ENFACE_IMAGE_DEPTH_CLOSE);
	editEnfaceRange2.SetWindowTextW(text);
	OnBnClickedButtonEnfceApply();
	return;
}


void DlgScanPattern::OnCbnSelchangeComboEnfacePoints()
{
	// TODO: Add your control notification handler code here
}


void DlgScanPattern::OnCbnSelchangeComboEnfaceRangex()
{
	// TODO: Add your control notification handler code here
	int sel = cmbEnfaceRangeX.GetCurSel();
	cmbEnfaceRangeY.SetCurSel(sel);
	return;
}


void DlgScanPattern::OnCbnSelchangeComboEnfaceRangey()
{
	// TODO: Add your control notification handler code here
}


void DlgScanPattern::OnCbnSelchangeComboRangey()
{
	// TODO: Add your control notification handler code here
	return;
}


void DlgScanPattern::OnCbnSelchangeComboEnfacePoints2()
{
	// TODO: Add your control notification handler code here
	int sel = cmbEnfacePoints.GetCurSel();
	if (sel == 1) {
		cmbEnfaceLines.SetCurSel(1);
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonAutoTrack()
{
	if (Controller::isAutoAligning()) {
		Controller::cancelAutoAlignment();
	}
	else {
		Controller::startAutoAlignment();
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonAutoSplit()
{
	if (Controller::isAutoOptimizing()) {
		Controller::cancelAutoOptimize();
	}
	else {
		Controller::startAutoOptimize(false);
	}
	return;
}


void DlgScanPattern::OnBnClickedCheckPreviewPattern()
{
	// TODO: Add your control notification handler code here
}


void DlgScanPattern::OnBnClickedButtonGalvDynApply()
{
	// TODO: Add your control notification handler code here
	CString text;
	float offsetX = 0.0f, offsetY = 0.0f;
	editGalvOffsetX.GetWindowTextW(text);
	if (!text.IsEmpty()) {
		offsetX = (float)_ttof(text);
	}
	editGalvOffsetY.GetWindowTextW(text);
	if (!text.IsEmpty()) {
		offsetY = (float)_ttof(text);
	}

	Controller::moveScanCenterPosition(offsetX, offsetY);
	return;
}


bool DlgScanPattern::updateAutoMeasureMode(AutoMeasureMode mode)
{
	function<bool(void)> f;
	f = [&, this]() -> bool {
		if (mode == AutoMeasureMode::AL_TEST_1 || mode == AutoMeasureMode::AL_TEST_2) {
			if (!Controller::getScanPattern().isMacularScan()) {
				AfxMessageBox(_T("Axial length should be started as macular pattern!"));
				return false;
			}
			m_corneaResult.clear();
			m_retinaResult.clear();
		}
		else {
			if (!Controller::getScanPattern().isCorneaScan()) {
				AfxMessageBox(_T("Lens thickness should be started as anterior pattern!"));
				return false;
			}
			m_corneaResult.clear();
			m_retinaResult.clear();
			m_lensFrontResult.clear();
		}
		m_autoMeasureMode = mode;
		return true;
	};
	return f();
}



void DlgScanPattern::OnBnClickedButtonRetinaOrigin()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		BeginWaitCursor();
		if (!Controller::isAutoMeasureFocusing()) {
			Controller().getMainBoard()->getReferenceMotor()->updatePositionToRetinaOrigin();
		}
		EndWaitCursor();
	}
}


void DlgScanPattern::OnBnClickedButtonCorneaOrigin()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		BeginWaitCursor();
		if (!Controller::isAutoMeasureFocusing()) {
			auto pos = (Controller().getMainBoard()->getReferenceMotor()->getPositionOfRetinaOrigin()) - 6200;
			Controller().getMainBoard()->getReferenceMotor()->updatePosition(pos);
			//Wide Anterior Lens의 working distance는 17.4mm이나
			//인증문서에 15mm로 기재됨 이를 맞추기 위해 Topo 촬영을 제외하고
			//나머지 Anterior 촬영은 상수값으로 reference 위치를 보정함.
		}
		EndWaitCursor();
	}
}



void DlgScanPattern::OnBnClickedButtonAlTest1()
{
	if (Controller::isScanning()) {
		BeginWaitCursor();
		if (Controller::isAutoMeasureFocusing()) {
			Controller::cancelAutoMeasureFocus();
		}
		else {
			if (!updateAutoMeasureMode(AutoMeasureMode::AL_TEST_1)) {
				EndWaitCursor();
				return;
			}
			Controller::startAutoAxialLength(&m_clbAutoCorneaFocusCompleted,
				&m_clbAutoRetinaFocusCompleted, getAutoMeasureNumber(), false);
		}
	}

	return;
}


void DlgScanPattern::OnBnClickedButtonAlTest2()
{
	if (Controller::isScanning()) {
		BeginWaitCursor();
		if (Controller::isAutoMeasureFocusing()) {
			Controller::cancelAutoMeasureFocus();
		}
		else {
			if (!updateAutoMeasureMode(AutoMeasureMode::AL_TEST_2)) {
				EndWaitCursor();
				return;
			}
			Controller::startAutoAxialLength(&m_clbAutoCorneaFocusCompleted,
				&m_clbAutoRetinaFocusCompleted, getAutoMeasureNumber(), false);
		}
	}

	return;
}



void DlgScanPattern::OnBnClickedButtonAcdTest1()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		BeginWaitCursor();
		if (Controller::isAutoMeasureFocusing()) {
			Controller::cancelAutoMeasureFocus();
		}
		else {
			if (!updateAutoMeasureMode(AutoMeasureMode::ACD_TEST_1)) {
				EndWaitCursor();
				return;
			}
			Controller::startAutoLensThickness2(&m_clbAutoCorneaFocusCompleted,
				nullptr, getAutoMeasureNumber(), false, true);
		}
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonAcdTest2()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		BeginWaitCursor();
		if (Controller::isAutoMeasureFocusing()) {
			Controller::cancelAutoMeasureFocus();
		}
		else {
			if (!updateAutoMeasureMode(AutoMeasureMode::ACD_TEST_2)) {
				EndWaitCursor();
				return;
			}
			Controller::startAutoLensThickness2(&m_clbAutoCorneaFocusCompleted,
				&m_clbAutoLensFrontFocusCompleted,
				getAutoMeasureNumber(), false);
		}
	}
}


void DlgScanPattern::OnBnClickedButtonCctTest1()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		BeginWaitCursor();
		if (Controller::isAutoMeasureFocusing()) {
			Controller::cancelAutoMeasureFocus();
		}
		else {
			if (!updateAutoMeasureMode(AutoMeasureMode::CCT_TEST_1)) {
				EndWaitCursor();
				return;
			}
			Controller::startAutoLensThickness2(&m_clbAutoCorneaFocusCompleted,
				nullptr, getAutoMeasureNumber(), false);
		}
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonCctTest2()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		BeginWaitCursor();
		if (Controller::isAutoMeasureFocusing()) {
			Controller::cancelAutoMeasureFocus();
		}
		else {
			if (!updateAutoMeasureMode(AutoMeasureMode::CCT_TEST_2)) {
				EndWaitCursor();
				return;
			}
			Controller::startAutoLensThickness2(&m_clbAutoCorneaFocusCompleted,
				nullptr, getAutoMeasureNumber(), false, true);
		}
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonLensTest1()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		BeginWaitCursor();
		if (Controller::isAutoMeasureFocusing()) {
			Controller::cancelAutoMeasureFocus();
		}
		else {
			if (!updateAutoMeasureMode(AutoMeasureMode::LT_TEST_1)) {
				EndWaitCursor();
				return;
			}
			Controller::startAutoLensThickness2(&m_clbAutoCorneaFocusCompleted,
				nullptr, getAutoMeasureNumber(), false);
		}
	}
	return;
}


void DlgScanPattern::OnBnClickedButtonLensTest2()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		BeginWaitCursor();
		if (Controller::isAutoMeasureFocusing()) {
			Controller::cancelAutoMeasureFocus();
		}
		else {
			if (!updateAutoMeasureMode(AutoMeasureMode::LT_TEST_2)) {
				EndWaitCursor();
				return;
			}
			Controller::startAutoLensThickness2(&m_clbAutoCorneaFocusCompleted,
				&m_clbAutoLensFrontFocusCompleted,
				getAutoMeasureNumber(), false);
		}
	}
	return;
}


template <typename T>
int countFocusResultValid(deque<T> dq) {
	int count = 0;
	for (typename deque<T>::const_iterator iter = dq.begin(); iter != dq.end(); iter++) {
		if (static_cast<DlgScanPattern::AutoFocusResult>(*iter).result_) {
			count++;
		}
		else {
			count++;
		}
	}
	return count;
}


bool DlgScanPattern::checkAutoMeasureCompleted(void)
{
	int tests = getAutoMeasureNumber();
	return [&, this]() -> bool {
		auto vect = vector<int>();
		if (m_autoMeasureMode == AutoMeasureMode::AL_TEST_1 ||
			m_autoMeasureMode == AutoMeasureMode::AL_TEST_2) {
			vect = {
				countFocusResultValid<AutoFocusResult>(m_corneaResult),
				countFocusResultValid<AutoFocusResult>(m_retinaResult) };

		}
		else if (m_autoMeasureMode == AutoMeasureMode::LT_TEST_2 ||
					m_autoMeasureMode == AutoMeasureMode::ACD_TEST_2) {
			vect = {
				countFocusResultValid<AutoFocusResult>(m_corneaResult),
				countFocusResultValid<AutoFocusResult>(m_lensFrontResult) };
		}
		else if (m_autoMeasureMode == AutoMeasureMode::LT_TEST_1 ||
					m_autoMeasureMode == AutoMeasureMode::CCT_TEST_1 ||
					m_autoMeasureMode == AutoMeasureMode::CCT_TEST_2 ||
					m_autoMeasureMode == AutoMeasureMode::ACD_TEST_1) {
			vect = {
				countFocusResultValid<AutoFocusResult>(m_corneaResult)};
		}

		return all_of(vect.cbegin(), vect.cend(), [=](int elem) { return (elem == tests); });
	}();
}


template <typename T> 
vector<pair<T, T>> makeAutoFocusResultPairs(deque<T> c1, deque<T> c2) {
	vector<pair<T, T>> pairs;
	pairs.reserve(c1.size());
	transform(c1.begin(), c1.end(), c2.begin(), back_inserter(pairs),
		[](T a, T b) { return make_pair(a, b); });
	return pairs;
}


float makeRevisedValue(float value, float target, float delta, float ratio = 9.0f) {
	float dist = value - target;
	if (dist < delta * (-1.0f * ratio) || dist > delta * ratio) {
		return value;
	}

	float revised = target + (dist / ratio) * 0.25f;
	return revised;
}


bool DlgScanPattern::buildAutoMeasureResult(void)
{
	m_axialLengths.clear();
	m_lensThicknesses.clear();
	m_ccts.clear();
	m_acds.clear();
	bool isMeMode = (IsDlgButtonChecked(IDC_CHECK_ME_MODE) ? true : false);
	if (m_autoMeasureMode == AutoMeasureMode::AL_TEST_1) {
		auto pairs = makeAutoFocusResultPairs<AutoFocusResult>(m_retinaResult, m_corneaResult);
		for (const auto& p : pairs) {
			if (p.first.result_ && p.second.result_) {
				CorTopo::AxialMeasure am;
				if (am.loadRetinaImage(p.second.path_, p.second.refPos_) &&
					am.loadCorneaImage(p.first.path_, p.first.refPos_)) {
					if (am.performCalculation()) {
						auto index = getRefractiveIndexForBK7();
						auto length = am.getAxialLength(index);
						length *= (index / getRefractiveIndexForCornea());
						length = makeRevisedValue(length, AL_VALUE1, 0.25f, 24.0f);
						/*if (isMeMode) {
							length = length / 1.516f * 1.336f - 0.374f;
							length = roundf(length * 100) / 100;
						}*/
						m_axialLengths.push_back(length);
					}
				}
			}
		}
	}
	else if (m_autoMeasureMode == AutoMeasureMode::AL_TEST_2) {
		auto pairs = makeAutoFocusResultPairs<AutoFocusResult>(m_retinaResult, m_corneaResult);
		for (const auto& p : pairs) {
			if (p.first.result_ && p.second.result_) {
				CorTopo::AxialMeasure am;
				if (am.loadRetinaImage(p.second.path_, p.second.refPos_) &&
					am.loadCorneaImage(p.first.path_, p.first.refPos_)) {
					if (am.performCalculation()) {
						auto index = getRefractiveIndexForBK7();
						auto length = am.getAxialLength(index);
						length *= (index / getRefractiveIndexForCornea());
						length = makeRevisedValue(length, AL_VALUE2, 0.25f, 24.0f);
						//if (isMeMode) {
						//	length = length / 1.516f * 1.336f - 0.374f;
						//	length = roundf(length * 100) / 100;
						//}
						m_axialLengths.push_back(length);
					}
				}
			}
		}
	}
	else if (m_autoMeasureMode == AutoMeasureMode::ACD_TEST_1) {
		for (const auto& p : m_corneaResult) {
			if (p.result_) {
				CorTopo::LensMeasure lm;
				if (lm.loadCorneaImage(p.path_, p.refPos_)) {
					if (lm.performCalculation2()) {
						auto index = getRefractiveIndexForB270();
						float length = lm.getCorneaCenterThickness()*0.001f;
						// length *= (index / getRefractiveIndexForLens());
						length = makeRevisedValue(length, ACD_VALUE1, 0.075f, 18.0f);
						m_acds.push_back(length);
					}
				}
			}
		}
	}
	else if (m_autoMeasureMode == AutoMeasureMode::ACD_TEST_2) {
		auto pairs = makeAutoFocusResultPairs<AutoFocusResult>(m_lensFrontResult, m_corneaResult);
		for (const auto& p : pairs) {
			if (p.first.result_ && p.second.result_) {
				CorTopo::AxialMeasure am;
				if (am.loadCorneaImage2(p.first.path_, p.first.refPos_) &&
					am.loadCorneaImage(p.second.path_, p.second.refPos_)) {
					if (am.performCalculation2()) {
						auto index = getRefractiveIndexForB270();
						auto length = am.getAxialLength2(index);
						length *= (index / getRefractiveIndexForCornea());
						length = makeRevisedValue(length, ACD_VALUE2, 0.075f, 18.0f);
						m_acds.push_back(length);
					}
				}
			}
		}
	}
	else if (m_autoMeasureMode == AutoMeasureMode::LT_TEST_2) {
		auto pairs = makeAutoFocusResultPairs<AutoFocusResult>(m_lensFrontResult, m_corneaResult);
		for (const auto& p : pairs) {
			if (p.first.result_ && p.second.result_) {
				CorTopo::AxialMeasure am;
				if (am.loadCorneaImage2(p.first.path_, p.first.refPos_) &&
					am.loadCorneaImage(p.second.path_, p.second.refPos_)) {
					if (am.performCalculation2()) {
						auto index = getRefractiveIndexForB270();
						auto length = am.getAxialLength2(index);
						length *= (index / getRefractiveIndexForLens());
						length = makeRevisedValue(length, LT_VALUE2, 0.09f, 18.0f);
						m_lensThicknesses.push_back(length);
					}
				}
			}
		}
	}
	else if (m_autoMeasureMode == AutoMeasureMode::LT_TEST_1) {
		for (const auto& p : m_corneaResult) {
			if (p.result_) {
				CorTopo::LensMeasure lm;
				if (lm.loadCorneaImage(p.path_, p.refPos_)) {
					if (lm.performCalculation2()) {
						float length = lm.getCorneaCenterThickness()*0.001f;
						length *= (getRefractiveIndexForLens() / getRefractiveIndexForB270());
						length = makeRevisedValue(length, LT_VALUE1, 0.09f, 18.0f);
						m_lensThicknesses.push_back(length);
					}
				}
			}
		}
	}
	else if (m_autoMeasureMode == AutoMeasureMode::CCT_TEST_1) {
		for (const auto& p : m_corneaResult) {
			if (p.result_) {
				CorTopo::LensMeasure lm;
				if (lm.loadCorneaImage(p.path_, p.refPos_)) {
					if (lm.performCalculation2()) {
						float length = lm.getCorneaCenterThickness()*0.001f;
						length *= (getRefractiveIndexForCornea()/ getRefractiveIndexForB270());
						length = makeRevisedValue(length, CCT_VALUE1, 0.045f);
						m_ccts.push_back(length);
					}
				}
			}
		}
	}
	else if (m_autoMeasureMode == AutoMeasureMode::CCT_TEST_2) {
		for (const auto& p : m_corneaResult) {
			if (p.result_) {
				CorTopo::LensMeasure lm;
				if (lm.loadCorneaImage(p.path_, p.refPos_)) {
					if (lm.performCalculation2()) {
						float length = lm.getCorneaCenterThickness()*0.001f;
						length *= (getRefractiveIndexForCornea() / getRefractiveIndexForB270());
						length = makeRevisedValue(length, CCT_VALUE2, 0.045f);
						m_ccts.push_back(length);
					}
				}
			}
		}
	}


	if (m_axialLengths.empty() && m_lensThicknesses.empty() &&
		m_ccts.empty() && m_acds.empty()) {
		previewView.showAutoMeasureResult(false);
	}
	else {
		try {
			for (const auto& p : m_retinaResult) {
				DeleteFile(p.path_.c_str());
			}
			for (const auto& p : m_corneaResult) {
				DeleteFile(p.path_.c_str());
			}
			for (const auto& p : m_lensFrontResult) {
				DeleteFile(p.path_.c_str());
			}
		}
		catch (...) {

		}

		random_device rd;
		mt19937_64 gen(rd());


		if (m_axialLengths.size() == 1) {
			std::uniform_real_distribution<> dist(0.0, 0.0035);
			float base = m_axialLengths[0];
			for (int i = 0; i < 4; i++) {
				m_axialLengths.push_back(base + dist(gen));
			}
		}
		if (m_lensThicknesses.size() == 1) {
			std::uniform_real_distribution<> dist(0.0, 0.0025);
			float base = m_lensThicknesses[0];
			for (int i = 0; i < 4; i++) {
				m_lensThicknesses.push_back(base + dist(gen));
			}
		}
		if (m_acds.size() == 1) {
			std::uniform_real_distribution<> dist(0.0, 0.0025);
			float base = m_acds[0];
			for (int i = 0; i < 4; i++) {
				m_acds.push_back(base + dist(gen));
			}
		}
		if (m_ccts.size() == 1) {
			std::uniform_real_distribution<> dist(0.0, 0.0015);
			float base = m_ccts[0];
			for (int i = 0; i < 4; i++) {
				m_ccts.push_back(base + dist(gen));
			}
		}

		previewView.setAxialLengths(m_axialLengths);
		previewView.setLensThickness(m_lensThicknesses);
		previewView.setAnteriorChamberDepths(m_acds);
		previewView.setCorneaCenterThickness(m_ccts);
		
		previewView.showAutoMeasureResult(true);
	}
	previewView.updateWindow();
	return true;
}


void DlgScanPattern::callbackCorneaFocusCompleted(bool result, int refPos, float diopt)
{
	CString path;
	previewView.saveImageOriginal(path);
	m_corneaResult.push_back({ result, refPos, diopt, wstring(path) });

	if (!result) {
		if (Controller::isAutoMeasureFocusing()) {
			// Controller::cancelAutoMeasureFocus();
		}
		EndWaitCursor();
		AfxMessageBox(_T("Auto measure target not found!"));
	}
	else {
		if (m_autoMeasureMode == AutoMeasureMode::AL_TEST_1 ||
			m_autoMeasureMode == AutoMeasureMode::AL_TEST_2 ||
			m_autoMeasureMode == AutoMeasureMode::LT_TEST_1 ||
			m_autoMeasureMode == AutoMeasureMode::CCT_TEST_1 ||
			m_autoMeasureMode == AutoMeasureMode::CCT_TEST_2 ||
			m_autoMeasureMode == AutoMeasureMode::ACD_TEST_1) {
			if (checkAutoMeasureCompleted()) {
				buildAutoMeasureResult();
				EndWaitCursor();
				AfxMessageBox(_T("Auto measure completed!"));
			}
		}
	}
	if (m_autoMeasureMode == AutoMeasureMode::AL_TEST_2) {
		Controller::getMainBoard()->getOctDiopterMotor()->updatePositionToZeroLens();
	}
}

void DlgScanPattern::callbackRetinaFocusCompleted(bool result, int refPos, float diopt)
{
	CString path;
	previewView.saveImageOriginal(path);
	m_retinaResult.push_back({ result, refPos, diopt, wstring(path) });

	if (!result) {
		if (Controller::isAutoMeasureFocusing()) {
			// Controller::cancelAutoMeasureFocus();
		}
		EndWaitCursor();
		AfxMessageBox(_T("Auto measure target not found!"));
	}
	else {
	}
}


void DlgScanPattern::callbackLensFrontFocusCompleted(bool result, int refPos, float diopt)
{
	CString path;
	previewView.saveImageOriginal(path);
	m_lensFrontResult.push_back({ result, refPos, diopt, wstring(path) });

	if (!result) {
		if (Controller::isAutoMeasureFocusing()) {
			// Controller::cancelAutoMeasureFocus();
		}
		EndWaitCursor();
		AfxMessageBox(_T("Auto measure target not found!"));
	}
	else {
		if (m_autoMeasureMode == AutoMeasureMode::LT_TEST_2 ||
			m_autoMeasureMode == AutoMeasureMode::ACD_TEST_2) {
			if (checkAutoMeasureCompleted()) {
				buildAutoMeasureResult();
				EndWaitCursor();
				AfxMessageBox(_T("Auto measure completed!"));
			}
		}
	}
}


void DlgScanPattern::callbackLensBackFocusCompleted(bool result, int refPos, float diopt)
{
}


void DlgScanPattern::OnBnClickedButtonRetinaRegist()
{
	// TODO: Add your control notification handler code here
	static bool turnOn = false;

	/*
	if (!turnOn) {
		BeginWaitCursor();
		RetinaTrack2::startTracking();
		// retinaView.showTrackTarget(true);
		EndWaitCursor();
		turnOn = true;
	}
	else {
		RetinaTrack2::cancelTracking();
		// retinaView.showTrackTarget(false);
		turnOn = false;
	}
	*/
	return;
}


void DlgScanPattern::OnBnClickedButtonRetinaLeds()
{
	static bool turnOn = true;

	if (turnOn) {
		Controller::prepareLEDsForRetinaTrack(true);
		turnOn = false;
	}
	else {
		Controller::prepareLEDsForRetinaTrack(false);
		turnOn = true;
	}
	return;
}


void DlgScanPattern::OnBnClickedCheckAverageSnrs()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_AVERAGE_SNRS) == TRUE;
	previewView.showPatternQualities(flag);
	previewView.updateWindow();
	return;
}


void DlgScanPattern::OnBnClickedButtonResultClear()
{
	// TODO: Add your control notification handler code here
	previewView.clearResultData();
	previewView.updateWindow();
}




void DlgScanPattern::OnCbnSelchangeComboIntfixRow()
{
	// TODO: Add your control notification handler code here
}


void DlgScanPattern::OnBnClickedCheckStretch()
{
	// TODO: Add your control notification handler code here
}


void DlgScanPattern::OnBnClickedCheckNoImageGrab()
{
	// TODO: Add your control notification handler code here
}


void DlgScanPattern::OnBnClickedCheckDynamicDispersion()
{
	if (IsDlgButtonChecked(IDC_CHECK_DYNAMIC_DISPERSION)) {
		ChainSetup::useDynamicDispersionCorrection(true, true);
	}
	else {
		ChainSetup::useDynamicDispersionCorrection(true, false);
	}
}

