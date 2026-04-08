// DlgMainboardTest.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgMainboardTest.h"
#include "afxdialogex.h"

#include "DlgKeratoCalibration.h"

// DlgMainboardTest dialog

IMPLEMENT_DYNAMIC(DlgMainboardTest, CDialogEx)

DlgMainboardTest::DlgMainboardTest(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGMAINBOARDTEST, pParent)
{

}

DlgMainboardTest::~DlgMainboardTest()
{
}


void DlgMainboardTest::initUsbLoopBack(void)
{
	editUsbDataSize.SetWindowTextW(_T("100"));
	editUsbRepeat.SetWindowTextW(_T("1"));
	editUsbStatus.SetWindowTextW(_T(""));

	_clbUsbLoopBackDownload = std::bind(&DlgMainboardTest::callbackUsbLoopBackDownload, this,
							std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, 
							std::placeholders::_4, std::placeholders::_5);
	_clbUsbLoopBackComplete = std::bind(&DlgMainboardTest::callbackUsbLoopBackComplete, this, std::placeholders::_1);
	
	UsbLoopBack::initialize(Controller::getMainBoard());
	return;
}


bool DlgMainboardTest::startUsbLoopBack(void)
{
	CString text;

	GetDlgItemText(IDC_USB_DATA_SIZE, text);
	int kbSize = max(min(_ttoi(text), 4096), 1);
	GetDlgItemText(IDC_USB_REPEAT, text);
	int repeat = _ttoi(text);

	bool res = UsbLoopBack::start(kbSize, repeat, &_clbUsbLoopBackDownload, &_clbUsbLoopBackComplete);
	return res ;
}


void DlgMainboardTest::cancelUsbLoopBack(void)
{
	UsbLoopBack::cancel(false);
	return;
}


void DlgMainboardTest::callbackUsbLoopBackDownload(unsigned int addr, int present, int block, int total, bool isError)
{
	CString text1, text2;

	text1.Format(_T("Address: %u, Block: %d/%d, Repeat: %d"), addr, block, total, present);
	text2.Format(_T(" ... %s"), (isError == false ? _T("Ok!") : _T("Error!")));
	text1 += text2;
	SetDlgItemText(IDC_USB_STATUS, text1);

	text1.Format(_T("%s"), UsbLoopBack::getBufferContext().c_str());
	SetDlgItemText(IDC_USB_BUFFER, text1);
	return;
}


void DlgMainboardTest::callbackUsbLoopBackComplete(bool success)
{
	CString text;

	text.Format(_T("%s"), (success ? _T("Loop Back Completed!") : _T("Loop Back Canceled!")));

	SetDlgItemText(IDC_USB_STATUS, text);
	SetDlgItemText(IDC_USB_LOOPBACK_START, _T("Start"));
	return;
}


void DlgMainboardTest::initPIStatus(void)
{
	SetDlgItemText(IDC_MPI1, _T(""));
	SetDlgItemText(IDC_MPI2, _T(""));
	SetDlgItemText(IDC_MPI3, _T(""));
	SetDlgItemText(IDC_MPI4, _T(""));

	SetDlgItemText(IDC_MPI5, _T(""));
	SetDlgItemText(IDC_MPI6, _T(""));
	SetDlgItemText(IDC_MPI7, _T(""));
	SetDlgItemText(IDC_MPI8, _T(""));

	SetDlgItemText(IDC_MPI9, _T(""));
	SetDlgItemText(IDC_MPI10, _T(""));
	SetDlgItemText(IDC_MPI11, _T(""));
	SetDlgItemText(IDC_MPI12, _T(""));

	SetDlgItemText(IDC_MPI13, _T(""));
	SetDlgItemText(IDC_MPI14, _T(""));
	SetDlgItemText(IDC_MPI15, _T(""));
	SetDlgItemText(IDC_MPI16, _T(""));
	return;
}


bool DlgMainboardTest::startPIStatusTest(void)
{
	SetTimer(1002, 500, NULL);
	return true;
}


void DlgMainboardTest::cancelPIStatusTest(void)
{
	KillTimer(1002);
	return;
}


bool DlgMainboardTest::reportPIStatusTest(void)
{
	OctDevice::MainBoard* board = Controller::getMainBoard();
	OctDevice::UsbComm& usbComm = board->getUsbComm();

	uint32_t status;
	if (usbComm.PI_ReadStatus(board->getBaseAddressOfGPStatus(), &status)) {
		updatePIStatus(status);
		return true;
	}
	else {
		// initPIStatus();
		// AfxMessageBox(_T("Read PI status failed!"));
		return false;
	}
}


void DlgMainboardTest::updatePIStatus(std::uint32_t status)
{
	SetDlgItemText(IDC_MPI1, (status & (0x01 << 0) ? _T("On") : _T("Off")));
	SetDlgItemText(IDC_MPI2, (status & (0x01 << 1) ? _T("On") : _T("Off")));
	SetDlgItemText(IDC_MPI3, (status & (0x01 << 2) ? _T("On") : _T("Off")));
	SetDlgItemText(IDC_MPI4, (status & (0x01 << 3) ? _T("On") : _T("Off")));

	SetDlgItemText(IDC_MPI5, (status & (0x01 << 4) ? _T("On") : _T("Off")));
	SetDlgItemText(IDC_MPI6, (status & (0x01 << 5) ? _T("On") : _T("Off")));
	SetDlgItemText(IDC_MPI7, (status & (0x01 << 6) ? _T("On") : _T("Off")));
	SetDlgItemText(IDC_MPI8, (status & (0x01 << 7) ? _T("On") : _T("Off")));

	SetDlgItemText(IDC_MPI9, (status & (0x01 << 8) ? _T("On") : _T("Off")));
	SetDlgItemText(IDC_MPI10, (status & (0x01 << 9) ? _T("On") : _T("Off")));
	SetDlgItemText(IDC_MPI11, (status & (0x01 << 10) ? _T("On") : _T("Off")));
	SetDlgItemText(IDC_MPI12, (status & (0x01 << 11) ? _T("On") : _T("Off")));

	SetDlgItemText(IDC_MPI13, (status & (0x01 << 12) ? _T("On") : _T("Off")));
	SetDlgItemText(IDC_MPI14, (status & (0x01 << 13) ? _T("On") : _T("Off")));
	SetDlgItemText(IDC_MPI15, (status & (0x01 << 14) ? _T("On") : _T("Off")));
	SetDlgItemText(IDC_MPI16, (status & (0x01 << 15) ? _T("On") : _T("Off")));
	return;
}


void DlgMainboardTest::initStepMotorTest(void)
{
	cmbStepMotor.SetCurSel(0);
	editStepRepeat.SetWindowTextW(_T("1"));
	editStepStatus.SetWindowTextW(_T(""));

	_clbStepMotorMoving = std::bind(&DlgMainboardTest::callbackStepMotorMoving, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	_clbStepMotorComplete = std::bind(&DlgMainboardTest::callbackStepMotorComplete, this, std::placeholders::_1);

	StepMotorTest::initialize(Controller::getMainBoard());
	return;
}


bool DlgMainboardTest::startStepMotorTest(void)
{
	CString text;
	GetDlgItemText(IDC_STEP_REPEAT, text);
	int repeat = _ttoi(text);

	int curSel = cmbStepMotor.GetCurSel();
	curSel = (curSel > 8 ? 10 : curSel);

	bool res = StepMotorTest::start(curSel, repeat, &_clbStepMotorMoving, &_clbStepMotorComplete);
	return res;
}


void DlgMainboardTest::cancelStepMotorTest(void)
{
	StepMotorTest::cancel(false);
	return;
}


void DlgMainboardTest::callbackStepMotorMoving(int stage, int present, int target, bool arrived)
{
	CString text1, text2;

	text1.Format(_T("Moving to %s, target: %d, repeat: %d"), (stage == 0 ? _T("Max") : _T("Min")),
		target, present);

	if (arrived) {
		text2 = _T(" ... Arrived");
		text1 += text2;
	}
	SetDlgItemText(IDC_STEP_STATUS, text1);
	return;
}


void DlgMainboardTest::callbackStepMotorComplete(bool success)
{
	CString text;

	text.Format(_T("%s"), (success ? _T("Step Motor Completed!") : _T("Step Motor Cancelled!")));

	SetDlgItemText(IDC_STEP_STATUS, text);
	SetDlgItemText(IDC_STEP_START, _T("Start"));
	return;
}


void DlgMainboardTest::initStageMotorTest(void)
{
	SetDlgItemText(IDC_STAGE_DUTY, _T("30"));
	SetDlgItemText(IDC_STAGE_STATUS, _T(""));

	CheckDlgButton(IDC_STAGE_X, TRUE);
	CheckDlgButton(IDC_STAGE_Z, FALSE);
	CheckDlgButton(IDC_STAGE_CW, TRUE);
	CheckDlgButton(IDC_STAGE_CCW, FALSE);

	_stageMoving = false;
	_isStageCw = true;
	_isStageX = true;
	return;
}


bool DlgMainboardTest::startStageMotorTest(void)
{
	if (_stageMoving) {
		return true;
	}

	StageMotorType type = (IsDlgButtonChecked(IDC_STAGE_X) ? StageMotorType::STAGE_X : StageMotorType::STAGE_Z);
	uint8_t dir = (IsDlgButtonChecked(IDC_STAGE_CW) ? 0 : 1);
	CString text;
	GetDlgItemText(IDC_STAGE_DUTY, text);
	uint8_t duty = (uint8_t)_ttoi(text);

	OctDevice::UsbComm& comm = Controller::getMainBoard()->getUsbComm();
	bool res = comm.StageMove(type, dir, duty);
	if (res) {
		_stageMoving = true;
		_stageType = type;
		SetTimer(1001, 500, NULL);
	}
	return res;
}


void DlgMainboardTest::cancelStageMotorTest(void)
{
	StageMotorType type = (IsDlgButtonChecked(IDC_STAGE_X) ? StageMotorType::STAGE_X : StageMotorType::STAGE_Z);
	
	OctDevice::UsbComm& comm = Controller::getMainBoard()->getUsbComm();
	bool res = comm.StageStop(type);
	_stageMoving = false;
	KillTimer(1001);
	return ;
}


void DlgMainboardTest::reportStageMotorTest(void)
{
	OctDevice::StageMotor* motor = Controller::getMainBoard()->getStageMotor(_stageType);
	if (motor) {
		if (motor->updateStatus()) {
			CString text;
			text.Format(_T("Motor Position: %d, Center: %d"), motor->getPosition(), motor->getCenterPosition());
			SetDlgItemText(IDC_STAGE_STATUS, text);
		}
	}
	return;
}


void DlgMainboardTest::initLcdFixationTest(void)
{
	editLcdBright.SetWindowTextW(_T("30"));
	editLcdBlink.SetWindowTextW(_T("1"));
	editLcdPeriod.SetWindowTextW(_T("2500"));
	editLcdOnTime.SetWindowTextW(_T("5000"));
	CheckDlgButton(IDC_LCD_CROSS, TRUE);
	CheckDlgButton(IDC_LCD_GREEN, FALSE);
	return;
}


bool DlgMainboardTest::updateLcdFixation(void)
{
	CString cstr;
	GetDlgItemText(IDC_LCD_BRIGHT, cstr);
	std::uint8_t bright = _ttoi(cstr);
	bright = min(max(bright, 0), 100);
	GetDlgItemText(IDC_LCD_BLINK, cstr);
	std::uint8_t blink = _ttoi(cstr);
	blink = max(blink, 0);
	GetDlgItemText(IDC_LCD_PERIOD, cstr);
	std::uint16_t period = _ttoi(cstr);
	period = max(period, 0);
	GetDlgItemText(IDC_LCD_ONTIME, cstr);
	std::uint16_t ontime = _ttoi(cstr);
	ontime = max(ontime, 0);

	std::uint8_t type = (IsDlgButtonChecked(IDC_LCD_CROSS) ? 0x00 : 0xFF);

	if (!Controller::getMainBoard()->updateLcdParameters(bright, blink, period, ontime, type)) {
		AfxMessageBox(_T("Update LCD Fixation parameters failed!"));
		return false;
	}
	return true;
}


void DlgMainboardTest::initSldInterfaceTest(void)
{
	editSldSld.SetWindowTextW(_T(""));
	editSldIpd.SetWindowTextW(_T(""));
	editSldEpd.SetWindowTextW(_T(""));
	editSldTemp.SetWindowTextW(_T(""));

	OnBnClickedSldOff();
	return;
}


bool DlgMainboardTest::updateSldInterface(void)
{
	OctDevice::UsbComm& usbComm = Controller::getMainBoard()->getUsbComm();

	if (!usbComm.UpdateDeviceStatus(0)) {
		AfxMessageBox(_T("Update Device status failed!"));
		return false;
	}

	SldSensorData status;
	if (!usbComm.SLD_ReadStatus(Controller::getMainBoard()->getBaseAddressOfGPStatus(), &status)) {
		AfxMessageBox(_T("Read SLD Sensor data failed!"));
		return false;
	}

	editSldSld.SetWindowTextW(std::to_wstring(status.SLD_current).c_str());
	editSldIpd.SetWindowTextW(std::to_wstring(status.IPD_current).c_str());
	editSldTemp.SetWindowTextW(std::to_wstring(status.temp).c_str());
	editSldEpd.SetWindowTextW(std::to_wstring(status.EPD_DN).c_str());
	return true;
}


void DlgMainboardTest::startSldInterfaceTest(void)
{
	SetTimer(1003, 500, NULL);
	return;
}


void DlgMainboardTest::cancelSldInterfaceTest(void)
{
	KillTimer(1003);
	return;
}


void DlgMainboardTest::initGalvanoScanTest(void)
{
	cmbScanPattern.SetCurSel(1);
	cmbScanRange.SetCurSel(3);
	editScanOffsetX.SetWindowTextW(_T(""));
	editScanOffsetY.SetWindowTextW(_T(""));
	return;
}


bool DlgMainboardTest::startGalvanoScan(void)
{
	int sel = cmbScanPattern.GetCurSel();
	auto name = (sel == 0 ? PatternName::CalibrationPoint : (sel == 1 ? PatternName::MacularLine : PatternName::MacularCross));

	sel = cmbScanRange.GetCurSel();
	float range = (sel == 0 ? 3.0f : (sel == 1 ? 6.0f : (sel == 2 ? 9.0f : 12.0f)));

	CString cstr;
	editScanOffsetX.GetWindowTextW(cstr);
	float offsetX = (float)(cstr.IsEmpty() ? 0.0f : _ttof(cstr));
	editScanOffsetY.GetWindowTextW(cstr);
	float offsetY = (float)(cstr.IsEmpty() ? 0.0f : _ttof(cstr));

	OctScanMeasure desc;
	desc.getPattern().setup(name, 1024, 1, range, range);
	desc.getPattern().setScanOffset(offsetX, offsetY, 0.0f, 1.0f, 1.0f, false);
	desc.usePreviewPattern(true, true);
	desc.useNoImageGrab(true, true);

	if (!Controller::startScan(desc, (OctGlobal::PreviewImageCallback*)nullptr)) {
		AfxMessageBox(_T("Can't start scanner!"));
		return false;
	}
	return true;
}


void DlgMainboardTest::cancelGalvanoScan(void)
{
	Controller::closeScan(false);
	return;
}


void DlgMainboardTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_USB_DATA_SIZE, editUsbDataSize);
	DDX_Control(pDX, IDC_USB_REPEAT, editUsbRepeat);
	DDX_Control(pDX, IDC_USB_STATUS, editUsbStatus);
	DDX_Control(pDX, IDC_COMBO_STEP_MOTOR, cmbStepMotor);
	//  DDX_Control(pDX, IDC_EDIT_STEP_REPEAT, editStepRepeat);
	DDX_Control(pDX, IDC_STEP_STATUS, editStepStatus);
	DDX_Control(pDX, IDC_STEP_REPEAT, editStepRepeat);
	DDX_Control(pDX, IDC_LCD_BLINK, editLcdBlink);
	DDX_Control(pDX, IDC_LCD_BRIGHT, editLcdBright);
	DDX_Control(pDX, IDC_LCD_ONTIME, editLcdOnTime);
	DDX_Control(pDX, IDC_LCD_PERIOD, editLcdPeriod);
	DDX_Control(pDX, IDC_SLD_EPD, editSldEpd);
	DDX_Control(pDX, IDC_SLD_IPD, editSldIpd);
	DDX_Control(pDX, IDC_SLD_SLD, editSldSld);
	DDX_Control(pDX, IDC_SLD_TEMP, editSldTemp);
	//  DDX_Control(pDX, IDC_SCAN_PATTERN, cmdScanPattern);
	//  DDX_Control(pDX, IDC_SCAN_RANGE, cmdScanRange);
	DDX_Control(pDX, IDC_SCAN_OFFSET_X, editScanOffsetX);
	DDX_Control(pDX, IDC_SCAN_OFFSET_Y, editScanOffsetY);
	DDX_Control(pDX, IDC_SCAN_PATTERN, cmbScanPattern);
	DDX_Control(pDX, IDC_SCAN_RANGE, cmbScanRange);
}


BEGIN_MESSAGE_MAP(DlgMainboardTest, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_USB_LOOPBACK_START, &DlgMainboardTest::OnBnClickedUsbLoopbackStart)
	ON_BN_CLICKED(IDC_MPI_START, &DlgMainboardTest::OnBnClickedMpiStart)
	ON_BN_CLICKED(IDC_USB_COPY, &DlgMainboardTest::OnBnClickedUsbCopy)
	ON_BN_CLICKED(IDC_STEP_START, &DlgMainboardTest::OnBnClickedStepStart)
	ON_BN_CLICKED(IDC_STAGE_START, &DlgMainboardTest::OnBnClickedStageStart)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_STAGE_X, &DlgMainboardTest::OnBnClickedStageX)
	ON_BN_CLICKED(IDC_STAGE_Z, &DlgMainboardTest::OnBnClickedStageZ)
	ON_BN_CLICKED(IDC_STAGE_CW, &DlgMainboardTest::OnBnClickedStageCw)
	ON_BN_CLICKED(IDC_STAGE_CCW, &DlgMainboardTest::OnBnClickedStageCcw)
	ON_BN_CLICKED(IDC_LCD_CROSS, &DlgMainboardTest::OnBnClickedLcdCross)
	ON_BN_CLICKED(IDC_LCD_GREEN, &DlgMainboardTest::OnBnClickedLcdGreen)
	ON_BN_CLICKED(IDC_LCD_UPDATE, &DlgMainboardTest::OnBnClickedLcdUpdate)
	ON_BN_CLICKED(IDC_SLD_ON, &DlgMainboardTest::OnBnClickedSldOn)
	ON_BN_CLICKED(IDC_SLD_OFF, &DlgMainboardTest::OnBnClickedSldOff)
	ON_BN_CLICKED(IDC_SLD_UPDATE, &DlgMainboardTest::OnBnClickedSldUpdate)
	ON_BN_CLICKED(IDC_CAMERA_OPEN, &DlgMainboardTest::OnBnClickedCameraOpen)
	ON_BN_CLICKED(IDC_SCAN_SLD, &DlgMainboardTest::OnBnClickedScanSld)
	ON_BN_CLICKED(IDC_SCAN_START, &DlgMainboardTest::OnBnClickedScanStart)
END_MESSAGE_MAP()


// DlgMainboardTest message handlers


void DlgMainboardTest::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	cancelUsbLoopBack();
	cancelPIStatusTest();
	cancelStepMotorTest();
	cancelStageMotorTest();
	cancelSldInterfaceTest();
	cancelGalvanoScan();

	CDialogEx::OnClose();
}


BOOL DlgMainboardTest::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialogEx::PreTranslateMessage(pMsg);
}


BOOL DlgMainboardTest::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	initUsbLoopBack();
	initPIStatus();
	initStepMotorTest();
	initStageMotorTest();
	initLcdFixationTest();
	initGalvanoScanTest();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void DlgMainboardTest::OnBnClickedUsbLoopbackStart()
{
	// TODO: Add your control notification handler code here
	if (UsbLoopBack::isRunning()) {
		cancelUsbLoopBack();
		SetDlgItemText(IDC_USB_LOOPBACK_START, _T("Start"));
	}
	else {
		startUsbLoopBack();
		SetDlgItemText(IDC_USB_LOOPBACK_START, _T("Cancel"));
	}
	return;
}


void CopyToClipboard(HWND owner, const std::wstring &w)
{
	if (OpenClipboard(owner))
	{
		HGLOBAL hgClipBuffer = nullptr;
		std::size_t sizeInWords = w.size() + 1;
		std::size_t sizeInBytes = sizeInWords * sizeof(wchar_t);
		hgClipBuffer = GlobalAlloc(GHND | GMEM_SHARE, sizeInBytes);
		if (!hgClipBuffer)
		{
			CloseClipboard();
			return;
		}
		wchar_t *wgClipBoardBuffer = static_cast<wchar_t*>(GlobalLock(hgClipBuffer));
		wcscpy_s(wgClipBoardBuffer, sizeInWords, w.c_str());
		GlobalUnlock(hgClipBuffer);
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, hgClipBuffer);
		CloseClipboard();
	}
	return;
}


void DlgMainboardTest::OnBnClickedUsbCopy()
{
	CString cstr;
	GetDlgItemText(IDC_USB_BUFFER, cstr);
	CopyToClipboard(::GetActiveWindow(), std::wstring(cstr));
	return;
}


void DlgMainboardTest::OnBnClickedMpiStart()
{
	// TODO: Add your control notification handler code here
	static bool start = false;

	if (!start) {
		startPIStatusTest();
		SetDlgItemText(IDC_MPI_START, _T("Cancel"));
	}
	else {
		cancelPIStatusTest();
		SetDlgItemText(IDC_MPI_START, _T("Start"));
	}
	start = !start;
	return;
}


void DlgMainboardTest::OnBnClickedStepStart()
{
	// TODO: Add your control notification handler code here
	if (StepMotorTest::isRunning()) {
		cancelStepMotorTest();
		SetDlgItemText(IDC_STEP_START, _T("Start"));
	}
	else {
		if (startStepMotorTest()) {
			SetDlgItemText(IDC_STEP_START, _T("Cancel"));
		}
	}
	return;
}


void DlgMainboardTest::OnBnClickedStageStart()
{
	// TODO: Add your control notification handler code here
	if (_stageMoving) {
		cancelStageMotorTest();
		SetDlgItemText(IDC_STAGE_START, _T("Start"));
		SetDlgItemText(IDC_STAGE_STATUS, _T("Stage Motor Cancelled!"));
	}
	else {
		if (startStageMotorTest()) {
			SetDlgItemText(IDC_STAGE_START, _T("Cancel"));
			SetDlgItemText(IDC_STAGE_STATUS, _T("Stage Motor Started!"));
		}
		else {
			SetDlgItemText(IDC_STAGE_STATUS, _T("Stage Motor Failed!"));
		}
	}
	return;
}


void DlgMainboardTest::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == 1001) {
		reportStageMotorTest();
	}
	else if (nIDEvent == 1002) {
		reportPIStatusTest();
	}
	else if (nIDEvent == 1003) {
		updateSldInterface();
	}

	CDialogEx::OnTimer(nIDEvent);
}


void DlgMainboardTest::OnBnClickedStageX()
{
	// TODO: Add your control notification handler code here
	_isStageX = true;
	CheckDlgButton(IDC_STAGE_X, _isStageX == true);
	CheckDlgButton(IDC_STAGE_Z, _isStageX != true);
	CheckDlgButton(IDC_STAGE_CW, _isStageCw == true);
	CheckDlgButton(IDC_STAGE_CCW, _isStageCw != true);
}


void DlgMainboardTest::OnBnClickedStageZ()
{
	// TODO: Add your control notification handler code here
	_isStageX = false;
	CheckDlgButton(IDC_STAGE_X, _isStageX == true);
	CheckDlgButton(IDC_STAGE_Z, _isStageX != true);
	CheckDlgButton(IDC_STAGE_CW, _isStageCw == true);
	CheckDlgButton(IDC_STAGE_CCW, _isStageCw != true);
}


void DlgMainboardTest::OnBnClickedStageCw()
{
	// TODO: Add your control notification handler code here
	_isStageCw = true;
	CheckDlgButton(IDC_STAGE_X, _isStageX == true);
	CheckDlgButton(IDC_STAGE_Z, _isStageX != true);
	CheckDlgButton(IDC_STAGE_CW, _isStageCw == true);
	CheckDlgButton(IDC_STAGE_CCW, _isStageCw != true);
}


void DlgMainboardTest::OnBnClickedStageCcw()
{
	// TODO: Add your control notification handler code here
	_isStageCw = false;
	CheckDlgButton(IDC_STAGE_X, _isStageX == true);
	CheckDlgButton(IDC_STAGE_Z, _isStageX != true);
	CheckDlgButton(IDC_STAGE_CW, _isStageCw == true);
	CheckDlgButton(IDC_STAGE_CCW, _isStageCw != true);
}


void DlgMainboardTest::OnBnClickedLcdCross()
{
	CheckDlgButton(IDC_LCD_GREEN, FALSE);
}


void DlgMainboardTest::OnBnClickedLcdGreen()
{
	CheckDlgButton(IDC_LCD_CROSS, FALSE);
}


void DlgMainboardTest::OnBnClickedLcdUpdate()
{
	updateLcdFixation();
	return;
}


void DlgMainboardTest::OnBnClickedSldOn()
{
	CheckDlgButton(IDC_SLD_ON, TRUE);
	CheckDlgButton(IDC_SLD_OFF, FALSE);
	Controller::getMainBoard()->getOctSldLed()->lightOn();

	if (!_turnOnSld) {
		OnBnClickedScanSld();
	}
	return;
}


void DlgMainboardTest::OnBnClickedSldOff()
{
	CheckDlgButton(IDC_SLD_ON, FALSE);
	CheckDlgButton(IDC_SLD_OFF, TRUE);
	Controller::getMainBoard()->getOctSldLed()->lightOff();

	if (_turnOnSld) {
		OnBnClickedScanSld();
	}
	return;
}


void DlgMainboardTest::OnBnClickedSldUpdate()
{
	static bool start = false;

	if (!start) {
		startSldInterfaceTest();
		SetDlgItemText(IDC_SLD_UPDATE, _T("Cancel"));
	}
	else {
		cancelSldInterfaceTest();
		SetDlgItemText(IDC_SLD_UPDATE, _T("Start"));
	}
	start = !start;
	return;
}


void DlgMainboardTest::OnBnClickedCameraOpen()
{
	// TODO: Add your control notification handler code here
	DlgKeratoCalibration dlg;
	dlg.DoModal();
	return;
}


void DlgMainboardTest::OnBnClickedCameraLed()
{
	DlgCameraControl dlg;
	dlg.DoModal();
	return;
}


void DlgMainboardTest::OnBnClickedScanSld()
{
	// TODO: Add your control notification handler code here
	OctSldLed* led = Controller::getMainBoard()->getOctSldLed();
	if (_turnOnSld) {
		led->lightOff();
		SetDlgItemText(IDC_SCAN_SLD, _T("SLD On"));
	}
	else {
		led->lightOn();
		SetDlgItemText(IDC_SCAN_SLD, _T("SLD Off"));
	}
	_turnOnSld = !_turnOnSld;
	return;
}


void DlgMainboardTest::OnBnClickedScanStart()
{
	if (Controller::isScanning()) {
		cancelGalvanoScan();
		SetDlgItemText(IDC_SCAN_START, _T("Start"));
	}
	else {
		if (startGalvanoScan()) {
			SetDlgItemText(IDC_SCAN_START, _T("Stop"));
			if (!_turnOnSld) {
				OnBnClickedScanSld();
			}
		}
	}
	return;
}

