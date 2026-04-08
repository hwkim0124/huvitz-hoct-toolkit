// DlgLightControl.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgLightControl.h"
#include "afxdialogex.h"

#include <math.h>


using namespace OctSystem;
using namespace OctDevice;
using namespace OctConfig;
using namespace std;

// DlgLightControl dialog

IMPLEMENT_DYNAMIC(DlgLightControl, CDialogEx)

DlgLightControl::DlgLightControl(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGLIGHTCONTROL, pParent)
{

}

DlgLightControl::~DlgLightControl()
{
}


void DlgLightControl::initControls(void)
{
	MainBoard* board = Controller::getMainBoard();
	flash = board->getFundusFlashLed();
	pannel = board->getPannelLed();
	wDots = board->getWorkingDotsLed();
	wDot2 = board->getWorkingDot2Led();
	split = board->getSplitFocusLed();
	octSld = board->getOctSldLed();
	extFix = board->getExtFixationLed();

	flash->setControls(&sldFlash, &editFlash);
	//pannel->setControls(&sldPannel, &editPannel);
	wDots->setControls(&sldWdots, &editWdots);
	wDot2->setControls(&sldWdots2, &editWdots2);
	split->setControls(&sldSplit, &editSplit);
	octSld->setControls(&sldSld, &editSld);
	extFix->setControls(&sldExtFix, &editExtFix);

	cmbRow.SetCurSel(0);
	cmbColumn.SetCurSel(0);

	CString text;
	text.Format(_T("%d"), GlobalSettings::fixationCenterOD().first);
	editLcdRow.SetWindowTextW(text);
	text.Format(_T("%d"), GlobalSettings::fixationCenterOD().second);
	editLcdCol.SetWindowTextW(text);

	text.Format(_T("%d"), octSld->highCode());
	editHighCode.SetWindowTextW(text);
	text.Format(_T("%d"), octSld->lowCode1());
	editLowCode1.SetWindowTextW(text);
	text.Format(_T("%d"), octSld->lowCode2());
	editLowCode2.SetWindowTextW(text);
	text.Format(_T("%d"), octSld->rsiCode());
	editRsiCode.SetWindowTextW(text);
	return;
}

void DlgLightControl::linkFlashToPannel(void)
{
	if (IsDlgButtonChecked(IDC_CHECK_FLASH_LINK)) {
		int value = flash->getIntensity();
		pannel->updateIntensity(value);
	}
	return;
}


void DlgLightControl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_INTFIX_COL, cmbColumn);
	DDX_Control(pDX, IDC_COMBO_INTFIX_ROW, cmbRow);
	DDX_Control(pDX, IDC_EDIT_EXTFIX, editExtFix);
	DDX_Control(pDX, IDC_EDIT_FLASH, editFlash);
	//DDX_Control(pDX, IDC_EDIT_PANNEL, editPannel);
	DDX_Control(pDX, IDC_EDIT_SLD, editSld);
	DDX_Control(pDX, IDC_EDIT_SPLIT, editSplit);
	DDX_Control(pDX, IDC_EDIT_WDOTS, editWdots);
	DDX_Control(pDX, IDC_SLIDER_EXTFIX, sldExtFix);
	DDX_Control(pDX, IDC_SLIDER_FLASH, sldFlash);
	//DDX_Control(pDX, IDC_SLIDER_PANNEL, sldPannel);
	DDX_Control(pDX, IDC_SLIDER_SLD, sldSld);
	DDX_Control(pDX, IDC_SLIDER_SPLIT, sldSplit);
	DDX_Control(pDX, IDC_SLIDER_WDOTS, sldWdots);
	DDX_Control(pDX, IDC_EDIT_HIGH_CODE, editHighCode);
	DDX_Control(pDX, IDC_EDIT_LOW_CODE1, editLowCode1);
	DDX_Control(pDX, IDC_EDIT_LOW_CODE2, editLowCode2);
	DDX_Control(pDX, IDC_EDIT_RSI_CODE, editRsiCode);
	DDX_Control(pDX, IDC_EDIT_WDOTS2, editWdots2);
	DDX_Control(pDX, IDC_SLIDER_WDOTS2, sldWdots2);
	DDX_Control(pDX, IDC_EDIT_LCD_ROW, editLcdRow);
	DDX_Control(pDX, IDC_EDIT_LCD_COL, editLcdCol);
}


BEGIN_MESSAGE_MAP(DlgLightControl, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_FLASH, &DlgLightControl::OnReleasedcaptureSliderFlash)
	//ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_PANNEL, &DlgLightControl::OnReleasedcaptureSliderPannel)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_WDOTS, &DlgLightControl::OnReleasedcaptureSliderWdots)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SPLIT, &DlgLightControl::OnReleasedcaptureSliderSplit)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SLD, &DlgLightControl::OnReleasedcaptureSliderSld)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_EXTFIX, &DlgLightControl::OnReleasedcaptureSliderExtfix)
	ON_BN_CLICKED(IDC_BUTTON_FLASH_STEP1, &DlgLightControl::OnBnClickedButtonFlashStep1)
	ON_BN_CLICKED(IDC_BUTTON_FLASH_STEP2, &DlgLightControl::OnBnClickedButtonFlashStep2)
	//ON_BN_CLICKED(IDC_BUTTON_PANNEL_STEP1, &DlgLightControl::OnBnClickedButtonPannelStep1)
	//ON_BN_CLICKED(IDC_BUTTON_PANNEL_STEP2, &DlgLightControl::OnBnClickedButtonPannelStep2)
	ON_BN_CLICKED(IDC_BUTTON_WDOTS_STEP1, &DlgLightControl::OnBnClickedButtonWdotsStep1)
	ON_BN_CLICKED(IDC_BUTTON_WDOTS_STEP2, &DlgLightControl::OnBnClickedButtonWdotsStep2)
	ON_BN_CLICKED(IDC_BUTTON_SPLIT_STEP1, &DlgLightControl::OnBnClickedButtonSplitStep1)
	ON_BN_CLICKED(IDC_BUTTON_SPLIT_STEP2, &DlgLightControl::OnBnClickedButtonSplitStep2)
	ON_BN_CLICKED(IDC_BUTTON_SLD_ON, &DlgLightControl::OnBnClickedButtonSldOn)
	ON_BN_CLICKED(IDC_BUTTON_SLD_OFF, &DlgLightControl::OnBnClickedButtonSldOff)
	ON_BN_CLICKED(IDC_BUTTON_EXTFIX_STEP1, &DlgLightControl::OnBnClickedButtonExtfixStep1)
	ON_BN_CLICKED(IDC_BUTTON_EXTFIX_STEP2, &DlgLightControl::OnBnClickedButtonExtfixStep2)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_ON, &DlgLightControl::OnBnClickedButtonIntfixOn)
	ON_BN_CLICKED(IDC_BUTTON_INTFIX_OFF, &DlgLightControl::OnBnClickedButtonIntfixOff)
	ON_BN_CLICKED(IDC_BUTTON_SLD_UPDATE, &DlgLightControl::OnBnClickedButtonSldUpdate)
	ON_BN_CLICKED(IDC_BUTTON_SLD_LOAD, &DlgLightControl::OnBnClickedButtonSldLoad)
	ON_BN_CLICKED(IDC_CHECK_STROBE_CONT, &DlgLightControl::OnBnClickedCheckStrobeCont)
	ON_BN_CLICKED(IDC_BUTTON_LCD_ON, &DlgLightControl::OnBnClickedButtonLcdOn)
	ON_BN_CLICKED(IDC_BUTTON_LCD_OFF, &DlgLightControl::OnBnClickedButtonLcdOff)
	ON_BN_CLICKED(IDC_BUTTON_LCD_PARAM, &DlgLightControl::OnBnClickedButtonLcdParam)
	ON_EN_CHANGE(IDC_EDIT_IMON, &DlgLightControl::OnEnChangeEditImon)
	ON_BN_CLICKED(IDC_BUTTON_WDOTS2_STEP1, &DlgLightControl::OnBnClickedButtonWdots2Step1)
	ON_BN_CLICKED(IDC_BUTTON_WDOTS2_STEP2, &DlgLightControl::OnBnClickedButtonWdots2Step2)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_WDOTS2, &DlgLightControl::OnReleasedcaptureSliderWdots2)
	ON_BN_CLICKED(IDC_BUTTON_SLD_CALC, &DlgLightControl::OnBnClickedButtonSldCalc)
	ON_BN_CLICKED(IDC_BUTTON_LCDFIX_ROW_STEP1, &DlgLightControl::OnBnClickedButtonLcdfixRowStep1)
	ON_BN_CLICKED(IDC_BUTTON_LCDFIX_ROW_STEP2, &DlgLightControl::OnBnClickedButtonLcdfixRowStep2)
	ON_BN_CLICKED(IDC_BUTTON_LCDFIX_COL_STEP1, &DlgLightControl::OnBnClickedButtonLcdfixColStep1)
	ON_BN_CLICKED(IDC_BUTTON_LCDFIX_COL_STEP2, &DlgLightControl::OnBnClickedButtonLcdfixColStep2)
	ON_BN_CLICKED(IDC_BUTTON_FLASH_UPDATE, &DlgLightControl::OnBnClickedButtonFlashUpdate)
END_MESSAGE_MAP()


// DlgLightControl message handlers


BOOL DlgLightControl::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgLightControl::PreTranslateMessage(MSG* pMsg)
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
			if (ctrlID == editFlash.GetDlgCtrlID()) {
				flash->updateIntensityByEdit();
				linkFlashToPannel();
			}
			else if (ctrlID == editWdots.GetDlgCtrlID()) {
				wDots->updateIntensityByEdit();
			}
			else if (ctrlID == editWdots2.GetDlgCtrlID()) {
				wDot2->updateIntensityByEdit();
			}
			else if (ctrlID == editSplit.GetDlgCtrlID()) {
				split->updateIntensityByEdit();
			}
			else if (ctrlID == editSld.GetDlgCtrlID()) {
				octSld->updateIntensityByEdit();
			}
			else if (ctrlID == editExtFix.GetDlgCtrlID()) {
				extFix->updateIntensityByEdit();
			}
			else if (ctrlID == editHighCode.GetDlgCtrlID()) {
				editHighCode.GetWindowTextW(text);
				octSld->setHighCode(_ttoi(text));
			}
			else if (ctrlID == editLowCode1.GetDlgCtrlID()) {
				editLowCode1.GetWindowTextW(text);
				octSld->setLowCode1(_ttoi(text));
			}
			else if (ctrlID == editLowCode2.GetDlgCtrlID()) {
				editLowCode2.GetWindowTextW(text);
				octSld->setLowCode2(_ttoi(text));
			}
			else if (ctrlID == editRsiCode.GetDlgCtrlID()) {
				editRsiCode.GetWindowTextW(text);
				octSld->setRsiCode(_ttoi(text));
			}
		}
		return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgLightControl::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	MainBoard* board = Controller::getMainBoard();
	flash = board->getFundusFlashLed();
	pannel = board->getPannelLed();
	wDots = board->getWorkingDotsLed();
	wDot2 = board->getWorkingDot2Led();
	split = board->getSplitFocusLed();
	octSld = board->getOctSldLed();
	extFix = board->getExtFixationLed();

	flash->setControls();
	//pannel->setControls(&sldPannel, &editPannel);
	wDots->setControls();
	wDot2->setControls();
	split->setControls();
	octSld->setControls();
	extFix->setControls();

	// To recycle the contents of the window after closing. 
	ShowWindow(SW_HIDE);

	// CDialogEx::OnClose();
}


void DlgLightControl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow) {
		initControls();

		resetHighCode();
	}
	return;
}


void DlgLightControl::OnReleasedcaptureSliderFlash(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	flash->updateIntensity(sldFlash.GetPos());
	linkFlashToPannel();
	*pResult = 0;
}


void DlgLightControl::OnReleasedcaptureSliderWdots(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	wDots->updateIntensity(sldWdots.GetPos());
	*pResult = 0;
}


void DlgLightControl::OnReleasedcaptureSliderWdots2(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	wDot2->updateIntensity(sldWdots2.GetPos());
	*pResult = 0;
}



void DlgLightControl::OnReleasedcaptureSliderSplit(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	split->updateIntensity(sldSplit.GetPos());
	*pResult = 0;
}


void DlgLightControl::OnReleasedcaptureSliderSld(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	octSld->updateIntensity(sldSld.GetPos());
	*pResult = 0;
}


void DlgLightControl::OnReleasedcaptureSliderExtfix(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	extFix->updateIntensity(sldExtFix.GetPos());
	*pResult = 0;
}


void DlgLightControl::OnBnClickedButtonFlashStep1()
{
	// TODO: Add your control notification handler code here
	flash->updateIntensityByOffset(-5);
}


void DlgLightControl::OnBnClickedButtonFlashStep2()
{
	// TODO: Add your control notification handler code here
	flash->updateIntensityByOffset(+5);
}


void DlgLightControl::OnBnClickedButtonWdotsStep1()
{
	// TODO: Add your control notification handler code here
	wDots->updateIntensityByOffset(-1);
}


void DlgLightControl::OnBnClickedButtonWdotsStep2()
{
	// TODO: Add your control notification handler code here
	wDots->updateIntensityByOffset(+1);
}


void DlgLightControl::OnBnClickedButtonWdots2Step1()
{
	// TODO: Add your control notification handler code here
	wDot2->updateIntensityByOffset(-1);
}


void DlgLightControl::OnBnClickedButtonWdots2Step2()
{
	// TODO: Add your control notification handler code here
	wDot2->updateIntensityByOffset(+1);
}



void DlgLightControl::OnBnClickedButtonSplitStep1()
{
	// TODO: Add your control notification handler code here
	split->updateIntensityByOffset(-5);
}


void DlgLightControl::OnBnClickedButtonSplitStep2()
{
	// TODO: Add your control notification handler code here
	split->updateIntensityByOffset(+5);
}


void DlgLightControl::OnBnClickedButtonSldOn()
{
	// TODO: Add your control notification handler code here
	octSld->lightOn();
}


void DlgLightControl::OnBnClickedButtonSldOff()
{
	// TODO: Add your control notification handler code here
	octSld->lightOff();
}


void DlgLightControl::OnBnClickedButtonExtfixStep1()
{
	// TODO: Add your control notification handler code here
	extFix->updateIntensityByOffset(-5);
}


void DlgLightControl::OnBnClickedButtonExtfixStep2()
{
	// TODO: Add your control notification handler code here
	extFix->updateIntensityByOffset(+5);
}


void DlgLightControl::OnBnClickedButtonIntfixOn()
{
	// TODO: Add your control notification handler code here
	int row = cmbRow.GetCurSel();
	int col = cmbColumn.GetCurSel();
	if (row >= 0 && col >= 0) {
		Controller::getMainBoard()->setInternalFixationOn(true, row, col);
	}
	return;
}


void DlgLightControl::OnBnClickedButtonIntfixOff()
{
	// TODO: Add your control notification handler code here
	Controller::getMainBoard()->setInternalFixationOn(false);

}


void DlgLightControl::OnBnClickedButtonLcdOn()
{
	// TODO: Add your control notification handler code here
	CString cstr;
	GetDlgItemText(IDC_EDIT_LCD_ROW, cstr);
	int row = _ttoi(cstr);
	row = min(max(row, 0), 159);
	GetDlgItemText(IDC_EDIT_LCD_COL, cstr);
	int col = _ttoi(cstr);
	col = min(max(col, 0), 127);
	Controller::getMainBoard()->setLcdFixationOn(true, row, col);
	return;
}


void DlgLightControl::OnBnClickedButtonLcdOff()
{
	Controller::getMainBoard()->setLcdFixationOn(false);
	return;
}


void DlgLightControl::OnBnClickedButtonLcdParam()
{
	std::uint8_t blink = (IsDlgButtonChecked(IDC_CHECK_LCD_BLINK) ? 1 : 0);

	CString cstr;
	GetDlgItemText(IDC_EDIT_LCD_BRIGHT, cstr);
	std::uint8_t bright = _ttoi(cstr);
	bright = min(max(bright, 0), 100);

	GetDlgItemText(IDC_EDIT_LCD_PERIOD, cstr);
	std::uint16_t period = _ttoi(cstr);
	period = max(period, 0);
	GetDlgItemText(IDC_EDIT_LCD_ONTIME, cstr);
	std::uint16_t ontime = _ttoi(cstr);
	ontime = max(ontime, 0);

	Controller::getMainBoard()->updateLcdParameters(bright, blink, period, ontime);
	return;
}



void DlgLightControl::OnBnClickedButtonSldUpdate()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	octSld->updateParametersToSldMemory();
	octSld->saveSystemParameters();
	EndWaitCursor();
	return;
}


void DlgLightControl::OnBnClickedButtonSldLoad()
{
	// TODO: Add your control notification handler code here
	CString text;

	editHighCode.GetWindowTextW(text);
	octSld->setHighCode(_ttoi(text));
	editLowCode1.GetWindowTextW(text);
	octSld->setLowCode1(_ttoi(text));
	editLowCode2.GetWindowTextW(text);
	octSld->setLowCode2(_ttoi(text));
	editRsiCode.GetWindowTextW(text);
	octSld->setRsiCode(_ttoi(text));
	return;
}


void DlgLightControl::OnBnClickedCheckStrobeCont()
{
	// TODO: Add your control notification handler code here
	if (GlobalSettings::useFundusFILR_Enable()) {
		MainBoard* board = Controller::getMainBoard();
		OctDevice::ColorCamera* camera = board->getColorCamera();

		if (IsDlgButtonChecked(IDC_CHECK_FLASH_CONT)) {
			camera->setDigitalIO(false);
		}
		else {
			camera->setDigitalIO(true);
		}
	}
	if (IsDlgButtonChecked(IDC_CHECK_FLASH_CONT)) {
		flash->setContinuousMode(true);
	}
	else {
		flash->setContinuousMode(false);
	}
	return;
}


void DlgLightControl::OnEnChangeEditImon()
{
	CString text;
	GetDlgItemText(IDC_EDIT_IMON, text);

	if (text.IsEmpty()) {
		return;
	}
	float imon = (float)_ttof(text);
	float high_max = (float)floor(6350 / imon - 3.27);
	int rmon_high = (int)(high_max - 1);
	int rmon_low = (int)(256000 / imon - 40.31488* rmon_high - 131.84);

	rmon_high = max(0, rmon_high);
	rmon_low = max(0, rmon_low);

	text.Format(_T("%d"), rmon_high);
	editHighCode.SetWindowTextW(text);
	text.Format(_T("%d"), rmon_low);
	editLowCode1.SetWindowTextW(text);
	text.Format(_T("%d"), rmon_low);
	editLowCode2.SetWindowTextW(text);
	return;
}

void DlgLightControl::OnBnClickedButtonSldCalc()
{
	// TODO: Add your control notification handler code here
	octSld->lightOn();
	Sleep(1000);

	OctDevice::UsbComm& usbComm = Controller::getMainBoard()->getUsbComm();

	auto current = 0.0f;
	for (int i = 0; i < 5; i++) {
		if (!usbComm.UpdateDeviceStatus(0)) {
			AfxMessageBox(_T("Update Device status failed!"));
			return;
		}

		SldSensorData status;
		if (!usbComm.SLD_ReadStatus(Controller::getMainBoard()->getBaseAddressOfGPStatus(), &status)) {
			AfxMessageBox(_T("Read SLD Sensor data failed!"));
			return;
		}
		current += status.SLD_current;
	}

	current = current / 5.0f;
	current += 10.0f;
	auto rsiCode = 0;

	if (current >= 180.00f) {
		rsiCode = 0;
	}
	else if (current >= 171.02f) {
		rsiCode = 0;
	}
	else if (current >= 162.90f) {
		rsiCode = 1;
	}
	else if (current >= 155.51f) {
		rsiCode = 2;
	}
	else if (current >= 148.76f) {
		rsiCode = 3;
	}
	else if (current >= 142.58f) {
		rsiCode = 4;
	}
	else if (current >= 136.89f) {
		rsiCode = 5;
	}
	else if (current >= 131.63f) {
		rsiCode = 6;
	}
	else if (current >= 126.77f) {
		rsiCode = 7;
	}
	else if (current >= 122.25f) {
		rsiCode = 8;
	}
	else if (current >= 118.04f) {
		rsiCode = 9;
	}
	else if (current >= 114.11f) {
		rsiCode = 10;
	}
	else if (current >= 110.43f) {
		rsiCode = 11;
	}
	else {
		rsiCode = 12;
	}

	CString text;
	text.Format(_T("%d"), rsiCode);
	SetDlgItemTextW(IDC_EDIT_RSI_CODE, text);
	return;
}


void DlgLightControl::blackSpotAlign()
{
	flash->updateIntensity(100);

	if (GlobalSettings::useFundusFILR_Enable()) {
		MainBoard* board = Controller::getMainBoard();
		OctDevice::ColorCamera* camera = board->getColorCamera();

		CheckDlgButton(IDC_CHECK_FLASH_CONT, true);

		if (IsDlgButtonChecked(IDC_CHECK_FLASH_CONT)) {
			camera->setDigitalIO(false);
		}
		else {
			camera->setDigitalIO(true);
		}
	}
	if (IsDlgButtonChecked(IDC_CHECK_FLASH_CONT)) {
		flash->setContinuousMode(true);
	}
	else {
		flash->setContinuousMode(false);
	}
	return;
}

void DlgLightControl::blackSpotCapture()
{
	flash->updateIntensity(100);

	if (GlobalSettings::useFundusFILR_Enable()) {
		MainBoard* board = Controller::getMainBoard();
		OctDevice::ColorCamera* camera = board->getColorCamera();

		CheckDlgButton(IDC_CHECK_FLASH_CONT, false);

		if (IsDlgButtonChecked(IDC_CHECK_FLASH_CONT)) {
			camera->setDigitalIO(false);
		}
		else {
			camera->setDigitalIO(true);
		}
	}
	if (IsDlgButtonChecked(IDC_CHECK_FLASH_CONT)) {
		flash->setContinuousMode(true);
	}
	else {
		flash->setContinuousMode(false);
	}
	return;
}


void DlgLightControl::ledAlign()
{
	flash->updateIntensity(0);
	extFix->updateIntensity(27);

	return;
}


void DlgLightControl::setHighCode100() 
{
	octSld->setHighCode(100);

	CString text; 
	text.Format(_T("%d"), octSld->highCode());
	editHighCode.SetWindowTextW(text);

	return;
}


void DlgLightControl::OnBnClickedButtonLcdfixRowStep1()
{
	CString text;

	editLcdRow.GetWindowTextW(text);
	int row = _ttoi(text) - 1;
	row = min(max(row, 0), 159);

	text.Format(_T("%d"), row);
	editLcdRow.SetWindowTextW(text);

	editLcdCol.GetWindowTextW(text);
	int col = _ttoi(text);
	col = min(max(col, 0), 127);

	Controller::getMainBoard()->setLcdFixationOn(true, row, col);

	return;
}


void DlgLightControl::OnBnClickedButtonLcdfixRowStep2()
{
	CString text;

	editLcdRow.GetWindowTextW(text);
	int row = _ttoi(text) + 1;
	row = min(max(row, 0), 159);

	text.Format(_T("%d"), row);
	editLcdRow.SetWindowTextW(text);

	editLcdCol.GetWindowTextW(text);
	int col = _ttoi(text);
	col = min(max(col, 0), 127);

	Controller::getMainBoard()->setLcdFixationOn(true, row, col);

	return;
}


void DlgLightControl::OnBnClickedButtonLcdfixColStep1()
{
	CString text;

	editLcdRow.GetWindowTextW(text);
	int row = _ttoi(text);
	row = min(max(row, 0), 159);

	editLcdCol.GetWindowTextW(text);
	int col = _ttoi(text) - 1;
	col = min(max(col, 0), 127);

	text.Format(_T("%d"), col);
	editLcdCol.SetWindowTextW(text);

	Controller::getMainBoard()->setLcdFixationOn(true, row, col);

	return;
}


void DlgLightControl::OnBnClickedButtonLcdfixColStep2()
{
	CString text;

	editLcdRow.GetWindowTextW(text);
	int row = _ttoi(text);
	row = min(max(row, 0), 159);

	editLcdCol.GetWindowTextW(text);
	int col = _ttoi(text) + 1;
	col = min(max(col, 0), 127);

	text.Format(_T("%d"), col);
	editLcdCol.SetWindowTextW(text);

	Controller::getMainBoard()->setLcdFixationOn(true, row, col);

	return;
}


void DlgLightControl::OnBnClickedButtonFlashUpdate()
{
	BeginWaitCursor();
	DeviceSettings* dset = SystemProfile::getDeviceSettings();
	FixationSettings* xset = SystemProfile::getFixationSettings();

	CString text;
	int value, row, col;

	if (editWdots.GetWindowTextLengthW() <= 0 ||
		editWdots2.GetWindowTextLengthW() <= 0 ||
		editSplit.GetWindowTextLengthW() <= 0 ||
		editLcdRow.GetWindowTextLengthW() <= 0 ||
		editLcdCol.GetWindowTextLengthW() <= 0) {
		return;
	}

	//working dot 1, 2
	editWdots.GetWindowTextW(text);
	value = min(max(_ttoi(text), 0), 100);
	dset->getWorkingDotsLed()->setValue(value);

	editWdots2.GetWindowTextW(text);
	value = min(max(_ttoi(text), 0), 100);
	dset->getWorkingDot2Led()->setValue(value);

	//split
	editSplit.GetWindowTextW(text);
	value = min(max(_ttoi(text), 0), 100);
	dset->getSplitFocusLed()->setValue(value);
	
	//Lcd Fix OD, OS
	editLcdRow.GetWindowTextW(text);
	row = min(max(_ttoi(text), 0), 199);
	editLcdCol.GetWindowTextW(text);
	col = min(max(_ttoi(text), 0), 199);
	xset->setCenterOD(std::pair<int, int>(row, col));
	xset->setCenterOS(std::pair<int, int>(row, col));

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


void DlgLightControl::resetHighCode()
{
	auto highcode = SystemConfig::sldParameterHighCode();

	CString text;
	text.Format(_T("%d"), highcode);
	editHighCode.SetWindowTextW(text);

	octSld->setHighCode(highcode);
	return;
}