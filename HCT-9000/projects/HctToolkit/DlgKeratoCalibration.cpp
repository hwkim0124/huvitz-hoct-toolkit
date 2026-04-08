// DlgKeratoCalibration.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgKeratoCalibration.h"
#include "afxdialogex.h"

#include "OctSystem2.h"
#include "CppUtil2.h"

using namespace OctSystem;
using namespace CppUtil;
using namespace std;


// DlgKeratoCalibration dialog

IMPLEMENT_DYNAMIC(DlgKeratoCalibration, CDialogEx)

DlgKeratoCalibration::DlgKeratoCalibration(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGKERATOCALIBRATION, pParent)
{

}


DlgKeratoCalibration::~DlgKeratoCalibration()
{
}


void DlgKeratoCalibration::initControls(void)
{
	// Create CStatic control.
	// https://msdn.microsoft.com/en-us/library/kaw7w663.aspx
	corneaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 0, 640, 480), this);
	retinaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(640, 0, 640 + 644, 480), this);
	corneaView.ModifyStyle(0, SS_NOTIFY);
	retinaView.ModifyStyle(0, SS_NOTIFY);

	CheckDlgButton(IDC_CHECK_CAMERA, TRUE);
	CheckDlgButton(IDC_CHECK_FOCUS, TRUE);
	CheckDlgButton(IDC_CHECK_KERATO, TRUE);
	CheckDlgButton(IDC_CHECK_SPOTS, TRUE);
	CheckDlgButton(IDC_CHECK_GUIDE, TRUE);
	CheckDlgButton(IDC_CHECK_WORKING_DOTS, TRUE);

	CheckDlgButton(IDC_CHECK_SPLIT_FOCUS, TRUE);
	CheckDlgButton(IDC_CHECK_SPLIT_WINDOWS, TRUE);
	CheckDlgButton(IDC_CHECK_SPLIT_POINTS, TRUE);

	corneaView.showCameraInfo(true);
	corneaView.showFocusStatus(true);
	corneaView.showKeratoPoints(true);
	corneaView.showAlignGuide(true);
	corneaView.showSpotWindows(true);

	retinaView.showCameraInfo(true);
	retinaView.showAlignGuide(true);
	retinaView.showSplitPoints(true);
	retinaView.showSplitWindows(true);
	retinaView.showFocusStatus(true);
	retinaView.showWorkingDots(true);

	CheckDlgButton(IDC_CHECK_STAGE_X, Controller::getCorneaAlign()->enableStageX());
	CheckDlgButton(IDC_CHECK_STAGE_Y, Controller::getCorneaAlign()->enableStageY());
	CheckDlgButton(IDC_CHECK_STAGE_Z, Controller::getCorneaAlign()->enableStageZ());

	//BOOL flag = GlobalSettings::useRetinaROI();
	CheckDlgButton(IDC_CHECK_MASK_ROI, false);
	GlobalSettings::useRetinaROI(true, false);

	//flag = (BOOL)GlobalSettings::useRetinaCenterMask();
	CheckDlgButton(IDC_CHECK_CENTER_MASK, false);
	GlobalSettings::useRetinaCenterMask(true, false);

	auto center = GlobalSettings::centerOfRetinaROI();
	auto mask = GlobalSettings::centerOfRetinaMask();
	auto radius = GlobalSettings::radiusOfRetinaROI();
	auto split = GlobalSettings::splitFocusCenter();

	CString text;
	text.Format(_T("%d"), center.first);
	editRoiX1.SetWindowTextW(text);
	text.Format(_T("%d"), center.second);
	editRoiY1.SetWindowTextW(text);
	text.Format(_T("%d"), radius);
	editRoiRadius.SetWindowTextW(text);
	
	text.Format(_T("%d"), mask.first);
	editMaskX.SetWindowTextW(text);
	text.Format(_T("%d"), mask.second);
	editMaskY.SetWindowTextW(text);
	text.Format(_T("%d"), GlobalSettings::retinaCenterMaskSize());
	editRetinaMask.SetWindowTextW(text);

	text.Format(_T("%d"), split.first);
	editSplitCenterX.SetWindowTextW(text);
	text.Format(_T("%d"), split.second);
	editSplitCenterY.SetWindowTextW(text);

	text.Format(_T("%.1f"), GlobalSettings::retinaImageClipLimit());
	editRetinaClipLimit.SetWindowTextW(text);
	text.Format(_T("%d"), GlobalSettings::retinaImageClipScalar());
	editRetinaClipScalar.SetWindowTextW(text);

	BOOL flag = GlobalSettings::useRetinaROI();
	flag = GlobalSettings::useRetinaImageEnhance();
	CheckDlgButton(IDC_CHECK_ENABLE_RETINA_EQ, flag);

	CheckDlgButton(IDC_RADIO_IR_FPS1, TRUE);
	CheckDlgButton(IDC_RADIO_IR_FPS2, FALSE);
	return;
}


void DlgKeratoCalibration::controlCameras(bool play)
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


void DlgKeratoCalibration::loadSetupData(bool init)
{
	if (init) {
		cmbDiopt.SetCurSel(0);
		updateFocusStepData();
	}
	else {
		if (!Controller::getCorneaAlign()->loadKeratoSetupData()) {
			if (!init) {
				AfxMessageBox(_T("Loading kerato setup data failed!"));
			}
		}
		else {
			cmbDiopt.SetCurSel(0);
			updateFocusStepData();
			AfxMessageBox(_T("Kerato setup data loaded successfully!"));
		}
	}
	return;
}


void DlgKeratoCalibration::saveSetupData(void)
{
	if (!Controller::getCorneaAlign()->saveKeratoSetupData(true)) {
		AfxMessageBox(_T("Saving kerato setup data failed!"));
	}
	else {
		AfxMessageBox(_T("Kerato setup data saved successfully!"));
	}
	return;
}


void DlgKeratoCalibration::importSetupData(void)
{
	if (!Controller::getCorneaAlign()->importKeratoSetupData()) {
		AfxMessageBox(_T("Importing kerato setup data failed!"));
	}
	else {
		cmbDiopt.SetCurSel(0);
		updateFocusStepData();
		AfxMessageBox(_T("Kerato setup data imported successfully!"));
	}
	return;
}


void DlgKeratoCalibration::exportSetupData(void)
{
	if (!Controller::getCorneaAlign()->exportKeratoSetupData()) {
		AfxMessageBox(_T("Exporting kerato setup data failed!"));
	}
	else {
		AfxMessageBox(_T("Kerato setup data exported successfully!"));
	}
	return;
}


void DlgKeratoCalibration::updateFocusStepData(bool init)
{
	int curSel = cmbDiopt.GetCurSel();

	float m, n, a, flen, mlen, fdif;
	CString text, data;

	lstData.ResetContent();
	for (int i = 0; i < KER_SETUP_FOCUS_STEP_NUM; i++) {
		text.Format(_T("Step: %d"), i);
		if (Controller::getCorneaAlign()->getFocusStepData(curSel, i, mlen, flen, fdif, m, n, a)) {
			data.Format(_T(", Fd: %.1f, Ml: %.1f, Fl: %.1f, MNA: %.1f, %.1f, %.0f"), fdif, mlen, flen, m, n, a);
			text += data;
		}
		lstData.AddString(text);
	}
	return;
}


void DlgKeratoCalibration::setRadioIrExposureTime(UINT value)
{
	if (value == 0) {
		Controller::getMainBoard()->getRetinaIrCamera()->setExposureTime(false);
	}
	else {
		Controller::getMainBoard()->getRetinaIrCamera()->setExposureTime(true);
	}
}


void DlgKeratoCalibration::setInternalFixation(UINT value)
{
	UpdateData(TRUE);

	switch (value) {
	case IDC_RADIO_INTFIX_OD_FUNDUS:
		Controller::turnOnInternalFixation(EyeSide::OD, FixationTarget::Fundus);
		eyeSide = EyeSide::OD;
		isDisc = false;
		break;
	case IDC_RADIO_INTFIX_OS_FUNDUS:
		Controller::turnOnInternalFixation(EyeSide::OS, FixationTarget::Fundus);
		eyeSide = EyeSide::OS;
		isDisc = false;
		break;
	case IDC_RADIO_INTFIX_OD_DISC:
		Controller::turnOnInternalFixation(EyeSide::OD, FixationTarget::OpticDisc);
		eyeSide = EyeSide::OD;
		isDisc = true;
		break;
	case IDC_RADIO_INTFIX_OS_DISC:
		Controller::turnOnInternalFixation(EyeSide::OS, FixationTarget::OpticDisc);
		eyeSide = EyeSide::OS;
		isDisc = true;
		break;
	}
}


void DlgKeratoCalibration::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DATA, lstData);
	DDX_Control(pDX, IDC_COMBO_DIOPT, cmbDiopt);
	DDX_Control(pDX, IDC_EDIT_DELTA_X, editDeltaX);
	DDX_Control(pDX, IDC_EDIT_DELTA_Y, editDeltaY);
	DDX_Control(pDX, IDC_EDIT_DELTA_Z, editDeltaZ);
	DDX_Control(pDX, IDC_EDIT_ROI_X1, editRoiX1);
	DDX_Control(pDX, IDC_EDIT_ROI_Y1, editRoiY1);
	DDX_Control(pDX, IDC_EDIT_ROI_RADIUS, editRoiRadius);
	DDX_Control(pDX, IDC_EDIT_SPLIT_CENT_X, editSplitCenterX);
	DDX_Control(pDX, IDC_EDIT_SPLIT_CENT_Y, editSplitCenterY);
	DDX_Control(pDX, IDC_EDIT_RETINA_MASK, editRetinaMask);
	DDX_Control(pDX, IDC_EDIT_MASK_X, editMaskX);
	DDX_Control(pDX, IDC_EDIT_MASK_Y, editMaskY);
	DDX_Control(pDX, IDC_EDIT_RETINA_CLIP_LIMIT, editRetinaClipLimit);
	DDX_Control(pDX, IDC_EDIT_RETINA_CLIP_SCALAR, editRetinaClipScalar);
}


BEGIN_MESSAGE_MAP(DlgKeratoCalibration, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_FOCUS, &DlgKeratoCalibration::OnBnClickedButtonFocus)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA, &DlgKeratoCalibration::OnBnClickedButtonCamera)
	ON_BN_CLICKED(IDC_BUTTON_MOTOR, &DlgKeratoCalibration::OnBnClickedButtonMotor)
	ON_BN_CLICKED(IDC_BUTTON_LIGHT, &DlgKeratoCalibration::OnBnClickedButtonLight)
	ON_BN_CLICKED(IDC_BUTTON_START, &DlgKeratoCalibration::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_CHECK_CAMERA, &DlgKeratoCalibration::OnBnClickedCheckCamera)
	ON_BN_CLICKED(IDC_CHECK_FOCUS, &DlgKeratoCalibration::OnBnClickedCheckFocus)
	ON_BN_CLICKED(IDC_CHECK_GUIDE, &DlgKeratoCalibration::OnBnClickedCheckGuide)
	ON_BN_CLICKED(IDC_CHECK_KERATO, &DlgKeratoCalibration::OnBnClickedCheckKerato)
	ON_BN_CLICKED(IDC_CHECK_SPOTS, &DlgKeratoCalibration::OnBnClickedCheckSpots)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CORNEA, &DlgKeratoCalibration::OnBnClickedButtonSaveCornea)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_RETINA, &DlgKeratoCalibration::OnBnClickedButtonSaveRetina)
	ON_BN_CLICKED(IDC_BUTTON_KERATO_EXIT, &DlgKeratoCalibration::OnBnClickedButtonKeratoExit)
	ON_CBN_SELCHANGE(IDC_COMBO_DIOPT, &DlgKeratoCalibration::OnSelchangeComboDiopt)
	ON_LBN_SELCHANGE(IDC_LIST_DATA, &DlgKeratoCalibration::OnSelchangeListData)
	ON_BN_CLICKED(IDC_BUTTON_DEL_DATA, &DlgKeratoCalibration::OnBnClickedButtonDelData)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &DlgKeratoCalibration::OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_SET_DATA, &DlgKeratoCalibration::OnBnClickedButtonSetData)
	ON_BN_CLICKED(IDC_BUTTON_STAGE2, &DlgKeratoCalibration::OnBnClickedButtonStage2)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_DATA, &DlgKeratoCalibration::OnBnClickedButtonLoadData)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_DATA, &DlgKeratoCalibration::OnBnClickedButtonSaveData)
	ON_BN_CLICKED(IDC_CHECK_SPLIT_FOCUS, &DlgKeratoCalibration::OnBnClickedCheckSplitFocus)
	ON_BN_CLICKED(IDC_CHECK_SPLIT_WINDOWS, &DlgKeratoCalibration::OnBnClickedCheckSplitWindows)
	ON_BN_CLICKED(IDC_CHECK_SPLIT_POINTS, &DlgKeratoCalibration::OnBnClickedCheckSplitPoints)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_FUNDUS, &DlgKeratoCalibration::OnBnClickedButtonAutoFundus)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_TRACK, &DlgKeratoCalibration::OnBnClickedButtonAutoTrack)
	ON_BN_CLICKED(IDC_CHECK_RETINA_ROI_MASK, &DlgKeratoCalibration::OnBnClickedCheckRetinaRoiMask)
	ON_BN_CLICKED(IDC_CHECK_STAGE_X, &DlgKeratoCalibration::OnBnClickedCheckStageX)
	ON_BN_CLICKED(IDC_CHECK_STAGE_Y, &DlgKeratoCalibration::OnBnClickedCheckStageY)
	ON_BN_CLICKED(IDC_CHECK_STAGE_Z, &DlgKeratoCalibration::OnBnClickedCheckStageZ)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_X, &DlgKeratoCalibration::OnBnClickedButtonMoveX)
	ON_BN_CLICKED(IDC_BUTTON_STOP_X, &DlgKeratoCalibration::OnBnClickedButtonStopX)
	ON_BN_CLICKED(IDC_CHECK_STAGE_INFO, &DlgKeratoCalibration::OnBnClickedCheckStageInfo)
	ON_BN_CLICKED(IDC_BUTTON_ROI_APPLY, &DlgKeratoCalibration::OnBnClickedButtonRoiApply)
	ON_BN_CLICKED(IDC_CHECK_STAGE_OVERRIDE, &DlgKeratoCalibration::OnBnClickedCheckStageOverride)
	ON_BN_CLICKED(IDC_BUTTON_SPLIT_APPLY, &DlgKeratoCalibration::OnBnClickedButtonSplitApply)
	ON_BN_CLICKED(IDC_CHECK_WORKING_DOTS, &DlgKeratoCalibration::OnBnClickedCheckWorkingDots)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT_DATA, &DlgKeratoCalibration::OnBnClickedButtonImportData)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_DATA2, &DlgKeratoCalibration::OnBnClickedButtonExportData2)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_RETINA_EQ, &DlgKeratoCalibration::OnBnClickedCheckEnableRetinaEq)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_RETINA_EQ, &DlgKeratoCalibration::OnBnClickedButtonApplyRetinaEq)
	ON_BN_CLICKED(IDC_RADIO_IR_FPS2, &DlgKeratoCalibration::OnBnClickedRadioIrFps2)
	ON_BN_CLICKED(IDC_RADIO_IR_FPS1, &DlgKeratoCalibration::OnBnClickedRadioIrFps1)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_INTFIX_OD_FUNDUS, IDC_RADIO_INTFIX_OS_DISC, DlgKeratoCalibration::setInternalFixation)
	ON_BN_CLICKED(IDC_BUTTON_RESET_RETINA_EQ, &DlgKeratoCalibration::OnBnClickedButtonResetRetinaEq)
	ON_BN_CLICKED(IDC_BUTTON_SPLIT_UP, &DlgKeratoCalibration::OnBnClickedButtonSplitUp)
	ON_BN_CLICKED(IDC_BUTTON_SPLIT_DOWN, &DlgKeratoCalibration::OnBnClickedButtonSplitDown)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA_ALIGN_ON, &DlgKeratoCalibration::OnBnClickedButtonCameraAlignOn)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA_ALIGN_OFF, &DlgKeratoCalibration::OnBnClickedButtonCameraAlignOff)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_ROI_X_UP, &DlgKeratoCalibration::OnBnClickedButtonRetinaRoiXUp)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_ROI_X_DOWN, &DlgKeratoCalibration::OnBnClickedButtonRetinaRoiXDown)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_ROI_Y_UP, &DlgKeratoCalibration::OnBnClickedButtonRetinaRoiYUp)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_ROI_Y_DOWN, &DlgKeratoCalibration::OnBnClickedButtonRetinaRoiYDown)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_ROI, &DlgKeratoCalibration::OnBnClickedButtonSaveRoi)
	ON_EN_CHANGE(IDC_EDIT_ROI_X1, &DlgKeratoCalibration::OnEnChangeEditRoiX1)
	ON_EN_CHANGE(IDC_EDIT_ROI_Y1, &DlgKeratoCalibration::OnEnChangeEditRoiY1)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_AUTO_ADJUST, &DlgKeratoCalibration::OnBnClickedButtonRetinaAutoAdjust)
	ON_BN_CLICKED(IDC_BUTTON_TRACK_REGIST, &DlgKeratoCalibration::OnBnClickedButtonTrackRegist)
	ON_BN_CLICKED(IDC_BUTTON_TRACK_PREPARE, &DlgKeratoCalibration::OnBnClickedButtonTrackPrepare)
	ON_BN_CLICKED(IDC_BUTTON_TRACK_RETINA, &DlgKeratoCalibration::OnBnClickedButtonTrackRetina)
END_MESSAGE_MAP()


// DlgKeratoCalibration message handlers


BOOL DlgKeratoCalibration::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	initControls();
	loadSetupData(true);
	isPlaying = false;
	OnBnClickedButtonStart();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgKeratoCalibration::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgKeratoCalibration::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	controlCameras(false);

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


void DlgKeratoCalibration::OnBnClickedButtonLoadData()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	loadSetupData(false);
	EndWaitCursor();
}


void DlgKeratoCalibration::OnBnClickedButtonSaveData()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	saveSetupData();
	EndWaitCursor();
}


void DlgKeratoCalibration::OnBnClickedButtonImportData()
{
	BeginWaitCursor();
	importSetupData();
	EndWaitCursor();
}


void DlgKeratoCalibration::OnBnClickedButtonExportData2()
{
	BeginWaitCursor();
	exportSetupData();
	EndWaitCursor();
}


void DlgKeratoCalibration::OnBnClickedButtonFocus()
{
	// TODO: Add your control notification handler code here
	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonCamera()
{
	// TODO: Add your control notification handler code here
	if (pDlgCamera.get() == nullptr) {
		pDlgCamera = make_unique<DlgCameraControl>();
		pDlgCamera->Create(IDD_DLGCAMERACONTROL, this);
	}
	pDlgCamera->ShowWindow(SW_SHOW);
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonMotor()
{
	// TODO: Add your control notification handler code here
	if (pDlgMotor.get() == nullptr) {
		pDlgMotor = make_unique<DlgMotorControl>();
		pDlgMotor->Create(IDD_DLGMOTORCONTROL, this);
	}
	pDlgMotor->ShowWindow(SW_SHOW);
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonLight()
{
	// TODO: Add your control notification handler code here
	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonStage2()
{
	// TODO: Add your control notification handler code here
	if (pDlgStage.get() == nullptr) {
		pDlgStage = make_unique<DlgStageControl>();
		pDlgStage->Create(IDD_DLGSTAGECONTROL, this);
	}
	pDlgStage->ShowWindow(SW_SHOW);
	return;
}



void DlgKeratoCalibration::OnBnClickedButtonStart()
{
	// TODO: Add your control notification handler code here
	isPlaying = (isPlaying ? false : true);
	controlCameras(isPlaying);

	if (isPlaying) {
		SetDlgItemText(IDC_BUTTON_START, _T("Pause Camera"));
	}
	else {
		SetDlgItemText(IDC_BUTTON_START, _T("Start Camera"));
	}
}


void DlgKeratoCalibration::OnBnClickedCheckCamera()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_CAMERA) ? false : true;
	corneaView.showCameraInfo(flag);
	retinaView.showCameraInfo(flag);
	CheckDlgButton(IDC_CHECK_CAMERA, flag);
}


void DlgKeratoCalibration::OnBnClickedCheckFocus()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_FOCUS) ? false : true;
	corneaView.showFocusStatus(flag);
	CheckDlgButton(IDC_CHECK_FOCUS, flag);
}

void DlgKeratoCalibration::OnBnClickedCheckGuide()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_GUIDE) ? false : true;
	corneaView.showAlignGuide(flag);
	retinaView.showAlignGuide(flag);
	CheckDlgButton(IDC_CHECK_GUIDE, flag);
}

void DlgKeratoCalibration::OnBnClickedCheckKerato()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_KERATO) ? false : true;
	corneaView.showKeratoPoints(flag);
	CheckDlgButton(IDC_CHECK_KERATO, flag);
}

void DlgKeratoCalibration::OnBnClickedCheckSpots()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_SPOTS) ? false : true;
	corneaView.showSpotWindows(flag);
	CheckDlgButton(IDC_CHECK_SPOTS, flag);
}


void DlgKeratoCalibration::OnBnClickedCheckSplitFocus()
{
	bool flag = IsDlgButtonChecked(IDC_CHECK_SPLIT_FOCUS) ? false : true;
	retinaView.showFocusStatus(flag);
	CheckDlgButton(IDC_CHECK_SPLIT_FOCUS, flag);
}


void DlgKeratoCalibration::OnBnClickedCheckSplitWindows()
{
	bool flag = IsDlgButtonChecked(IDC_CHECK_SPLIT_WINDOWS) ? false : true;
	retinaView.showSplitWindows(flag);
	CheckDlgButton(IDC_CHECK_SPLIT_WINDOWS, flag);
}


void DlgKeratoCalibration::OnBnClickedCheckSplitPoints()
{
	bool flag = IsDlgButtonChecked(IDC_CHECK_SPLIT_POINTS) ? false : true;
	retinaView.showSplitPoints(flag);
	CheckDlgButton(IDC_CHECK_SPLIT_POINTS, flag);
}


void DlgKeratoCalibration::OnBnClickedCheckRetinaRoiMask()
{
	bool flag = IsDlgButtonChecked(IDC_CHECK_RETINA_ROI_MASK) ? false : true;
	retinaView.showCircleMask(flag);
	CheckDlgButton(IDC_CHECK_RETINA_ROI_MASK, flag);
}



void DlgKeratoCalibration::OnBnClickedCheckStageInfo()
{
	bool flag = IsDlgButtonChecked(IDC_CHECK_STAGE_INFO) ? false : true;
	corneaView.showStageStatus(flag);
	CheckDlgButton(IDC_CHECK_STAGE_INFO, flag);
}


void DlgKeratoCalibration::OnBnClickedCheckWorkingDots()
{
	bool flag = IsDlgButtonChecked(IDC_CHECK_WORKING_DOTS) ? false : true;
	retinaView.showWorkingDots(flag);
	CheckDlgButton(IDC_CHECK_WORKING_DOTS, flag);
}



void DlgKeratoCalibration::OnBnClickedButtonSaveCornea()
{
	// TODO: Add your control notification handler code here
	/*
	if (isPlaying) {
		OnBnClickedButtonStart();
	}
	*/

	BeginWaitCursor();
	CString path, text;
	if (corneaView.saveImage(path)) {
		text.Format(_T("Image file: '%s' saved!"), path);
		AfxMessageBox(text);
	}
	EndWaitCursor();
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonSaveRetina()
{
	// TODO: Add your control notification handler code here
	/*
	if (isPlaying) {
		OnBnClickedButtonStart();
	}
	*/

	BeginWaitCursor();
	CString path, text;
	if (retinaView.saveImage(path)) {
		text.Format(_T("Image file: '%s' saved!"), path);
		AfxMessageBox(text);
	}
	EndWaitCursor();
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonKeratoExit()
{
	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);
	return;
}


void DlgKeratoCalibration::OnSelchangeComboDiopt()
{
	// TODO: Add your control notification handler code here
	updateFocusStepData();
}


void DlgKeratoCalibration::OnSelchangeListData()
{
	// TODO: Add your control notification handler code here
}


void DlgKeratoCalibration::OnBnClickedButtonDelData()
{
	// TODO: Add your control notification handler code here
	int istep = lstData.GetCurSel();
	if (istep != LB_ERR) {
		int diopt = cmbDiopt.GetCurSel();
		Controller::getCorneaAlign()->clearFocusStepData(diopt, istep);
	}
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonClear()
{
	// TODO: Add your control notification handler code here
	Controller::getCorneaAlign()->clearFocusStepData();
	updateFocusStepData();
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonSetData()
{
	// TODO: Add your control notification handler code here
	int istep = lstData.GetCurSel();
	if (istep != LB_ERR) {
		int diopt = cmbDiopt.GetCurSel();
		bool ret = Controller::getCorneaAlign()->updateFocusStepDataFromImage(diopt, istep);

		if (ret) {
			updateFocusStepData();

			int inext;
			switch (istep)
			{
			case 2:
				inext = 1;
				break;
			case 1:
				inext = 0;
				break;
			case 0:
				inext = 3;
				break;
			case 3:
				inext = 4;
				break;
			case 4:
				inext = 4;
				break;
			default:
				break;
			}
			lstData.SetCurSel(inext);
		}
		else {
			AfxMessageBox(_T("Update focus step data failed!"));
		}
	}
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonAutoFundus()
{
	if (Controller::isAutoOptimizing()) {
		Controller::cancelAutoOptimize();
	}
	else {
		Controller::startAutoOptimize(false);
	}
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonAutoTrack()
{
	if (Controller::isAutoAligning()) {
		Controller::cancelAutoAlignment();
	}
	else {
		Controller::startAutoAlignment();
	}
	return;
}


void DlgKeratoCalibration::OnBnClickedCheckStageX()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_STAGE_X) ? false : true;
	Controller::getCorneaAlign()->enableStageX(true, flag);
	CheckDlgButton(IDC_CHECK_STAGE_X, flag);
	return;
}


void DlgKeratoCalibration::OnBnClickedCheckStageY()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_STAGE_Y) ? false : true;
	Controller::getCorneaAlign()->enableStageY(true, flag);
	CheckDlgButton(IDC_CHECK_STAGE_Y, flag);
	return;
}


void DlgKeratoCalibration::OnBnClickedCheckStageZ()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_STAGE_Z) ? false : true;
	Controller::getCorneaAlign()->enableStageZ(true, flag);
	CheckDlgButton(IDC_CHECK_STAGE_Z, flag);
	return;
}

void DlgKeratoCalibration::OnBnClickedButtonMoveX()
{
	// TODO: Add your control notification handler code here
	CString text;
	int offset;

	editDeltaX.GetWindowTextW(text);
	offset = _ttoi(text);
	Controller::getMainBoard()->getStageXMotor()->updatePositionByOffset(offset);
	// Controller::getMainBoard()->getStageXMotor()->controlMove(offset);
	editDeltaY.GetWindowTextW(text);
	offset = _ttoi(text);
	Controller::getMainBoard()->getStageYMotor()->updatePositionByOffset(offset);
	editDeltaZ.GetWindowTextW(text);
	offset = _ttoi(text);
	Controller::getMainBoard()->getStageZMotor()->updatePositionByOffset(offset);
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonStopX()
{
	// TODO: Add your control notification handler code here
	Controller::getMainBoard()->getStageXMotor()->controlStop();
	Controller::getMainBoard()->getStageYMotor()->controlStop();
	Controller::getMainBoard()->getStageZMotor()->controlStop();
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonRoiApply()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_MASK_ROI) ? true : false;
	GlobalSettings::useRetinaROI(true, flag);

	CString text;
	editRoiX1.GetWindowTextW(text);
	GlobalSettings::centerOfRetinaROI().first = _ttoi(text);
	editRoiY1.GetWindowTextW(text);
	GlobalSettings::centerOfRetinaROI().second = _ttoi(text);

	editRoiRadius.GetWindowTextW(text);
	GlobalSettings::radiusOfRetinaROI() = _ttoi(text);

	flag = IsDlgButtonChecked(IDC_CHECK_CENTER_MASK) ? true : false;
	GlobalSettings::useRetinaCenterMask(true, flag);

	editMaskX.GetWindowTextW(text);
	GlobalSettings::centerOfRetinaMask().first = _ttoi(text);
	editMaskY.GetWindowTextW(text);
	GlobalSettings::centerOfRetinaMask().second = _ttoi(text);

	editRetinaMask.GetWindowTextW(text);
	GlobalSettings::retinaCenterMaskSize() = _ttoi(text);
	return;
}


void DlgKeratoCalibration::OnBnClickedCheckStageOverride()
{
	// TODO: Add your control notification handler code here
	bool flag = IsDlgButtonChecked(IDC_CHECK_STAGE_OVERRIDE) ? false : true;
	Controller::getMainBoard()->setStageLimitOverride(flag);
	CheckDlgButton(IDC_CHECK_STAGE_OVERRIDE, flag);
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonSplitApply()
{
	// TODO: Add your control notification handler code here
	CString text;
	editSplitCenterX.GetWindowTextW(text);
	GlobalSettings::splitFocusCenter().first = _ttoi(text);
	editSplitCenterY.GetWindowTextW(text);
	GlobalSettings::splitFocusCenter().second = _ttoi(text);
	return;
}


void DlgKeratoCalibration::OnBnClickedCheckEnableRetinaEq()
{
	bool flag = IsDlgButtonChecked(IDC_CHECK_ENABLE_RETINA_EQ) ? false : true;
	GlobalSettings::useRetinaImageEnhance(true, flag);
	CheckDlgButton(IDC_CHECK_ENABLE_RETINA_EQ, flag);
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonApplyRetinaEq()
{
	// TODO: Add your control notification handler code here
	CString text;
	editRetinaClipLimit.GetWindowTextW(text);
	GlobalSettings::retinaImageClipLimit() = (float)_ttof(text);
	editRetinaClipScalar.GetWindowTextW(text);
	GlobalSettings::retinaImageClipScalar() = _ttoi(text);

	LogD() << "Clip Limit: " << GlobalSettings::retinaImageClipLimit() << ", enable: " << GlobalSettings::useRetinaImageEnhance();
	return;
}


void DlgKeratoCalibration::OnBnClickedRadioIrFps2()
{
	// TODO: Add your control notification handler code here
	setRadioIrExposureTime(1);
}


void DlgKeratoCalibration::OnBnClickedRadioIrFps1()
{
	// TODO: Add your control notification handler code here
	setRadioIrExposureTime(0);
}

void DlgKeratoCalibration::OnBnClickedButtonTrackRegist()
{
	BeginWaitCursor();
	auto* rett = RetinaTracker::getInstance();
	if (rett) {
		rett->requestRegistration(true);
		bool result = rett->waitForRegistration(100);
		if (result) {
			AfxMessageBox(_T("Retina feature registered!"));
		}
		else {
			AfxMessageBox(_T("Retina feature registration failed!"));
		}
	}
	EndWaitCursor();
	return;
}

void DlgKeratoCalibration::OnBnClickedButtonTrackRetina()
{
	static bool started = true;

	auto* inst = RetinaTracker::getInstance();
	if (inst) {
		if (started) {
			if (inst->isFeatureRegistered()) {
				inst->startRetinaTracking();
				SetDlgItemTextW(IDC_BUTTON_TRACK_RETINA, _T("Pause"));
				AfxMessageBox(_T("Retina tracking started!"));
				started = true;
			}
			else {
				AfxMessageBox(_T("Target is not registered!"));
			}
		}
		else {
			inst->pauseRetinaTracking();
			SetDlgItemTextW(IDC_BUTTON_TRACK_RETINA, _T("Start"));
			AfxMessageBox(_T("Retina tracking paused!"));
			started = false;
		}
	}
}


void DlgKeratoCalibration::OnBnClickedButtonTrackPrepare()
{
	static bool turnOn = true;

	if (turnOn) {
		Controller::prepareLEDsForRetinaTrack(true);
		turnOn = false;
		AfxMessageBox(_T("Retina camera prepared for tracking!"));
	}
	else {
		auto* rett = RetinaTracker::getInstance();
		rett->cancelRetinaTracking();

		Controller::prepareLEDsForRetinaTrack(false);
		turnOn = true;
		AfxMessageBox(_T("Retina camera released after tracking!"));
	}
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonRetinaAutoAdjust()
{
	// TODO: Add your control notification handler code here
	static bool enabled = false;

	auto* inst = RetinaTracker::getInstance();
	if (inst) {
		if (!enabled) {
			inst->setupRetinaTracking(eyeSide, isDisc, 4.5f, 4.5f);
			inst->requestAutoAdjustment(false);
			retinaView.showTrackFeature(true);
			enabled = true;
			AfxMessageBox(_T("Retina camera auto adjust started!"));
		}
		else {
			if (inst->completeAutoAdjustment()) {
				AfxMessageBox(_T("Retina camera auto adjust completed!"));
			}
			enabled = false;
		}
	}
}

void DlgKeratoCalibration::OnBnClickedButtonResetRetinaEq()
{
	// TODO: Add your control notification handler code here
}


void DlgKeratoCalibration::OnBnClickedButtonSplitUp()
{
	CString text;
	editSplitCenterY.GetWindowTextW(text);

	int temp = _ttoi(text) + 1;
	text.Format(_T("%d"), temp);
	editSplitCenterY.SetWindowTextW(text);

	OnBnClickedButtonSplitApply();
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonSplitDown()
{
	CString text;
	editSplitCenterY.GetWindowTextW(text);

	int temp = _ttoi(text) - 1;
	text.Format(_T("%d"), temp);
	editSplitCenterY.SetWindowTextW(text);

	OnBnClickedButtonSplitApply();
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonCameraAlignOn()
{
	bool flag = false;

	CheckDlgButton(IDC_CHECK_CAMERA, flag);
	CheckDlgButton(IDC_CHECK_GUIDE, flag);
	CheckDlgButton(IDC_CHECK_FOCUS, flag);
	CheckDlgButton(IDC_CHECK_KERATO, flag);
	CheckDlgButton(IDC_CHECK_SPOTS, flag);
	CheckDlgButton(IDC_CHECK_SPLIT_FOCUS, flag);
	CheckDlgButton(IDC_CHECK_SPLIT_POINTS, flag);
	CheckDlgButton(IDC_CHECK_SPLIT_WINDOWS, flag);
	CheckDlgButton(IDC_CHECK_WORKING_DOTS, flag);

	OnBnClickedCheckCamera();
	OnBnClickedCheckGuide();
	OnBnClickedCheckFocus();
	OnBnClickedCheckKerato();
	OnBnClickedCheckSpots();
	OnBnClickedCheckSplitFocus();
	OnBnClickedCheckSplitPoints();
	OnBnClickedCheckSplitWindows();
	OnBnClickedCheckWorkingDots();

	return;
}


void DlgKeratoCalibration::OnBnClickedButtonCameraAlignOff()
{
	bool flag = true;

	CheckDlgButton(IDC_CHECK_CAMERA, flag);
	CheckDlgButton(IDC_CHECK_GUIDE, flag);
	CheckDlgButton(IDC_CHECK_FOCUS, flag);
	CheckDlgButton(IDC_CHECK_KERATO, flag);
	CheckDlgButton(IDC_CHECK_SPOTS, flag);
	CheckDlgButton(IDC_CHECK_SPLIT_FOCUS, flag);
	CheckDlgButton(IDC_CHECK_SPLIT_POINTS, flag);
	CheckDlgButton(IDC_CHECK_SPLIT_WINDOWS, flag);
	CheckDlgButton(IDC_CHECK_WORKING_DOTS, flag);

	OnBnClickedCheckCamera();
	OnBnClickedCheckGuide();
	OnBnClickedCheckFocus();
	OnBnClickedCheckKerato();
	OnBnClickedCheckSpots();
	OnBnClickedCheckSplitFocus();
	OnBnClickedCheckSplitPoints();
	OnBnClickedCheckSplitWindows();
	OnBnClickedCheckWorkingDots();

	return;
}


void DlgKeratoCalibration::OnBnClickedButtonRetinaRoiXUp()
{
	CString text;
	editRoiX1.GetWindowTextW(text);

	int temp = _ttoi(text) + 1;
	text.Format(_T("%d"), temp);
	editRoiX1.SetWindowTextW(text);

	OnBnClickedButtonRoiApply();
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonRetinaRoiXDown()
{
	CString text;
	editRoiX1.GetWindowTextW(text);

	int temp = _ttoi(text) - 1;
	text.Format(_T("%d"), temp);
	editRoiX1.SetWindowTextW(text);

	OnBnClickedButtonRoiApply();
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonRetinaRoiYUp()
{
	CString text;
	editRoiY1.GetWindowTextW(text);

	int temp = _ttoi(text) + 1;
	text.Format(_T("%d"), temp);
	editRoiY1.SetWindowTextW(text);

	OnBnClickedButtonRoiApply();
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonRetinaRoiYDown()
{
	CString text;
	editRoiY1.GetWindowTextW(text);

	int temp = _ttoi(text) - 1;
	text.Format(_T("%d"), temp);
	editRoiY1.SetWindowTextW(text);

	OnBnClickedButtonRoiApply();
	return;
}


void DlgKeratoCalibration::OnBnClickedButtonSaveRoi()
{
	BeginWaitCursor();
	RetinaSettings* rset = SystemProfile::getRetinaSettings();
	CString text;
	int x, y = 0;

	if (editRoiX1.GetWindowTextLengthW() <= 0 ||
		editRoiY1.GetWindowTextLengthW() <= 0 ||
		editSplitCenterX.GetWindowTextLengthW() <= 0 ||
		editSplitCenterY.GetWindowTextLengthW() <= 0) {
		return;
	}

	//Retina ROI & MASK
	editRoiX1.GetWindowTextW(text);
	x = min(max(_ttoi(text), 0), 640);
	editRoiY1.GetWindowTextW(text);
	y = min(max(_ttoi(text), 0), 480);

	rset->setCenterPointOfROI(std::pair<int, int>(x, y));
	rset->setCenterPointOfMask(std::pair<int, int>(x, y));

	//Split Focus
	editSplitCenterX.GetWindowTextW(text);
	x = min(max(_ttoi(text), 0), 640);
	editSplitCenterY.GetWindowTextW(text);
	y = min(max(_ttoi(text), 0), 480);

	rset->setSplitCenter(std::pair<int, int>(x, y));

	if (Loader::saveSetupProfile(false, false)) {
		Loader::applySystemProfile(true);
		AfxMessageBox(_T("Factory settings saved to mainboard!"));
	}
	else {
		AfxMessageBox(_T("Failed to save Factory settings to mainboard!"));
	}

	OnBnClickedButtonRoiApply();

	EndWaitCursor();
	return;
}


void DlgKeratoCalibration::OnEnChangeEditRoiX1()
{
	CString text;
	editRoiX1.GetWindowTextW(text);

	editMaskX.SetWindowTextW(text);
	return;
}


void DlgKeratoCalibration::OnEnChangeEditRoiY1()
{
	CString text;
	editRoiY1.GetWindowTextW(text);

	editMaskY.SetWindowTextW(text);
	return;
}

