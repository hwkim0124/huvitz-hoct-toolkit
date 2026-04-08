// DlgSignalTester.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgSignalTester.h"
#include "afxdialogex.h"
#include <time.h>

using namespace OctSystem;
using namespace OctDevice;
using namespace OctConfig;
using namespace std;

// DlgSignalTester dialog

IMPLEMENT_DYNAMIC(DlgSignalTester, CDialogEx)

DlgSignalTester::DlgSignalTester(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGSIGNALTESTER, pParent)
{
	referMotor = Controller::getMainBoard()->getReferenceMotor();
	focusMotor = Controller::getMainBoard()->getOctFocusMotor();
	polarMotor = Controller::getMainBoard()->getPolarizationMotor();
}


DlgSignalTester::~DlgSignalTester()
{
}


void DlgSignalTester::initControls(void)
{
	previewView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 0, 512, 256), this);
	previewView.setDisplaySize(512, 256);

	spectrumView.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, CRect(512, 0, 512+512, 256), this);
	spectrumView.setDisplaySize(512, 256);

	corneaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(1024, 0, 1024+320, 240), this);
	corneaView.setDisplaySize(320, 240);

	signalView.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, CRect(0, 256, 1024, 512), this);
	signalView.setDisplaySize(1024, 512);

	CheckDlgButton(IDC_CHECK_BACKGROUND2, (ChainSetup::useBackgroundSubtraction() ? TRUE : FALSE));
	CheckDlgButton(IDC_CHECK_DISPERSION2, (ChainSetup::useDispersionCompensation() ? TRUE : FALSE));
	
	// Production Line Requirements
	CheckDlgButton(IDC_CHECK_REDUCTION2, FALSE);
	CheckDlgButton(IDC_CHECK_GRAYSCALE2, FALSE);
	/*CheckDlgButton(IDC_CHECK_REDUCTION2, (ChainSetup::useFixedNoiseReduction() ? TRUE : FALSE));
	CheckDlgButton(IDC_CHECK_GRAYSCALE2, (ChainSetup::useAdaptiveGrayscale() ? TRUE : FALSE));*/
	
	CheckDlgButton(IDC_CHECK_CORNEA_DISP2, TRUE);
	CheckDlgButton(IDC_CHECK_WINDOWING, (ChainSetup::useWindowingSpectrum() ? TRUE : FALSE));
	CheckDlgButton(IDC_CHECK_FFT_WINDOW, (ChainSetup::useFFTWindowing() ? TRUE : FALSE));
	CheckDlgButton(IDC_CHECK_RESAMPLE, (ChainSetup::useKLinearResampling() ? TRUE : FALSE));
	CheckDlgButton(IDC_CHECK_EDI_SCAN, AngioSetup::isEdiScanningMode());

	CString text;
	text.Format(_T("%.2f"), ChainSetup::getWindowingFunctionAlpha());
	editWindowAlpha.SetWindowTextW(text);

	OnBnClickedCheckGrayscale2();
	OnBnClickedCheckReduction2();     
	OnBnClickedCheckCorneaDisp2();
	startTime = 0;
	endTime = 0;

	updateGraphDisplay(true);
	updateCaptureControl(true, true);
	updateScanControl(true);
	updateDispersionParams(true, false);
	updateSpectrometerParams(true, false);

	CheckDlgButton(IDC_RADIO_CAMERA1, FALSE);
	CheckDlgButton(IDC_RADIO_CAMERA2, FALSE);
	CheckDlgButton(IDC_RADIO_CAMERA3, TRUE);
	updateRadioCameraSpeed(2);

	if (GlobalSettings::isOctUsbLineCamera80k()) {
		cmbModel.SetCurSel(1);
	}
	else {
		cmbModel.SetCurSel(0);
	}
	return;
}


void DlgSignalTester::controlCameras(bool play)
{
	BeginWaitCursor();
	if (play) {
		Controller::setCorneaCameraImageCallback(corneaView.getCallbackFunction());
		Controller::getMainBoard()->getCorneaIrCamera()->play();
	}
	else {
		Controller::getMainBoard()->getCorneaIrCamera()->pause();
		Controller::setCorneaCameraImageCallback(nullptr);
	}
	EndWaitCursor();
	return;
}


void DlgSignalTester::updateGraphDisplay(bool init)
{
	if (init) {
		cmbPeakMax.SetCurSel(8);
		cmbPeakMin.SetCurSel(0);
		cmbZero.SetCurSel(0);

		editPeakOffset1.SetWindowTextW(_T("10"));
		editPeakOffset2.SetWindowTextW(_T("50"));
		editDataRange1.SetWindowTextW(_T("0"));
		editDataRange2.SetWindowTextW(_T("1025"));

		CheckDlgButton(IDC_CHECK_INTENSITY, TRUE);
		CheckDlgButton(IDC_CHECK_FWHM, FALSE);
		CheckDlgButton(IDC_CHECK_SNR, FALSE);

		editAverageSize.SetWindowTextW(_T("1"));
		editNoiseLevel.SetWindowTextW(_T("0"));
	}
	else {
		CString text;
		int peakMax, peakMin;
		cmbPeakMax.GetWindowText(text);
		peakMax = _ttoi(text);
		cmbPeakMin.GetWindowText(text);
		peakMin = _ttoi(text);

		int range1, range2;
		int offset1, offset2;
		editPeakOffset1.GetWindowTextW(text);
		offset1 = _ttoi(text);
		editPeakOffset2.GetWindowTextW(text);
		offset2 = _ttoi(text);

		editDataRange1.GetWindowTextW(text);
		range1 = _ttoi(text);
		editDataRange2.GetWindowTextW(text);
		range2 = _ttoi(text);

		bool isset;
		isset = (IsDlgButtonChecked(IDC_CHECK_INTENSITY) ? true : false);
		signalView.setChartPeak(isset);
		isset = (IsDlgButtonChecked(IDC_CHECK_FWHM) ? true : false);
		signalView.setChartFWHM(isset);
		isset = (IsDlgButtonChecked(IDC_CHECK_SNR) ? true : false);
		signalView.setChartSNR(isset);

		signalView.setChartPeakRange(peakMax, peakMin);
		signalView.setChartPeakOffset(offset1, offset2);
		signalView.setChartDataRange(range1, range2);

		if (!Controller::isScanning()) {
			signalView.updateWindow();
		}

		int curSel = cmbZero.GetCurSel();
		if (curSel == 0) {
			isZeroPadding = false;
			multipleOfZeros = 1;
		}
		else {
			isZeroPadding = true;
			multipleOfZeros = (curSel == 1 ? 2 : 4);
		}
		signalView.setDataMultiple(multipleOfZeros);

		editAverageSize.GetWindowTextW(text);
		int size = _ttoi(text);
		signalView.setAverageSize(size);

		editNoiseLevel.GetWindowTextW(text);
		float value = (float)_ttof(text);
		signalView.setFixedNoiseLevel(value);
	}
	return;
}


void DlgSignalTester::updateCaptureControl(bool init, bool reset)
{
	int curSel;
	CString text;

	if (reset) {
		cmbControl.SetCurSel(0);
		cmbCStep.SetCurSel(4);
		cmbCount.SetCurSel(1);
	}

	curSel = cmbControl.GetCurSel();

	if (init) {
		if (curSel == 0) {
			referMotor->setControls(nullptr, &editPosition);
		}
		else if (curSel == 1) {
			focusMotor->setControls(nullptr, &editPosition);
		}
		else {
			polarMotor->setControls(nullptr, &editPosition);
		}
		editPosition.GetWindowTextW(text);
		editStart.SetWindowTextW(text);
	}
	else {
		if (curSel == 0) {
			signalView.setCaptureMode(ViewScanSignal::CaptureMode::REFER);
		}
		else if (curSel == 1) {
			signalView.setCaptureMode(ViewScanSignal::CaptureMode::FOCUS);
		}
		else {
			signalView.setCaptureMode(ViewScanSignal::CaptureMode::POLAR);
		}

		editStart.GetWindowText(text);
		int start = _ttoi(text);
		cmbCStep.GetWindowText(text);
		int istep = _ttoi(text);
		cmbCount.GetWindowTextW(text);
		int count = _ttoi(text);
		signalView.setCaptureParams(start, istep, count);
	}
	return;
}


void DlgSignalTester::updateCapturePosition(void)
{
	int curSel = cmbControl.GetCurSel();

	if (curSel == 0) {
		referMotor->updatePositionByEdit();
	}
	else if (curSel == 1) {
		focusMotor->updatePositionByEdit();
	}
	else {
		polarMotor->updatePositionByEdit();
	}

	CString text;
	editPosition.GetWindowTextW(text);
	editStart.SetWindowTextW(text);
	return;
}


void DlgSignalTester::updateScanControl(bool init)
{
	if (init) {
		cmbPattern.SetCurSel(3);
		cmbPoints.SetCurSel(1);
		cmbRange.SetCurSel(6);

		cmbOffsetX.SetCurSel(2);//editOffsetX.SetWindowTextW(_T("0.0"));
		cmbOffsetY.SetCurSel(2);//editOffsetY.SetWindowTextW(_T("0.0"));
		editAngle.SetWindowTextW(_T("0"));
	}
	else {
		int curSel;
		CString text;

		peakIndex = 0;
		peakMax = 0.f;
		scanLines = 1;
		_direction = 0;

		curSel = cmbPattern.GetCurSel();
		switch (curSel) {
		case 1:
			_patternName = PatternName::MacularLine;
			break;
		case 2:
			_patternName = PatternName::MacularLine;
			_direction = 1;
			break;
		case 3:
			_patternName = PatternName::AnteriorPoint;
			break;
		case 4:
			_patternName = PatternName::AnteriorLine;
			break;
		case 5:
			_patternName = PatternName::AnteriorLine;
			_direction = 1;
			break;
		case 0:
		default:
			_patternName = PatternName::MacularPoint;
			break;
		}

		curSel = cmbPoints.GetCurSel();
		if (curSel == 0 || curSel == 3) {
			scanPoints = 512;
		}
		else {
			scanPoints = 1024;
		}
		
		cmbRange.GetWindowText(text);
		scanRange = (float)_ttof(text);

		
		cmbOffsetX.GetWindowTextW(text); //editOffsetX.GetWindowTextW(text);
		scanOffsetX = (float)_ttof(text);
		cmbOffsetY.GetWindowTextW(text); //editOffsetY.GetWindowTextW(text);
		scanOffsetY = (float)_ttof(text);
		editAngle.GetWindowTextW(text);
		scanAngle = (float)_ttof(text);
	}
	return;
}


void DlgSignalTester::updateDispersionParams(bool init, bool zero)
{
	CString text;
	double param[3] = { 0.0 };

	if (init) {
		if (!zero) {
			if (useCorneaDisp) {
				param[0] = SystemConfig::dispersionParameterToCornea(0);
				param[1] = SystemConfig::dispersionParameterToCornea(1);
				param[2] = SystemConfig::dispersionParameterToCornea(2);
			}
			else {
				param[0] = SystemConfig::dispersionParameterToRetina(0);
				param[1] = SystemConfig::dispersionParameterToRetina(1);
				param[2] = SystemConfig::dispersionParameterToRetina(2);
			}
		}

		text.Format(_T("%lf"), param[0]);
		editAlpha2.SetWindowTextW(text);
		text.Format(_T("%lf"), param[1]);
		editAlpha3.SetWindowTextW(text);
		text.Format(_T("%lf"), param[2]);
		editAlpha4.SetWindowTextW(text);
	}
	else {
		editAlpha2.GetWindowTextW(text);
		dispParams[0] = _ttof(text);
		editAlpha3.GetWindowTextW(text);
		dispParams[1] = _ttof(text);
		editAlpha4.GetWindowTextW(text);
		dispParams[2] = _ttof(text);

		if (useCorneaDisp) {
			SystemConfig::dispersionParameterToCornea(0, true, dispParams[0]);
			SystemConfig::dispersionParameterToCornea(1, true, dispParams[1]);
			SystemConfig::dispersionParameterToCornea(2, true, dispParams[2]);
		}
		else {
			SystemConfig::dispersionParameterToRetina(0, true, dispParams[0]);
			SystemConfig::dispersionParameterToRetina(1, true, dispParams[1]);
			SystemConfig::dispersionParameterToRetina(2, true, dispParams[2]);
		}
	}
	return;
}


void DlgSignalTester::updateSpectrometerParams(bool init, bool zero)
{
	CString text;
	double param[4] = { 0.0 };

	if (init) {
		if (!zero) {
			param[0] = SystemConfig::spectrometerParameter(0);
			param[1] = SystemConfig::spectrometerParameter(1);
			param[2] = SystemConfig::spectrometerParameter(2);
			param[3] = SystemConfig::spectrometerParameter(3);
		}

		text.Format(_T("%e"), param[0]);
		editParam1.SetWindowTextW(text);
		text.Format(_T("%e"), param[1]);
		editParam2.SetWindowTextW(text);
		text.Format(_T("%e"), param[2]);
		editParam3.SetWindowTextW(text);
		text.Format(_T("%e"), param[3]);
		editParam4.SetWindowTextW(text);
	}
	else {
		editParam1.GetWindowTextW(text);
		specParams[0] = _ttof(text);
		editParam2.GetWindowTextW(text);
		specParams[1] = _ttof(text);
		editParam3.GetWindowTextW(text);
		specParams[2] = _ttof(text);
		editParam4.GetWindowTextW(text);
		specParams[3] = _ttof(text);

		SystemConfig::spectrometerParameter(0, true, specParams[0]);
		SystemConfig::spectrometerParameter(1, true, specParams[1]);
		SystemConfig::spectrometerParameter(2, true, specParams[2]);
		SystemConfig::spectrometerParameter(3, true, specParams[3]);
	}
	return;
}


void DlgSignalTester::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PEAK_MAX, cmbPeakMax);
	DDX_Control(pDX, IDC_COMBO_PEAK_MIN, cmbPeakMin);
	DDX_Control(pDX, IDC_COMBO_CONTROL, cmbControl);
	DDX_Control(pDX, IDC_EDIT_POSITION, editPosition);
	DDX_Control(pDX, IDC_EDIT_START, editStart);
	//  DDX_Control(pDX, IDC_EDIT_STEP, editStep);
	DDX_Control(pDX, IDC_COMBO_COUNT, cmbCount);
	DDX_Control(pDX, IDC_COMBO_PATTERN, cmbPattern);
	DDX_Control(pDX, IDC_COMBO_RANGE, cmbRange);
	DDX_Control(pDX, IDC_COMBO_POINTS, cmbPoints);
	//	DDX_Control(pDX, IDC_EDIT_OFFSET_X, editOffsetX);
	//	DDX_Control(pDX, IDC_EDIT_OFFSET_Y, editOffsetY);
	DDX_Control(pDX, IDC_EDIT_ANGLE, editAngle);
	DDX_Control(pDX, IDC_EDIT_ALPHA2, editAlpha2);
	DDX_Control(pDX, IDC_EDIT_ALPHA3, editAlpha3);
	DDX_Control(pDX, IDC_EDIT_PARAM1, editParam1);
	DDX_Control(pDX, IDC_EDIT_PARAM3, editParam3);
	DDX_Control(pDX, IDC_EDIT_PARAM2, editParam2);
	DDX_Control(pDX, IDC_EDIT_PARAM4, editParam4);
	DDX_Control(pDX, IDC_COMBO_CSTEP, cmbCStep);
	//  DDX_Control(pDX, IDC_EDIT_Z_RANGE1, editRange1);
	//  DDX_Control(pDX, IDC_EDIT_Z_RANGE2, editRange2);
	// DDX_Control(pDX, IDC_EDIT_PEAK_RANGE1, editPeakRange1);
	// DDX_Control(pDX, IDC_EDIT_PEAK_RANGE2, editPeakRange2);
	DDX_Control(pDX, IDC_EDIT_DATA_RANGE1, editDataRange1);
	DDX_Control(pDX, IDC_EDIT_DATA_RANGE2, editDataRange2);
	DDX_Control(pDX, IDC_EDIT_PEAK_OFFSET1, editPeakOffset1);
	DDX_Control(pDX, IDC_EDIT_PEAK_OFFSET2, editPeakOffset2);
	DDX_Control(pDX, IDC_COMBO_ZERO, cmbZero);
	DDX_Control(pDX, IDC_EDIT_ALPHA4, editAlpha4);
	DDX_Control(pDX, IDC_EDIT_SCALE_X2, editScaleX);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y2, editScaleY);
	DDX_Control(pDX, IDC_EDIT_INTENSITY_AVERAGE, editAverageSize);
	DDX_Control(pDX, IDC_EDIT_INTENSITY_NOISE, editNoiseLevel);
	DDX_Control(pDX, IDC_COMBO_OFFSET_X, cmbOffsetX);
	DDX_Control(pDX, IDC_COMBO_OFFSET_Y, cmbOffsetY);
	DDX_Control(pDX, IDC_COMBO_MODEL, cmbModel);
	DDX_Control(pDX, IDC_EDIT_EXPOS_TIME, editExposTime);
	DDX_Radio(pDX, IDC_RADIO_CAMERA1, _radioCameraSpeed);
	DDX_Control(pDX, IDC_EDIT_WINDOW_ALPHA, editWindowAlpha);
}


BEGIN_MESSAGE_MAP(DlgSignalTester, CDialogEx)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_COMBO_PEAK_MAX, &DlgSignalTester::OnSelchangeComboPeakMax)
	ON_CBN_SELCHANGE(IDC_COMBO_PEAK_MIN, &DlgSignalTester::OnSelchangeComboPeakMin)
	ON_BN_CLICKED(IDC_CHECK_INTENSITY, &DlgSignalTester::OnBnClickedCheckIntensity)
	ON_BN_CLICKED(IDC_CHECK_FWHM, &DlgSignalTester::OnBnClickedCheckFwhm)
	ON_BN_CLICKED(IDC_CHECK_SNR, &DlgSignalTester::OnBnClickedCheckSnr)
	ON_CBN_SELCHANGE(IDC_COMBO_CONTROL, &DlgSignalTester::OnCbnSelchangeComboControl)
	ON_BN_CLICKED(IDC_BUTTON_DISP_INIT, &DlgSignalTester::OnBnClickedButtonDispInit)
	ON_BN_CLICKED(IDC_BUTTON_DISP_ZERO, &DlgSignalTester::OnBnClickedButtonDispZero)
	ON_BN_CLICKED(IDC_BUTTON_PARAM_INIT, &DlgSignalTester::OnBnClickedButtonParamInit)
	ON_BN_CLICKED(IDC_BUTTON_PARAM_ZERO, &DlgSignalTester::OnBnClickedButtonParamZero)
	ON_BN_CLICKED(IDC_CHECK_BACKGROUND2, &DlgSignalTester::OnBnClickedCheckBackground2)
	ON_BN_CLICKED(IDC_CHECK_REDUCTION2, &DlgSignalTester::OnBnClickedCheckReduction2)
	ON_BN_CLICKED(IDC_CHECK_DISPERSION2, &DlgSignalTester::OnBnClickedCheckDispersion2)
	ON_BN_CLICKED(IDC_CHECK_GRAYSCALE2, &DlgSignalTester::OnBnClickedCheckGrayscale2)
	ON_BN_CLICKED(IDC_CHECK_ALIGN_GUIDE, &DlgSignalTester::OnBnClickedCheckAlignGuide)
	ON_BN_CLICKED(IDC_BUTTON_BACKGROUND, &DlgSignalTester::OnBnClickedButtonBackground)
	ON_BN_CLICKED(IDC_BUTTON_START_SCAN2, &DlgSignalTester::OnBnClickedButtonStartScan2)
	ON_BN_CLICKED(IDC_BUTTON_EXIT2, &DlgSignalTester::OnBnClickedButtonExit2)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE, &DlgSignalTester::OnBnClickedButtonCapture)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS, &DlgSignalTester::OnBnClickedButtonFocus)
	ON_BN_CLICKED(IDC_BUTTON_MOTOR, &DlgSignalTester::OnBnClickedButtonMotor)
	ON_BN_CLICKED(IDC_BUTTON_STAGE3, &DlgSignalTester::OnBnClickedButtonStage3)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_FOCUS, &DlgSignalTester::OnBnClickedButtonAutoFocus)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_REFER3, &DlgSignalTester::OnBnClickedButtonAutoRefer3)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_POLAR2, &DlgSignalTester::OnBnClickedButtonAutoPolar2)
	ON_CBN_SELCHANGE(IDC_COMBO_ZERO, &DlgSignalTester::OnSelchangeComboZero)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDC_BUTTON_CAMERA, &DlgSignalTester::OnBnClickedButtonCamera)
	ON_BN_CLICKED(IDC_BUTTON_LEDS, &DlgSignalTester::OnBnClickedButtonLeds)
	ON_BN_CLICKED(IDC_CHECK_CORNEA_DISP2, &DlgSignalTester::OnBnClickedCheckCorneaDisp2)
	ON_BN_CLICKED(IDC_CHECK_NO_IMAGE_GRAB, &DlgSignalTester::OnBnClickedCheckNoImageGrab)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_CAMERA1, IDC_RADIO_CAMERA3, &DlgSignalTester::updateRadioCameraSpeed)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_AUTO_TEST, &DlgSignalTester::OnBnClickedButtonAutoTest)
	ON_BN_CLICKED(IDC_CHECK_FFT_WINDOW, &DlgSignalTester::OnBnClickedCheckFftWindow)
	ON_BN_CLICKED(IDC_BUTTON_POLAR_LOOP, &DlgSignalTester::OnBnClickedButtonPolarLoop)
	ON_BN_CLICKED(IDC_BUTTON_POLAR_PEAK, &DlgSignalTester::OnBnClickedButtonPolarPeak)
	ON_BN_CLICKED(IDC_CHECK_RESAMPLE, &DlgSignalTester::OnBnClickedCheckResample)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_REF_26, &DlgSignalTester::OnBnClickedButtonAutoRef26)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_REF_51, &DlgSignalTester::OnBnClickedButtonAutoRef51)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_REF_381, &DlgSignalTester::OnBnClickedButtonAutoRef381)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_REF_751, &DlgSignalTester::OnBnClickedButtonAutoRef751)
	ON_CBN_SELCHANGE(IDC_COMBO_MODEL, &DlgSignalTester::OnSelchangeComboModel)
	ON_BN_CLICKED(IDC_BUTTON_SET_EXPOS_TIME, &DlgSignalTester::OnBnClickedButtonSetExposTime)
	ON_BN_CLICKED(IDC_CHECK_WINDOWING, &DlgSignalTester::OnBnClickedCheckWindowing)
END_MESSAGE_MAP()


// DlgSignalTester message handlers


BOOL DlgSignalTester::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialogEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


BOOL DlgSignalTester::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	initControls();
	controlCameras(true);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgSignalTester::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL) {
			if (pFocused->GetDlgCtrlID() == editAlpha2.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editAlpha3.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editAlpha4.GetDlgCtrlID()) {
				updateDispersionParams(false, false);
			}
			if (pFocused->GetDlgCtrlID() == editPosition.GetDlgCtrlID()) {
				updateCapturePosition();
			}
			if (pFocused->GetDlgCtrlID() == editAverageSize.GetDlgCtrlID()) {
				CString text;
				editAverageSize.GetWindowTextW(text);
				if (!text.IsEmpty()) {
					int size = _ttoi(text);
					signalView.setAverageSize(size);
				}
			}
			if (pFocused->GetDlgCtrlID() == editNoiseLevel.GetDlgCtrlID()) {
				CString text;
				editNoiseLevel.GetWindowTextW(text);
				if (!text.IsEmpty()) {
					float value = (float)_ttof(text);
					signalView.setFixedNoiseLevel(value);
				}
			}
			if (pFocused->GetDlgCtrlID() == editWindowAlpha.GetDlgCtrlID()) {
				CString text;
				editWindowAlpha.GetWindowTextW(text);
				if (!text.IsEmpty()) {
					float value = (float)_ttof(text);
					ChainSetup::setWindowingFunctionAlpha(value);
				}
			}
		}
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgSignalTester::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	controlCameras(false);
	Controller::closeScan(false);

	referMotor->setControls();
	focusMotor->setControls();
	polarMotor->setControls();

	if (pDlgFocus.get() != nullptr) {
		pDlgFocus->DestroyWindow();
	}
	if (pDlgMotor.get() != nullptr) {
		pDlgMotor->DestroyWindow();
	}
	if (pDlgStage.get() != nullptr) {
		pDlgStage->DestroyWindow();
	}

	CDialogEx::OnClose();
}


void DlgSignalTester::OnSelchangeComboPeakMax()
{
	// TODO: Add your control notification handler code here
	updateGraphDisplay(false);
}


void DlgSignalTester::OnSelchangeComboPeakMin()
{
	// TODO: Add your control notification handler code here
	updateGraphDisplay(false);
}


void DlgSignalTester::OnBnClickedCheckIntensity()
{
	// TODO: Add your control notification handler code here
	updateGraphDisplay(false);
}


void DlgSignalTester::OnBnClickedCheckFwhm()
{
	// TODO: Add your control notification handler code here
	updateGraphDisplay(false);
}


void DlgSignalTester::OnBnClickedCheckSnr()
{
	// TODO: Add your control notification handler code here
	updateGraphDisplay(false);
}


void DlgSignalTester::OnCbnSelchangeComboControl()
{
	// TODO: Add your control notification handler code here
	updateCaptureControl(true, false);
}


void DlgSignalTester::OnBnClickedButtonDispInit()
{
	// TODO: Add your control notification handler code here
	updateDispersionParams(true, false);
}


void DlgSignalTester::OnBnClickedButtonDispZero()
{
	// TODO: Add your control notification handler code here
	updateDispersionParams(true, true);
}


void DlgSignalTester::OnBnClickedButtonParamInit()
{
	// TODO: Add your control notification handler code here
	updateSpectrometerParams(true, false);
}


void DlgSignalTester::OnBnClickedButtonParamZero()
{
	// TODO: Add your control notification handler code here
	updateSpectrometerParams(true, true);
}


void DlgSignalTester::OnBnClickedCheckBackground2()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_BACKGROUND2) ? true : false);
	ChainSetup::useBackgroundSubtraction(true, checked);
}


void DlgSignalTester::OnBnClickedCheckReduction2()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_REDUCTION2) ? true : false);
	ChainSetup::useFixedNoiseReduction(true, checked);
}


void DlgSignalTester::OnBnClickedCheckDispersion2()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_DISPERSION2) ? true : false);
	ChainSetup::useDispersionCompensation(true, checked);
}


void DlgSignalTester::OnBnClickedCheckGrayscale2()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_GRAYSCALE2) ? true : false);
	ChainSetup::useAdaptiveGrayscale(true, checked);
}


void DlgSignalTester::OnBnClickedCheckFftWindow()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_FFT_WINDOW) ? true : false);
	ChainSetup::useFFTWindowing(true, checked);
}


void DlgSignalTester::OnBnClickedCheckWindowing()
{
	bool checked = (IsDlgButtonChecked(IDC_CHECK_WINDOWING) ? true : false);
	ChainSetup::useWindowingSpectrum(true, checked);
}


void DlgSignalTester::OnBnClickedCheckResample()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_RESAMPLE) ? true : false);
	ChainSetup::useKLinearResampling(true, checked);
}



void DlgSignalTester::OnBnClickedCheckAlignGuide()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_ALIGN_GUIDE) ? true : false);

	updateScanControl(false);
	previewView.showAlignGuide(checked, scanRange);
}


void DlgSignalTester::OnBnClickedCheckNoImageGrab()
{
	bool checked = (IsDlgButtonChecked(IDC_CHECK_NO_IMAGE_GRAB) ? true : false);
	useNoImageGrab = checked;
}


void DlgSignalTester::OnBnClickedButtonBackground()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Controller::isScanning()) {
		OnBnClickedButtonStartScan2();
	}

	unsigned short* data = spectrumView.getSpectrumData();
	int width = spectrumView.getDataWidth();
	int height = spectrumView.getDataHeight();

	ChainSetup::getBackground()->makeBackgroundSpectrum(data, width, height);
	EndWaitCursor();
	return;
}


void DlgSignalTester::OnBnClickedButtonStartScan2()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Controller::isScanning()) {
		Controller::closeScan(false);
		SetDlgItemText(IDC_BUTTON_START_SCAN2, L"Start Scan");
	}
	else {
		signalView.clearChartHistory();

		updateGraphDisplay(false);
		updateScanControl(false);
		updateDispersionParams(false);
		updateSpectrometerParams(false);

		ChainSetup::useFFTZeroPadding(true, isZeroPadding);
		ChainSetup::setMultipleOfZeroPaddingSize(multipleOfZeros);
		ChainSetup::useRefreshDispersionParams(true, true);

		bool windowing = (IsDlgButtonChecked(IDC_CHECK_WINDOWING) == TRUE);
		ChainSetup::useWindowingSpectrum(true, windowing);
		if (windowing) {
			CString text;
			editWindowAlpha.GetWindowTextW(text);
			if (!text.IsEmpty()) {
				float value = (float)_ttof(text);
				ChainSetup::setWindowingFunctionAlpha(value);
			}
		}

		Controller::setPreviewImageCallback2(previewView.getCallbackFunction());
		Controller::setSpectrumDataCallback(spectrumView.getSpectrumCallback());
		Controller::setResampleDataCallback(spectrumView.getResampleCallback());
		Controller::setIntensityDataCallback(signalView.getIntensityCallback());

		Controller::presetScanOffset(scanOffsetX, scanOffsetY, scanAngle);

		OctScanMeasure measure;
		measure.getPattern().setup(_patternName, scanPoints, scanLines, 
			scanRange, scanRange, _direction);
		measure.getPattern().setScanOffset(scanOffsetX, scanOffsetY, scanAngle);
		measure.getPattern().setScanSpeed(m_scanSpeed);

		measure.usePreviewPattern(true, true);
		measure.useNoImageGrab(true, useNoImageGrab);

		bool clearPreset = true;
		bool ediScanning = (IsDlgButtonChecked(IDC_CHECK_EDI_SCAN) == TRUE);

		if (Controller::startScan2(measure,
			previewView.getCallbackFunction(), nullptr, clearPreset, false, ediScanning)) {
			SetDlgItemText(IDC_BUTTON_START_SCAN2, L"Close Scan");
		}
		/*
		if (Controller::startScan(EyeSide::OD, PatternDomain::Macular, 
			scanPatternType, scanPoints, scanLines, scanRange, scanRange)) {
			SetDlgItemText(IDC_BUTTON_START_SCAN2, L"Capture");
		}
		*/
	}
	EndWaitCursor();
	return;
}


void DlgSignalTester::updateRadioCameraSpeed(UINT value)
{
	UpdateData(TRUE);

	bool retb = true;
	switch (_radioCameraSpeed) {
	case 0:
		retb = Scanner::changeScanSpeedToNormal();
		m_scanSpeed = ScanSpeed::Normal;
		break;
	case 1:
		retb = Scanner::changeScanSpeedToFaster();
		m_scanSpeed = ScanSpeed::Faster;
		break;
	case 2:
		retb = Scanner::changeScanSpeedToFastest();
		m_scanSpeed = ScanSpeed::Fastest;
		break;
	}

	if (!retb) {
		AfxMessageBox(_T("Change scan speed failed!"));
	}
	else {
		updateExposureTime();
	}
	return;
}

void DlgSignalTester::updateExposureTime(void)
{
	CString text;
	auto expTime = Scanner::getCameraExposureTime();
	text.Format(_T("%.1f"), expTime);
	SetDlgItemText(IDC_EDIT_EXPOS_TIME, text);
}

void DlgSignalTester::OnSelchangeComboModel()
{
	// TODO: Add your control notification handler code here
	if (cmbModel.GetCurSel() == 1) {
		GlobalSettings::setOctUsbLineCameraMode((int)OctLineCameraMode::LINE_CAMERA_80K);
	}
	else {
		GlobalSettings::setOctUsbLineCameraMode((int)OctLineCameraMode::LINE_CAMERA_68K);
	}
	updateRadioCameraSpeed(_radioCameraSpeed);
}


void DlgSignalTester::OnBnClickedButtonExit2()
{
	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);
	return;
}


void DlgSignalTester::OnBnClickedButtonCapture()
{
	// TODO: Add your control notification handler code here
	const int TIMER_ID_REFERENCE_LOOP = 1001;
	const int TIMER_EVENT_DELAY = 100;

	if (runReferenceLoop) {
		KillTimer(TIMER_ID_REFERENCE_LOOP);
		runReferenceLoop = false;
	}
	else {
		SetTimer(TIMER_ID_REFERENCE_LOOP, TIMER_EVENT_DELAY, NULL);
		runReferenceLoop = true;
		isMovingToUpper = true;
		referMotor->updatePositionByOffset(+500);
	}

	return;
}



void DlgSignalTester::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (runReferenceLoop) {
		if (referMotor->isEndOfUpperPosition()) {
			isMovingToUpper = false;
		}
		if (referMotor->isEndOfLowerPosition()) {
			isMovingToUpper = true;
		}
		referMotor->updatePositionByOffset(500 * (isMovingToUpper ? +1 : -1));
	}

	if (runPolarizationLoop) {
		polarMotor->updateStatus();

		if (polarMotor->isEndOfUpperPosition()) {
			isMovingToUpperPolar = false;
		}
		if (polarMotor->isEndOfLowerPosition()) {
			isMovingToUpperPolar = true;
		}
		polarMotor->updatePositionByOffset(10 * (isMovingToUpperPolar ? +1 : -1));
	}

	if (runFindPeakPolar) {
		
		polarMotor->updatePositionByOffset(+5);
		if (peakMax < signalView.getPeakTopValue()) {
			peakMax = signalView.getPeakTopValue();
			peakIndex = polarMotor->getPosition();
		}
		if (polarMotor->isEndOfUpperPosition()) {
			SetDlgItemText(IDC_BUTTON_POLAR_PEAK, L"Find Peak Polar");
			polarMotor->updatePosition(peakIndex);
			runFindPeakPolar = false;
		}
		
	}

	if (runAutoTestLoop) {
		switch (autoTestStep)
		{
		case 1:
			endTime = clock();
			if (endTime - startTime >= 12000) {
				OnBnClickedButtonStartScan2();
				cmbOffsetX.SetCurSel(4);//editOffsetX.SetWindowTextW(_T("3.0"));
				cmbOffsetY.SetCurSel(2);//editOffsetY.SetWindowTextW(_T("0.0"));
				OnBnClickedButtonStartScan2();
				autoTestStep += 1;
				startTime = clock();
			}
			break;
		case 2:
			endTime = clock();
			if (endTime - startTime >= 12000) {
				OnBnClickedButtonStartScan2();
				cmbOffsetX.SetCurSel(2);//editOffsetX.SetWindowTextW(_T("0.0"));
				cmbOffsetY.SetCurSel(2);//editOffsetY.SetWindowTextW(_T("0.0"));
				OnBnClickedButtonStartScan2();
				autoTestStep += 1;
				startTime = clock();
			}
			break;
		case 3:
			endTime = clock();
			if (endTime - startTime >= 12000) {
				OnBnClickedButtonStartScan2();
				cmbOffsetX.SetCurSel(1);//editOffsetX.SetWindowTextW(_T("-3.0"));
				cmbOffsetY.SetCurSel(2);//editOffsetY.SetWindowTextW(_T("0.0"));
				OnBnClickedButtonStartScan2();
				autoTestStep += 1;
				startTime = clock();
			}
			break;
		case 4:
			endTime = clock();
			if (endTime - startTime >= 12000) {
				OnBnClickedButtonStartScan2();
				cmbOffsetX.SetCurSel(0);//editOffsetX.SetWindowTextW(_T("-4.5"));
				cmbOffsetY.SetCurSel(2);//editOffsetY.SetWindowTextW(_T("0.0"));
				OnBnClickedButtonStartScan2();
				autoTestStep += 1;
				startTime = clock();
			}
			break;
		case 5:
			endTime = clock();
			if (endTime - startTime >= 12000) {
				OnBnClickedButtonStartScan2();
				cmbOffsetX.SetCurSel(2);//editOffsetX.SetWindowTextW(_T("0.0"));
				cmbOffsetY.SetCurSel(5);//editOffsetY.SetWindowTextW(_T("4.5"));
				OnBnClickedButtonStartScan2();
				autoTestStep += 1;
				startTime = clock();
			}
			break;
		case 6:
			endTime = clock();
			if (endTime - startTime >= 12000) {
				OnBnClickedButtonStartScan2();
				cmbOffsetX.SetCurSel(2);//editOffsetX.SetWindowTextW(_T("0.0"));
				cmbOffsetY.SetCurSel(4);//editOffsetY.SetWindowTextW(_T("3.0"));
				OnBnClickedButtonStartScan2();
				autoTestStep += 1;
				startTime = clock();
			}
			break;
		case 7:
			endTime = clock();
			if (endTime - startTime >= 12000) {
				OnBnClickedButtonStartScan2();
				cmbOffsetX.SetCurSel(2);//editOffsetX.SetWindowTextW(_T("0.0"));
				cmbOffsetY.SetCurSel(1);//editOffsetY.SetWindowTextW(_T("-3.0"));
				OnBnClickedButtonStartScan2();
				autoTestStep += 1;
				startTime = clock();
			}
			break;
		case 8:
			endTime = clock();
			if (endTime - startTime >= 12000) {
				OnBnClickedButtonStartScan2();
				cmbOffsetX.SetCurSel(2);//editOffsetX.SetWindowTextW(_T("0.0"));
				cmbOffsetY.SetCurSel(0);//editOffsetY.SetWindowTextW(_T("-4.5"));
				OnBnClickedButtonStartScan2();
				autoTestStep += 1;
				startTime = clock();
			}
			break;
		case 9:
			endTime = clock();
			if (endTime - startTime >= 12000) {
				OnBnClickedButtonAutoTest();
				cmbOffsetX.SetCurSel(2);//editOffsetX.SetWindowTextW(_T("0.0"));
				cmbOffsetY.SetCurSel(2);//editOffsetY.SetWindowTextW(_T("0.0"));
			}
			break;
		}
	}

	else if (runFindPeakRef) {
		int diff = 0;

		switch (autoFindRefStep)
		{
		case 0://26
			diff = signalView.getPeakIndexDiff(26);

			if (diff <= 20) {
				if (diff <= 2) {
					runFindPeakRef = false;
					SetDlgItemText(IDC_BUTTON_AUTO_REF_26, L"Peak at 26");
					break;
				}
				referMotor->updatePositionByOffset(-1);
			}
			else if (diff <= 100) {
				referMotor->updatePositionByOffset(-5);
			}
			else if (diff > 100) {
				referMotor->updatePositionByOffset(-30);
			}
			break;

		case 1: //51
			diff = signalView.getPeakIndexDiff(51);

			if (diff <= 20) {
				if (diff <= 2) {
					runFindPeakRef = false;
					SetDlgItemText(IDC_BUTTON_AUTO_REF_51, L"Peak at 51");
					break;
				}
				referMotor->updatePositionByOffset(-1);
			}
			else if (diff <= 100) {
				referMotor->updatePositionByOffset(-5);
			}
			else if (diff > 100) {
				referMotor->updatePositionByOffset(-30);
			}
			break;

		case 2://381
			diff = signalView.getPeakIndexDiff(381);

			if (diff <= 20) {
				if (diff <= 2) {
					runFindPeakRef = false;
					SetDlgItemText(IDC_BUTTON_AUTO_REF_381, L"Peak at 381");
					break;
				}
				referMotor->updatePositionByOffset(1);
			}
			else if (diff <= 100) {
				referMotor->updatePositionByOffset(5);
			}
			else if (diff > 100) {
				referMotor->updatePositionByOffset(30);
			}
			break;
		case 3://751
			diff = signalView.getPeakIndexDiff(751);

			if (diff <= 20) {
				if (diff <= 2) {
					runFindPeakRef = false;
					SetDlgItemText(IDC_BUTTON_AUTO_REF_751, L"Peak at 751");
					break;
				}
				referMotor->updatePositionByOffset(1);
			}
			else if (diff <= 100) {
				referMotor->updatePositionByOffset(5);
			}
			else if (diff > 100) {
				referMotor->updatePositionByOffset(30);
			}
			break;
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}



void DlgSignalTester::OnBnClickedButtonFocus()
{
	// TODO: Add your control notification handler code here
	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);
	return;
}


void DlgSignalTester::OnBnClickedButtonMotor()
{
	// TODO: Add your control notification handler code here
	if (pDlgMotor.get() == nullptr) {
		pDlgMotor = make_unique<DlgMotorControl>();
		pDlgMotor->Create(IDD_DLGMOTORCONTROL, this);
	}
	pDlgMotor->ShowWindow(SW_SHOW);
	return;
}


void DlgSignalTester::OnBnClickedButtonStage3()
{
	// TODO: Add your control notification handler code here
	if (pDlgStage.get() == nullptr) {
		pDlgStage = make_unique<DlgStageControl>();
		pDlgStage->Create(IDD_DLGSTAGECONTROL, this);
	}
	pDlgStage->ShowWindow(SW_SHOW);
	return;
}


void DlgSignalTester::OnBnClickedButtonCamera()
{
	// TODO: Add your control notification handler code here
	if (pDlgCamera.get() == nullptr) {
		pDlgCamera = make_unique<DlgCameraControl>();
		pDlgCamera->Create(IDD_DLGCAMERACONTROL, this);
	}
	pDlgCamera->ShowWindow(SW_SHOW);
	return;
}


void DlgSignalTester::OnBnClickedButtonLeds()
{
	// TODO: Add your control notification handler code here
	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);
	return;
}


void DlgSignalTester::OnBnClickedButtonAutoFocus()
{
	// TODO: Add your control notification handler code here
	if (Controller::isAutoDiopterFocusing()) {
		Controller::cancelAutoDiopterFocus();
	}
	else {
		Controller::startAutoDiopterFocus();
	}
	return;
}


void DlgSignalTester::OnBnClickedButtonAutoRefer3()
{
	// TODO: Add your control notification handler code here
	if (Controller::isAutoReferencing()) {
		Controller::cancelAutoReference();
	}
	else {
		Controller::startAutoReference();
	}
	return;
}


void DlgSignalTester::OnBnClickedButtonAutoPolar2()
{
	// TODO: Add your control notification handler code here
	if (Controller::isAutoPolarizing()) {
		Controller::cancelAutoPolarization();
	}
	else {
		Controller::startAutoPolarization();
	}
	return;
}


void DlgSignalTester::OnSelchangeComboZero()
{
	// TODO: Add your control notification handler code here
}


void DlgSignalTester::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rect = CRect(512, 0, 512 + 512, 256);
	if (rect.PtInRect(point)) {
		point -= CPoint(rect.left, rect.top);
		spectrumView.OnLButtonDown(nFlags, point);
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}


void DlgSignalTester::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rect = CRect(512, 0, 512 + 512, 256);
	if (rect.PtInRect(point)) {
		point -= CPoint(rect.left, rect.top);
		spectrumView.OnRButtonDown(nFlags, point);
	}

	CDialogEx::OnRButtonDown(nFlags, point);
}


void DlgSignalTester::OnBnClickedCheckCorneaDisp2()
{
	// TODO: Add your control notification handler code here
	useCorneaDisp = (IsDlgButtonChecked(IDC_CHECK_CORNEA_DISP2) ? true : false);
	updateDispersionParams(true, false);
	return;
}



void DlgSignalTester::OnBnClickedButtonAutoTest()
{
	const int TIMER_ID_AUTO_TEST_LOOP = 1002;
	const int TIMER_EVENT_DELAY = 100;

	if (runAutoTestLoop) {
		SetDlgItemText(IDC_BUTTON_AUTO_TEST, L"Auto Test");
		cmbOffsetX.SetCurSel(2); //editOffsetX.SetWindowTextW(_T("0.0"));
		cmbOffsetY.SetCurSel(2); //editOffsetY.SetWindowTextW(_T("0.0"));
		runAutoTestLoop = false;
		OnBnClickedButtonStartScan2();
		KillTimer(TIMER_ID_AUTO_TEST_LOOP);
	}
	else {
		SetDlgItemText(IDC_BUTTON_AUTO_TEST, L"Stop Test");
		cmbOffsetX.SetCurSel(5); //editOffsetX.SetWindowTextW(_T("4.5"));
		cmbOffsetY.SetCurSel(2); //editOffsetY.SetWindowTextW(_T("0.0"));
		OnBnClickedButtonStartScan2();
		SetTimer(TIMER_ID_AUTO_TEST_LOOP, TIMER_EVENT_DELAY, NULL);
		runAutoTestLoop = true;
		startTime = clock();
		autoTestStep = 1;
	}
}




void DlgSignalTester::OnBnClickedButtonPolarLoop()
{
	// TODO: Add your control notification handler code here
	const int TIMER_ID_POLARIZATION_LOOP = 1002;
	const int TIMER_EVENT_DELAY = 100;

	if (runPolarizationLoop) {
		KillTimer(TIMER_ID_POLARIZATION_LOOP);
		runPolarizationLoop = false;
	}
	else {
		SetTimer(TIMER_ID_POLARIZATION_LOOP, TIMER_EVENT_DELAY, NULL);
		runPolarizationLoop = true;
		isMovingToUpperPolar = true;
		polarMotor->updatePositionByOffset(+10);
	}

	return;
}


void DlgSignalTester::OnBnClickedButtonPolarPeak()
{
	const int TIMER_ID_FIND_PEAK_POLARIZATION = 1003;
	const int TIMER_EVENT_DELAY = 10;

	if (!Controller::isScanning()) {
		OnBnClickedButtonStartScan2();
	}
	
	if (runFindPeakPolar) {
		SetDlgItemText(IDC_BUTTON_POLAR_PEAK, L"Find Peak Polar");
		KillTimer(TIMER_ID_FIND_PEAK_POLARIZATION);
		runFindPeakPolar = false;
		polarMotor->updatePosition(peakIndex);
	}
	else {
		SetDlgItemText(IDC_BUTTON_POLAR_PEAK, L"Finding...");
		polarMotor->updatePosition(polarMotor->getRangeMin());
		SetTimer(TIMER_ID_FIND_PEAK_POLARIZATION, TIMER_EVENT_DELAY, NULL);
		runFindPeakPolar = true;	
	}

	return;
}


void DlgSignalTester::OnBnClickedButtonAutoRef26()
{
	const int TIMER_ID_FIND_PEAK_REFERENCE = 1004;
	const int TIMER_EVENT_DELAY = 10;

	if (!Controller::isScanning()) {
		OnBnClickedButtonStartScan2();
	}

	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);

	if (runFindPeakRef) {
		SetDlgItemText(IDC_BUTTON_AUTO_REF_26, L"Peak at 26");
		KillTimer(TIMER_ID_FIND_PEAK_REFERENCE);
		runFindPeakRef = false;
	}
	else {
		SetDlgItemText(IDC_BUTTON_AUTO_REF_26, L"Finding...");
		SetTimer(TIMER_ID_FIND_PEAK_REFERENCE, TIMER_EVENT_DELAY, NULL);
		runFindPeakRef = true;
		autoFindRefStep = 0;
	}
	return;
}


void DlgSignalTester::OnBnClickedButtonAutoRef51()
{
	const int TIMER_ID_FIND_PEAK_REFERENCE = 1004;
	const int TIMER_EVENT_DELAY = 10;

	if (!Controller::isScanning()) {
		OnBnClickedButtonStartScan2();
	}

	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);

	if (runFindPeakRef) {
		SetDlgItemText(IDC_BUTTON_AUTO_REF_51, L"Peak at 51");
		KillTimer(TIMER_ID_FIND_PEAK_REFERENCE);
		runFindPeakRef = false;
	}
	else {
		SetDlgItemText(IDC_BUTTON_AUTO_REF_51, L"Finding...");
		SetTimer(TIMER_ID_FIND_PEAK_REFERENCE, TIMER_EVENT_DELAY, NULL);
		runFindPeakRef = true;
		autoFindRefStep = 1;
	}
	return;
}


void DlgSignalTester::OnBnClickedButtonAutoRef381()
{
	const int TIMER_ID_FIND_PEAK_REFERENCE = 1004;
	const int TIMER_EVENT_DELAY = 10;

	if (!Controller::isScanning()) {
		OnBnClickedButtonStartScan2();
	}

	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);

	if (runFindPeakRef) {
		SetDlgItemText(IDC_BUTTON_AUTO_REF_381, L"Peak at 381");
		KillTimer(TIMER_ID_FIND_PEAK_REFERENCE);
		runFindPeakRef = false;
	}
	else {
		SetDlgItemText(IDC_BUTTON_AUTO_REF_381, L"Finding...");
		SetTimer(TIMER_ID_FIND_PEAK_REFERENCE, TIMER_EVENT_DELAY, NULL);
		runFindPeakRef = true;
		autoFindRefStep = 2;
	}
	return;
}


void DlgSignalTester::OnBnClickedButtonAutoRef751()
{
	const int TIMER_ID_FIND_PEAK_REFERENCE = 1004;
	const int TIMER_EVENT_DELAY = 10;

	if (!Controller::isScanning()) {
		OnBnClickedButtonStartScan2();
	}

	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);

	if (runFindPeakRef) {
		SetDlgItemText(IDC_BUTTON_AUTO_REF_751, L"Peak at 751");
		KillTimer(TIMER_ID_FIND_PEAK_REFERENCE);
		runFindPeakRef = false;
	}
	else {
		SetDlgItemText(IDC_BUTTON_AUTO_REF_751, L"Finding...");
		SetTimer(TIMER_ID_FIND_PEAK_REFERENCE, TIMER_EVENT_DELAY, NULL);
		runFindPeakRef = true;
		autoFindRefStep = 3;
	}
	return;
}


void DlgSignalTester::OnBnClickedButtonSetExposTime()
{
	CString text;
	editExposTime.GetWindowTextW(text);
	if (!text.IsEmpty()) {
		float expTime = _ttof(text);
		GlobalMemory::setOctManualExposureTime(expTime);
	}
	else {
		GlobalMemory::clearOctManualExposureTime();
	}
}
