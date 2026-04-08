// DlgAngioPattern.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgAngioPattern.h"
#include "CppUtil2.h"
#include "OctAngio2.h"
#include "afxdialogex.h"

#include <string>
#include <vector>
#include <boost/format.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <numeric>

using namespace std;
using namespace CppUtil;
using namespace OctAngio;


// DlgAngioPattern dialog

IMPLEMENT_DYNAMIC(DlgAngioPattern, CDialogEx)

DlgAngioPattern::DlgAngioPattern(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGANGIOPATTERN, pParent), m_cameraSpeed(0)
{
	m_clbScanPatternCompleted = std::bind(&DlgAngioPattern::callbackScanPatternCompleted, this, std::placeholders::_1);

	cbJoystick = std::bind(&DlgAngioPattern::OnBnClickedButtonStartScan, this);
	Controller::setJoystickEventCallback(&cbJoystick);
}

DlgAngioPattern::~DlgAngioPattern()
{
}

void DlgAngioPattern::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DIRECTION, cmbDirection);
	DDX_Control(pDX, IDC_COMBO_LINES, cmbLines);
	DDX_Control(pDX, IDC_COMBO_OVERLAPS, cmbOverlaps);
	DDX_Control(pDX, IDC_COMBO_PATTERN, cmbPattern);
	DDX_Control(pDX, IDC_COMBO_POINTS, cmbPoints);
	DDX_Control(pDX, IDC_COMBO_RANGEX, cmbRangeX);
	DDX_Control(pDX, IDC_COMBO_RANGEY, cmbRangeY);
	DDX_Control(pDX, IDC_COMBO_SPACE, cmbSpace);
	DDX_Control(pDX, IDC_EDIT_OFFSET_X, editOffsetX);
	DDX_Control(pDX, IDC_EDIT_OFFSET_Y, editOffsetY);
	DDX_Control(pDX, IDC_EDIT_PATTERN_NO, editPatternNo);
	DDX_Control(pDX, IDC_EDIT_PREVIEW_NO, editPreviewNo);
	DDX_Control(pDX, IDC_EDIT_OVERLAP_NO, editOverlapNo);
	DDX_Control(pDX, IDC_EDIT_SCALE_X, editScaleX);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y, editScaleY);
	DDX_Control(pDX, IDC_SLIDER_PATTERN, sldPattern);
	DDX_Control(pDX, IDC_SLIDER_PREVIEW, sldPreview);
	DDX_Control(pDX, IDC_SLIDER_OVERLAP, sldOverlap);
	DDX_Control(pDX, IDC_EDIT_ANGLE, editAngle);
	DDX_Radio(pDX, IDC_RADIO_CAMERA1, m_cameraSpeed);
	DDX_Control(pDX, IDC_COMBO_ANGIO_LOWER, cmbAngioLower);
	DDX_Control(pDX, IDC_COMBO_ANGIO_UPPER, cmbAngioUpper);
	DDX_Control(pDX, IDC_EDIT_ANGIO_LINES, editAngioLines);
	DDX_Control(pDX, IDC_EDIT_ANGiO_OVERLAPS, editAngioOverlaps);
	DDX_Control(pDX, IDC_EDIT_ANGIO_POINTS, editAngioPoints);
	DDX_Control(pDX, IDC_EDIT_DECOR_THRESH, editDecorThresh);
	DDX_Control(pDX, IDC_EDIT_ANGIO_CONTRAST, editAngioContrast);
	DDX_Control(pDX, IDC_EDIT_DECOR_THRESH2, editDecorThresh2);
	DDX_Control(pDX, IDC_EDIT_DIFFER_THRESH, editDifferThresh);
	DDX_Control(pDX, IDC_EDIT_DIFFER_THRESH2, editDifferThresh2);
	DDX_Control(pDX, IDC_EDIT_ANGIO_FOVEA_CX, editFoveaCx);
	DDX_Control(pDX, IDC_EDIT_ANGIO_FOVEA_CY, editFoveaCy);
	DDX_Control(pDX, IDC_EDIT_ANGIO_LAYER_OFFSET1, editLayerOffset1);
	DDX_Control(pDX, IDC_EDIT_ANGIO_LAYER_OFFSET2, editLayerOffset2);
	DDX_Control(pDX, IDC_EDIT_TEST_AVG_LINES, editTestAvgLines);
	DDX_Control(pDX, IDC_EDIT_TEST_DECORR_THRESH, editTestDecorrThresh);
	DDX_Control(pDX, IDC_EDIT_TEST_TOP_LINES, editTestTopLines);
	DDX_Control(pDX, IDC_EDIT_CAMERA_GAIN, editCameraGain);
	DDX_Control(pDX, IDC_EDIT_FLOW_AVG_OFFS1, editFlowAvgOffs1);
	DDX_Control(pDX, IDC_EDIT_FLOW_AVG_OFFS2, editFlowAvgOffs2);
	DDX_Control(pDX, IDC_EDIT_FLOW_THRESH1, editFlowThresh1);
	DDX_Control(pDX, IDC_EDIT_FLOW_THRESH2, editFlowThresh2);
	DDX_Control(pDX, IDC_COMBO_PROJ_LOWER, cmbProjLower);
	DDX_Control(pDX, IDC_COMBO_PROJ_UPPER, cmbProjUpper);
	DDX_Control(pDX, IDC_COMBO_VASC_LOWER, cmbVascLower);
	DDX_Control(pDX, IDC_COMBO_VASC_UPPER, cmbVascUpper);
	DDX_Control(pDX, IDC_EDIT_VASC_AVG_OFFS, editVascAvgOffs);
	DDX_Control(pDX, IDC_EDIT_MASK_AVG_OFFS, editMaskAvgOffs);
	DDX_Control(pDX, IDC_EDIT_MASK_WEIGHT, editMaskWeight);
	DDX_Control(pDX, IDC_EDIT_PROJ_AVG_OFFS1, editProjAvgOffs1);
	DDX_Control(pDX, IDC_EDIT_PROJ_AVG_OFFS2, editProjAvgOffs2);
}


BEGIN_MESSAGE_MAP(DlgAngioPattern, CDialogEx)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_PATTERN, &DlgAngioPattern::OnReleasedcaptureSliderPattern)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_PREVIEW, &DlgAngioPattern::OnReleasedcaptureSliderPreview)
	ON_BN_CLICKED(IDC_BUTTON_START_SCAN, &DlgAngioPattern::OnBnClickedButtonStartScan)
	ON_BN_CLICKED(IDC_BUTTON_EXIT2, &DlgAngioPattern::OnBnClickedButtonExit2)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_FOCUS, &DlgAngioPattern::OnBnClickedButtonAutoFocus)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_POLAR2, &DlgAngioPattern::OnBnClickedButtonAutoPolar2)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_REFER3, &DlgAngioPattern::OnBnClickedButtonAutoRefer3)
	ON_BN_CLICKED(IDC_BUTTON_OPTIMIZE, &DlgAngioPattern::OnBnClickedButtonOptimize)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS, &DlgAngioPattern::OnBnClickedButtonFocus)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA, &DlgAngioPattern::OnBnClickedButtonCamera)
	ON_BN_CLICKED(IDC_BUTTON_MOTOR, &DlgAngioPattern::OnBnClickedButtonMotor)
	ON_BN_CLICKED(IDC_BUTTON_LEDS, &DlgAngioPattern::OnBnClickedButtonLeds)
	ON_BN_CLICKED(IDC_BUTTON_RESULT_EXPORT, &DlgAngioPattern::OnBnClickedButtonResultExport)
	ON_BN_CLICKED(IDC_BUTTON_SEGMENT, &DlgAngioPattern::OnBnClickedButtonSegment)
	ON_CBN_SELCHANGE(IDC_COMBO_DIRECTION, &DlgAngioPattern::OnCbnSelchangeComboDirection)
	ON_CBN_SELCHANGE(IDC_COMBO_PATTERN, &DlgAngioPattern::OnCbnSelchangeComboPattern)
	ON_CBN_SELCHANGE(IDC_COMBO_POINTS, &DlgAngioPattern::OnCbnSelchangeComboPoints)
	ON_CBN_SELCHANGE(IDC_COMBO_RANGEX, &DlgAngioPattern::OnCbnSelchangeComboRangex)
	ON_CBN_SELCHANGE(IDC_COMBO_RANGEY, &DlgAngioPattern::OnCbnSelchangeComboRangey)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_CAMERA1, IDC_RADIO_CAMERA3, DlgAngioPattern::setRadioCameraSpeed)
	ON_CBN_SELCHANGE(IDC_COMBO_OVERLAPS, &DlgAngioPattern::OnCbnSelchangeComboOverlaps)
	ON_CBN_SELCHANGE(IDC_COMBO_LINES, &DlgAngioPattern::OnCbnSelchangeComboLines)
	ON_BN_CLICKED(IDC_BUTTON_ANGIO_RELOAD, &DlgAngioPattern::OnBnClickedButtonAngioReload)
	ON_BN_CLICKED(IDC_BUTTON_ANGIO_REDRAW, &DlgAngioPattern::OnBnClickedButtonAngioRedraw)
	ON_BN_CLICKED(IDC_BUTTON_ANGIO_EXPORT, &DlgAngioPattern::OnBnClickedButtonAngioExport)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_PATTERN, &DlgAngioPattern::OnNMCustomdrawSliderPattern)
	ON_EN_CHANGE(IDC_EDIT_PATTERN_NO, &DlgAngioPattern::OnEnChangeEditPatternNo)
	ON_BN_CLICKED(IDC_CHECK_EXPORT_BUFFERS, &DlgAngioPattern::OnBnClickedCheckExportBuffers)
	ON_BN_CLICKED(IDC_BUTTON_ANGIO_EXPORT_DATA, &DlgAngioPattern::OnBnClickedButtonAngioExportData)
	ON_BN_CLICKED(IDC_BUTTON_ANGIO_IMPORT_DATA, &DlgAngioPattern::OnBnClickedButtonAngioImportData)
	ON_BN_CLICKED(IDC_BUTTON_ANGIO_IMPORT_BINS, &DlgAngioPattern::OnBnClickedButtonAngioImportBins)
	ON_BN_CLICKED(IDC_CHECK_ANGIO_ENHANCE, &DlgAngioPattern::OnBnClickedCheckAngioEnhance)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_REGIST, &DlgAngioPattern::OnBnClickedButtonRetinaRegist)
	ON_BN_CLICKED(IDC_CHECK_OVERLAY_DECORR, &DlgAngioPattern::OnBnClickedCheckOverlayDecorr)
	ON_BN_CLICKED(IDC_CHECK_FIT_TO_HEIGHT, &DlgAngioPattern::OnBnClickedCheckFitToHeight)
	ON_BN_CLICKED(IDC_CHECK_VIEW_AMPLITUDES, &DlgAngioPattern::OnBnClickedCheckViewAmplitudes)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_OVERLAP, &DlgAngioPattern::OnCustomdrawSliderOverlap)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_OVERLAP, &DlgAngioPattern::OnReleasedcaptureSliderOverlap)
	ON_BN_CLICKED(IDC_CHECK_PREVIEW_DECORRS, &DlgAngioPattern::OnBnClickedCheckPreviewDecorrs)
	ON_BN_CLICKED(IDC_BUTTON_ANGIO_IMPORT_DATA2, &DlgAngioPattern::OnBnClickedButtonAngioImportData2)
	ON_BN_CLICKED(IDC_RADIO_CAMERA3, &DlgAngioPattern::OnBnClickedRadioCamera3)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_INTFIX_OD_FUNDUS, IDC_RADIO_INTFIX_OS_DISC, DlgAngioPattern::setInternalFixation)
	ON_BN_CLICKED(IDC_CHECK_RETINA_TRACK, &DlgAngioPattern::OnBnClickedCheckRetinaTrack)
	ON_BN_CLICKED(IDC_RADIO_INTFIX_OD_FUNDUS, &DlgAngioPattern::OnBnClickedRadioIntfixOdFundus)
	ON_BN_CLICKED(IDC_BUTTON_TEST_SET_PARAMS, &DlgAngioPattern::OnBnClickedButtonTestSetParams)
	ON_BN_CLICKED(IDC_CHECK_FIXED_NOISE_REDUCTION, &DlgAngioPattern::OnBnClickedCheckFixedNoiseReduction)
	ON_BN_CLICKED(IDC_CHECK_TEST_HORZ_LINES, &DlgAngioPattern::OnBnClickedCheckTestHorzLines)
	ON_BN_CLICKED(IDC_CHECK_TEST_VERT_LINES, &DlgAngioPattern::OnBnClickedCheckTestVertLines)
	ON_BN_CLICKED(IDC_BUTTON_TEST_RESULT, &DlgAngioPattern::OnBnClickedButtonTestResult)
	ON_BN_CLICKED(IDC_BUTTON_TEST_SAVE_PARAMS, &DlgAngioPattern::OnBnClickedButtonTestSaveParams)
	ON_BN_CLICKED(IDC_BUTTON_TEST_INIT_PARAMS, &DlgAngioPattern::OnBnClickedButtonTestInitParams)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA_SET, &DlgAngioPattern::OnBnClickedButtonCameraSet)
	ON_BN_CLICKED(IDC_CHECK_ANGIO_PROJECT_REMOVAL, &DlgAngioPattern::OnBnClickedCheckAngioProjectRemoval)
	ON_BN_CLICKED(IDC_BUTTON_ANGIO_EXPORT_DATA2, &DlgAngioPattern::OnBnClickedButtonAngioExportData2)
	ON_BN_CLICKED(IDC_BUTTON_ANGIO_IMPORT_DATA3, &DlgAngioPattern::OnBnClickedButtonAngioImportData3)
	ON_BN_CLICKED(IDC_BUTTON_ANGIO_OPEN_DATA2, &DlgAngioPattern::OnBnClickedButtonAngioOpenData2)
	ON_BN_CLICKED(IDC_RADIO_INTFIX_OD_DISC, &DlgAngioPattern::OnBnClickedRadioIntfixOdDisc)
	ON_BN_CLICKED(IDC_BUTTON_ANGIO_OPEN_BINS, &DlgAngioPattern::OnBnClickedButtonAngioOpenBins)
	ON_BN_CLICKED(IDC_CHECK_EXPORT_CAMERA_BUFFER, &DlgAngioPattern::OnBnClickedCheckExportCameraBuffer)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_ADJUST, &DlgAngioPattern::OnBnClickedButtonAutoAdjust)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_PREPARE, &DlgAngioPattern::OnBnClickedButtonRetinaPrepare)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_START, &DlgAngioPattern::OnBnClickedButtonRetinaStart)
END_MESSAGE_MAP()


// DlgAngioPattern message handlers


BOOL DlgAngioPattern::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	PatternHelper::initializePatternHelper();

	initControls();
	showIrCameras(true);
	prepareSystem();

	OnCbnSelchangeComboPattern();
	OnCbnSelchangeComboPoints();

	cmbPoints.SetCurSel(1);
	cmbLines.SetCurSel(1);
	cmbOverlaps.SetCurSel(1);
	updateAngioControls(true);

	CheckDlgButton(IDC_CHECK_EXPORT_BUFFERS, AngioSetup::isSavingBuffersToFiles());
	CheckDlgButton(IDC_CHECK_EXPORT_CAMERA_BUFFER, AngioSetup::isSavingFramesToFiles());
	CheckDlgButton(IDC_CHECK_EDI_SCAN, AngioSetup::isEdiScanningMode());

	Measure::createDefaultExportDirectory(true);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgAngioPattern::PreTranslateMessage(MSG* pMsg)
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


void DlgAngioPattern::OnClose()
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

		Analysis::clearAllReports();
		Analysis::clearAllScanData();

		showIrCameras(false);
		CDialogEx::OnClose();
	}
}


void DlgAngioPattern::OnReleasedcaptureSliderPattern(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	showPatternImageSelected();
	*pResult = 0;
}


void DlgAngioPattern::OnReleasedcaptureSliderPreview(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	showPreviewImageSelected();
	*pResult = 0;
}


void DlgAngioPattern::OnReleasedcaptureSliderOverlap(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	showOverlapImageSelected();
	*pResult = 0;
}


void DlgAngioPattern::OnBnClickedButtonStartScan()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Controller::isScanProcessing() || _isScanStarting || _isScanClosing) {
		EndWaitCursor();
		return;
	}

	bool retinaTrack = (IsDlgButtonChecked(IDC_CHECK_RETINA_TRACK) == TRUE);
	bool ediScanning = (IsDlgButtonChecked(IDC_CHECK_EDI_SCAN) == TRUE);

	if (Controller::isScanning())
	{
		_isScanClosing = true;
		// updateScannerResult(false);
		Controller::closeScan(true, nullptr, &m_clbScanPatternCompleted, retinaTrack);
		SetDlgItemText(IDC_BUTTON_START_SCAN, L"Start Scan");

		showIrCameras(false);
		Sleep(1500);
		return;
	}
	else {
		_isScanStarting = true;
		showIrCameras(true);
		updateScanControls();
		m_segmented = false;

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

		m_measure.setEyeSide(m_eyeSide);
		Controller::presetScanEnface(m_measure.getEnface());

		m_measure.getPattern().setup(m_patternName, m_points, m_lines, m_rangeX, m_rangeY,
			m_direction, m_overlaps, m_space, speed);
		m_measure.getPattern().setScanOffset(m_offsetX, m_offsetY, m_angle, m_scaleX, m_scaleY);

		ChainSetup::useRefreshDispersionParams(true, false);

		if (Controller::startScan2(m_measure,
			previewView.getCallbackFunction(),
			enfaceView.getCallbackFunction(), 
			true, retinaTrack, ediScanning)) {
			SetDlgItemText(IDC_BUTTON_START_SCAN, L"Capture");
			previewView.clearResultData();
			retinaView.showTrackFeature(retinaTrack);
			Angiography::clearAngiography();
		}

		Sleep(1500);
		_isScanStarting = false;
	}
	EndWaitCursor();
	return;
}


void DlgAngioPattern::OnBnClickedButtonExit2()
{
	// TODO: Add your control notification handler code here
	RetinaTrack::cancelTracking();
	SendMessage(WM_CLOSE, NULL, NULL);
	return;
}


void DlgAngioPattern::OnBnClickedButtonAutoFocus()
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


void DlgAngioPattern::OnBnClickedButtonAutoPolar2()
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


void DlgAngioPattern::OnBnClickedButtonAutoRefer3()
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


void DlgAngioPattern::OnBnClickedButtonOptimize()
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


void DlgAngioPattern::OnBnClickedButtonFocus()
{
	// TODO: Add your control notification handler code here
	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);
	return;
}


void DlgAngioPattern::OnBnClickedButtonCamera()
{
	// TODO: Add your control notification handler code here
	if (pDlgCamera.get() == nullptr) {
		pDlgCamera = make_unique<DlgCameraControl>();
		pDlgCamera->Create(IDD_DLGCAMERACONTROL, this);
	}
	pDlgCamera->ShowWindow(SW_SHOW);
	return;
}


void DlgAngioPattern::OnBnClickedButtonMotor()
{
	// TODO: Add your control notification handler code here
	if (pDlgMotor.get() == nullptr) {
		pDlgMotor = make_unique<DlgMotorControl>();
		pDlgMotor->Create(IDD_DLGMOTORCONTROL, this);
	}
	pDlgMotor->ShowWindow(SW_SHOW);
	return;
}


void DlgAngioPattern::OnBnClickedButtonLeds()
{
	// TODO: Add your control notification handler code here
	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);
	return;
}


void DlgAngioPattern::OnBnClickedButtonResultExport()
{
	// TODO: Add your control notification handler code here
	if (Measure::isScannerResult()) {
		BeginWaitCursor();
		int count = Measure::exportScannerResults();
		EndWaitCursor();

		// AngioSetup::saveAmplitudesAsBinaryFiles();

		CString text;
		text.Format(_T("%d scanner results has been exported!"), count);
		AfxMessageBox(text);
	}
	return;
}


void DlgAngioPattern::OnBnClickedButtonSegment()
{
	// TODO: Add your control notification handler code here
	if (Analysis::checkIfProtocolDataExist()) {
		BeginWaitCursor();
		auto data = Analysis::getProtocolData();
		Analysis::doSegmentation(data, true);
		m_segmented = true;
		// Analysis::clearAllReports();
		// Analysis::clearAllScanData();
		EndWaitCursor();

		CString text;
		text.Format(_T("segmentation results has been exported!"));
		AfxMessageBox(text);
	}
	return;
}


void DlgAngioPattern::updateAngioControls(bool fetch)
{
	if (fetch) {
		if (PatternHelper::isAngioPatternCode(m_patternCode)) {
			int index;
			CString text;
			index = cmbPoints.GetCurSel();
			cmbPoints.GetLBText(index, text);
			editAngioPoints.SetWindowTextW(text);
			index = cmbLines.GetCurSel();
			cmbLines.GetLBText(index, text);
			editAngioLines.SetWindowTextW(text);
			index = cmbOverlaps.GetCurSel();
			cmbOverlaps.GetLBText(index, text);
			editAngioOverlaps.SetWindowTextW(text);
		}
		return;
	}

	auto func = [](int sel) -> std::string {
		switch (sel) {
		case 0:
			return "ILM";
		case 1:
			return "NFL";
		case 2:
			return "IPL";
		case 3:
			return "OPL";
		case 4:
			return "IOS";
		case 5:
			return "RPE";
		default:
			return "BRM";
		}
	};

	auto func2 = [](int sel) -> OcularLayerType {
		switch (sel) {
		case 0:
			return OcularLayerType::ILM;
		case 1:
			return OcularLayerType::NFL;
		case 2:
			return OcularLayerType::IPL;
		case 3:
			return OcularLayerType::OPL;
		case 4:
			return OcularLayerType::IOS;
		case 5:
			return OcularLayerType::RPE;
		default:
			return OcularLayerType::BRM;
		}
	};

	m_angioLayerUpper = func(cmbAngioUpper.GetCurSel());
	m_angioLayerLower = func(cmbAngioLower.GetCurSel());
	m_upperLayerType = func2(cmbAngioUpper.GetCurSel());
	m_lowerLayerType = func2(cmbAngioLower.GetCurSel());

	_vascUpperLayer = func2(cmbVascUpper.GetCurSel());
	_vascLowerLayer = func2(cmbVascLower.GetCurSel());
	_maskUpperLayer = func2(cmbProjUpper.GetCurSel());
	_maskLowerLayer = func2(cmbProjLower.GetCurSel());

	CString text;

	editDecorThresh.GetWindowTextW(text);
	m_angioDecorThresh = (float)(text.IsEmpty() ? 0.0f : _ttof(text));
	editDecorThresh2.GetWindowTextW(text);
	m_angioDecorThresh2 = (float)(text.IsEmpty() ? 0.0f : _ttof(text));
	editDifferThresh.GetWindowTextW(text);
	m_angioDifferThresh = (float)(text.IsEmpty() ? 0.0f : _ttof(text));
	editDifferThresh2.GetWindowTextW(text);
	m_angioDifferThresh2 = (float)(text.IsEmpty() ? 0.0f : _ttof(text));

	editAngioLines.GetWindowTextW(text);
	m_angioLines = (text.IsEmpty() ? 384 : _ttoi(text));
	editAngioPoints.GetWindowTextW(text);
	m_angioPoints = (text.IsEmpty() ? 384 : _ttoi(text));
	editAngioOverlaps.GetWindowTextW(text);
	m_angioOverlaps = (text.IsEmpty() ? 3 : _ttoi(text));
	editAngioContrast.GetWindowTextW(text);
	m_angioContrastEq = (float)(text.IsEmpty() ? 0.0f : _ttof(text));

	editFlowAvgOffs1.GetWindowTextW(text);
	_avgOffsetInnFlows = (int)(text.IsEmpty() ? 0 : _ttoi(text));
	editFlowAvgOffs2.GetWindowTextW(text);
	_avgOffsetOutFlows = (int)(text.IsEmpty() ? 0 : _ttoi(text));
	editFlowThresh1.GetWindowTextW(text);
	_thresholdInnFlows = (float)(text.IsEmpty() ? 0.0f : _ttof(text));
	editFlowThresh2.GetWindowTextW(text);
	_thresholdOutFlows = (float)(text.IsEmpty() ? 0.0f : _ttof(text));

	editVascAvgOffs.GetWindowTextW(text);
	_vascAvgOffset = (int)(text.IsEmpty() ? 0 : _ttoi(text));
	editMaskAvgOffs.GetWindowTextW(text);
	_maskAvgOffset = (int)(text.IsEmpty() ? 0 : _ttoi(text));
	editMaskWeight.GetWindowTextW(text);
	_maskWeight = (float)(text.IsEmpty() ? 0.0f : _ttof(text));

	editProjAvgOffs1.GetWindowTextW(text);
	_projAvgOffset1 = (int)(text.IsEmpty() ? 0 : _ttoi(text));
	editProjAvgOffs2.GetWindowTextW(text);
	_projAvgOffset2 = (int)(text.IsEmpty() ? 0 : _ttoi(text));

	editFoveaCx.GetWindowTextW(text);
	m_angioFoveaCx = (float)(text.IsEmpty() ? 0.0f : _ttof(text));
	editFoveaCy.GetWindowTextW(text);
	m_angioFoveaCy = (float)(text.IsEmpty() ? 0.0f : _ttof(text));
	editLayerOffset1.GetWindowTextW(text);
	m_angioLayerOffset1 = (float)(text.IsEmpty() ? 0.0f : _ttof(text));
	editLayerOffset2.GetWindowTextW(text);
	m_angioLayerOffset2 = (float)(text.IsEmpty() ? 0.0f : _ttof(text));

	m_angioVertical = IsDlgButtonChecked(IDC_CHECK_ANGIO_VERTICAL) == TRUE;
	m_angioLayerSel = IsDlgButtonChecked(IDC_CHECK_ANGIO_LAYERS_SELECT) == TRUE;
	m_angioAlignAxial = IsDlgButtonChecked(IDC_CHECK_ANGIO_ALIGN_AXIAL) == TRUE;
	m_angioAlignLateral = IsDlgButtonChecked(IDC_CHECK_ANGIO_ALIGN_LATERAL) == TRUE;
	m_angioEnhance = IsDlgButtonChecked(IDC_CHECK_ANGIO_ENHANCE) == TRUE;

	m_useVascularLayers = IsDlgButtonChecked(IDC_CHECK_ANGIO_VASCULAR_LAYERS) == TRUE;
	m_usePostProcessing = IsDlgButtonChecked(IDC_CHECK_ANGIO_POST_PROCESSING) == TRUE;
	m_useNormProjection = IsDlgButtonChecked(IDC_CHECK_ANGIO_NORM_PROJECTION) == TRUE;
	m_useDecorrProjection = IsDlgButtonChecked(IDC_CHECK_ANGIO_DECOR_PROJECTION) == TRUE;
	m_useDifferProjection = IsDlgButtonChecked(IDC_CHECK_ANGIO_DIFFER_PROJECTION) == TRUE;
	m_useMotionCorrect = IsDlgButtonChecked(IDC_CHECK_ANGIO_MOTION_CORRECT) == TRUE;
	m_useProjectRemoval = IsDlgButtonChecked(IDC_CHECK_ANGIO_PROJECT_REMOVAL) == TRUE;
	m_useReflectCorrect = IsDlgButtonChecked(IDC_CHECK_ANGIO_REFLECT_CORRECT) == TRUE;
	m_useBiasFieldCorrect = IsDlgButtonChecked(IDC_CHECK_BIAS_FIELD_CORRECT) == TRUE;
	return;
}

void DlgAngioPattern::updateAngioSettings(bool clear)
{
	if (clear) {
		Angiography::clearAngiography();
	}
	auto& angio = Angiography::getAngiogram();
	angio.useReflectionCorrection() = m_useReflectCorrect;
	angio.useBiasFieldCorrection() = m_useBiasFieldCorrect;
	angio.useDifferOutput() = m_useDifferProjection;

	angio.numberOfOverlaps() = m_angioOverlaps;
	angio.avgOffsetInnFlows() = _avgOffsetInnFlows;
	angio.avgOffsetOutFlows() = _avgOffsetOutFlows;
	angio.thresholdRatioInnFlows() = _thresholdInnFlows;
	angio.thresholdRatioOutFlows() = _thresholdOutFlows;

	angio.avgOffsetVascularMask() = _vascAvgOffset;
	angio.avgOffsetProjectionMask() = _maskAvgOffset;
	angio.weightProjectionMask() = _maskWeight;
	angio.avgOffsetInnProjection() = _projAvgOffset1;
	angio.avgOffsetOutProjection() = _projAvgOffset2;

	return;
}


void DlgAngioPattern::updateAngioTestControls(void)
{
	CString text;
	editTestDecorrThresh.GetWindowTextW(text);
	m_testDecorrThresh = (float)(text.IsEmpty() ? 0.15f : _ttof(text));
	editTestAvgLines.GetWindowTextW(text);
	m_testNumAvgLines = (text.IsEmpty() ? 5 : _ttoi(text));
	editTestTopLines.GetWindowTextW(text);
	m_testNumTopLines = (text.IsEmpty() ? 5 : _ttoi(text));
	return;
}


void DlgAngioPattern::refreshAngioControls(void)
{
	auto layout = Angiography::getAngiogram().Layout();

	CString text;
	text.Format(_T("%d"), layout.numberOfLines());
	editAngioLines.SetWindowTextW(text);
	text.Format(_T("%d"), layout.numberOfPoints());
	editAngioPoints.SetWindowTextW(text);
	text.Format(_T("%d"), layout.numberOfRepeats());
	editAngioOverlaps.SetWindowTextW(text);

	text.Format(_T("%.1f"), layout.foveaCenterX());
	editFoveaCx.SetWindowTextW(text);
	text.Format(_T("%.1f"), layout.foveaCenterY());
	editFoveaCy.SetWindowTextW(text);
	return;
}


void DlgAngioPattern::OnCbnSelchangeComboDirection()
{
	// TODO: Add your control notification handler code here
}


void DlgAngioPattern::OnCbnSelchangeComboPattern()
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
	cmbPoints.SetCurSel(PatternHelper::isAngioPatternCode(code) ? 1 : 0);

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
	cmbLines.SetCurSel(PatternHelper::isAngioPatternCode(code) ? 1 : 0);
	updateAngioControls(true);
	return;
}


void DlgAngioPattern::OnCbnSelchangeComboPoints()
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


	updateAngioControls(true);
	return;
}


void DlgAngioPattern::OnCbnSelchangeComboRangex()
{
	// TODO: Add your control notification handler code here
	CString text;
	int sel = cmbRangeX.GetCurSel();
	cmbRangeX.GetLBText(sel, text);
	m_rangeX = (float)_ttof(text);

	cmbRangeY.SetCurSel(sel);
	return;
}


void DlgAngioPattern::OnCbnSelchangeComboRangey()
{
	// TODO: Add your control notification handler code here
	CString text;
	int sel = cmbRangeY.GetCurSel();
	cmbRangeY.GetLBText(sel, text);
	m_rangeY = (float)_ttof(text);
	return;
}


void DlgAngioPattern::initControls(void)
{
	CString text;

	corneaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 0, 320, 240), this);
	retinaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 240, 320, 240 + 240), this);
	corneaView.setDisplaySize(320, 240);
	retinaView.setDisplaySize(320, 240);
	corneaView.showCameraInfo(true);
	retinaView.showCameraInfo(true);
	retinaView.showTrackTarget(false);

	previewView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(320, 0, 320 + 1024, 512), this);
	previewView.setDisplaySize(1024, 512);
	previewView.ModifyStyle(0, SS_NOTIFY);
	previewView.setDisplayStretched(true);
	previewView.overlayDecorrelations(true);
	previewView.clearResultData();

	angioView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(320 + 1024, 0, 320 + 1024 + 480, 480), this);
	angioView.setDisplaySize(480, 480);
	angioView.ModifyStyle(0, SS_NOTIFY);

	enfaceView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 480, 320, 480 + 320), this);
	enfaceView.setDisplaySize(320, 320);
	enfaceView.ModifyStyle(0, SS_NOTIFY);

	cmbPattern.ResetContent();
	int size = PatternHelper::getNumberOfPatternTypes();
	for (int i = 0; i < size; i++) {
		cmbPattern.AddString(CString(PatternHelper::getPatternNameStringFromIndex(i)));
	}
	cmbPattern.SetCurSel(5);

	cmbRangeX.SetCurSel(5);
	cmbRangeY.SetCurSel(5);
	cmbSpace.SetCurSel(3);
	cmbPoints.SetCurSel(1);
	cmbLines.SetCurSel(2);
	cmbDirection.SetCurSel(0);
	cmbOverlaps.SetCurSel(1);

	editOffsetX.SetWindowTextW(_T("0.0"));
	editOffsetY.SetWindowTextW(_T("0.0"));
	editAngle.SetWindowTextW(_T("0.0"));
	editScaleX.SetWindowTextW(_T("1.0"));
	editScaleY.SetWindowTextW(_T("1.0"));

	updateScannerResult();

	CheckDlgButton(IDC_RADIO_CAMERA1, FALSE);
	CheckDlgButton(IDC_RADIO_CAMERA2, FALSE);
	CheckDlgButton(IDC_RADIO_CAMERA3, TRUE);
	setRadioCameraSpeed(2);

	CheckDlgButton(IDC_CHECK_VIEW_AMPLITUDES, TRUE);
	CheckDlgButton(IDC_CHECK_FIT_TO_HEIGHT, TRUE);
	CheckDlgButton(IDC_CHECK_OVERLAY_DECORR, TRUE);
	CheckDlgButton(IDC_CHECK_RETINA_TRACK, FALSE);
	CheckDlgButton(IDC_CHECK_FIXED_NOISE_REDUCTION, FALSE);
	ChainSetup::useFixedNoiseReduction(true, false);

	editTestAvgLines.SetWindowTextW(_T("5"));
	editTestDecorrThresh.SetWindowTextW(_T("0.15"));
	editTestTopLines.SetWindowTextW(_T("5"));

	editDecorThresh.SetWindowTextW(_T("0.0"));
	editDecorThresh2.SetWindowTextW(_T("0.0"));
	editDifferThresh.SetWindowTextW(_T("0.0"));
	editDifferThresh2.SetWindowTextW(_T("0.0"));
	editAngioContrast.SetWindowTextW(_T("1.0"));
	editAngioLines.SetWindowTextW(_T("384"));
	editAngioPoints.SetWindowTextW(_T("384"));
	editAngioOverlaps.SetWindowTextW(_T("3"));

	editFlowAvgOffs1.SetWindowTextW(_T("0"));
	editFlowAvgOffs2.SetWindowTextW(_T("0"));
	editFlowThresh1.SetWindowTextW(_T("1.0"));
	editFlowThresh2.SetWindowTextW(_T("1.0"));

	editVascAvgOffs.SetWindowTextW(_T("11"));
	editMaskAvgOffs.SetWindowTextW(_T("11"));
	editMaskWeight.SetWindowTextW(_T("1.0"));
	editProjAvgOffs1.SetWindowTextW(_T("11"));
	editProjAvgOffs2.SetWindowTextW(_T("11"));

	editLayerOffset1.SetWindowTextW(_T("0.0"));
	editLayerOffset2.SetWindowTextW(_T("0.0"));

	CheckDlgButton(IDC_CHECK_ANGIO_LAYERS_SELECT, TRUE);
	CheckDlgButton(IDC_CHECK_ANGIO_ALIGN_AXIAL, TRUE);
	CheckDlgButton(IDC_CHECK_ANGIO_ALIGN_LATERAL, FALSE);
	CheckDlgButton(IDC_CHECK_ANGIO_ENHANCE, FALSE);
	CheckDlgButton(IDC_CHECK_ANGIO_AVERAGE, FALSE);
	CheckDlgButton(IDC_CHECK_ANGIO_PROJECT_REMOVAL, TRUE);
	CheckDlgButton(IDC_CHECK_ANGIO_REFLECT_CORRECT, TRUE);
	CheckDlgButton(IDC_CHECK_BIAS_FIELD_CORRECT, TRUE);

	CheckDlgButton(IDC_CHECK_ANGIO_POST_PROCESSING, TRUE);
	CheckDlgButton(IDC_CHECK_ANGIO_NORM_PROJECTION, TRUE);
	CheckDlgButton(IDC_CHECK_ANGIO_DECOR_PROJECTION, FALSE);
	CheckDlgButton(IDC_CHECK_ANGIO_DIFFER_PROJECTION, TRUE);
	CheckDlgButton(IDC_CHECK_ANGIO_VASCULAR_LAYERS, TRUE);
	CheckDlgButton(IDC_CHECK_ANGIO_MOTION_CORRECT, TRUE);
	
	cmbAngioUpper.SetCurSel(1);
	cmbAngioLower.SetCurSel(6);
	cmbProjUpper.SetCurSel(1);
	cmbProjLower.SetCurSel(3);
	cmbVascUpper.SetCurSel(1);
	cmbVascLower.SetCurSel(2);
	return;
}

void DlgAngioPattern::showIrCameras(bool play)
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

void DlgAngioPattern::prepareSystem(void)
{
	Controller::turnOnInternalFixationAtCenter();
	return;
}

void DlgAngioPattern::updateScanControls(void)
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

	m_patternCode = code;
	m_patternName = PatternHelper::getPatternNameFromCode(code);

	index = cmbRangeX.GetCurSel();
	cmbRangeX.GetLBText(index, text);
	m_rangeX = (float)_ttof(text);

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


void DlgAngioPattern::setRadioCameraSpeed(UINT value)
{
	UpdateData(TRUE);
	CString text;

	bool m_useNoImageGrab = false;

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

	//text.Format(_T("%d"), Scanner::getTriggerForePaddings(m_speed));
	//SetDlgItemText(IDC_EDIT_FORE_PADDS, text);
	//text.Format(_T("%d"), Scanner::getTriggerPostPaddings(m_speed));
	//SetDlgItemText(IDC_EDIT_POST_PADDS, text);

	if (!ret) {
		AfxMessageBox(_T("Reset scan speed failed!"));
	}
	else {
		// OnBnClickedButtonGalvApply();
	}
	return;
}


void DlgAngioPattern::setInternalFixation(UINT value)
{
	UpdateData(TRUE);

	switch (value) {
	case IDC_RADIO_INTFIX_OD_FUNDUS:
		Controller::turnOnInternalFixation(EyeSide::OD, FixationTarget::Fundus);
		m_eyeSide = EyeSide::OD;
		m_isDisc = false;
		break;
	case IDC_RADIO_INTFIX_OS_FUNDUS:
		Controller::turnOnInternalFixation(EyeSide::OS, FixationTarget::Fundus);
		m_eyeSide = EyeSide::OS;
		m_isDisc = false;
		break;
	case IDC_RADIO_INTFIX_OD_DISC:
		Controller::turnOnInternalFixation(EyeSide::OD, FixationTarget::OpticDisc);
		m_eyeSide = EyeSide::OD;
		m_isDisc = true;
		break;
	case IDC_RADIO_INTFIX_OS_DISC:
		Controller::turnOnInternalFixation(EyeSide::OS, FixationTarget::OpticDisc);
		m_eyeSide = EyeSide::OS;
		m_isDisc = true;
		break;
	}
	return;
}


void DlgAngioPattern::updateScannerResult(bool enable)
{
	sldPreview.EnableWindow(FALSE);
	sldPattern.EnableWindow(FALSE);
	sldOverlap.EnableWindow(FALSE);

	if (!enable) {
		return;
	}

	if (Measure::isScannerResult()) {
		int size = Measure::getCountOfPreviewSections();
		updatePreviewImagesSlider(size);
	}

	bool angio = Angiography::getAngiogram().isAngioImage();

	if (Measure::isScannerResult()) {
		const OctEnfaceImage* enface = Measure::getEnfaceImageDescript();
		if (enface) {
			enfaceView.callbackEnfaceImage(enface->getBuffer(), enface->getWidth(), enface->getHeight());
		}
	}

	if (Measure::isScannerResult() && (!m_viewAmplitudes || !angio)) {
		int size = Measure::getCountOfPatternSections();
		updatePatternImagesSlider(size);
	}
	else {
		int size = Angiography::getAngiogram().numberOfDecorrImages();
		updatePatternImagesSlider(size);
		size = Angiography::getAngiogram().numberOfOverlapImages();
		updateOverlapImagesSlider(size);
	}
	return;
}


void DlgAngioPattern::updatePreviewImagesSlider(int size)
{
	int cpos;
	CString text;

	if (size > 0) {
		sldPreview.SetRange(1, size);
		sldPreview.SetLineSize(1);
		sldPreview.SetPageSize(1);
		cpos = (size <= 1 ? 1 : size / 2);
		sldPreview.SetPos(cpos);

		text.Format(_T("%d"), cpos);
		editPreviewNo.SetWindowTextW(text);
		sldPreview.EnableWindow(TRUE);
	}
	else {
		sldPreview.EnableWindow(FALSE);
	}
	return;
}


void DlgAngioPattern::updatePatternImagesSlider(int size)
{
	int cpos;
	CString text;
	static int size_last = -1;

	if (size > 0) {
		if (size_last != size) {
			sldPattern.SetRange(1, size);
			sldPattern.SetLineSize(1);
			sldPattern.SetPageSize(1);
			cpos = (size <= 1 ? 1 : size / 2);
			sldPattern.SetPos(cpos);

			text.Format(_T("%d"), cpos);
			editPatternNo.SetWindowTextW(text);
			size_last = size;
		}

		sldPattern.EnableWindow(TRUE);
		showPatternImageSelected();
	}
	else {
		sldPattern.EnableWindow(FALSE);
	}
	return;
}


void DlgAngioPattern::updateOverlapImagesSlider(int size)
{
	int cpos;
	CString text;

	if (size > 0) {
		sldOverlap.SetRange(1, size);
		sldOverlap.SetLineSize(1);
		sldOverlap.SetPageSize(1);
		cpos = (size <= 1 ? 1 : size / 2);
		sldOverlap.SetPos(cpos);

		text.Format(_T("%d"), cpos);
		editOverlapNo.SetWindowTextW(text);

		sldOverlap.EnableWindow(TRUE);
	}
	else {
		sldOverlap.EnableWindow(FALSE);
	}
	return;
}


void DlgAngioPattern::showPreviewImageSelected(void)
{
	CString text;
	int pos = sldPreview.GetPos();
	text.Format(_T("%d"), pos);
	editPreviewNo.SetWindowText(text);

	const OctScanImage* image = Measure::getPreviewImageDescript(pos - 1);
	previewView.overlayDecorrelations(false);
	previewView.setDecorrelationValues(0.0f, 0.0f, 0.0f);
	previewView.setIntensityValues(0.0f, 0.0f, 0.0f);
	previewView.setSlabLayers();
	previewView.drawScanImage(image, m_segmented);
	return;
}


void DlgAngioPattern::showPatternImageSelected(void)
{
	CString text;
	int pos = sldPattern.GetPos();
	text.Format(_T("%d"), pos);
	editPatternNo.SetWindowText(text);

	int index = pos - 1;
	bool angio = Angiography::getAngiogram().isAngioImage();

	if (Measure::isScannerResult() && (!m_viewAmplitudes || !angio)) {
		const OctScanImage* image = Measure::getPatternImageDescript(index);
		previewView.setDecorrelationValues();
		previewView.setIntensityValues();
		previewView.setSlabLayers();
		previewView.drawScanImage(image, m_segmented);
	}
	else {
		auto& angio = Angiography::getAngiogram();
		auto* data = angio.getDecorrImageBits(index);
		float mean, stdev, maxval;
		angio.Decorr().getDecorrelationStatOfBscan(index, mean, stdev, maxval);
		previewView.setDecorrelationValues(mean, stdev, maxval);

		angio.Decorr().getIntensityStatOfBscan(index, mean, stdev, maxval);
		previewView.setIntensityValues(mean, stdev, maxval);

		bool flag = IsDlgButtonChecked(IDC_CHECK_OVERLAY_DECORR) == TRUE;
		previewView.overlayDecorrelations(flag);

		float quality = 0.0f;
		if (Measure::isScannerResult()) {
			const OctScanImage* image = Measure::getPatternImageDescript(index);
			if (image) {
				quality = image->getQuality();
			}
		}
		previewView.setDecorrelationData(data);
		previewView.setSlabLayers(angio.getUpperLayerPoints(index), angio.getLowerLayerPoints(index));

		previewView.callbackPreviewImage(angio.getScanImageBits(index), angio.scanImageWidth(), angio.scanImageHeight(), quality);
		sldOverlap.SetPos(0);

		angioView.selectBscanLine(index);
	}

	/*
	int index = pos - 1;
	auto& angiogram = AngioSetup::getAngiogram();

	const OctScanImage* image = Measure::getPatternImageDescript(index);
	previewView.setDecorrelationValues(angiogram.getBscanDecorrMean(index),
		angiogram.getBscanDecorrStddev(index), angiogram.getBscanDecorrMaxVal(index));
	previewView.setIntensityValues(angiogram.getBscanIntensityMean(index),
		angiogram.getBscanIntensityStddev(index), angiogram.getBscanIntensityMaxVal(index));
	previewView.drawScanImage(image, m_segmented);
	*/
	return;
}


void DlgAngioPattern::showOverlapImageSelected(void)
{
	CString text;
	int pos = sldOverlap.GetPos();
	text.Format(_T("%d"), pos);
	editOverlapNo.SetWindowText(text);

	int pos2 = sldPattern.GetPos();
	int lineIdx = pos2 - 1;
	int repeatIdx = pos - 1;

	if (Measure::isScannerResult() && !m_viewAmplitudes) {
	}
	else {
		auto& angio = Angiography::getAngiogram();
		auto* data = angio.getDecorrImageBits(lineIdx);
		previewView.setDecorrelationData(data);
		previewView.callbackPreviewImage(angio.getScanImageBits(lineIdx, repeatIdx), angio.scanImageWidth(), angio.scanImageHeight());
	}
}


void DlgAngioPattern::callbackScanPatternCompleted(bool result)
{
	if (Measure::isScannerResult()) {
		auto result = Measure::fetchScannerResult();
		auto data = Analysis::insertProtocolData(result, false);
		m_segmented = true;
	}

	updateScannerResult();

	float acqTime = Controller::getScanner()->getAcquisitionTime();
	previewView.setAcquisitionTime(acqTime);
	previewView.setQualityIndex(Measure::getPatternQualityIndex());
	previewView.updateWindow();

	_isScanClosing = false;
	EndWaitCursor();
	return;
}


void DlgAngioPattern::OnBnClickedButtonAngioReload()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	updateAngioControls();
	updateAngioSettings(false);

	m_prepared = false;
	if (Analysis::checkIfProtocolDataExist()) {
		auto data = Analysis::getProtocolData();
		if (!Angiography::initiateAngiogram(data, false)) {
			AfxMessageBox(_T("Angio protocol result is not initialized!"));
			EndWaitCursor();
			return;
		}
		else {
			m_openAngioData2 = false;
		}
	}

	if (m_openAngioData2) {
		AfxMessageBox(_T("Angio data is already prepared!"));
	}
	else if (!Angiography::prepareAngiogram(m_angioAlignAxial, m_angioAlignLateral, !m_dataAligned)) {
		AfxMessageBox(_T("Failed to prepare angio data!"));
	}
	else {
		Angiography::generateMotionData();
		AfxMessageBox(_T("Angio data prepared successfully!"));
		updateScannerResult();
		m_prepared = true;
		m_dataAligned = true;
	}
	EndWaitCursor();
	return;
}


void DlgAngioPattern::OnBnClickedButtonAngioRedraw()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	updateScanControls();
	updateAngioControls();

	auto& angio = Angiography::getAngiogram();
	angio.useLayersSelected() = m_angioLayerSel;
	angio.usePostProcessing() = m_usePostProcessing;
	angio.useNormProjection() = m_useNormProjection;
	angio.useDecorrOutput() = m_useDecorrProjection;
	angio.useDifferOutput() = m_useDifferProjection;
	angio.useProjectionArtifactRemoval() = m_useProjectRemoval;

	angio.decorrLowerThreshold() = m_angioDecorThresh;
	angio.decorrUpperThreshold() = m_angioDecorThresh2;
	angio.differLowerThreshold() = m_angioDifferThresh;
	angio.differUpperThreshold() = m_angioDifferThresh2;
	angio.numberOfOverlaps() = m_angioOverlaps;

	angio.avgOffsetInnFlows() = _avgOffsetInnFlows;
	angio.avgOffsetOutFlows() = _avgOffsetOutFlows;
	angio.thresholdRatioInnFlows() = _thresholdInnFlows;
	angio.thresholdRatioOutFlows() = _thresholdOutFlows;

	angio.avgOffsetVascularMask() = _vascAvgOffset;
	angio.avgOffsetProjectionMask() = _maskAvgOffset;
	angio.weightProjectionMask() = _maskWeight;
	angio.avgOffsetInnProjection() = _projAvgOffset1;
	angio.avgOffsetOutProjection() = _projAvgOffset2;

	Angiography::useMotionCorrection(m_useMotionCorrect);
	Angiography::useBiasFieldCorrection(m_useBiasFieldCorrect);
	Angiography::useVascularLayers(m_useVascularLayers);

	if (m_openAngioData2) {
		if (!Angiography::updateAngiogram2(m_upperLayerType, m_lowerLayerType, m_angioLayerOffset1, m_angioLayerOffset2,
			m_rangeX, m_rangeY, m_angioFoveaCx, m_angioFoveaCy)) {
			AfxMessageBox(_T("Angiogram update failed!"));
		}
		else {
			angioView.setEnhancement(m_angioEnhance, m_angioContrastEq);
			angioView.drawImage(angio.imageBits(), angio.imageWidth(), angio.imageHeight());

			updateScannerResult(false);
		}
	}
	else {
		if (!Angiography::updateAngiogram(m_upperLayerType, m_lowerLayerType, m_angioLayerOffset1, m_angioLayerOffset2,
			m_rangeX, m_rangeY, m_angioFoveaCx, m_angioFoveaCy)) {
			AfxMessageBox(_T("Angiogram update failed!"));
		}
		else {
			float mean, stdev, maxval;
			angio.Decorr().getDecorrelationStat(mean, stdev, maxval);
			angioView.setDecorrelationValues(mean, stdev, maxval);

			angio.Decorr().getIntensityStat(mean, stdev, maxval);
			angioView.setIntensityValues(mean, stdev, maxval);

			angioView.setEnhancement(m_angioEnhance, m_angioContrastEq);
			angioView.drawImage(angio.imageBits(), angio.imageWidth(), angio.imageHeight());

			updateScannerResult();
		}
	}
	EndWaitCursor();
	return;
}

void DlgAngioPattern::OnBnClickedButtonAngioExport()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	CString dirPath, fileName, text;

	auto path = Angiography::getImportDirPath();
	if (!path.empty()) {
		dirPath = atow(path).c_str();
	}
	else {
		dirPath = _T(".//export");
	}

	CTime time = CTime::GetCurrentTime();
	fileName = time.Format(_T("ANGIO_%y%m%d_%H%M%S.png"));

	CString path2 = dirPath + _T("//");
	path2 = path2 + fileName;

	auto contrast = m_angioEnhance ? m_angioContrastEq : 0.0f;
	if (!m_openAngioData2 && Angiography::getAngioImage(0, 0, contrast).saveFile(wtoa(path2), false, 99)) {
		text.Format(_T("Image file: '%s' saved!"), fileName);
		AfxMessageBox(text);
	}
	else {
		if (angioView.saveImage(dirPath, fileName)) {
			text.Format(_T("Image file: '%s' saved!"), fileName);
			AfxMessageBox(text);
		}
		else {
			text = "Saving angio image file failed!";
			AfxMessageBox(text);
		}
	}
	EndWaitCursor();
	return;
}


void DlgAngioPattern::OnCbnSelchangeComboOverlaps()
{
	// TODO: Add your control notification handler code here
	CString text;
	int index = cmbOverlaps.GetCurSel();
	cmbOverlaps.GetLBText(index, text);

	if (PatternHelper::isAngioPatternCode(m_patternCode)) {
		editAngioOverlaps.SetWindowTextW(text);
	}
}


void DlgAngioPattern::OnCbnSelchangeComboLines()
{
	// TODO: Add your control notification handler code here
	CString text;
	int index = cmbLines.GetCurSel();
	cmbLines.GetLBText(index, text);

	if (PatternHelper::isAngioPatternCode(m_patternCode)) {
		editAngioLines.SetWindowTextW(text);
	}
}



void DlgAngioPattern::OnNMCustomdrawSliderPattern(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


void DlgAngioPattern::OnEnChangeEditPatternNo()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void DlgAngioPattern::OnBnClickedCheckExportBuffers()
{
	// TODO: Add your control notification handler code here
	bool check = IsDlgButtonChecked(IDC_CHECK_EXPORT_BUFFERS) == TRUE;
	AngioSetup::setSavingBuffersToFiles(check);
	return;
}


void DlgAngioPattern::OnBnClickedCheckExportCameraBuffer()
{
	// TODO: Add your control notification handler code here
	bool check = IsDlgButtonChecked(IDC_CHECK_EXPORT_CAMERA_BUFFER) == TRUE;
	AngioSetup::setSavingFramesToFiles(check);
}


void DlgAngioPattern::OnBnClickedCheckFixedNoiseReduction()
{
	// TODO: Add your control notification handler code here
	bool checked = IsDlgButtonChecked(IDC_CHECK_FIXED_NOISE_REDUCTION) == TRUE;
	ChainSetup::useFixedNoiseReduction(true, checked);
}



void DlgAngioPattern::OnBnClickedButtonAngioExportData()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	updateAngioControls();

	if (m_openAngioData2 || !Angiography::exportDataFile()) {
		AfxMessageBox(_T("Angio data files exporting error!"));
	}
	else {
		AfxMessageBox(_T("Angio data files exported ok!"));
	}
	EndWaitCursor();
	return;
}


void DlgAngioPattern::OnBnClickedButtonAngioExportData2()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	updateAngioControls();

	if (!Angiography::exportDataFile2()) {
		AfxMessageBox(_T("Exporting angio data file2 error!"));
	}
	else {
		AfxMessageBox(_T("Angio data file2 exported ok!"));
	}
	EndWaitCursor();
	return;
}



void DlgAngioPattern::OnBnClickedButtonAngioImportData()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	updateAngioControls();
	updateAngioSettings(true);

	if (!Angiography::initiateAngiogramByDataFile(m_angioLines, m_angioPoints, m_angioOverlaps, m_angioVertical)) {
		AfxMessageBox(_T("Angio data files importing error!"));
		return;
	}
	else {
		AfxMessageBox(_T("Angio data files imported successfully!"));
		Analysis::clearAllScanData();
		m_openAngioData2 = false;
		m_dataAligned = false;
		updateScannerResult();
	}
	EndWaitCursor();
	return;
}



void DlgAngioPattern::OnBnClickedButtonAngioImportBins()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	updateAngioControls();

	Angiography::clearAngiography();
	if (!Angiography::initiateAngiogramByImageFiles(m_angioLines, m_angioPoints, m_angioOverlaps, m_angioVertical)) {
		AfxMessageBox(_T("Angio images files importing error!"));
		EndWaitCursor();
		return;
	}
	else {
		AfxMessageBox(_T("Angio images files imported successfully!"));
		Analysis::clearAllScanData();
		m_openAngioData2 = false;
		m_dataAligned = false;
	}

	EndWaitCursor();
	return;
}


void DlgAngioPattern::OnBnClickedButtonAngioOpenBins()
{
	// TODO: Add your control notification handler code here
	CString szFilter = _T("Angio Bins (*.bin)|*.bin");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);

	if (IDOK == dlg.DoModal()) {
		CString strDirName = dlg.GetFolderPath();

		BeginWaitCursor();
		updateAngioControls();

		Angiography::clearAngiography();
		auto dirPath = wtoa(strDirName);

		if (!Angiography::initiateAngiogramByImageFiles(m_angioLines, m_angioPoints, m_angioOverlaps, m_angioVertical, dirPath)) {
			AfxMessageBox(_T("Angio images files importing error!"));
			EndWaitCursor();
			return;
		}
		else {
			AfxMessageBox(_T("Angio images files imported successfully!"));
			Analysis::clearAllScanData();
			m_openAngioData2 = false;
			m_dataAligned = false;
			updateScannerResult();
		}

		EndWaitCursor();
	}
}


void DlgAngioPattern::OnBnClickedButtonAngioImportData2()
{
	// TODO: Add your control notification handler code here
	CString szFilter = _T("Angio Data (*.dat)|*.dat");

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);

	if (IDOK == dlg.DoModal()) {
		CString strDirName = dlg.GetFolderPath();
		CString strFileName = dlg.GetFileName();

		auto dirPath = wtoa(strDirName);
		auto fileName = wtoa(strFileName);

		BeginWaitCursor();
		updateAngioControls();
		updateAngioSettings(true);

		if (!Angiography::initiateAngiogramByDataFile(m_angioLines, m_angioPoints, m_angioOverlaps, m_angioVertical, dirPath, fileName)) {
			AfxMessageBox(_T("Angio data files importing error!"));
			return;
		}
		else {
			AfxMessageBox(_T("Angio data files imported successfully!"));
			Analysis::clearAllScanData();
			m_openAngioData2 = false;
			m_dataAligned = false;
			updateScannerResult();
		}
		EndWaitCursor();
	}
}


void DlgAngioPattern::OnBnClickedButtonAngioImportData3()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	updateAngioControls();
	updateAngioSettings(true);

	if (!Angiography::initiateAngiogramByDataFile2()) {
		AfxMessageBox(_T("Importing angio data file2 error!"));
		return;
	}
	else {
		AfxMessageBox(_T("Angio data file2 imported successfully!"));
		Analysis::clearAllScanData();
		m_openAngioData2 = true;
		m_dataAligned = true;
		refreshAngioControls();
	}
	EndWaitCursor();
	return;
}


void DlgAngioPattern::OnBnClickedButtonAngioOpenData2()
{
	CString szFilter = _T("Angio Data2 (*.dat)|*.dat");

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);

	if (IDOK == dlg.DoModal()) {
		CString strDirName = dlg.GetFolderPath();
		CString strFileName = dlg.GetFileName();

		auto dirPath = wtoa(strDirName);
		auto fileName = wtoa(strFileName);

		BeginWaitCursor();
		updateAngioControls();
		updateAngioSettings(true);
		

		if (!Angiography::initiateAngiogramByDataFile2(dirPath, fileName)) {
			AfxMessageBox(_T("Importing angio data file2 error!"));
			return;
		}
		else {
			AfxMessageBox(_T("Angio data file2 imported successfully!"));
			Analysis::clearAllScanData();
			m_openAngioData2 = true;
			m_dataAligned = true;
			refreshAngioControls();
		}
		EndWaitCursor();
	}
}


void DlgAngioPattern::OnBnClickedCheckAngioEnhance()
{
	// TODO: Add your control notification handler code here
}


void DlgAngioPattern::OnBnClickedButtonRetinaRegist()
{
	BeginWaitCursor();
	auto* track = RetinaTracking::getInstance();
	if (track) {
		if (track->requestTargetRegistration() && track->waitForTargetRegistration()) {
			AfxMessageBox(_T("Retina target registered!"));
		}
		else {
			AfxMessageBox(_T("Retina target registration failed!"));
		}
	}
	EndWaitCursor();
	return;
}

void DlgAngioPattern::OnBnClickedButtonRetinaPrepare()
{
	static bool turnOn = true;

	if (turnOn) {
		Controller::prepareLEDsForRetinaTrack(true);
		turnOn = false;
		AfxMessageBox(_T("Retina camera prepared for tracking!"));
	}
	else {
		auto* track = RetinaTracking::getInstance();
		if (track) {
			track->cancelRetinaTracking(true);

			Controller::prepareLEDsForRetinaTrack(false);
			turnOn = true;
			AfxMessageBox(_T("Retina camera released after tracking!"));
		}
	}
	return;
}


void DlgAngioPattern::OnBnClickedButtonRetinaStart()
{
	// TODO: Add your control notification handler code here
	auto* track = RetinaTracking::getInstance();
	if (track) {
		if (!track->isTrackingTargetOn()) {
			if (track->startRetinaTracking(true)) {
				SetDlgItemTextW(IDC_BUTTON_RETINA_START, _T("Pause"));
				AfxMessageBox(_T("Retina tracking started!"));
			}
			else {
				AfxMessageBox(_T("Target is not registered!"));
			}
		}
		else {
			track->pauseRetinaTracking();
			SetDlgItemTextW(IDC_BUTTON_RETINA_START, _T("Start"));
			AfxMessageBox(_T("Retina tracking paused!"));
		}
	}
}


void DlgAngioPattern::OnBnClickedButtonAutoAdjust()
{
	// TODO: Add your control notification handler code here
	static bool enabled = false;

	auto* track = RetinaTracking::getInstance();
	if (track) {
		if (!enabled) {
			track->prepareRetinaTracking(m_eyeSide, m_isDisc, 4.5f, 4.5f);
			track->requestFrameAdjustment(false);
			retinaView.showTrackFeature(true);
			enabled = true;
			AfxMessageBox(_T("Retina camera auto adjust started!"));
		}
		else {
			if (track->completeFrameAdjustment()) {
				AfxMessageBox(_T("Retina camera auto adjust completed!"));
			}
			enabled = false;
		}
	}
}


void DlgAngioPattern::OnBnClickedCheckOverlayDecorr()
{
	bool flag = IsDlgButtonChecked(IDC_CHECK_OVERLAY_DECORR) == TRUE;
	previewView.overlayDecorrelations(flag);
	previewView.updateWindow();
	return;
}


void DlgAngioPattern::OnBnClickedCheckFitToHeight()
{
	bool flag = IsDlgButtonChecked(IDC_CHECK_FIT_TO_HEIGHT) == TRUE;
	previewView.setDisplayStretched(flag);
	previewView.updateWindow();
	return;
}


void DlgAngioPattern::OnBnClickedCheckViewAmplitudes()
{
	m_viewAmplitudes = IsDlgButtonChecked(IDC_CHECK_VIEW_AMPLITUDES) == TRUE;
	return;
}


void DlgAngioPattern::OnCustomdrawSliderOverlap(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


void DlgAngioPattern::OnBnClickedCheckPreviewDecorrs()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_PREVIEW_DECORRS) == TRUE;
	AngioSetup::setPreviewDecorrelations(flag);
	return;
}


void DlgAngioPattern::OnBnClickedRadioCamera3()
{
	// TODO: Add your control notification handler code here
}



void DlgAngioPattern::OnBnClickedCheckRetinaTrack()
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_CHECK_RETINA_TRACK)) {
		retinaView.showTrackTarget(true);
		
	}
	else {
		retinaView.showTrackTarget(false);
	}
}


void DlgAngioPattern::OnBnClickedRadioIntfixOdFundus()
{
	// TODO: Add your control notification handler code here
}


void DlgAngioPattern::OnBnClickedButtonTestSetParams()
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_CHECK_FIXED_NOISE_REDUCTION, m_paramNoiseReduction);
	OnBnClickedCheckFixedNoiseReduction();

	CheckDlgButton(IDC_CHECK_RETINA_TRACK, m_paramRetinaTracking);

	cmbPattern.SetCurSel(m_paramPattern);
	OnCbnSelchangeComboPattern();

	cmbRangeX.SetCurSel(m_paramRangeX);
	cmbRangeY.SetCurSel(m_paramRangeY);
	cmbOverlaps.SetCurSel(m_paramOverlaps);
	cmbPoints.SetCurSel(m_paramPoints);
	cmbLines.SetCurSel(m_paramLines);

	cmbAngioUpper.SetCurSel(m_paramUpperLayer);
	cmbAngioLower.SetCurSel(m_paramLowerLayer);

	CheckDlgButton(IDC_CHECK_ANGIO_POST_PROCESSING, m_paramPostProcessing);
	CheckDlgButton(IDC_CHECK_ANGIO_NORM_PROJECTION, m_paramNormProjection);
	CheckDlgButton(IDC_CHECK_ANGIO_DECOR_PROJECTION, m_paramDecorrProjection);
	CheckDlgButton(IDC_CHECK_ANGIO_LAYERS_SELECT, m_paramLayersSelection);
	CheckDlgButton(IDC_CHECK_ANGIO_MOTION_CORRECT, m_paramMotionCorrection);
	CheckDlgButton(IDC_CHECK_ANGIO_PROJECT_REMOVAL, m_paramProjectionRemoval);
	CheckDlgButton(IDC_CHECK_ANGIO_REFLECT_CORRECT, m_paramReflectCorrect);
	CheckDlgButton(IDC_CHECK_ANGIO_VASCULAR_LAYERS, m_paramVascularLayers);

	CString text;
	text.Format(_T("%d"), m_paramTestAvgLines);
	editTestAvgLines.SetWindowTextW(text);
	text.Format(_T("%d"), m_paramTestTopLines);
	editTestTopLines.SetWindowTextW(text);
	text.Format(_T("%.2f"), m_paramTestDecorrThresh);
	editTestDecorrThresh.SetWindowTextW(text);
}


void DlgAngioPattern::OnBnClickedButtonTestSaveParams()
{
	// TODO: Add your control notification handler code here
	m_paramPattern = cmbPattern.GetCurSel();
	m_paramRangeX = cmbRangeX.GetCurSel();
	m_paramRangeY = cmbRangeY.GetCurSel();
	m_paramOverlaps = cmbOverlaps.GetCurSel();
	m_paramPoints = cmbPoints.GetCurSel();
	m_paramLines = cmbLines.GetCurSel();
	m_paramUpperLayer = cmbAngioLower.GetCurSel();
	m_paramLowerLayer = cmbAngioLower.GetCurSel();

	m_paramNoiseReduction = IsDlgButtonChecked(IDC_CHECK_FIXED_NOISE_REDUCTION);
	m_paramRetinaTracking = IsDlgButtonChecked(IDC_CHECK_RETINA_TRACK);
	m_paramPostProcessing = IsDlgButtonChecked(IDC_CHECK_ANGIO_POST_PROCESSING);
	m_paramNormProjection = IsDlgButtonChecked(IDC_CHECK_ANGIO_NORM_PROJECTION);
	m_paramDecorrProjection = IsDlgButtonChecked(IDC_CHECK_ANGIO_DECOR_PROJECTION);
	m_paramLayersSelection = IsDlgButtonChecked(IDC_CHECK_ANGIO_LAYERS_SELECT);
	m_paramMotionCorrection = IsDlgButtonChecked(IDC_CHECK_ANGIO_MOTION_CORRECT);
	m_paramProjectionRemoval = IsDlgButtonChecked(IDC_CHECK_ANGIO_PROJECT_REMOVAL);
	m_paramReflectCorrect = IsDlgButtonChecked(IDC_CHECK_ANGIO_REFLECT_CORRECT);
	m_paramVascularLayers = IsDlgButtonChecked(IDC_CHECK_ANGIO_VASCULAR_LAYERS);

	CString text;
	editTestAvgLines.GetWindowTextW(text);
	if (!text.IsEmpty()) {
		m_paramTestAvgLines = _ttoi(text);
	}
	editTestTopLines.GetWindowTextW(text);
	if (!text.IsEmpty()) {
		m_paramTestTopLines = _ttoi(text);
	}
	editTestDecorrThresh.GetWindowTextW(text);
	if (!text.IsEmpty()) {
		m_paramTestDecorrThresh = (float)_ttof(text);
	}
}


void DlgAngioPattern::OnBnClickedButtonTestInitParams()
{
	// TODO: Add your control notification handler code here
	m_paramNoiseReduction = FALSE;
	m_paramRetinaTracking = FALSE;
	m_paramPostProcessing = FALSE;
	m_paramNormProjection = TRUE;
	m_paramDecorrProjection = TRUE;
	m_paramLayersSelection = TRUE;
	m_paramMotionCorrection = FALSE;
	m_paramProjectionRemoval = FALSE;
	m_paramVascularLayers = FALSE;

	m_paramPattern = 5;
	m_paramRangeX = 8;
	m_paramRangeY = 8;
	m_paramOverlaps = 1;
	m_paramPoints = 1;
	m_paramLines = 1;
	m_paramUpperLayer = 0;
	m_paramLowerLayer = 6;

	m_paramTestAvgLines = 5;
	m_paramTestTopLines = 5;
	m_paramTestDecorrThresh = 0.15f;
}




void DlgAngioPattern::OnBnClickedCheckTestHorzLines()
{
	// TODO: Add your control notification handler code here
	updateAngioTestControls();
	angioView.setDecorrOverLines(m_testNumAvgLines, m_testNumTopLines, m_testDecorrThresh);
	bool flag = IsDlgButtonChecked(IDC_CHECK_TEST_HORZ_LINES) == TRUE;
	angioView.showDecorrOverHorzLines(flag);
}


void DlgAngioPattern::OnBnClickedCheckTestVertLines()
{
	// TODO: Add your control notification handler code here
	updateAngioTestControls();
	angioView.setDecorrOverLines(m_testNumAvgLines, m_testNumTopLines, m_testDecorrThresh);
	bool flag = IsDlgButtonChecked(IDC_CHECK_TEST_VERT_LINES) == TRUE;
	angioView.showDecorrOverVertLines(flag);
}


void DlgAngioPattern::OnBnClickedButtonTestResult()
{
	// TODO: Add your control notification handler code here
	CString text;

	if (Measure::isScannerResult()) {
		BeginWaitCursor();
		int count = Measure::exportScannerResults();
		if (Analysis::checkIfProtocolDataExist()) {
			auto data = Analysis::getProtocolData();
			Analysis::doSegmentation(data, true);
			m_segmented = true;

			OnBnClickedButtonAngioReload();
			if (m_prepared) {
				OnBnClickedButtonAngioRedraw();
			}
		}
		else {
			text.Format(_T("Scan protocol data is not available!"));
			AfxMessageBox(text);
		}
	}
	else {
		text.Format(_T("Angio scan result is not available!"));
		AfxMessageBox(text);
	}

	EndWaitCursor();
	return;
}


void DlgAngioPattern::OnBnClickedButtonCameraSet()
{
	// TODO: Add your control notification handler code here
	CString text;
	editCameraGain.GetWindowText(text);
	if (!text.IsEmpty()) {
		auto value = (unsigned short)_tcstol(text, NULL, 16);
		Controller::getMainBoard()->setLineCameraGain(value);
	}
}


void DlgAngioPattern::OnBnClickedCheckAngioProjectRemoval()
{
	// TODO: Add your control notification handler code here
}


void DlgAngioPattern::OnBnClickedRadioIntfixOdDisc()
{
	// TODO: Add your control notification handler code here
}



