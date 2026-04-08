// DlgDispersionCompensation.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgDispersionCompensation.h"
#include "afxdialogex.h"


// DlgDispersionCompensation dialog

IMPLEMENT_DYNAMIC(DlgDispersionCompensation, CDialogEx)

DlgDispersionCompensation::DlgDispersionCompensation(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGDISPERSIONCOMPENSATION, pParent)
{

}


DlgDispersionCompensation::~DlgDispersionCompensation()
{
}


void DlgDispersionCompensation::initControls(void)
{
	corneaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 0, 320, 240), this);
	retinaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 240, 320, 240 + 240), this);

	corneaView.setDisplaySize(320, 240);
	retinaView.setDisplaySize(320, 240);

	previewView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(320, 0, 320+1024, 512), this);
	previewView.setDisplaySize(1024, 512);

	spectrumView.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, CRect(0, 512 + 5, 512, 512 + 5 + 256), this);
	spectrumView.setDisplaySize(512, 256);

	intensityView.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, CRect(512, 512 + 5, 512 + 512, 512 + 5 + 256), this);
	intensityView.setDisplaySize(512, 256);

	updateDispersionParams(true);
	updateScanControls(true);

	CheckDlgButton(IDC_CHECK_BACKGROUND, (ChainSetup::useBackgroundSubtraction() ? TRUE : FALSE));
	CheckDlgButton(IDC_CHECK_DISPERSION, (ChainSetup::useDispersionCompensation() ? TRUE : FALSE));
	CheckDlgButton(IDC_CHECK_REDUCTION, (ChainSetup::useFixedNoiseReduction() ? TRUE : FALSE));
	CheckDlgButton(IDC_CHECK_GRAYSCALE, (ChainSetup::useAdaptiveGrayscale() ? TRUE : FALSE));

	CheckDlgButton(IDC_CHECK_ALIGN_GUIDE, FALSE);
	CheckDlgButton(IDC_CHECK_VSCAN, FALSE);
	return;
}


void DlgDispersionCompensation::controlCameras(bool play)
{
	BeginWaitCursor();
	if (play) {
		Controller::setCorneaCameraImageCallback(corneaView.getCallbackFunction());
		Controller::setRetinaCameraImageCallback(retinaView.getCallbackFunction());

		Controller::getMainBoard()->getCorneaIrCamera()->play();
		Controller::getMainBoard()->getRetinaIrCamera()->play();
	}
	else {
		Controller::getMainBoard()->getCorneaIrCamera()->pause();
		Controller::getMainBoard()->getRetinaIrCamera()->pause();

		Controller::setCorneaCameraImageCallback(nullptr);
		Controller::setRetinaCameraImageCallback(nullptr);
	}
	EndWaitCursor();
	return;
}


void DlgDispersionCompensation::updateDispersionParams(bool init)
{
	CString str;
	if (init == true) {
		double param1, param2;

		if (useCorneaDisp) {
			param1 = SystemConfig::dispersionParameterToCornea(0);
			param2 = SystemConfig::dispersionParameterToCornea(1);
		}
		else {
			param1 = SystemConfig::dispersionParameterToRetina(0);
			param2 = SystemConfig::dispersionParameterToRetina(1);
		}

		str.Format(_T("%.1lf"), param1);
		editAlpha2.SetWindowText(str);
		str.Format(_T("%.1lf"), param2);
		editAlpha3.SetWindowText(str);
	}
	else {
		editAlpha2.GetWindowText(str);
		double alpha2 = (float)(str.IsEmpty() ? 0.0 : _ttof(str));
		editAlpha3.GetWindowText(str);
		double alpha3 = (float)(str.IsEmpty() ? 0.0 : _ttof(str));

		if (useCorneaDisp) {
			SystemConfig::dispersionParameterToCornea(0, true, alpha2);
			SystemConfig::dispersionParameterToCornea(1, true, alpha3);
			SystemConfig::dispersionParameterToCornea(2, true, 0.0);
		}
		else {
			SystemConfig::dispersionParameterToRetina(0, true, alpha2);
			SystemConfig::dispersionParameterToRetina(1, true, alpha3);
			SystemConfig::dispersionParameterToRetina(2, true, 0.0);
		}
	}
	return;
}


void DlgDispersionCompensation::updateScanControls(bool init)
{
	CString cstr;
	if (init == true) {
		cmbPattern.SetCurSel(0);
		cmbRange.SetCurSel(3);
		cmbPoints.SetCurSel(1);

		editOffsetX.SetWindowTextW(_T("0.0"));
		editOffsetY.SetWindowTextW(_T("0.0"));
		editAngle.SetWindowTextW(_T("0"));
		editScaleX.SetWindowTextW(_T("1.0"));
		editScaleY.SetWindowTextW(_T("1.0"));
	}
	else {
		OnSelchangeComboPattern();
		OnSelchangeComboPoints();
		OnSelchangeComboRange();

		editOffsetX.GetWindowText(cstr);
		offsetX = (float)(cstr.IsEmpty() ? 0.0f : _ttof(cstr));
		editOffsetY.GetWindowText(cstr);
		offsetY = (float)(cstr.IsEmpty() ? 0.0f : _ttof(cstr));
		editScaleX.GetWindowText(cstr);
		scaleX = (float)(cstr.IsEmpty() ? 1.0f : _ttof(cstr));
		editScaleY.GetWindowText(cstr);
		scaleY = (float)(cstr.IsEmpty() ? 1.0f : _ttof(cstr));
		editAngle.GetWindowText(cstr);
		angle = (float)(cstr.IsEmpty() ? 0.0f : _ttof(cstr));
	}
	return;
}


void DlgDispersionCompensation::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_START_SCAN, btnStartScan);
	DDX_Control(pDX, IDC_EDIT_ALPHA2, editAlpha2);
	DDX_Control(pDX, IDC_EDIT_ALPHA3, editAlpha3);
	DDX_Control(pDX, IDC_COMBO_RANGE, cmbRange);
	DDX_Control(pDX, IDC_COMBO_POINTS, cmbPoints);
	DDX_Control(pDX, IDC_COMBO_PATTERN, cmbPattern);
	DDX_Control(pDX, IDC_EDIT_ANGLE, editAngle);
	DDX_Control(pDX, IDC_EDIT_OFFSET_X, editOffsetX);
	DDX_Control(pDX, IDC_EDIT_OFFSET_Y, editOffsetY);
	DDX_Control(pDX, IDC_EDIT_SCALE_X, editScaleX);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y, editScaleY);
}


BEGIN_MESSAGE_MAP(DlgDispersionCompensation, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_START_SCAN, &DlgDispersionCompensation::OnClickedButtonStartScan)
	ON_BN_CLICKED(IDC_BUTTON_BACKGROUND, &DlgDispersionCompensation::OnBnClickedButtonBackground)
	ON_BN_CLICKED(IDC_CHECK_BACKGROUND, &DlgDispersionCompensation::OnBnClickedCheckBackground)
	ON_BN_CLICKED(IDC_CHECK_DISPERSION, &DlgDispersionCompensation::OnBnClickedCheckDispersion)
	ON_BN_CLICKED(IDC_CHECK_REDUCTION, &DlgDispersionCompensation::OnBnClickedCheckReduction)
	ON_BN_CLICKED(IDC_BUTTON_PARAM_DEFAULT, &DlgDispersionCompensation::OnBnClickedButtonParamDefault)
	ON_BN_CLICKED(IDC_BUTTON_PARAM_ZERO, &DlgDispersionCompensation::OnBnClickedButtonParamZero)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS, &DlgDispersionCompensation::OnBnClickedButtonFocus)
	ON_BN_CLICKED(IDC_BUTTON_MOTOR2, &DlgDispersionCompensation::OnBnClickedButtonMotor2)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA2, &DlgDispersionCompensation::OnBnClickedButtonCamera2)
	ON_BN_CLICKED(IDC_BUTTON_LIGHT2, &DlgDispersionCompensation::OnBnClickedButtonLight2)
	ON_BN_CLICKED(IDC_BUTTON_EXIT2, &DlgDispersionCompensation::OnBnClickedButtonExit2)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_IMAGE, &DlgDispersionCompensation::OnBnClickedButtonSaveImage)
	ON_BN_CLICKED(IDC_CHECK_ALIGN_GUIDE, &DlgDispersionCompensation::OnBnClickedCheckAlignGuide)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_FOCUS, &DlgDispersionCompensation::OnBnClickedButtonAutoFocus)
	ON_BN_CLICKED(IDC_CHECK_GRAYSCALE, &DlgDispersionCompensation::OnBnClickedCheckGrayscale)
	ON_BN_CLICKED(IDC_CHECK_VSCAN, &DlgDispersionCompensation::OnBnClickedCheckVscan)
	ON_CBN_SELCHANGE(IDC_COMBO_RANGE, &DlgDispersionCompensation::OnSelchangeComboRange)
	ON_CBN_SELCHANGE(IDC_COMBO_POINTS, &DlgDispersionCompensation::OnSelchangeComboPoints)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_POLAR, &DlgDispersionCompensation::OnBnClickedButtonAutoPolar)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_REFER, &DlgDispersionCompensation::OnBnClickedButtonAutoRefer)
	ON_CBN_SELCHANGE(IDC_COMBO_PATTERN, &DlgDispersionCompensation::OnSelchangeComboPattern)
	ON_BN_CLICKED(IDC_BUTTON_STAGE, &DlgDispersionCompensation::OnBnClickedButtonStage)
	ON_BN_CLICKED(IDC_CHECK_SNR_RATIOS, &DlgDispersionCompensation::OnBnClickedCheckSnrRatios)
	ON_BN_CLICKED(IDC_CHECK_AUTO_ALIGN, &DlgDispersionCompensation::OnBnClickedCheckAutoAlign)
	ON_BN_CLICKED(IDC_CHECK_CORNEA_DISP, &DlgDispersionCompensation::OnBnClickedCheckCorneaDisp)
END_MESSAGE_MAP()


// DlgDispersionCompensation message handlers


void DlgDispersionCompensation::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	controlCameras(false);
	Controller::closeScan(false);

	if (pDlgFocus.get() != nullptr) {
		pDlgFocus->DestroyWindow();
	}
	if (pDlgCamera.get() != nullptr) {
		pDlgCamera->DestroyWindow();
	}
	if (pDlgLight.get() != nullptr) {
		pDlgLight->DestroyWindow();
	}
	if (pDlgMotor.get() != nullptr) {
		pDlgMotor->DestroyWindow();
	}
	if (pDlgStage.get() != nullptr) {
		pDlgStage->DestroyWindow();
	}

	CDialogEx::OnClose();
}


BOOL DlgDispersionCompensation::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	initControls();
	controlCameras(true);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgDispersionCompensation::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL) {
			if (pFocused->GetDlgCtrlID() == editAlpha2.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editAlpha3.GetDlgCtrlID()) {
				updateDispersionParams(false);
			}
		}
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

/*
unsigned char result[1024 * 1024];
float init_data[2048];
float shifted[10];


void DlgDispersionCompensation::OnClickedButtonCalibrate()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Controller::isScanning()) {
		OnClickedButtonStartScan();
	}

	float* data = spectrumView.getResampleLine();
	float output[1025];

	phaseCorr.setSampleData(data);
	phaseCorr.performPhaseZero(output);
	intensityView.setIntensityLine(output);

	phaseCorr.performPhaseShift(output);
	intensityView.setCompensatedLine(output);
	intensityView.updateWindow();

	data = spectrumView.getResampleData();
	float alpha2 = 0.0f;
	float alpha3 = 0.0f;
	CString path;

	for (int n = 0; n < 1; n++) 
	{
		alpha2 = (float)(7.2 * pow(10, 9));
		// alpha2 = (float)(14.50f * pow(10, 7));// 130000000 + n * 500000;
		// alpha2 = (float)(1.42f * pow(10, 8));
		// alpha3 = (float)pow(10, 9) * n;
		alpha3 = (float)(-1.9 * pow(10, 13));

		for (int i = 0; i < 1024; i++) {
			phaseCorr.setSampleData((data + i * LINE_CAMERA_CCD_PIXELS));
			phaseCorr.performPhaseShift(alpha2, alpha3, output);

			for (int j = 0; j < 1024; j++) {
				// int value = ((std::log(output[j] + 1.0) - 5.0) / 5.0) * 255;
				int value = (int)(((log10(output[j]) - 2.2552) / (4.0094 - 2.2552)) * 255);
				value = (value < 0 ? 0 : value);
				value = (value > 255 ? 255 : value);
				result[i + j * 1024] = value;
			}
		}

		CppUtil::CvImage image;
		image.fromBitsData(result, 1024, 1024);

		path.Format(_T("C:\\test_%+03d.bmp"), n);
		image.saveFile(wtoa(path));
	}
	*/

	/*
	for (int i = 0; i < 1024; i++) {
		phaseCorr.setSampleData((data + i * LINE_CAMERA_CCD_PIXELS));
		phaseCorr.performPhaseShift(output);

		for (int j = 0; j < 1024; j++) {
			// int value = ((std::log(output[j] + 1.0) - 5.0) / 5.0) * 255;
			int value = ((log10(output[j]) - 2.2552) / (4.0094 - 2.2552)) * 255;
			value = (value < 0 ? 0 : value);
			value = (value > 255 ? 255 : value);
			result[i + j * 1024] = value;
		}
	}

	CppUtil::CvImage image;
	image.fromBitsData(result, 1024, 1024);
	image.saveFile("c:\\test.bmp");
	*/

	/*
	EndWaitCursor();
	return;
}


void DlgDispersionCompensation::OnClickedButtonLoad()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		OnClickedButtonStartScan();
	}

	intensityView.loadIntensityData(_T("disp_comp_ints.dat"));
	intensityView.loadCompensatedData(_T("disp_comp_comp.dat"));
	intensityView.loadBackgroundData(_T("disp_comp_back.dat"));
	intensityView.updateWindow();

	spectrumView.loadSpectrumData(_T("disp_comp_spec.dat"));
	spectrumView.loadResampleData(_T("disp_comp_resa.dat"));
	spectrumView.updateWindow();

	// phaseCorr.loadBackgroundData(_T("disp_comp_phase_back.dat"));
	return;
}


void DlgDispersionCompensation::OnClickedButtonLeds()
{
	// TODO: Add your control notification handler code here
	if (pDlgLedsCtrl.get() == nullptr) {
		pDlgLedsCtrl = make_unique<DlgLedsControl>();
		pDlgLedsCtrl->Create(IDD_DLGLEDSCONTROL, this);
	}
	pDlgLedsCtrl->ShowWindow(SW_SHOW);
	return;
}


void DlgDispersionCompensation::OnClickedButtonMotors()
{
	// TODO: Add your control notification handler code here
	if (pDlgMotorsCtrl.get() == nullptr) {
		pDlgMotorsCtrl = make_unique<DlgMotorsControl>();
		pDlgMotorsCtrl->Create(IDD_DLGMOTORSCONTROL, this);
	}
	pDlgMotorsCtrl->ShowWindow(SW_SHOW);
	return;
}


void DlgDispersionCompensation::OnClickedButtonSave()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		OnClickedButtonStartScan();
	}

	intensityView.saveIntensityData(_T("disp_comp_ints.dat"));
	intensityView.saveCompensatedData(_T("disp_comp_comp.dat"));
	intensityView.saveBackgroundData(_T("disp_comp_back.dat"));

	spectrumView.saveSpectrumData(_T("disp_comp_spec.dat"));
	spectrumView.saveResampleData(_T("disp_comp_resa.dat"));

	phaseCorr.saveBackgroundData(_T("disp_comp_phase_back.dat"));
	return;
}
*/


void DlgDispersionCompensation::OnClickedButtonStartScan()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Controller::isScanning()) {
		Controller::closeScan(false);
		btnStartScan.SetWindowTextW(L"Start Scan");
	}
	else {
		updateScanControls(false);

		ChainSetup::useRefreshDispersionParams(true, true);

		Controller::setPreviewImageCallback2(previewView.getCallbackFunction());
		Controller::setSpectrumDataCallback(spectrumView.getSpectrumCallback());
		Controller::setResampleDataCallback(spectrumView.getResampleCallback());
		Controller::setIntensityDataCallback(intensityView.getIntensityCallback());

		Controller::presetScanOffset(offsetX, offsetY, angle, scaleX, scaleY);

		OctScanMeasure measure;
		measure.getPattern().setup(_patternName, numPoints, 1,
			scanRange, scanRange, _direction);
		measure.getPattern().setScanOffset(offsetX, offsetY, angle, scaleX, scaleY);

		/*
		if (Controller::startScan(EyeSide::OD, patternDomain, patternType, 
									numPoints, 1, scanRange, scanRange)) {
			btnStartScan.SetWindowTextW(L"Capture");
		}
		*/
		bool clearPreset = true;
		if (Controller::startScan2(measure,
			previewView.getCallbackFunction(), nullptr, clearPreset)) {
			btnStartScan.SetWindowTextW(L"Capture");
		}
	}
	EndWaitCursor();
	return;
}


void DlgDispersionCompensation::OnBnClickedButtonBackground()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Controller::isScanning()) {
		OnClickedButtonStartScan();
	}

	/*
	float* data = spectrumView.getResampleData();
	float backgd[LINE_CAMERA_CCD_PIXELS] = { 0.0f };
	float output[1025];

	for (int i = 0; i < 1024; i++) {
		for (int j = 0; j < LINE_CAMERA_CCD_PIXELS; j++) {
			backgd[j] += data[i * LINE_CAMERA_CCD_PIXELS + j];
		}
	}

	for (int i = 0; i < LINE_CAMERA_CCD_PIXELS; i++) {
		backgd[i] /= 1024.0;
	}

	phaseCorr.setSampleData(backgd);
	phaseCorr.performPhaseZero(output);
	phaseCorr.setBackgroundData(backgd);

	intensityView.setBackgroundLine(output);
	intensityView.updateWindow();
	*/

	// Make an average axial scan profile from resampled data.
	unsigned short* data = spectrumView.getSpectrumData();
	unsigned int buff[LINE_CAMERA_CCD_PIXELS] = { 0 };
	unsigned short bgrd[LINE_CAMERA_CCD_PIXELS] = { 0 };

	int width = spectrumView.getDataWidth();
	int height = spectrumView.getDataHeight();

	int sidx = height / 10;
	int eidx = height - sidx;
	int size = (eidx - sidx + 1);

	for (int i = 0; i < width; i++) {
		for (int j = sidx; j < eidx; j++) {
			buff[i] += data[i + j*width];
		}
		bgrd[i] = (unsigned short)(buff[i] / size);
	}

	ChainSetup::setBackgroundSpectrum(bgrd);
	EndWaitCursor();
	return;
}


void DlgDispersionCompensation::OnBnClickedCheckBackground()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_BACKGROUND) ? true : false);
	ChainSetup::useBackgroundSubtraction(true, checked);
	return;
}


void DlgDispersionCompensation::OnBnClickedCheckDispersion()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_DISPERSION) ? true : false);
	ChainSetup::useDispersionCompensation(true, checked);
	return;
}


void DlgDispersionCompensation::OnBnClickedCheckReduction()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_REDUCTION) ? true : false);
	ChainSetup::useFixedNoiseReduction(true, checked);
	return;
}


void DlgDispersionCompensation::OnBnClickedCheckAlignGuide()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_ALIGN_GUIDE) ? true : false);
	
	updateScanControls(false);
	previewView.showAlignGuide(checked, scanRange);
	return;
}


void DlgDispersionCompensation::OnBnClickedCheckGrayscale()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_GRAYSCALE) ? true : false);
	ChainSetup::useAdaptiveGrayscale(true, checked);
	return;
}


void DlgDispersionCompensation::OnBnClickedCheckVscan()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_VSCAN) ? true : false);
	isVertScan = checked;
	return;
}


void DlgDispersionCompensation::OnBnClickedCheckSnrRatios()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_SNR_RATIOS) ? true : false);
	intensityView.setDisplayMode(checked ? ViewScanIntensity::DisplayMode::SNR_RATIO : ViewScanIntensity::DisplayMode::INTENSITY);
	intensityView.updateWindow();
	return;
}


void DlgDispersionCompensation::OnBnClickedCheckAutoAlign()
{
	// TODO: Add your control notification handler code here
	bool checked = (IsDlgButtonChecked(IDC_CHECK_AUTO_ALIGN) ? true : false);
	if (checked) {
		Controller::startAutoAlignment();
	}
	else {
		Controller::cancelAutoAlignment();
	}
	return;
}


void DlgDispersionCompensation::OnBnClickedButtonParamDefault()
{
	// TODO: Add your control notification handler code here
	updateDispersionParams(true);
	return;
}


void DlgDispersionCompensation::OnBnClickedButtonParamZero()
{
	// TODO: Add your control notification handler code here
	editAlpha2.SetWindowText(_T("0.0"));
	editAlpha3.SetWindowText(_T("0.0"));
	updateDispersionParams(false);


	return;
}


void DlgDispersionCompensation::OnBnClickedButtonFocus()
{
	// TODO: Add your control notification handler code here
	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);
	return;
}


void DlgDispersionCompensation::OnBnClickedButtonMotor2()
{
	// TODO: Add your control notification handler code here
	if (pDlgMotor.get() == nullptr) {
		pDlgMotor = make_unique<DlgMotorControl>();
		pDlgMotor->Create(IDD_DLGMOTORCONTROL, this);
	}
	pDlgMotor->ShowWindow(SW_SHOW);
	return;
}


void DlgDispersionCompensation::OnBnClickedButtonCamera2()
{
	// TODO: Add your control notification handler code here
	if (pDlgCamera.get() == nullptr) {
		pDlgCamera = make_unique<DlgCameraControl>();
		pDlgCamera->Create(IDD_DLGCAMERACONTROL, this);
	}
	pDlgCamera->ShowWindow(SW_SHOW);
	return;
}


void DlgDispersionCompensation::OnBnClickedButtonLight2()
{
	// TODO: Add your control notification handler code here
	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);
	return;
}


void DlgDispersionCompensation::OnBnClickedButtonStage()
{
	// TODO: Add your control notification handler code here
	if (pDlgStage.get() == nullptr) {
		pDlgStage = make_unique<DlgStageControl>();
		pDlgStage->Create(IDD_DLGSTAGECONTROL, this);
	}
	pDlgStage->ShowWindow(SW_SHOW);
	return;
}



void DlgDispersionCompensation::OnBnClickedButtonExit2()
{
	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);
	return;
}


void DlgDispersionCompensation::OnBnClickedButtonSaveImage()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		OnClickedButtonStartScan();
	}

	BeginWaitCursor();
	CString path, text;
	if (previewView.saveImage(path)) {
		text.Format(_T("Image file: '%s' saved!"), path);
		AfxMessageBox(text);
	}
	EndWaitCursor();
	return;
}


void DlgDispersionCompensation::OnBnClickedButtonAutoFocus()
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


void DlgDispersionCompensation::OnBnClickedButtonAutoPolar()
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


void DlgDispersionCompensation::OnBnClickedButtonAutoRefer()
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


void DlgDispersionCompensation::OnSelchangeComboRange()
{
	// TODO: Add your control notification handler code here
	int curSel = cmbRange.GetCurSel();

	CString text;
	cmbRange.GetLBText(curSel, text);
	scanRange = (float)_ttof(text);
	return;
}


void DlgDispersionCompensation::OnSelchangeComboPoints()
{
	// TODO: Add your control notification handler code here
	numPoints = 1024;
	return;
}


void DlgDispersionCompensation::OnSelchangeComboPattern()
{
	// TODO: Add your control notification handler code here
	int curSel = cmbPattern.GetCurSel();
	_direction = 0;

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
	return;
}


void DlgDispersionCompensation::OnBnClickedCheckCorneaDisp()
{
	// TODO: Add your control notification handler code here
	useCorneaDisp = (IsDlgButtonChecked(IDC_CHECK_CORNEA_DISP) ? true : false);
	updateDispersionParams(true);
}
