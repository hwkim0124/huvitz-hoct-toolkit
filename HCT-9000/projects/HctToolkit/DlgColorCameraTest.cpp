// DlgColorCameraTest.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgColorCameraTest.h"
#include "afxdialogex.h"

#include "OctSystem2.h"
#include "CppUtil2.h"
#include "OctResult2.h"
#include "ImgProc2.h"

#include <direct.h>  // _mkdir
#include <io.h>      // _access

#include <opencv2/opencv.hpp>
#include <json\json.hpp>
#include <boost\format.hpp>

using namespace OctSystem;
using namespace OctDevice;
using namespace OctResult;
using namespace OctConfig;
using namespace ImgProc;
using namespace CppUtil;
using namespace std;

// DlgColorCameraTest dialog

IMPLEMENT_DYNAMIC(DlgColorCameraTest, CDialogEx)

DlgColorCameraTest::DlgColorCameraTest(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGCOLORCAMERATEST, pParent)
{
	cbJoystick = std::bind(&DlgColorCameraTest::OnBnClickedButtonCapture, this);
	Controller::setJoystickEventCallback(&cbJoystick);

	cbEyeSide = std::bind(&DlgColorCameraTest::callbackEyeSideEvent, this, std::placeholders::_1);
	Controller::setEyeSideEventCallback(&cbEyeSide);
}


DlgColorCameraTest::~DlgColorCameraTest()
{
}


void DlgColorCameraTest::initControls(void)
{
	corneaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 0, 480, 360), this);
	retinaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 360, 480, 360 + 360), this);
	colorView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(480, 0, 480 + 960, 750), this);
	colorView.ModifyStyle(0, SS_NOTIFY);

	corneaView.setDisplaySize(480, 360);
	retinaView.setDisplaySize(480, 360);
	colorView.setViewSize(960, 750);
	//colorView.setViewSize(750, 750);

	corneaView.showAlignGuide(true);
	retinaView.showAlignGuide(true);

	Controller::setColorCameraImageCallback(colorView.getImageCallbackFunction());
	Controller::setColorCameraFrameCallback(colorView.getFrameCallbackFunction());

	fundusMotor = Controller::getMainBoard()->getFundusFocusMotor();
	fundusMotor->setControls(&sldFundus, &editFundus, &editDiopt);

	flashLed = Controller::getMainBoard()->getFundusFlashLed();
	flashLed->setControls(&sldFlash, &editFlash);

	colorCamera = Controller::getMainBoard()->getColorCamera();

	CString text;
	if (GlobalSettings::useFundusFILR_Enable()) {
		cmbAgain.EnableWindow(false);

		editBrightR.EnableWindow(false);
		editBrightG.EnableWindow(false);
		editBrightB.EnableWindow(false);
		editDarkR.EnableWindow(false);
		editDarkG.EnableWindow(false);
		editDarkB.EnableWindow(false);

		GetDlgItem(IDC_BUTTON_WHITES)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_PIXEL_DETECT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_PIXEL_CORRECT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_BLACKS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_BP_LOAD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_BP_SAVE)->EnableWindow(FALSE);

	}
	else {
		cmbAgain.SetCurSel(colorCamera->getAnalogGain(true));

		text.Format(_T("%d"), 3900);
		editBrightR.SetWindowTextW(text);
		editBrightG.SetWindowTextW(text);
		editBrightB.SetWindowTextW(text);
		text.Format(_T("%d"), 500);
		editDarkR.SetWindowTextW(text);
		text.Format(_T("%d"), 400);
		editDarkG.SetWindowTextW(text);
		text.Format(_T("%d"), 300);
		editDarkB.SetWindowTextW(text);
	}

	text.Format(_T("%d"), 12);
	editThreshold.SetWindowTextW(text);

	text.Format(_T("%d"), GlobalSettings::removeReflectionLight1());
	editLightRadius.SetWindowTextW(text);
	text.Format(_T("%d"), GlobalSettings::removeReflectionLight2());
	editShadowRadius.SetWindowTextW(text);
	text.Format(_T("%d"), GlobalSettings::removeReflectionLight3());
	editBorderRadius.SetWindowTextW(text);
	CheckDlgButton(IDC_CHECK_REMOVE_LIGHT, GlobalSettings::useRemoveLight());

	text.Format(_T("%.2f"), colorCamera->getDigitalGain());
	editDgain.SetWindowTextW(text);

	cmbInfixRow.SetCurSel(7);
	cmbInfixCol.SetCurSel(4);
	OnBnClickedButtonIntfixOn();

	CheckDlgButton(IDC_RADIO_NORMAL, TRUE);

	SetDlgItemText(IDC_EDIT_AWB_PARAM1, _T("1.0"));
	SetDlgItemText(IDC_EDIT_AWB_PARAM2, _T("1.0"));
	CheckDlgButton(IDC_CHECK_AWB, FALSE);

	if (GlobalSettings::useFundusROI()) {
		CheckDlgButton(IDC_CHECK_MASK_ROI, TRUE);
	}

	text.Format(_T("%d"), GlobalSettings::centerOfFundusROI().first);
	SetDlgItemText(IDC_EDIT_ROI_X1, text);
	text.Format(_T("%d"), GlobalSettings::centerOfFundusROI().second);
	SetDlgItemText(IDC_EDIT_ROI_Y1, text);
	text.Format(_T("%d"), GlobalSettings::radiusOfFundusROI());
	SetDlgItemText(IDC_EDIT_ROI_RADIUS, text);
	OnBnClickedCheckRemovalLight();

	CheckDlgButton(IDC_CHECK_ALIGN_GUIDE, TRUE);
	CheckDlgButton(IDC_CHECK_REMOVE_GUIDE, FALSE);

	return;
}


void DlgColorCameraTest::controlIrCameras(bool play)
{
	BeginWaitCursor();
	if (play) {
		Controller::setCorneaCameraImageCallback(corneaView.getCallbackFunction());
		Controller::setRetinaCameraImageCallback(retinaView.getCallbackFunction());

		Controller::getMainBoard()->getCorneaCamera()->play();
		Controller::getMainBoard()->getRetinaCamera()->play();
	}
	else {
		Controller::getMainBoard()->getCorneaCamera()->pause();
		Controller::getMainBoard()->getRetinaCamera()->pause();

		Controller::setCorneaCameraImageCallback(nullptr);
		Controller::setRetinaCameraImageCallback(nullptr);
	}
	EndWaitCursor();
	return;
}


void DlgColorCameraTest::updateFundusFocus(void)
{
	if (pDlgFocus.get() != nullptr) {
		CString text;
		text.Format(_T("%.1f"), fundusMotor->getCurrentDiopter());
		editDiopt.SetWindowTextW(text);
		text.Format(_T("%d"), fundusMotor->getPosition());
		editFundus.SetWindowTextW(text);
		sldFundus.SetPos(fundusMotor->getPosition());
	}
	return;
}


void DlgColorCameraTest::updateFundusFlash(void)
{
	if (pDlgLight.get() != nullptr) {
		CString text;
		text.Format(_T("%d"), flashLed->getIntensity());
		editFlash.SetWindowTextW(text);
		sldFlash.SetPos(flashLed->getIntensity());
	}
	return;
}


void DlgColorCameraTest::callbackEyeSideEvent(bool od)
{
	DebugOut2() << "Eye Side changed OD: " << od;
	return;
}


void DlgColorCameraTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_AGAIN, cmbAgain);
	DDX_Control(pDX, IDC_COMBO_INTFIX_COL, cmbInfixCol);
	DDX_Control(pDX, IDC_COMBO_INTFIX_ROW, cmbInfixRow);
	DDX_Control(pDX, IDC_EDIT_DGAIN, editDgain);
	DDX_Control(pDX, IDC_EDIT_FLASH, editFlash);
	DDX_Control(pDX, IDC_SLIDER_FLASH, sldFlash);
	DDX_Control(pDX, IDC_SLIDER_FUNDUS, sldFundus);
	DDX_Control(pDX, IDC_EDIT_FUNDUS, editFundus);
	DDX_Control(pDX, IDC_EDIT_FUNDUS_DIOPT, editDiopt);
	DDX_Control(pDX, IDC_EDIT_BP_BB, editBrightB);
	DDX_Control(pDX, IDC_EDIT_BP_BG, editBrightG);
	DDX_Control(pDX, IDC_EDIT_BP_BR, editBrightR);
	DDX_Control(pDX, IDC_EDIT_BP_DB, editDarkB);
	DDX_Control(pDX, IDC_EDIT_BP_DG, editDarkG);
	DDX_Control(pDX, IDC_EDIT_BP_DR, editDarkR);
	DDX_Control(pDX, IDC_EDIT_LIGHT_RADIUS, editLightRadius);
	DDX_Control(pDX, IDC_EDIT_SHADOW_RADIUS, editShadowRadius);
	DDX_Control(pDX, IDC_EDIT_BORDER_RADIUS, editBorderRadius);
	DDX_Control(pDX, IDC_EDIT_THRESHOLD, editThreshold);
}


BEGIN_MESSAGE_MAP(DlgColorCameraTest, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_FOCUS, &DlgColorCameraTest::OnBnClickedButtonFocus)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA, &DlgColorCameraTest::OnBnClickedButtonCamera)
	ON_BN_CLICKED(IDC_BUTTON_MOTOR, &DlgColorCameraTest::OnBnClickedButtonMotor)
	ON_BN_CLICKED(IDC_BUTTON_ORIGINAL, &DlgColorCameraTest::OnBnClickedButtonOriginal)
	ON_BN_CLICKED(IDC_BUTTON_LEDS, &DlgColorCameraTest::OnBnClickedButtonLeds)
	ON_BN_CLICKED(IDC_BUTTON_COLOR, &DlgColorCameraTest::OnBnClickedButtonColor)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_RAW, &DlgColorCameraTest::OnBnClickedButtonSaveRaw)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_IMAGE, &DlgColorCameraTest::OnBnClickedButtonSaveImage)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE, &DlgColorCameraTest::OnBnClickedButtonCapture)
	ON_BN_CLICKED(IDC_BUTTON_LIVE, &DlgColorCameraTest::OnBnClickedButtonLive)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_EXIT, &DlgColorCameraTest::OnBnClickedButtonColorExit)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_RAW, &DlgColorCameraTest::OnBnClickedButtonLoadRaw)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_FUNDUS, &DlgColorCameraTest::OnReleasedcaptureSliderFundus)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_FLASH, &DlgColorCameraTest::OnReleasedcaptureSliderFlash)
	ON_CBN_SELCHANGE(IDC_COMBO_AGAIN, &DlgColorCameraTest::OnSelchangeComboAgain)
	ON_BN_CLICKED(IDC_BUTTON_FUNDUS_STEP1, &DlgColorCameraTest::OnBnClickedButtonFundusStep1)
	ON_BN_CLICKED(IDC_BUTTON_FUNDUS_STEP2, &DlgColorCameraTest::OnBnClickedButtonFundusStep2)
	ON_BN_CLICKED(IDC_BUTTON_FUNDUS_STEP3, &DlgColorCameraTest::OnBnClickedButtonFundusStep3)
	ON_BN_CLICKED(IDC_BUTTON_FUNDUS_STEP4, &DlgColorCameraTest::OnBnClickedButtonFundusStep4)
	ON_BN_CLICKED(IDC_BUTTON_FUNDUS_ORIGIN, &DlgColorCameraTest::OnBnClickedButtonFundusOrigin)
	ON_BN_CLICKED(IDC_BUTTON_FLASH_STEP1, &DlgColorCameraTest::OnBnClickedButtonFlashStep1)
	ON_BN_CLICKED(IDC_BUTTON_FLASH_STEP2, &DlgColorCameraTest::OnBnClickedButtonFlashStep2)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_ON, &DlgColorCameraTest::OnBnClickedButtonIntfixOn)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_OFF, &DlgColorCameraTest::OnBnClickedButtonIntfixOff)
	ON_BN_CLICKED(IDC_RADIO_NORMAL, &DlgColorCameraTest::OnBnClickedRadioNormal)
	ON_BN_CLICKED(IDC_RADIO_SMALL, &DlgColorCameraTest::OnBnClickedRadioSmall)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_FLASH, &DlgColorCameraTest::OnNMCustomdrawSliderFlash)
	ON_CBN_SELCHANGE(IDC_COMBO_INTFIX_ROW, &DlgColorCameraTest::OnCbnSelchangeComboIntfixRow)
	ON_CBN_SELCHANGE(IDC_COMBO_INTFIX_COL, &DlgColorCameraTest::OnCbnSelchangeComboIntfixCol)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_FUNDUS, &DlgColorCameraTest::OnNMCustomdrawSliderFundus)
	ON_BN_CLICKED(IDC_BUTTON_AWB_CALC, &DlgColorCameraTest::OnBnClickedButtonAwbCalc)
	ON_BN_CLICKED(IDC_BUTTON_AWB_INIT, &DlgColorCameraTest::OnBnClickedButtonAwbInit)
	ON_BN_CLICKED(IDC_CHECK_AWB, &DlgColorCameraTest::OnBnClickedCheckAwb)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_OD, &DlgColorCameraTest::OnBnClickedButtonIntfixOd)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_OS, &DlgColorCameraTest::OnBnClickedButtonIntfixOs)
	ON_BN_CLICKED(IDC_CHECK_RADIAL, &DlgColorCameraTest::OnBnClickedCheckRadial)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_DATA, &DlgColorCameraTest::OnBnClickedButtonExportData)
	ON_BN_CLICKED(IDC_CHECK_MASK_ROI, &DlgColorCameraTest::OnBnClickedCheckMaskRoi)
	ON_BN_CLICKED(IDC_BUTTON_WHITES, &DlgColorCameraTest::OnBnClickedButtonWhites)
	ON_BN_CLICKED(IDC_BUTTON_BLACKS, &DlgColorCameraTest::OnBnClickedButtonBlacks)
	ON_BN_CLICKED(IDC_BUTTON_PIXEL_DETECT, &DlgColorCameraTest::OnBnClickedButtonPixelDetect)
	ON_BN_CLICKED(IDC_BUTTON_PIXEL_CORRECT, &DlgColorCameraTest::OnBnClickedButtonPixelCorrect)
	ON_BN_CLICKED(IDC_BUTTON_BP_LOAD, &DlgColorCameraTest::OnBnClickedButtonBpLoad)
	ON_BN_CLICKED(IDC_BUTTON_BP_SAVE, &DlgColorCameraTest::OnBnClickedButtonBpSave)
	ON_BN_CLICKED(IDC_CHECK_ALIGN_GUIDE, &DlgColorCameraTest::OnBnClickedCheckAlignGuide)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_PRESET1, &DlgColorCameraTest::OnBnClickedButtonRetinaPreset1)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_PRESET2, &DlgColorCameraTest::OnBnClickedButtonRetinaPreset2)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_PRESET3, &DlgColorCameraTest::OnBnClickedButtonRetinaPreset3)
	ON_BN_CLICKED(IDC_BUTTON_ORIGINLIVE, &DlgColorCameraTest::OnBnClickedButtonOriginlive)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_REMOVAL_LIGHT, &DlgColorCameraTest::OnBnClickedButtonApplyRemovalLight)
	ON_BN_CLICKED(IDC_CHECK_REMOVAL_LIGHT, &DlgColorCameraTest::OnBnClickedCheckRemovalLight)
	ON_BN_CLICKED(IDC_CHECK_REMOVE_GUIDE, &DlgColorCameraTest::OnBnClickedCheckRemoveGuide)
	ON_EN_CHANGE(IDC_EDIT_THRESHOLD, &DlgColorCameraTest::OnEnChangeEditThreshold)
	ON_BN_CLICKED(IDC_BUTTON_BlackSpot_Align, &DlgColorCameraTest::OnBnClickedButtonBlackspotAlign)
	ON_BN_CLICKED(IDC_BUTTON_BlackSpot_Capture, &DlgColorCameraTest::OnBnClickedButtonBlackspotCapture)
	ON_BN_CLICKED(IDC_BUTTON_LED_Align, &DlgColorCameraTest::OnBnClickedButtonLedAlign)
	ON_BN_CLICKED(IDC_BUTTON_ROI_X_UP, &DlgColorCameraTest::OnBnClickedButtonRoiXUp)
	ON_BN_CLICKED(IDC_BUTTON_ROI_X_DOWN, &DlgColorCameraTest::OnBnClickedButtonRoiXDown)
	ON_BN_CLICKED(IDC_BUTTON_ROI_Y_UP, &DlgColorCameraTest::OnBnClickedButtonRoiYUp)
	ON_BN_CLICKED(IDC_BUTTON_ROI_Y_DOWN, &DlgColorCameraTest::OnBnClickedButtonRoiYDown)
	ON_BN_CLICKED(IDC_BUTTON_ROI_RADIUS_DOWN, &DlgColorCameraTest::OnBnClickedButtonRoiRadiusDown)
	ON_BN_CLICKED(IDC_BUTTON_ROI_RADIUS_UP, &DlgColorCameraTest::OnBnClickedButtonRoiRadiusUp)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_COLOR_ROI, &DlgColorCameraTest::OnBnClickedButtonUpdateColorRoi)
	ON_BN_CLICKED(IDC_BUTTON_FUNDUS_CALIBRATION, &DlgColorCameraTest::OnBnClickedButtonFundusCalibration)
END_MESSAGE_MAP()


// DlgColorCameraTest message handlers


BOOL DlgColorCameraTest::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	initControls();
	isPlaying = true;
	controlIrCameras(isPlaying);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgColorCameraTest::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL)
		{
			CString text;
			int ctrlID = pFocused->GetDlgCtrlID();
			if (ctrlID == editDgain.GetDlgCtrlID()) {
				editDgain.GetWindowTextW(text);
				if (!text.IsEmpty()) {
					colorCamera->setDigitalGain((float)_ttof(text));
					colorCamera->updateParameters();
				}
			}
		}

		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgColorCameraTest::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	controlIrCameras(false);

	fundusMotor = Controller::getMainBoard()->getFundusFocusMotor();
	fundusMotor->setControls();

	flashLed = Controller::getMainBoard()->getFundusFlashLed();
	flashLed->setControls();

	ColorCamera* camera = Controller::getMainBoard()->getColorCamera();
	if (camera->isLiveMode()) {
		camera->pauseLiveMode();
	}

	Controller::setColorCameraImageCallback(nullptr);
	Controller::setColorCameraFrameCallback(nullptr);

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
	if (pDlgColor.get() != nullptr) {
		pDlgColor->DestroyWindow();
	}
	if (pDlgOriginal.get() != nullptr) {
		pDlgOriginal->DestroyWindow();
	}
	if (pDlgLive.get() != nullptr) {
		pDlgLive->DestroyWindow();
	}

	Controller::setJoystickEventCallback(nullptr);
	Controller::setEyeSideEventCallback(nullptr);
	CDialogEx::OnClose();
}


void DlgColorCameraTest::OnBnClickedButtonFocus()
{
	// TODO: Add your control notification handler code here
	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);
	return;
}


void DlgColorCameraTest::OnBnClickedButtonCamera()
{
	// TODO: Add your control notification handler code here
	if (pDlgCamera.get() == nullptr) {
		pDlgCamera = make_unique<DlgCameraControl>();
		pDlgCamera->Create(IDD_DLGCAMERACONTROL, this);
	}
	pDlgCamera->ShowWindow(SW_SHOW);
	return;
}


void DlgColorCameraTest::OnBnClickedButtonMotor()
{
	// TODO: Add your control notification handler code here
	if (pDlgMotor.get() == nullptr) {
		pDlgMotor = make_unique<DlgMotorControl>();
		pDlgMotor->Create(IDD_DLGMOTORCONTROL, this);
	}
	pDlgMotor->ShowWindow(SW_SHOW);
	return;
}

void DlgColorCameraTest::OnBnClickedButtonOriginal()
{
	// TODO: Add your control notification handler code here
	if (pDlgOriginal.get() == nullptr) {
		pDlgOriginal = make_unique<DlgOriginalImage>();
		pDlgOriginal->Create(IDD_DLGORIGINALIMAGE, this);
	}
	pDlgOriginal->setImage(colorView.getOriginalImage());
	pDlgOriginal->ShowWindow(SW_SHOW);
	pDlgOriginal->updateWindow();

	return;
}

void DlgColorCameraTest::OnBnClickedButtonOriginlive()
{
	// TODO: Add your control notification handler code here
	if (pDlgLive.get() == nullptr) {
		pDlgLive = make_unique<DlgOriginalLive>();
		pDlgLive->Create(IDD_DLGORIGINALLIVE, this);
	}
	ColorCamera* camera = Controller::getMainBoard()->getColorCamera();
	Controller::setColorCameraImageCallback(nullptr);
	Controller::setColorCameraFrameCallback(nullptr);
	colorView.useLiveMode(true, false);
	camera->pauseLiveMode();
	SetDlgItemText(IDC_BUTTON_LIVE, _T("Start Live"));

	bool flagMaskROI = (IsDlgButtonChecked(IDC_CHECK_MASK_ROI) ? true : false);

	CString cstr;
	GetDlgItemText(IDC_EDIT_ROI_X1, cstr);
	int xMaskROI = _ttoi(cstr);
	GetDlgItemText(IDC_EDIT_ROI_Y1, cstr);
	int yMaskROI = _ttoi(cstr);
	GetDlgItemText(IDC_EDIT_ROI_RADIUS, cstr);
	int radiusMaskROI = _ttoi(cstr);

	bool flagWB = (IsDlgButtonChecked(IDC_CHECK_AWB) ? true : false);

	float WBparam1 = 1.0f, WBparam2 = 1.0f;
	CString text;
	GetDlgItemText(IDC_EDIT_AWB_PARAM1, text);
	if (!text.IsEmpty()) {
		WBparam1 = (float)_ttof(text);
	}
	GetDlgItemText(IDC_EDIT_AWB_PARAM2, text);
	if (!text.IsEmpty()) {
		WBparam2 = (float)_ttof(text);
	}

	pDlgLive->setParameter(flagMaskROI, xMaskROI, yMaskROI, radiusMaskROI, flagWB, WBparam1, WBparam2);
	pDlgLive->ShowWindow(SW_SHOW);
	return;
}

void DlgColorCameraTest::OnBnClickedButtonLeds()
{
	// TODO: Add your control notification handler code here
	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);
	return;
}


void DlgColorCameraTest::OnBnClickedButtonColor()
{
	// TODO: Add your control notification handler code here
	if (pDlgColor.get() == nullptr) {
		pDlgColor = make_unique<DlgColorCameraControl>();
		pDlgColor->Create(IDD_DLGCOLORCAMERACONTROL, this);
	}
	pDlgColor->ShowWindow(SW_SHOW);
	return;
}


void DlgColorCameraTest::OnBnClickedButtonSaveRaw()
{
	// TODO: Add your control notification handler code here
	if (Controller::getMainBoard()->getColorCamera()->isLiveMode()) {
		OnBnClickedButtonLive();
	}

	BeginWaitCursor();
	CString path, text;
	if (colorView.saveFrameData(path)) {
		text.Format(_T("Raw file: '%s' saved!"), path);
		AfxMessageBox(text);
	}
	EndWaitCursor();
	return;
}


void DlgColorCameraTest::OnBnClickedButtonSaveImage()
{
	// TODO: Add your control notification handler code here
	if (Controller::getMainBoard()->getColorCamera()->isLiveMode()) {
		OnBnClickedButtonLive();
	}

	BeginWaitCursor();
	CString path, text;
	if (colorView.saveImage(path)) {
		text.Format(_T("Image file: '%s' saved!"), path);
		AfxMessageBox(text);
	}
	EndWaitCursor();
	return;
}


void DlgColorCameraTest::OnBnClickedButtonLoadRaw()
{
	// TODO: Add your control notification handler code here
	if (Controller::getMainBoard()->getColorCamera()->isLiveMode()) {
		OnBnClickedButtonLive();
	}

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Raw Image|*.yml||"));
	if (IDOK == dlg.DoModal()) {
		BeginWaitCursor();
		CString path = dlg.GetPathName();
		colorView.loadFrameData(path);
		EndWaitCursor();
	}
	return;
}


void DlgColorCameraTest::OnBnClickedButtonCapture()
{
	// TODO: Add your control notification handler code here
	OnBnClickedCheckMaskRoi();
	OnBnClickedCheckRemovalLight();

	ColorCamera* camera = Controller::getMainBoard()->getColorCamera();

	if (camera->isOriginalMode()) {
		return;
	}
	else {
		if (camera->isLiveMode()) {
			OnBnClickedButtonLive();
		}

		BeginWaitCursor();

		OctFundusMeasure measure;
		measure.eyeSide() = Controller::getEyeSide();

		auto tempFundusRoi = GlobalSettings::centerOfFundusROI();
		auto tempRadius = GlobalSettings::radiusOfFundusROI();

		//change global settings for Radial correction 
		CString cstr;
		GetDlgItemText(IDC_EDIT_ROI_X1, cstr);
		int x1 = _ttoi(cstr);
		GetDlgItemText(IDC_EDIT_ROI_Y1, cstr);
		int y1 = _ttoi(cstr);
		GetDlgItemText(IDC_EDIT_ROI_RADIUS, cstr);
		int radius = _ttoi(cstr);

		GlobalSettings::centerOfFundusROI() = std::pair<int, int>(x1, y1);
		GlobalSettings::radiusOfFundusROI() = radius;
		Controller::takeColorFundus(measure, nullptr, nullptr,
			colorView.getImageCallbackFunction(),
			colorView.getFrameCallbackFunction(),
			false);

		//restore global settings for Radial correction
		GlobalSettings::centerOfFundusROI() = tempFundusRoi;
		GlobalSettings::radiusOfFundusROI() = tempRadius;

		EndWaitCursor();
		return;
	}
}


void DlgColorCameraTest::OnBnClickedButtonLive()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	ColorCamera* camera = Controller::getMainBoard()->getColorCamera();
	if (camera->isOriginalMode()) {
		return;
	}
	else {
		if (camera->isLiveMode()) {
			Controller::setColorCameraImageCallback(nullptr);
			Controller::setColorCameraFrameCallback(nullptr);
			colorView.useLiveMode(true, false);
			camera->pauseLiveMode();
			SetDlgItemText(IDC_BUTTON_LIVE, _T("Start Live"));
			GetDlgItem(IDC_BUTTON_ORIGINAL)->EnableWindow(TRUE);
		}
		else {
			((CButton*)GetDlgItem(IDC_CHECK_RADIAL))->SetCheck(0);
			colorView.useRadialColorCorrection(true, false);
			Controller::setColorCameraImageCallback(colorView.getImageCallbackFunction());
			Controller::setColorCameraFrameCallback(colorView.getFrameCallbackFunction());
			colorView.useLiveMode(true, true);
			camera->startLiveMode();
			SetDlgItemText(IDC_BUTTON_LIVE, _T("Pause Live"));
			GetDlgItem(IDC_BUTTON_ORIGINAL)->EnableWindow(FALSE);
		}
	}
	EndWaitCursor();
	return;
}

void DlgColorCameraTest::OnBnClickedButtonColorExit()
{
	auto desc = Measure::getFundusImageDescript();

	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);
	return;
}



void DlgColorCameraTest::OnReleasedcaptureSliderFundus(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	fundusMotor->updatePosition(sldFundus.GetPos());
	updateFundusFocus();
	*pResult = 0;
}


void DlgColorCameraTest::OnReleasedcaptureSliderFlash(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	flashLed->updateIntensity(sldFlash.GetPos());
	updateFundusFlash();
	*pResult = 0;
}


void DlgColorCameraTest::OnSelchangeComboAgain()
{
	// TODO: Add your control notification handler code here
	colorCamera->setAnalogGain(cmbAgain.GetCurSel(), true);
	colorCamera->updateParameters();
	return;
}


void DlgColorCameraTest::OnBnClickedButtonFundusStep1()
{
	// TODO: Add your control notification handler code here
	fundusMotor->updateDiopterByOffset(+1.0f);
	updateFundusFocus();
}


void DlgColorCameraTest::OnBnClickedButtonFundusStep2()
{
	// TODO: Add your control notification handler code here
	fundusMotor->updateDiopterByOffset(+0.5f);
	updateFundusFocus();
}


void DlgColorCameraTest::OnBnClickedButtonFundusStep3()
{
	// TODO: Add your control notification handler code here
	fundusMotor->updateDiopterByOffset(-0.5f);
	updateFundusFocus();
}


void DlgColorCameraTest::OnBnClickedButtonFundusStep4()
{
	// TODO: Add your control notification handler code here
	fundusMotor->updateDiopterByOffset(-1.0f);
	updateFundusFocus();
}


void DlgColorCameraTest::OnBnClickedButtonFundusOrigin()
{
	// TODO: Add your control notification handler code here
	fundusMotor->updateDiopter(0.0f);
	updateFundusFocus();
}


void DlgColorCameraTest::OnBnClickedButtonFlashStep1()
{
	// TODO: Add your control notification handler code here
	flashLed->updateIntensityByOffset(-5);
	updateFundusFlash();
}


void DlgColorCameraTest::OnBnClickedButtonFlashStep2()
{
	// TODO: Add your control notification handler code here
	flashLed->updateIntensityByOffset(+5);
	updateFundusFlash();
}


void DlgColorCameraTest::OnBnClickedButtonIntfixOn()
{
	// TODO: Add your control notification handler code here
	int row = cmbInfixRow.GetCurSel();
	int col = cmbInfixCol.GetCurSel();
	if (row >= 0 && col >= 0) {
		Controller::getMainBoard()->setInternalFixationOn(true, row, col);
	}
	return;
}


void DlgColorCameraTest::OnBnClickedButtonIntfixOff()
{
	// TODO: Add your control notification handler code here
	Controller::getMainBoard()->setInternalFixationOn(false);
}


void DlgColorCameraTest::OnBnClickedRadioNormal()
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_RADIO_SMALL, FALSE);
	//Controller::getMainBoard()->setSmallPupilMask(false);
	Controller::getMainBoard()->getPupilMaskMotor()->updatePositionToNormal();
}


void DlgColorCameraTest::OnBnClickedRadioSmall()
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_RADIO_NORMAL, FALSE);
	//Controller::getMainBoard()->setSmallPupilMask(true);
	Controller::getMainBoard()->getPupilMaskMotor()->updatePositionToSmall();
}


void DlgColorCameraTest::OnNMCustomdrawSliderFlash(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


void DlgColorCameraTest::OnCbnSelchangeComboIntfixRow()
{
	// TODO: Add your control notification handler code here
	OnBnClickedButtonIntfixOn();
}


void DlgColorCameraTest::OnCbnSelchangeComboIntfixCol()
{
	// TODO: Add your control notification handler code here
	OnBnClickedButtonIntfixOn();
}


void DlgColorCameraTest::OnNMCustomdrawSliderFundus(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}


void DlgColorCameraTest::OnBnClickedButtonAwbCalc()
{
	// TODO: Add your control notification handler code here
	float param1, param2;
	if (colorView.processWhiteBalance(&param1, &param2)) {
		CString text;
		text.Format(_T("%.1f"), param1);
		SetDlgItemText(IDC_EDIT_AWB_PARAM1, text);
		text.Format(_T("%.1f"), param2);
		SetDlgItemText(IDC_EDIT_AWB_PARAM2, text);
	}
	return;
}


void DlgColorCameraTest::OnBnClickedButtonAwbInit()
{
	// TODO: Add your control notification handler code here
	float param1 = 1.0f, param2 = 1.0f;
	CString text;
	GetDlgItemText(IDC_EDIT_AWB_PARAM1, text);
	if (!text.IsEmpty()) {
		param1 = (float)_ttof(text);
	}
	GetDlgItemText(IDC_EDIT_AWB_PARAM2, text);
	if (!text.IsEmpty()) {
		param2 = (float)_ttof(text);
	}
	colorView.setWhiteBalanceParameters(param1, param2);
	return;
}


void DlgColorCameraTest::OnBnClickedCheckAwb()
{
	// TODO: Add your control notification handler code here
	bool flag = (IsDlgButtonChecked(IDC_CHECK_AWB) ? true : false);
	colorView.useAutoWhiteBalance(true, flag);
	return;
}


void DlgColorCameraTest::OnBnClickedCheckRadial()
{
	// TODO: Add your control notification handler code here
	bool flag = (IsDlgButtonChecked(IDC_CHECK_RADIAL) ? true : false);
	colorView.useRadialColorCorrection(true, flag);
	return;
}


void DlgColorCameraTest::OnBnClickedCheckMaskRoi()
{
	// TODO: Add your control notification handler code here
	bool flag = (IsDlgButtonChecked(IDC_CHECK_MASK_ROI) ? true : false);

	CString cstr;
	GetDlgItemText(IDC_EDIT_ROI_X1, cstr);
	int x1 = _ttoi(cstr);
	GetDlgItemText(IDC_EDIT_ROI_Y1, cstr);
	int y1 = _ttoi(cstr);
	GetDlgItemText(IDC_EDIT_ROI_RADIUS, cstr);
	int radius = _ttoi(cstr);

	colorView.useMaskROI(true, flag, x1, y1, radius);

	return;
}


void DlgColorCameraTest::OnBnClickedButtonIntfixOd()
{
	// TODO: Add your control notification handler code here
	Controller::turnOnInternalFixationAtCenter(EyeSide::OD);

	int row, col;
	Controller::getInternalFixationOn(row, col);
	cmbInfixRow.SetCurSel(row);
	cmbInfixCol.SetCurSel(col);
	return;
}


void DlgColorCameraTest::OnBnClickedButtonIntfixOs()
{
	// TODO: Add your control notification handler code here
	Controller::turnOnInternalFixationAtCenter(EyeSide::OS);

	int row, col;
	Controller::getInternalFixationOn(row, col);
	cmbInfixRow.SetCurSel(row);
	cmbInfixCol.SetCurSel(col);
	return;
}


void DlgColorCameraTest::OnBnClickedButtonExport()
{
	// TODO: Add your control notification handler code here

}


void DlgColorCameraTest::OnBnClickedButtonExportResult()
{
	// TODO: Add your control notification handler code here

}

void DlgColorCameraTest::OnBnClickedButtonExportData()
{
	// TODO: Add your control notification handler code here
	if (Measure::isFundusResult()) {
		int count = Measure::exportFundusResults(95/*default quality*/);
		CString text;
		text.Format(_T("%d fundus results has been exported!"), count);
		AfxMessageBox(text);
	}
	return;
}


void DlgColorCameraTest::OnBnClickedButtonWhites()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	int count = 0;
	for (int i = 0; i < PIXEL_CORRECT_IMAGES_NUM; i++) {
		OnBnClickedButtonCapture();
		this_thread::sleep_for(chrono::milliseconds(4000));

		wostringstream oss;
		oss << L".\\white" << (i + 1) << ".raw";

		auto frame = Measure::getFundusFrameObject();
		if (frame != nullptr) {
			frame->exportFile(oss.str());
			count++;
		}
	}
	CString text;
	text.Format(_T("%d white images saved successfully!"), count);
	AfxMessageBox(text);
	EndWaitCursor();
	return;
}


void DlgColorCameraTest::OnBnClickedButtonBlacks()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	int count = 0;
	for (int i = 0; i < PIXEL_CORRECT_IMAGES_NUM; i++) {
		OnBnClickedButtonCapture();
		this_thread::sleep_for(chrono::milliseconds(4000));

		wostringstream oss;
		oss << L".\\black" << (i + 1) << ".raw";

		auto frame = Measure::getFundusFrameObject();
		if (frame != nullptr) {
			frame->exportFile(oss.str());
			count++;
		}
	}
	CString text;
	text.Format(_T("%d black images saved successfully!"), count);
	AfxMessageBox(text);
	EndWaitCursor();
	return;
}


void DlgColorCameraTest::OnBnClickedButtonPixelDetect()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	std::vector<std::wstring> whites;
	for (int i = 0; i < PIXEL_CORRECT_IMAGES_NUM; i++) {
		wostringstream oss;
		oss << L".\\white" << (i + 1) << ".raw";
		whites.push_back(oss.str());
	}

	std::vector<std::wstring> blacks;
	for (int i = 0; i < PIXEL_CORRECT_IMAGES_NUM; i++) {
		wostringstream oss;
		oss << L".\\black" << (i + 1) << ".raw";
		blacks.push_back(oss.str());
	}

	if (PixelCorrect::readWhitePictures(whites) == PIXEL_CORRECT_IMAGES_NUM &&
		PixelCorrect::readBlackPictures(blacks) == PIXEL_CORRECT_IMAGES_NUM)
	{
		int r, g, b;
		CString text;
		editDarkR.GetWindowTextW(text);
		r = _wtoi(text);
		editDarkG.GetWindowTextW(text);
		g = _wtoi(text);
		editDarkB.GetWindowTextW(text);
		b = _wtoi(text);
		PixelCorrect::setDarkThresholds(r, g, b);

		editBrightR.GetWindowTextW(text);
		r = _wtoi(text);
		editBrightG.GetWindowTextW(text);
		g = _wtoi(text);
		editBrightB.GetWindowTextW(text);
		b = _wtoi(text);
		PixelCorrect::setBrightThresholds(r, g, b);

		int count = PixelCorrect::detectBadPixels(PIXEL_CORRECT_IMAGES_NUM);
		if (count >= 0) {
			colorView.setPixelCoords(PixelCorrect::getBadPixelCoords());
			PixelCorrect::exportBadPixels();

			wostringstream oss;
			oss << count << L" bad pixels detected!";
			AfxMessageBox(oss.str().c_str());
		}
		else {
			AfxMessageBox(_T("Error on detect bad pixels!"));
		}
	}
	EndWaitCursor();
	return;
}


void DlgColorCameraTest::OnBnClickedButtonPixelCorrect()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (PixelCorrect::countBadPixels() <= 0) {
		AfxMessageBox(_T("Bad pixels not found on the current camera!"));
	}
	else {
		auto data = (unsigned short*)colorView.getFrameData();
		if (data == nullptr) {
			AfxMessageBox(_T("Frame data is empty!"));
		}
		else {
			if (!PixelCorrect::correctBadPixels(data)) {
				AfxMessageBox(_T("Bad pixels correction failed!"));
			}
			else {
				colorView.updateFrameImage();
			}
		}
	}
	EndWaitCursor();
	return;
}


void DlgColorCameraTest::OnBnClickedButtonBpLoad()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	wostringstream oss;
	if (Loader::loadDefectPixelsOfColorCamera()) {
		oss << PixelCorrect::countBadPixels() << L" bad pixels loaded from camera!";
		colorView.setPixelCoords(PixelCorrect::getBadPixelCoords());
		PixelCorrect::exportBadPixels();
	}
	else {
		oss << "Load bad pxiels from camera failed!";
	}
	AfxMessageBox(oss.str().c_str());
	EndWaitCursor();
	return;
}


void DlgColorCameraTest::OnBnClickedButtonBpSave()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	wostringstream oss;
	if (Loader::saveDefectPixelsOfColorCamera()) {
		oss << PixelCorrect::countBadPixels() << L" bad pixels saved to camera!";
	}
	else {
		oss << "Save bad pxiels from camera failed!";
	}
	AfxMessageBox(oss.str().c_str());
	EndWaitCursor();
	return;
}


void DlgColorCameraTest::OnBnClickedCheckAlignGuide()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_ALIGN_GUIDE) ? true : false;
	colorView.showAlignGuide(flag);
	retinaView.showAlignGuide(flag);
	corneaView.showAlignGuide(flag);
	return;
}

void DlgColorCameraTest::OnBnClickedButtonRetinaPreset1()
{
	// TODO: Add your control notification handler code here
	Controller::applyRetinaCameraPreset(0);
}


void DlgColorCameraTest::OnBnClickedButtonRetinaPreset2()
{
	// TODO: Add your control notification handler code here
	Controller::applyRetinaCameraPreset(1);
}


void DlgColorCameraTest::OnBnClickedButtonRetinaPreset3()
{
	// TODO: Add your control notification handler code here
	Controller::applyRetinaCameraPreset(2);
}


void DlgColorCameraTest::OnBnClickedButtonApplyRemovalLight()
{
	// TODO: Add your control notification handler code here
	CString cstr;
	GetDlgItemText(IDC_EDIT_LIGHT_RADIUS, cstr);
	int lightRadius = _ttoi(cstr);
	GetDlgItemText(IDC_EDIT_SHADOW_RADIUS, cstr);
	int shadowRadius = _ttoi(cstr);
	GetDlgItemText(IDC_EDIT_BORDER_RADIUS, cstr);
	int borderRadius = _ttoi(cstr);

	colorView.removeReflectionLight(lightRadius, shadowRadius, borderRadius);

	return;
}


void DlgColorCameraTest::OnBnClickedCheckRemovalLight()
{
	// TODO: Add your control notification handler code here
	bool flag = (IsDlgButtonChecked(IDC_CHECK_REMOVAL_LIGHT) ? true : false);
	CString cstr;
	GetDlgItemText(IDC_EDIT_LIGHT_RADIUS, cstr);
	int lightRadius = _ttoi(cstr);
	GetDlgItemText(IDC_EDIT_SHADOW_RADIUS, cstr);
	int shadowRadius = _ttoi(cstr);
	GetDlgItemText(IDC_EDIT_BORDER_RADIUS, cstr);
	int borderRadius = _ttoi(cstr);
	colorView.useRemoveReflectionLight(true, flag, lightRadius, shadowRadius, borderRadius);

	return;
}


void DlgColorCameraTest::OnBnClickedCheckRemoveGuide()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_REMOVE_GUIDE) ? true : false;
	colorView.showCircleGuide(flag);
}


void DlgColorCameraTest::OnEnChangeEditThreshold()
{
	CString text;
	editThreshold.GetWindowTextW(text);
	float threshold = _ttof(text);

	colorView.setOverBrightPixelThreshold(threshold);

	return;
}


void DlgColorCameraTest::OnBnClickedButtonBlackspotAlign()
{
	BeginWaitCursor();

	// Align guide -> OFF
	CheckDlgButton(IDC_CHECK_ALIGN_GUIDE, false);
	OnBnClickedCheckAlignGuide();

	//Digital Gain -> 8.00
	colorCamera->setDigitalGain(8.0f);
	colorCamera->updateParameters();
	editDgain.SetWindowTextW(_T("8.00"));

	//Fundus Focus -> -13.7
	fundusMotor->updateDiopter(-13.7f);
	updateFundusFocus();

	//Radial Correction OFF
	((CButton*)GetDlgItem(IDC_CHECK_RADIAL))->SetCheck(0);
	colorView.useRadialColorCorrection(true, false);

	//LEDs
	//flash -> 100 / Continous Mode -> ON
	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);
	pDlgLight->blackSpotAlign();

	//Motor
	//Mirror & Split -> OUT
	if (pDlgMotor.get() == nullptr) {
		pDlgMotor = make_unique<DlgMotorControl>();
		pDlgMotor->Create(IDD_DLGMOTORCONTROL, this);
	}
	pDlgMotor->ShowWindow(SW_SHOW);
	pDlgMotor->blackSpotAlign();

	CString text;
	GetDlgItemTextW(IDC_BUTTON_LIVE, text);

	if (!text.CompareNoCase(_T("Start Live"))) {
		OnBnClickedButtonLive();
	}

	EndWaitCursor();
	return;
}


void DlgColorCameraTest::OnBnClickedButtonBlackspotCapture()
{
	BeginWaitCursor();

	CString text;
	GetDlgItemTextW(IDC_BUTTON_LIVE, text);

	if (!text.CompareNoCase(_T("Pause Live"))) {
		OnBnClickedButtonLive();
	}

	// Align guide -> OFF
	CheckDlgButton(IDC_CHECK_ALIGN_GUIDE, false);
	OnBnClickedCheckAlignGuide();

	//Digital Gain -> 1.00
	colorCamera->setDigitalGain(1.0f);
	colorCamera->updateParameters();
	editDgain.SetWindowTextW(_T("1.00"));

	//Fundus Focus
	fundusMotor->updateDiopter(0.0f);
	updateFundusFocus();

	// Radial Correction ON
	((CButton*)GetDlgItem(IDC_CHECK_RADIAL))->SetCheck(1);
	colorView.useRadialColorCorrection(true, true);

	//LEDs
	//continous Mode -> OFF
	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);
	pDlgLight->blackSpotCapture();

	OnBnClickedButtonCapture();
	EndWaitCursor();
	return;
}


void DlgColorCameraTest::OnBnClickedButtonLedAlign()
{
	CString text;
	GetDlgItemTextW(IDC_BUTTON_LIVE, text);

	if (!text.CompareNoCase(_T("Pause Live"))) {
		OnBnClickedButtonLive();
	}

	// Align guide -> OFF
	CheckDlgButton(IDC_CHECK_ALIGN_GUIDE, false);
	OnBnClickedCheckAlignGuide();

	//Radial Correction ON
	((CButton*)GetDlgItem(IDC_CHECK_RADIAL))->SetCheck(1);
	colorView.useRadialColorCorrection(true, true);

	//LEDs
	//flash -> 0 / Ext -> 27
	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);
	pDlgLight->ledAlign();

	return;
}

void DlgColorCameraTest::OnBnClickedButtonRoiXUp()
{
	CString text;
	GetDlgItemText(IDC_EDIT_ROI_X1, text);

	int temp = _ttoi(text) + 5;
	text.Format(_T("%d"), temp);
	SetDlgItemText(IDC_EDIT_ROI_X1, text);

	return;
}


void DlgColorCameraTest::OnBnClickedButtonRoiXDown()
{
	CString text;
	GetDlgItemText(IDC_EDIT_ROI_X1, text);

	int temp = _ttoi(text) - 5;
	text.Format(_T("%d"), temp);
	SetDlgItemText(IDC_EDIT_ROI_X1, text);

	return;
}


void DlgColorCameraTest::OnBnClickedButtonRoiYUp()
{
	CString text;
	GetDlgItemText(IDC_EDIT_ROI_Y1, text);

	int temp = _ttoi(text) + 5;
	text.Format(_T("%d"), temp);
	SetDlgItemText(IDC_EDIT_ROI_Y1, text);

	return;
}


void DlgColorCameraTest::OnBnClickedButtonRoiYDown()
{
	CString text;
	GetDlgItemText(IDC_EDIT_ROI_Y1, text);

	int temp = _ttoi(text) - 5;
	text.Format(_T("%d"), temp);
	SetDlgItemText(IDC_EDIT_ROI_Y1, text);

	return;
}


void DlgColorCameraTest::OnBnClickedButtonRoiRadiusUp()
{
	CString text;
	GetDlgItemText(IDC_EDIT_ROI_RADIUS, text);

	int temp = _ttoi(text) + 2;
	text.Format(_T("%d"), temp);
	SetDlgItemText(IDC_EDIT_ROI_RADIUS, text);

	return;
}


void DlgColorCameraTest::OnBnClickedButtonRoiRadiusDown()
{
	CString text;
	GetDlgItemText(IDC_EDIT_ROI_RADIUS, text);

	int temp = _ttoi(text) - 2;
	text.Format(_T("%d"), temp);
	SetDlgItemText(IDC_EDIT_ROI_RADIUS, text);

	return;
}


void DlgColorCameraTest::OnBnClickedButtonUpdateColorRoi()
{
	BeginWaitCursor();
	FundusSettings* fset = SystemProfile::getFundusSettings();
	int x, y, value;
	CString cstr;

	GetDlgItemText(IDC_EDIT_ROI_X1, cstr);
	x = min(max(_ttoi(cstr), 0), 3840);
	GetDlgItemText(IDC_EDIT_ROI_Y1, cstr);
	y = min(max(_ttoi(cstr), 0), 2440);
	fset->setCenterPointOfROI(std::pair<int, int>(x, y));

	GetDlgItemText(IDC_EDIT_ROI_RADIUS, cstr);
	value = min(max(_ttoi(cstr), 0), 2440);
	fset->setRadiusOfROI(value);

	if (Loader::saveSetupProfile(false, false)) {
		Loader::applySystemProfile(true);
		AfxMessageBox(_T("Factory settings saved to mainboard!"));
	}
	else {
		AfxMessageBox(_T("Failed to save Factory settings to mainboard!"));
	}
	EndWaitCursor();

	return;
}

void DlgColorCameraTest::OnBnClickedButtonFundusCalibration()
{
	int fileNum = 0;
	//1. chage diopter
	fundusMotor->updateDiopter(-13.0f);
	updateFundusFocus();

	float minD = -13.f;
	float maxD = 13.f;

	int minF = 10;
	int maxF = 100;

	//float stepD = 0.25f;
	int stepD = 1;
	int stepF = 10;

	int countD = (int)((maxD - minD) / stepD);
	int countF = (maxF - minF) / stepF;

	// mkdir for calibration.
	std::string roitemp_folderPath = "D:\\Fundus_Calibration";
	// 폴더 존재 여부 확인
	if (_access(roitemp_folderPath.c_str(), 0) != 0) {
		// 폴더가 없으면 생성
		_mkdir(roitemp_folderPath.c_str());
	}
	int level = 0;
	//TEST -> only flash : 60
	//for (int i = 0; i < 3; i++) {
	for (int i = 0; i <= countF; i++) {
		level += stepF;
		flashLed->updateIntensity(level);
		updateFundusFlash();
		Sleep(250);
		switch (level) {
		case 10:
		case 70:
		case 80:
		case 90:
		case 100:
			continue;  // 해당 값일 경우 반복문 스킵
		default:
			break;
		}


		for (int j = 0; j < countD + 2; j++) {
			// start capture

			BeginWaitCursor();

			if (j == 0) { //start change flash
				fundusMotor->updateDiopter(-13.0f);
				updateFundusFocus();
			}

			OctFundusMeasure measure;
			measure.eyeSide() = Controller::getEyeSide();

			auto tempFundusRoi = GlobalSettings::centerOfFundusROI();
			auto tempRadius = GlobalSettings::radiusOfFundusROI();

			//change global settings for Radial correction 
			CString cstr;
			GetDlgItemText(IDC_EDIT_ROI_X1, cstr);
			int x1 = _ttoi(cstr);
			GetDlgItemText(IDC_EDIT_ROI_Y1, cstr);
			int y1 = _ttoi(cstr);
			GetDlgItemText(IDC_EDIT_ROI_RADIUS, cstr);
			int radius = _ttoi(cstr);

			GlobalSettings::centerOfFundusROI() = std::pair<int, int>(x1, y1);
			GlobalSettings::radiusOfFundusROI() = radius;
			Controller::takeColorFundus(measure, nullptr, nullptr,
				colorView.getImageCallbackFunction(),
				colorView.getFrameCallbackFunction(),
				false);

			//restore global settings for Radial correction
			GlobalSettings::centerOfFundusROI() = tempFundusRoi;
			GlobalSettings::radiusOfFundusROI() = tempRadius;

			EndWaitCursor();

			// end capture

			BeginWaitCursor();

			std::remove("fundus_info.txt");

			std::string src_filename = ".\\raw.tiff";
			std::string dst_filename = "D:\\Fundus_Calibration\\raw_"+ std::to_string(level) + "_" + std::to_string(j-1) + ".tiff";

			if (j != 0) {
				// 이동
				if (std::rename(src_filename.c_str(), dst_filename.c_str()) != 0) {
					std::perror("Failed to move file");
					// 필요시 errno 확인: errno
				}
			}
			else {
				std::remove("raw.tiff");
			}
			EndWaitCursor();

			//change D
			BeginWaitCursor();

			fundusMotor->updateDiopterByOffset(stepD);
			updateFundusFocus();
			EndWaitCursor();
			Sleep(250);
		}
	}
	CString text;

	text.Format(_T("Calibration Data has been saved."));

	AfxMessageBox(text);

	return;
}

void DlgColorCameraTest::saveROIData(const std::string& outputFilename, const std::vector<std::string>& imageFilenames) {
	if (imageFilenames.empty()) {
		throw std::runtime_error("No image files provided for saving.");
	}

	std::ofstream ofs(outputFilename, std::ios::binary);
	if (!ofs.is_open()) {
		throw std::runtime_error("Failed to open file for writing: " + outputFilename);
	}

	int imageCount = imageFilenames.size();
	ofs.write((char*)&imageCount, sizeof(int));

	std::vector<size_t> offsets(imageCount);
	std::vector<size_t> sizes(imageCount);
	size_t metadataOffset = ofs.tellp();
	ofs.seekp(metadataOffset + (sizeof(size_t) * 2 * imageCount), std::ios::beg);

	size_t currentOffset = ofs.tellp();
	for (int i = 0; i < imageCount; ++i) {

		std::ifstream ifs(imageFilenames[i], std::ios::binary | std::ios::ate);
		if (!ifs.is_open()) {
			throw std::runtime_error("Failed to open image file: " + imageFilenames[i]);
		}

		size_t fileSize = ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		std::vector<char> buffer(fileSize);
		ifs.read(buffer.data(), fileSize);
		ifs.close();

		ofs.write(buffer.data(), fileSize);

		offsets[i] = currentOffset;
		sizes[i] = fileSize;
		currentOffset += fileSize;
	}

	ofs.seekp(metadataOffset, std::ios::beg);
	for (int i = 0; i < imageCount; ++i) {
		ofs.write((char*)&offsets[i], sizeof(size_t));
		ofs.write((char*)&sizes[i], sizeof(size_t));
	}

	ofs.close();
}
