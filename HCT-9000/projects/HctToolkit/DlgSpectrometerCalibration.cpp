// DlgSpectrometerCalibration.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgSpectrometerCalibration.h"
#include "afxdialogex.h"



// DlgSpectrometerCalibration dialog

IMPLEMENT_DYNAMIC(DlgSpectrometerCalibration, CDialogEx)

DlgSpectrometerCalibration::DlgSpectrometerCalibration(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGSPECTROMETERCALIBRATION, pParent)
{
	this->callback = std::bind(&DlgSpectrometerCalibration::callbackProfileImage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

}


DlgSpectrometerCalibration::~DlgSpectrometerCalibration()
{
}


void DlgSpectrometerCalibration::initControls(void)
{
	cmbClass.ResetContent();
	for (int i = 0; i < NUMBER_OF_FBG_CLASSES; i++) {
		cmbClass.AddString(CString(spectCalib.getFbgClassName(i)));
	}

	cmbClass.SetCurSel(2);
	OnSelchangeComboClass();
	OnClickedButtonWaveReset();

	cmbProfile.SetCurSel(9);

	auto again = Scanner::getCameraAnalogGain();
	auto csel = (again == 2.0f ? 1 : (again == 4.0f ? 2 : 0));
	cmbLineCamAgain.SetCurSel(csel);

	sldThresh.SetRange(0, FBG_PEAK_THRESHOLD_MAX);
	sldThresh.SetPos(FBG_PEAK_THRESHOLD_INIT);
	sldThresh2.SetRange(0, FBG_PEAK_THRESHOLD_MAX);
	sldThresh2.SetPos(FBG_PEAK_THRESHOLD_INIT);

	updateThreshPeak(false);
	updateThreshSubs(false);
	updateProfiles();

	updateGraphDisplay(true);
	updateGraphEndsDisplay(true);

	CheckDlgButton(IDC_RADIO_ORDER1, FALSE);
	CheckDlgButton(IDC_RADIO_ORDER2, TRUE);

	CheckDlgButton(IDC_RADIO_CAMERA1, FALSE);
	CheckDlgButton(IDC_RADIO_CAMERA2, FALSE);
	CheckDlgButton(IDC_RADIO_CAMERA3, TRUE);
	updateRadioCameraSpeed(IDC_RADIO_CAMERA3);
	return;
}


void DlgSpectrometerCalibration::alignControls(void)
{
	corneaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 0, 320, 240), this);
	retinaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 240, 320, 480), this);

	corneaView.setDisplaySize(320, 240);
	retinaView.setDisplaySize(320, 240);

	profileView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(320, 0, 320 + 1024, 512), this);
	profileView.setDisplaySize(1024, 512);

	profileHeadView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(320+1024, 0, 320+1024+320, 256), this);
	profileHeadView.setDisplaySize(320, 256);
	profileTailView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(320+1024, 256, 320+1024+320, 256+256), this);
	profileTailView.setDisplaySize(320, 256);

	profileHeadView.setShowGraphInfo(false);
	profileTailView.setShowGraphInfo(false);
	profileHeadView.setIsProfileEnds(true);
	profileTailView.setIsProfileEnds(true);
	return;
}


void DlgSpectrometerCalibration::controlCameras(bool play)
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


void DlgSpectrometerCalibration::updateThreshPeak(bool redraw)
{
	CString text;
	editThresh.GetWindowText(text);
	int value;
	if (text.GetLength() > 0) {
		value = _ttoi(text);
		value = (value < 0 ? 0 : value);
		value = (value > FBG_PEAK_THRESHOLD_MAX ? FBG_PEAK_THRESHOLD_MAX : value);
		sldThresh.SetPos(value);
	}
	else {
		value = sldThresh.GetPos();
	}
	text.Format(_T("%d"), value);
	editThresh.SetWindowTextW(text);
	editThresh.SetSel(0, -1, FALSE);

	profileView.setThresholdPeak(value);
	if (redraw) {
		profileView.updateWindow();
	}
	return;
}


void DlgSpectrometerCalibration::updateThreshSubs(bool redraw)
{
	CString text;
	editThresh2.GetWindowText(text);
	int value;
	if (text.GetLength() > 0) {
		value = _ttoi(text);
		value = (value < 0 ? 0 : value);
		value = (value > FBG_PEAK_THRESHOLD_MAX ? FBG_PEAK_THRESHOLD_MAX : value);
		sldThresh2.SetPos(value);
	}
	else {
		value = sldThresh2.GetPos();
	}
	text.Format(_T("%d"), value);
	editThresh2.SetWindowTextW(text);
	editThresh2.SetSel(0, -1, FALSE);

	profileView.setThresholdSubs(value);
	if (redraw) {
		profileView.updateWindow();
	}
	return;
}


void DlgSpectrometerCalibration::updateProfiles(void)
{
	CString text;
	int value;
	profileView.clearProfiles();
	profileHeadView.clearProfiles();
	profileTailView.clearProfiles();

	for (int i = 0; i < lstProfiles.GetCount(); i++) {
		lstProfiles.GetText(i, text);
		if (text.GetLength() > 0) {
			value = _ttoi(text);
			profileView.insertProfile(value);
			profileHeadView.insertProfile(value);
			profileTailView.insertProfile(value);
		}
	}
	return;
}

void DlgSpectrometerCalibration::updateExposureTime(void)
{
	CString text;
	auto expTime = Scanner::getCameraExposureTime();
	text.Format(_T("%.1f"), expTime);
	SetDlgItemText(IDC_EDIT_EXPOSURE, text);
}


void DlgSpectrometerCalibration::updateSpectroParams(double * params)
{
	CString text;

	if (params != nullptr)
	{
		text.Format(_T("%e"), params[0]);
		editParam1.SetWindowTextW(text);
		text.Format(_T("%e"), params[1]);
		editParam2.SetWindowTextW(text);
		text.Format(_T("%e"), params[2]);
		editParam3.SetWindowTextW(text);
		text.Format(_T("%e"), params[3]);
		editParam4.SetWindowTextW(text);

		profileView.setResamplingParameters(params);
	}
	return;
}


void DlgSpectrometerCalibration::clearSpectroParams(void)
{
	editParam1.SetWindowTextW(_T(""));
	editParam2.SetWindowTextW(_T(""));
	editParam3.SetWindowTextW(_T(""));
	editParam4.SetWindowTextW(_T(""));
	return;
}


void DlgSpectrometerCalibration::updateGraphDisplay(bool reset)
{
	if (reset) {
		editRangeX1.SetWindowTextW(L"0");
		editRangeX2.SetWindowTextW(L"2047");
		editRangeY1.SetWindowTextW(L"0");
		editRangeY2.SetWindowTextW(L"4095");
		editCenterX.SetWindowTextW(L"1023");

		CheckDlgButton(IDC_CHECK_SHOW_TEXT, TRUE);
		CheckDlgButton(IDC_CHECK_RECORD_VALUES, TRUE);
	}
	else {
		CString text;
		editRangeX1.GetWindowTextW(text);
		int x1 = _ttoi(text);
		editRangeX2.GetWindowTextW(text);
		int x2 = _ttoi(text);
		editRangeY1.GetWindowTextW(text);
		int y1 = _ttoi(text);
		editRangeY2.GetWindowTextW(text);
		int y2 = _ttoi(text);
		editCenterX.GetWindowTextW(text);
		int cx = _ttoi(text);

		profileView.setRangeX(x1, x2);
		profileView.setRangeY(y1, y2);
		profileView.setCenterX(cx);
	}
	return;
}

void DlgSpectrometerCalibration::updateGraphEndsDisplay(bool reset)
{
	if (reset) {
		editHeadX1.SetWindowTextW(L"0");
		editHeadX2.SetWindowTextW(L"1");
		editHeadY1.SetWindowTextW(L"0");
		editHeadY2.SetWindowTextW(L"30");
		editTailX1.SetWindowTextW(L"2046");
		editTailX2.SetWindowTextW(L"2047");
		editTailY1.SetWindowTextW(L"0");
		editTailY2.SetWindowTextW(L"500");
	}
	else {
		CString text;
		editHeadX1.GetWindowTextW(text);
		int x1 = _ttoi(text);
		editHeadX2.GetWindowTextW(text);
		int x2 = _ttoi(text);
		editHeadY1.GetWindowTextW(text);
		int y1 = _ttoi(text);
		editHeadY2.GetWindowTextW(text);
		int y2 = _ttoi(text);

		profileHeadView.setRangeX(x1, x2);
		profileHeadView.setRangeY(y1, y2);

		editTailX1.GetWindowTextW(text);
		x1 = _ttoi(text);
		editTailX2.GetWindowTextW(text);
		x2 = _ttoi(text);
		editTailY1.GetWindowTextW(text);
		y1 = _ttoi(text);
		editTailY2.GetWindowTextW(text);
		y2 = _ttoi(text);

		profileTailView.setRangeX(x1, x2);
		profileTailView.setRangeY(y1, y2);
	}
	return;
}


void DlgSpectrometerCalibration::updateRadioCameraSpeed(UINT value)
{
	bool retb = true;
	switch (value) {
	case IDC_RADIO_CAMERA1:
		retb = Scanner::changeScanSpeedToNormal();
		m_scanSpeed = ScanSpeed::Normal;

		CheckDlgButton(IDC_RADIO_BO1, false);
		CheckDlgButton(IDC_RADIO_BO2, false);
		CheckDlgButton(IDC_RADIO_BO3, true);
		updateRadioBlackOffset(IDC_RADIO_BO3);
		break;
	case IDC_RADIO_CAMERA2:
		retb = Scanner::changeScanSpeedToFaster();
		m_scanSpeed = ScanSpeed::Faster;

		CheckDlgButton(IDC_RADIO_BO1, true);
		CheckDlgButton(IDC_RADIO_BO2, false);
		CheckDlgButton(IDC_RADIO_BO3, false);
		updateRadioBlackOffset(IDC_RADIO_BO1);
		break;
	case IDC_RADIO_CAMERA3:
		retb = Scanner::changeScanSpeedToFastest();
		m_scanSpeed = ScanSpeed::Fastest;

		CheckDlgButton(IDC_RADIO_BO1, false);
		CheckDlgButton(IDC_RADIO_BO2, true);
		CheckDlgButton(IDC_RADIO_BO3, false);
		updateRadioBlackOffset(IDC_RADIO_BO2);
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


void DlgSpectrometerCalibration::updateRadioBlackOffset(UINT value)
{
	std::string offset;

	switch (value) {
	case IDC_RADIO_BO1:
		m_blackOffset = 0;
		break;
	case IDC_RADIO_BO2:
		m_blackOffset = 1;
		break;
	case IDC_RADIO_BO3:
		m_blackOffset = 2;
		break;
	}

	OnBnClickedButtonOffsetRead();
	return;
}



void DlgSpectrometerCalibration::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_CALIBRATE, btnCalibrate);
	DDX_Control(pDX, IDC_BUTTON_PEAK_ADD, btnPeakAdd);
	DDX_Control(pDX, IDC_BUTTON_PEAK_DEL, btnPeakDel);
	DDX_Control(pDX, IDC_BUTTON_START_SCAN, btnStartScan);
	DDX_Control(pDX, IDC_BUTTON_WAVE_ADD, btnWaveAdd);
	DDX_Control(pDX, IDC_BUTTON_WAVE_DEL, btnWaveDel);
	DDX_Control(pDX, IDC_BUTTON_WAVE_RESET, btnWaveReset);
	DDX_Control(pDX, IDC_COMBO_CLASS, cmbClass);
	//  DDX_Control(pDX, IDC_EDIT_PARAM5, editParam);
	DDX_Control(pDX, IDC_EDIT_THRESH, editThresh);
	DDX_Control(pDX, IDC_EDIT_THRESH2, editThresh2);
	DDX_Control(pDX, IDC_LIST_FBGS, lstFbgs);
	DDX_Control(pDX, IDC_LIST_PEAKS, lstPeaks);
	DDX_Control(pDX, IDC_LIST_WAVES, lstWaves);
	DDX_Control(pDX, IDC_SLIDER_THRESH, sldThresh);
	DDX_Control(pDX, IDC_SLIDER_THRESH2, sldThresh2);
	DDX_Control(pDX, IDC_BUTTON_PROFILE_ADD, btnProfileAdd);
	DDX_Control(pDX, IDC_BUTTON_PROFILE_DEL, btnProfileDel);
	DDX_Control(pDX, IDC_BUTTON_PROFILE_RESET, btnProfileReset);
	DDX_Control(pDX, IDC_COMBO_PROFILE, cmbProfile);
	DDX_Control(pDX, IDC_LIST_PROFILES, lstProfiles);
	DDX_Control(pDX, IDC_EDIT_PEAK, editPeak);
	DDX_Control(pDX, IDC_EDIT_WAVE, editWave);
	DDX_Control(pDX, IDC_EDIT_SPECT_PARAM1, editParam1);
	DDX_Control(pDX, IDC_EDIT_SPECT_PARAM2, editParam2);
	DDX_Control(pDX, IDC_EDIT_SPECT_PARAM3, editParam3);
	DDX_Control(pDX, IDC_EDIT_SPECT_PARAM4, editParam4);
	DDX_Control(pDX, IDC_EDIT_EXPOSURE, editExposure);
	DDX_Control(pDX, IDC_EDIT_RANGE_X1, editRangeX1);
	DDX_Control(pDX, IDC_EDIT_RANGE_X2, editRangeX2);
	DDX_Control(pDX, IDC_EDIT_RANGE_Y1, editRangeY1);
	DDX_Control(pDX, IDC_EDIT_RANGE_Y2, editRangeY2);
	DDX_Control(pDX, IDC_EDIT_BLACK_OFFSET, editBlackOffset);
	DDX_Control(pDX, IDC_EDIT_CENTER_X, editCenterX);
	DDX_Control(pDX, IDC_EDIT_HEAD_X1, editHeadX1);
	DDX_Control(pDX, IDC_EDIT_HEAD_X2, editHeadX2);
	DDX_Control(pDX, IDC_EDIT_HEAD_Y1, editHeadY1);
	DDX_Control(pDX, IDC_EDIT_HEAD_Y2, editHeadY2);
	DDX_Control(pDX, IDC_EDIT_TAIL_X1, editTailX1);
	DDX_Control(pDX, IDC_EDIT_TAIL_X2, editTailX2);
	DDX_Control(pDX, IDC_EDIT_TAIL_Y1, editTailY1);
	DDX_Control(pDX, IDC_EDIT_TAIL_Y2, editTailY2);
	DDX_Control(pDX, IDC_COMBO_LINE_CAM_AGAIN, cmbLineCamAgain);
}


BEGIN_MESSAGE_MAP(DlgSpectrometerCalibration, CDialogEx)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_THRESH, &DlgSpectrometerCalibration::OnReleasedcaptureSliderThresh)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_THRESH2, &DlgSpectrometerCalibration::OnReleasedcaptureSliderThresh2)
	ON_CBN_SELCHANGE(IDC_COMBO_CLASS, &DlgSpectrometerCalibration::OnSelchangeComboClass)
	ON_BN_CLICKED(IDC_BUTTON_CALIBRATE, &DlgSpectrometerCalibration::OnClickedButtonCalibrate)
	ON_BN_CLICKED(IDC_BUTTON_PEAK_ADD, &DlgSpectrometerCalibration::OnClickedButtonPeakAdd)
	ON_BN_CLICKED(IDC_BUTTON_PEAK_DEL, &DlgSpectrometerCalibration::OnClickedButtonPeakDel)
	ON_BN_CLICKED(IDC_BUTTON_START_SCAN, &DlgSpectrometerCalibration::OnClickedButtonStartScan)
	ON_BN_CLICKED(IDC_BUTTON_WAVE_ADD, &DlgSpectrometerCalibration::OnClickedButtonWaveAdd)
	ON_BN_CLICKED(IDC_BUTTON_WAVE_DEL, &DlgSpectrometerCalibration::OnClickedButtonWaveDel)
	ON_BN_CLICKED(IDC_BUTTON_WAVE_RESET, &DlgSpectrometerCalibration::OnClickedButtonWaveReset)
	ON_BN_CLICKED(IDC_BUTTON_PROFILE_ADD, &DlgSpectrometerCalibration::OnClickedButtonProfileAdd)
	ON_BN_CLICKED(IDC_BUTTON_PROFILE_DEL, &DlgSpectrometerCalibration::OnClickedButtonProfileDel)
	ON_BN_CLICKED(IDC_BUTTON_PROFILE_RESET, &DlgSpectrometerCalibration::OnClickedButtonProfileReset)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS, &DlgSpectrometerCalibration::OnBnClickedButtonFocus)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA2, &DlgSpectrometerCalibration::OnBnClickedButtonCamera2)
	ON_BN_CLICKED(IDC_BUTTON_MOTOR2, &DlgSpectrometerCalibration::OnBnClickedButtonMotor2)
	ON_BN_CLICKED(IDC_BUTTON_LEDS2, &DlgSpectrometerCalibration::OnBnClickedButtonLeds2)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_IMAGE, &DlgSpectrometerCalibration::OnBnClickedButtonSaveImage)
	ON_BN_CLICKED(IDC_BUTTON_EXIT2, &DlgSpectrometerCalibration::OnBnClickedButtonExit2)
	ON_BN_CLICKED(IDC_BUTTON_FIND_PEAKS, &DlgSpectrometerCalibration::OnBnClickedButtonFindPeaks)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_PARAMS, &DlgSpectrometerCalibration::OnBnClickedButtonLoadParams)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_PARAMS, &DlgSpectrometerCalibration::OnBnClickedButtonSaveParams)
	ON_BN_CLICKED(IDC_BUTTON_DRAW_LINES, &DlgSpectrometerCalibration::OnBnClickedButtonDrawLines)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_LINES, &DlgSpectrometerCalibration::OnBnClickedButtonClearLines)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_CAMERA1, IDC_RADIO_CAMERA3, &DlgSpectrometerCalibration::updateRadioCameraSpeed)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_BO1, IDC_RADIO_BO3, &DlgSpectrometerCalibration::updateRadioBlackOffset)
	ON_BN_CLICKED(IDC_BUTTON_EXPOSURE_APPLY, &DlgSpectrometerCalibration::OnBnClickedButtonExposureApply)
	ON_BN_CLICKED(IDC_CHECK_SHOW_TEXT, &DlgSpectrometerCalibration::OnBnClickedCheckShowText)
	ON_BN_CLICKED(IDC_CHECK_RECORD_VALUES, &DlgSpectrometerCalibration::OnBnClickedCheckRecordValues)
	ON_BN_CLICKED(IDC_BUTTON_RANGE_RESET, &DlgSpectrometerCalibration::OnBnClickedButtonRangeReset)
	ON_BN_CLICKED(IDC_BUTTON_RANGE_SIZE, &DlgSpectrometerCalibration::OnBnClickedButtonRangeSize)
	ON_BN_CLICKED(IDC_BUTTON_OFFSET_WRITE, &DlgSpectrometerCalibration::OnBnClickedButtonOffsetWrite)
	ON_BN_CLICKED(IDC_BUTTON_OFFSET_SAVE, &DlgSpectrometerCalibration::OnBnClickedButtonOffsetSave)
	ON_BN_CLICKED(IDC_BUTTON_OFFSET_READ, &DlgSpectrometerCalibration::OnBnClickedButtonOffsetRead)
	ON_BN_CLICKED(IDC_BUTTON_SLD, &DlgSpectrometerCalibration::OnBnClickedButtonSld)
	ON_BN_CLICKED(IDC_BUTTON_REFER_LOOP, &DlgSpectrometerCalibration::OnBnClickedButtonReferLoop)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RADIO_ORDER1, &DlgSpectrometerCalibration::OnBnClickedRadioOrder1)
	ON_BN_CLICKED(IDC_RADIO_ORDER2, &DlgSpectrometerCalibration::OnBnClickedRadioOrder2)
	ON_BN_CLICKED(IDC_BUTTON_POLAR_LOOP, &DlgSpectrometerCalibration::OnBnClickedButtonPolarLoop)
	ON_BN_CLICKED(IDC_BUTTON_CAL_RESET, &DlgSpectrometerCalibration::OnBnClickedButtonCalReset)
	ON_BN_CLICKED(IDC_BUTTON_ENDS_RESET, &DlgSpectrometerCalibration::OnBnClickedButtonEndsReset)
	ON_CBN_SELCHANGE(IDC_COMBO_LINE_CAM_AGAIN, &DlgSpectrometerCalibration::OnSelchangeComboLineCamAgain)
	ON_BN_CLICKED(IDC_BUTTON_HIGH_CODE, &DlgSpectrometerCalibration::OnBnClickedButtonHighCode)
END_MESSAGE_MAP()


// DlgSpectrometerCalibration message handlers


BOOL DlgSpectrometerCalibration::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL) {
			if (pFocused->GetDlgCtrlID() == editThresh.GetDlgCtrlID()) {
				updateThreshPeak(true);
			}
			else if (pFocused->GetDlgCtrlID() == editThresh2.GetDlgCtrlID()) {
				updateThreshSubs(true);
			}
			else if (pFocused->GetDlgCtrlID() == editRangeX1.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editRangeX2.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editRangeY1.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editRangeY2.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editCenterX.GetDlgCtrlID()) {
				updateGraphDisplay(false);
				profileView.updateWindow();
			}
			else if (pFocused->GetDlgCtrlID() == editHeadX1.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editHeadX2.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editHeadY1.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editHeadY2.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editTailX1.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editTailX2.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editTailY1.GetDlgCtrlID() ||
				pFocused->GetDlgCtrlID() == editTailY2.GetDlgCtrlID()) {
				updateGraphEndsDisplay(false);
				profileHeadView.updateWindow();
				profileTailView.updateWindow();
			}
		}
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


BOOL DlgSpectrometerCalibration::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialogEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


BOOL DlgSpectrometerCalibration::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	initControls();
	alignControls();
	controlCameras(true);

	OnClickedButtonProfileAdd();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void DlgSpectrometerCalibration::OnClose()
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
	CDialogEx::OnClose();
}


void DlgSpectrometerCalibration::OnReleasedcaptureSliderThresh(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editThresh.SetWindowTextW(to_wstring(sldThresh.GetPos()).c_str());
	updateThreshPeak(true);
	*pResult = 0;
}


void DlgSpectrometerCalibration::OnReleasedcaptureSliderThresh2(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editThresh2.SetWindowTextW(to_wstring(sldThresh2.GetPos()).c_str());
	updateThreshSubs(true);
	*pResult = 0;
}


void DlgSpectrometerCalibration::OnSelchangeComboClass()
{
	// TODO: Add your control notification handler code here
	CString text;
	int index = 1;

	int curSel = cmbClass.GetCurSel();
	auto vect = spectCalib.getFbgClass(curSel);

	lstFbgs.ResetContent();
	for (auto freq : vect) {
		text.Format(_T("[%02d] %s"), index, to_wstring(freq).c_str());
		lstFbgs.AddString(text);
		index++;
	}
	return;
}



void DlgSpectrometerCalibration::OnBnClickedButtonFindPeaks()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		OnClickedButtonStartScan();
	}

	CString text;
	int selIdx = lstProfiles.GetCurSel();
	if (selIdx < 0) {
		return;
	}

	lstProfiles.GetText(selIdx, text);
	int index = _ttoi(text);

	unsigned short* buff = profileView.getProfile(index);
	if (buff == nullptr) {
		return;
	}

	spectCalib.setThresholdOfPeak(profileView.getThresholdPeak());
	spectCalib.setThresholdOfSubs(profileView.getThresholdSubs());
	spectCalib.findPeaksOnSpectrum(buff, LINE_CAMERA_CCD_PIXELS);

	int peaks = spectCalib.getNumberOfPeaks();
	text.Format(_T("Peak positions %d found!"), peaks);
	AfxMessageBox(text);

	auto indexs = spectCalib.getPeakIndexs();
	auto values = spectCalib.getPeakValues();
	profileView.setPeakIndexs(indexs);
	profileView.setPeakValues(values);
	profileView.updateWindow();

	lstPeaks.ResetContent();
	for (int i = 0; i < indexs.size(); i++) {
		text.Format(_T("[%02d] %d"), i + 1, indexs[i]); // values[i]);
		lstPeaks.AddString(text);
	}

	return;
}


void DlgSpectrometerCalibration::OnClickedButtonCalibrate()
{
	// TODO: Add your control notification handler code here
	if (lstPeaks.GetCount() <= 0 || lstWaves.GetCount() <= 0 ||
		lstPeaks.GetCount() != lstWaves.GetCount()) {
		return;
	}

	vector<double>& waves = spectCalib.getWavelengths();
	waves.clear();

	CString text;
	for (int i = 0; i < lstWaves.GetCount(); i++) {
		lstWaves.GetText(i, text);
		waves.push_back((double)_ttof(text.Mid(5)));
	}

	vector<int>& peaks = spectCalib.getPeakIndexs();
	peaks.clear();

	for (int i = 0; i < lstPeaks.GetCount(); i++) {
		lstPeaks.GetText(i, text);
		peaks.push_back((int)_ttoi(text.Mid(5)));
	}

	if (spectCalib.calculateParameters(m_coeffsOrder)) {
		auto params = spectCalib.getParameters();
		
		// text.Format(_T("%e, %e, %e, %e"), params[0], params[1], params[2], params[3]);
		// editParam.SetWindowTextW(text);
		updateSpectroParams(&params[0]);
	}
	else {
		clearSpectroParams();
	}
	return;
}


void DlgSpectrometerCalibration::OnClickedButtonPeakAdd()
{
	// TODO: Add your control notification handler code here
	CString text;
	editPeak.GetWindowTextW(text);
	text.Trim();

	if (!text.IsEmpty())
	{
		vector<int> values;
		int added = _ttoi(text);
		values.push_back(added);

		CString item;
		int data;

		for (int i = 0; i < lstPeaks.GetCount(); i++) {
			lstPeaks.GetText(i, item);
			data = _ttoi(item.Mid(5));
			values.push_back(data);
		}
		sort(begin(values), end(values));

		lstPeaks.ResetContent();
		for (int i = 0; i < values.size(); i++) {
			item.Format(_T("[%02d] %d"), i + 1, values[i]);
			lstPeaks.AddString(item);
		}
	}
	return;
}


void DlgSpectrometerCalibration::OnClickedButtonPeakDel()
{
	// TODO: Add your control notification handler code here
	int index = lstPeaks.GetCurSel();
	if (index >= 0) {
		lstPeaks.DeleteString(index);

		vector<int> values;
		CString item;
		int data;

		for (int i = 0; i < lstPeaks.GetCount(); i++) {
			lstPeaks.GetText(i, item);
			data = _ttoi(item.Mid(5));
			values.push_back(data);
		}

		lstPeaks.ResetContent();
		for (int i = 0; i < values.size(); i++) {
			item.Format(_T("[%02d] %d"), i + 1, values[i]);
			lstPeaks.AddString(item);
		}
	}
	return;
}



void DlgSpectrometerCalibration::OnClickedButtonWaveAdd()
{
	// TODO: Add your control notification handler code here
	CString text;
	editWave.GetWindowTextW(text);
	text.Trim();

	if (!text.IsEmpty()) 
	{
		vector<double> values;
		double added = _ttof(text);
		values.push_back(added);

		CString item;
		double data ;
		
		for (int i = 0; i < lstWaves.GetCount(); i++) {
			lstWaves.GetText(i, item);
			data = _ttof(item.Mid(5));
			values.push_back(data);
		}
		sort(begin(values), end(values));

		lstWaves.ResetContent();
		for (int i = 0; i < values.size(); i++) {
			item.Format(_T("[%02d] %.2lf"), i+1, values[i]);
			lstWaves.AddString(item);
		}
	}
	return;
}


void DlgSpectrometerCalibration::OnClickedButtonWaveDel()
{
	// TODO: Add your control notification handler code here
	int index = lstWaves.GetCurSel();
	if (index >= 0) {
		lstWaves.DeleteString(index);

		vector<double> values;
		CString item;
		double data;

		for (int i = 0; i < lstWaves.GetCount(); i++) {
			lstWaves.GetText(i, item);
			data = _ttof(item.Mid(5));
			values.push_back(data);
		}

		lstWaves.ResetContent();
		for (int i = 0; i < values.size(); i++) {
			item.Format(_T("[%02d] %.2lf"), i + 1, values[i]);
			lstWaves.AddString(item);
		}
	}
	return;
}


void DlgSpectrometerCalibration::OnClickedButtonWaveReset()
{
	// TODO: Add your control notification handler code here
	lstWaves.ResetContent();
	CString text;
	for (int i = 0; i < lstFbgs.GetCount(); i++) {
		lstFbgs.GetText(i, text);
		lstWaves.AddString(text);
	}
	return;
}


void DlgSpectrometerCalibration::OnClickedButtonStartScan()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Controller::isScanning()) {
		Controller::closeScan(false);
		btnStartScan.SetWindowTextW(L"Start Scan");
	}
	else {
		profileView.clearPeaks();
		profileView.clearGraphInfo();
		updateGraphDisplay(false);
		updateGraphEndsDisplay(false);

		OctScanPattern pattern;
		pattern.setup(PatternName::CalibrationPoint, 512, 1, 0.0f, 0.0f); 
		pattern.setScanSpeed(m_scanSpeed);
		OctScanMeasure measure;
		measure.setup(EyeSide::OD, pattern);

		ChainSetup::useRefreshDispersionParams(true, false);

		CString text;
		editExposure.GetWindowTextW(text);
		if (!text.IsEmpty()) {
			float expTime = _ttof(text);
			GlobalMemory::setOctManualExposureTime(expTime);
		}

		Controller::setSpectrumDataCallback(&this->callback);
		if (Controller::startScan(measure, nullptr, nullptr, true)) {
			btnStartScan.SetWindowTextW(L"Capture");
		}
	}
	EndWaitCursor();
	return;
}


void DlgSpectrometerCalibration::callbackProfileImage(unsigned short * data, unsigned int width, unsigned int height)
{
	profileView.callbackProfileImage(data, width, height);
	profileHeadView.callbackProfileImage(data, width, height);
	profileTailView.callbackProfileImage(data, width, height);
}


void DlgSpectrometerCalibration::OnClickedButtonProfileAdd()
{
	// TODO: Add your control notification handler code here
	CString text;
	cmbProfile.GetLBText(cmbProfile.GetCurSel(), text);
	if (text.GetLength() > 0) {
		int index = lstProfiles.AddString(text);
		lstProfiles.SetCurSel(index);
		updateProfiles();
	}
	return;
}


void DlgSpectrometerCalibration::OnClickedButtonProfileDel()
{
	// TODO: Add your control notification handler code here
	int index = lstProfiles.GetCurSel();
	if (index >= 0) {
		lstProfiles.DeleteString(index);
		updateProfiles();
	}
	return;
}


void DlgSpectrometerCalibration::OnClickedButtonProfileReset()
{
	// TODO: Add your control notification handler code here
	lstProfiles.ResetContent();
	updateProfiles();
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonFocus()
{
	// TODO: Add your control notification handler code here
	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonCamera2()
{
	// TODO: Add your control notification handler code here
	if (pDlgCamera.get() == nullptr) {
		pDlgCamera = make_unique<DlgCameraControl>();
		pDlgCamera->Create(IDD_DLGCAMERACONTROL, this);
	}
	pDlgCamera->ShowWindow(SW_SHOW);
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonMotor2()
{
	// TODO: Add your control notification handler code here
	if (pDlgMotor.get() == nullptr) {
		pDlgMotor = make_unique<DlgMotorControl>();
		pDlgMotor->Create(IDD_DLGMOTORCONTROL, this);
	}
	pDlgMotor->ShowWindow(SW_SHOW);
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonLeds2()
{
	// TODO: Add your control notification handler code here
	if (!Controller::isScanning()) {
		OnClickedButtonStartScan();
	}

	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonSaveImage()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		OnClickedButtonStartScan();
	}

	
	BeginWaitCursor();
	CString text;
	CTime time = CTime::GetCurrentTime();
	CString dirname = _T(".//export");
	CreateDirectory(dirname, NULL);

	int wMain = profileView.getWidth();
	int hMain = profileView.getHeight();
	int wHead = profileHeadView.getWidth();
	int hHead = profileHeadView.getHeight();
	int wTail = profileTailView.getWidth();
	int hTail = profileTailView.getHeight();

	int width = wMain + max(wHead, wTail);
	int height = max(hMain, hHead + hTail);

	if (width > 0 && height > 0) {
		CImage image;
		if (SUCCEEDED(image.Create(width, height, 32))) {
			HDC hDC = image.GetDC();

			{
				CClientDC dcMain(&profileView);
				BitBlt(hDC, 0, 0, wMain, hMain, dcMain.GetSafeHdc(), 0, 0, SRCCOPY);
			}
			{
				CClientDC dcHead(&profileHeadView);
				BitBlt(hDC, wMain, 0, wHead, hHead, dcHead.GetSafeHdc(), 0, 0, SRCCOPY);
			}
			{
				CClientDC dcTail(&profileTailView);
				BitBlt(hDC, wMain, hHead, wTail, hTail, dcTail.GetSafeHdc(), 0, 0, SRCCOPY);
			}

			image.ReleaseDC();

			CString pathAll;
			CString baseName = time.Format(_T("profileView_%y%m%d_%H%M%S"));
			pathAll.Format(_T("%s//%s.png"), dirname, baseName);

			HRESULT hr = image.Save(pathAll);
			image.Destroy();

			if (hr == S_OK) {
				text.Format(_T("Image file: '%s' saved!"), pathAll);
				AfxMessageBox(text);
			}
		}
	}
	EndWaitCursor();
	
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonExit2()
{
	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonLoadParams()
{
	// TODO: Add your control notification handler code here
	CString text;
	double param[4] = { 0.0 };

	param[0] = SystemConfig::spectrometerParameter(0);
	param[1] = SystemConfig::spectrometerParameter(1);
	param[2] = SystemConfig::spectrometerParameter(2);
	param[3] = SystemConfig::spectrometerParameter(3);

	updateSpectroParams(param);
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonSaveParams()
{
	// TODO: Add your control notification handler code here
	CString text;
	double param[4] = { 0.0 };

	if (editParam1.GetWindowTextLengthW() <= 0 ||
		editParam2.GetWindowTextLengthW() <= 0 ||
		editParam3.GetWindowTextLengthW() <= 0 ||
		editParam4.GetWindowTextLengthW() <= 0) {
		return;
	}

	editParam1.GetWindowTextW(text);
	param[0] = _ttof(text);
	editParam2.GetWindowTextW(text);
	param[1] = _ttof(text);
	editParam3.GetWindowTextW(text);
	param[2] = _ttof(text);
	editParam4.GetWindowTextW(text);
	param[3] = _ttof(text);

	SystemConfig::spectrometerParameter(0, true, param[0]);
	SystemConfig::spectrometerParameter(1, true, param[1]);
	SystemConfig::spectrometerParameter(2, true, param[2]);
	SystemConfig::spectrometerParameter(3, true, param[3]);
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonDrawLines()
{
	// TODO: Add your control notification handler code here
	CString text;
	double param[4] = { 0.0 };

	editParam1.GetWindowTextW(text);
	param[0] = _ttof(text);
	editParam2.GetWindowTextW(text);
	param[1] = _ttof(text);
	editParam3.GetWindowTextW(text);
	param[2] = _ttof(text);
	editParam4.GetWindowTextW(text);
	param[3] = _ttof(text);

	double wlens[2048];
	double value;
	for (int idx = 0; idx < LINE_CAMERA_CCD_PIXELS; idx++) {
		value = param[0];
		value += (idx * param[1]);
		value += (pow(idx, 2.0) * param[2]);
		value += (pow(idx, 3.0) * param[3]);
		wlens[idx] = value;
	}

	vector<int> points;

	double wave;
	for (int i = 0, j = 0; i < lstWaves.GetCount(); i++) {
		lstWaves.GetText(i, text);
		wave = (double)_ttof(text.Mid(5));

		for (; j < LINE_CAMERA_CCD_PIXELS; j++) {
			if (wave <= wlens[j]) {
				points.push_back(j);
				break;
			}
		}
	}

	profileView.setCalibrationPoints(points);
	profileView.updateWindow();
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonClearLines()
{
	// TODO: Add your control notification handler code here
	profileView.clearCalibrationPoints();
	profileView.updateWindow();
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonExposureApply()
{
	// TODO: Add your control notification handler code here
	/*
	bool retb = Scanner::changeScanSpeedToCustom();
	m_scanSpeed = ScanSpeed::Custom;

	if (!retb) {
		AfxMessageBox(_T("Change scan speed failed!"));
	}
	*/

	/*
	CString text;
	float exp_time;

	GetDlgItemText(IDC_EDIT_EXPOSURE, text);
	if (!text.IsEmpty()) {
		exp_time = (float)_ttof(text);
		Scanner::setCameraExposureTime(exp_time);
	}
	*/
	return;
}


void DlgSpectrometerCalibration::OnBnClickedCheckShowText()
{
	// TODO: Add your control notification handler code here
	bool flag = (IsDlgButtonChecked(IDC_CHECK_SHOW_TEXT) ? true : false);
	profileView.setShowGraphInfo(flag);
	profileView.updateWindow();
	return;
}


void DlgSpectrometerCalibration::OnBnClickedCheckRecordValues()
{
	// TODO: Add your control notification handler code here
	bool flag = (IsDlgButtonChecked(IDC_CHECK_RECORD_VALUES) ? true : false);
	profileView.setRecordValues(flag);
	profileView.updateWindow();
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonRangeReset()
{
	// TODO: Add your control notification handler code here
	editRangeX1.SetWindowTextW(L"0");
	editRangeX2.SetWindowTextW(L"2047");
	editRangeY1.SetWindowTextW(L"0");
	editRangeY2.SetWindowTextW(L"4095");

	updateGraphDisplay(false);
	profileView.updateWindow();
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonRangeSize()
{
	// TODO: Add your control notification handler code here
	editRangeX1.SetWindowTextW(L"924");
	editRangeX2.SetWindowTextW(L"1124");
	editRangeY1.SetWindowTextW(L"0");
	editRangeY2.SetWindowTextW(L"4095");

	updateGraphDisplay(false);
	profileView.updateWindow();
	return;
}

void DlgSpectrometerCalibration::OnBnClickedButtonEndsReset()
{
	// TODO: Add your control notification handler code here
	editHeadX1.SetWindowTextW(L"0");
	editHeadX2.SetWindowTextW(L"7");
	editHeadY1.SetWindowTextW(L"0");
	editHeadY2.SetWindowTextW(L"128");
	editTailX1.SetWindowTextW(L"2040");
	editTailX2.SetWindowTextW(L"2047");
	editTailY1.SetWindowTextW(L"0");
	editTailY2.SetWindowTextW(L"128");

	updateGraphEndsDisplay(false);
	profileHeadView.updateWindow();
	profileTailView.updateWindow();
}



void DlgSpectrometerCalibration::OnBnClickedButtonOffsetWrite()
{
	// TODO: Add your control notification handler code here
	if (GlobalSettings::useUsbCmosCameraEnable()) {
		return;
	}

	if (!Controller::getMainBoard()->flashWriteLineCameraBlackOffset()) {
		AfxMessageBox(_T("Flash write line camera's black offset failed!"));
	}
	else {
		AfxMessageBox(_T("Flash write line camera's black offset ok!"));
	}
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonOffsetSave()
{
	// TODO: Add your control notification handler code here
	if (GlobalSettings::useUsbCmosCameraEnable()) {
		return;
	}

	CString str;
	editBlackOffset.GetWindowTextW(str);
	if (str.GetLength() > 0) {
		std::string offset = wtoa(str);
		if (!Controller::getMainBoard()->saveLineCameraBlackOffset(m_blackOffset, offset)) {
			AfxMessageBox(_T("Saving black offset failed!"));
			return;
		}
		OnBnClickedButtonOffsetRead();
	}
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonOffsetRead()
{
	// TODO: Add your control notification handler code here
	if (GlobalSettings::useUsbCmosCameraEnable()) {
		return;
	}

	std::string offset;

	if (!Controller::getMainBoard()->readLineCameraBlackOffset(m_blackOffset, offset)) {
		AfxMessageBox(_T("Reading black offset failed!"));
		return;
	}

	editBlackOffset.SetWindowTextW(atow(offset).c_str());
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonSld()
{
	// TODO: Add your control notification handler code here
	OctDevice::OctSldLed* led = Controller::getMainBoard()->getOctSldLed();
	static bool turnOn = true;
	if (turnOn) {
		led->lightOff();
		SetDlgItemText(IDC_BUTTON_SLD, _T("SLD On"));
	}
	else {
		led->lightOn();
		SetDlgItemText(IDC_BUTTON_SLD, _T("SLD Off"));
	}
	turnOn = !turnOn;
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonReferLoop()
{
	// TODO: Add your control notification handler code here
	const int TIMER_ID_REFERENCE_LOOP = 1001;
	const int TIMER_EVENT_DELAY = 100;

	auto referMotor = Controller::getMainBoard()->getReferenceMotor();

	if (runReferenceLoop) {
		KillTimer(TIMER_ID_REFERENCE_LOOP);
		BeginWaitCursor();
		Sleep(2000);
		EndWaitCursor();
		runReferenceLoop = false;
	}
	else {
		SetTimer(TIMER_ID_REFERENCE_LOOP, TIMER_EVENT_DELAY, NULL);
		runReferenceLoop = true;
		isMovingToUpper = true;
		referMotor->controlMove(referMotor->getRangeMax(), true);
	}
	return;
}


void DlgSpectrometerCalibration::OnTimer(UINT_PTR nIDEvent)
{
	auto referMotor = Controller::getMainBoard()->getReferenceMotor();
	auto polarMotor = Controller::getMainBoard()->getPolarizationMotor();

	// TODO: Add your message handler code here and/or call default
	if (runReferenceLoop) {
		referMotor->updateStatus();

		if (referMotor->isEndOfUpperPosition() && isMovingToUpper) {
			referMotor->controlMove(referMotor->getRangeMin(), true);
			isMovingToUpper = false;
		}
		if (referMotor->isEndOfLowerPosition() && !isMovingToUpper) {
			referMotor->controlMove(referMotor->getRangeMax(), true);
			isMovingToUpper = true;
		}
	}

	if (runPolarizationLoop) {
		polarMotor->updateStatus();

		if (polarMotor->isEndOfUpperPosition() && isMovingToUpperPolar) {
			polarMotor->controlMove(referMotor->getRangeMin(), true);
			isMovingToUpperPolar = false;
		}
		if (polarMotor->isEndOfLowerPosition() && !isMovingToUpperPolar) {
			polarMotor->controlMove(polarMotor->getRangeMax(), true);
			isMovingToUpperPolar = true;
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void DlgSpectrometerCalibration::OnBnClickedRadioOrder1()
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_RADIO_ORDER2, FALSE);
	m_coeffsOrder = 3;
}


void DlgSpectrometerCalibration::OnBnClickedRadioOrder2()
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_RADIO_ORDER1, FALSE);
	m_coeffsOrder = 4;
}


void DlgSpectrometerCalibration::OnBnClickedButtonPolarLoop()
{
	// TODO: Add your control notification handler code here
	const int TIMER_ID_POLARIZATION_LOOP = 1002;
	const int TIMER_EVENT_DELAY = 100;

	auto polarMotor = Controller::getMainBoard()->getPolarizationMotor();

	if (runPolarizationLoop) {
		KillTimer(TIMER_ID_POLARIZATION_LOOP);
		BeginWaitCursor();
		Sleep(2000);
		EndWaitCursor();
		runPolarizationLoop = false;
	}
	else {
		SetTimer(TIMER_ID_POLARIZATION_LOOP, TIMER_EVENT_DELAY, NULL);
		runPolarizationLoop = true;
		isMovingToUpperPolar = true;
		polarMotor->controlMove(polarMotor->getRangeMax(), true);
	}
	return;
}


void DlgSpectrometerCalibration::OnBnClickedButtonCalReset()
{
	editThresh.SetWindowTextW(_T("300"));
	editThresh2.SetWindowTextW(_T("150"));

	updateThreshPeak(true);
	updateThreshSubs(true);

	return;
}


void DlgSpectrometerCalibration::OnSelchangeComboLineCamAgain()
{
	int csel = cmbLineCamAgain.GetCurSel();
	float gain = (csel == 1 ? 2.0f : (csel == 2 ? 4.0f : 1.0f));
	bool flag = Scanner::setCameraAnalogGain(gain);
}


void DlgSpectrometerCalibration::OnBnClickedButtonHighCode()
{
	// TODO: Add your control notification handler code here
	if (!Controller::isScanning()) {
		OnClickedButtonStartScan();
	}

	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);

	pDlgLight->setHighCode100();
	return;
}
