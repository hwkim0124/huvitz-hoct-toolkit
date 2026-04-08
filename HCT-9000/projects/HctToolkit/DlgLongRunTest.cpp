// DlgLongRunTest.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgLongRunTest.h"
#include "afxdialogex.h"

#include "OctSystem2.h"
#include "CppUtil2.h"

using namespace OctSystem;
using namespace CppUtil;
using namespace std;

// DlgLongRunTest dialog

IMPLEMENT_DYNAMIC(DlgLongRunTest, CDialogEx)

DlgLongRunTest::DlgLongRunTest(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGLONGRUNTEST, pParent)
{

}


DlgLongRunTest::~DlgLongRunTest()
{
}


void DlgLongRunTest::initControls(void)
{
	corneaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 0, 320, 240), this);
	retinaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(0, 240, 320, 240 + 240), this);
	corneaView.setDisplaySize(320, 240);
	retinaView.setDisplaySize(320, 240);

	previewView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(320, 0, 320 + 512, 512), this);
	previewView.setDisplaySize(512, 512);

	colorView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(320 + 512, 0, 320 + 512 + 512, 512), this);
	colorView.setViewSize(512, 512);

	colorView.showAlignGuide(false);

	Controller::setColorCameraImageCallback(colorView.getImageCallbackFunction());
	Controller::setColorCameraFrameCallback(colorView.getFrameCallbackFunction());

	CheckDlgButton(IDC_CHECK_STAGE_X, TRUE);
	CheckDlgButton(IDC_CHECK_STAGE_Y, TRUE);
	CheckDlgButton(IDC_CHECK_STAGE_Z, TRUE);

	if (!GlobalSettings::useOctEnable()) {
		CheckDlgButton(IDC_CHECK_SCANNING, FALSE);
		CheckDlgButton(IDC_CHECK_OPTIMIZING, FALSE);
		GetDlgItem(IDC_CHECK_SCANNING)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_OPTIMIZING)->EnableWindow(FALSE);
	}
	else {
		CheckDlgButton(IDC_CHECK_SCANNING, TRUE);
		CheckDlgButton(IDC_CHECK_OPTIMIZING, TRUE);
	}
	CheckDlgButton(IDC_CHECK_CHINREST, TRUE);

	if (!GlobalSettings::useFundusEnable() && !GlobalSettings::useFundusFILR_Enable()) {
		CheckDlgButton(IDC_CHECK_COLOR_FUNDUS, FALSE);
		GetDlgItem(IDC_CHECK_COLOR_FUNDUS)->EnableWindow(FALSE);
	}
	else {
		CheckDlgButton(IDC_CHECK_COLOR_FUNDUS, TRUE);
	}
	editStopAfter.SetWindowTextW(_T("1000"));

	if (!GlobalSettings::useFundusEnable()) {
		if (!GlobalSettings::useFundusFILR_Enable()) {
			CheckDlgButton(IDC_CHECK_COLOR_FUNDUS, FALSE);
			GetDlgItem(IDC_CHECK_COLOR_FUNDUS)->EnableWindow(FALSE);
			m_useColorFundus = false;
		}
	}


	return;
}


void DlgLongRunTest::showIrCameras(bool play)
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
	Scanner::changeScanSpeedToFaster();
	EndWaitCursor();
	return;
}


void DlgLongRunTest::prepareSystem(void)
{
	Controller::turnOnInternalFixationAtCenter();

	pauseSystem();
	return;
}


void DlgLongRunTest::pauseSystem(void)
{
	auto xmotor = Controller::getMainBoard()->getStageXMotor();
	auto ymotor = Controller::getMainBoard()->getStageYMotor();
	auto zmotor = Controller::getMainBoard()->getStageZMotor();

	xmotor->controlStop();
	ymotor->controlStop();
	zmotor->controlStop();

	Controller::getMainBoard()->stopChinrest();

	if (Controller::isAutoOptimizing()) {
		Controller::cancelAutoOptimize();
	}
	Controller::closeScan(false);
	return;
}


void DlgLongRunTest::showStatus(CString text)
{
	CString status, info, tstr;

	tstr = m_startTime.Format("%Y-%m-%d %H:%M:%S");
	info.Format(_T("Running %d / %d since %s"), m_currCount, m_stopAfter, tstr);

	status = text;
	status += _T("\r\n");
	status += info;

	editStatus.SetWindowTextW(status);
	return;
}


void DlgLongRunTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Control(pDX, IDC_EDIT_CURR_COUNT, editCurrCount);
	DDX_Control(pDX, IDC_EDIT_STOP_AFTER, editStopAfter);
	DDX_Control(pDX, IDC_EDIT_STATUS, editStatus);
	DDX_Control(pDX, IDC_EDIT_DELAY_TIME, editDelayTime);
}


BEGIN_MESSAGE_MAP(DlgLongRunTest, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_START, &DlgLongRunTest::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &DlgLongRunTest::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_CHECK_STAGE_X, &DlgLongRunTest::OnBnClickedCheckStageX)
	ON_BN_CLICKED(IDC_CHECK_STAGE_Y, &DlgLongRunTest::OnBnClickedCheckStageY)
	ON_BN_CLICKED(IDC_CHECK_STAGE_Z, &DlgLongRunTest::OnBnClickedCheckStageZ)
	ON_BN_CLICKED(IDC_CHECK_SCANNING, &DlgLongRunTest::OnBnClickedCheckScanning)
	ON_BN_CLICKED(IDC_CHECK_OPTIMIZING, &DlgLongRunTest::OnBnClickedCheckOptimizing)
	ON_BN_CLICKED(IDC_CHECK_COLOR_FUNDUS, &DlgLongRunTest::OnBnClickedCheckColorFundus)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS, &DlgLongRunTest::OnBnClickedButtonFocus)
	ON_BN_CLICKED(IDC_BUTTON_MOTOR, &DlgLongRunTest::OnBnClickedButtonMotor)
	ON_BN_CLICKED(IDC_BUTTON_STAGE2, &DlgLongRunTest::OnBnClickedButtonStage2)
	ON_BN_CLICKED(IDC_BUTTON_LIGHT, &DlgLongRunTest::OnBnClickedButtonLight)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA, &DlgLongRunTest::OnBnClickedButtonCamera)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &DlgLongRunTest::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_CHECK_CHINREST, &DlgLongRunTest::OnBnClickedCheckChinrest)
END_MESSAGE_MAP()


// DlgLongRunTest message handlers


void DlgLongRunTest::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	showIrCameras(false);
	prepareSystem();

	CDialogEx::OnClose();
}


BOOL DlgLongRunTest::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	initControls();
	showIrCameras(true);
	prepareSystem();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgLongRunTest::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgLongRunTest::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	auto xmotor = Controller::getMainBoard()->getStageXMotor();
	auto ymotor = Controller::getMainBoard()->getStageYMotor();
	auto zmotor = Controller::getMainBoard()->getStageZMotor();	

	static int targetPos = 0;
	static int offsetPos = 0;
	static int tickCount = 0;
	static int longCount = 0;
	CString text;

	// if (++longCount % 10 == 0) {
		xmotor->updateStatus();
		ymotor->updateStatus();
		zmotor->updateStatus();
		longCount = 0;
	// }

	int posX = xmotor->getPosition();
	int posY = ymotor->getPosition();
	int posZ = zmotor->getPosition();

	switch (m_longRunStep) 
	{
		case 1:
		{
			showStatus(L"Starting Longrun test...");

			m_startPosX = xmotor->getPosition();
			m_startPosY = ymotor->getPosition();
			m_startPosZ = zmotor->getPosition();
			m_longRunStep = (m_useStageX ? 2 : 8);
			longCount = 0;
			tickCount = 0;
		}
		break;

		case 2:
		{
			targetPos = xmotor->getRangeMin();
			offsetPos = targetPos - posX;
			xmotor->updatePositionByOffset(offsetPos);
			// xmotor->updatePositionByDelta(offsetPos * 10);
			m_longRunStep = 3;
		}
		break;

		case 3:
		{
			text.Format(_T("Moving to left end ... %d/%d"), posX, targetPos);
			showStatus(text);

			if (xmotor->isEndOfLowerPosition() || ++tickCount > 30) {
				xmotor->controlStop();
				m_longRunStep = 4;		
				tickCount = 0;
			}
		}
		break;

		case 4:
		{
			if (++tickCount > 20) {
				targetPos = xmotor->getRangeMax();
				offsetPos = targetPos - posX ;
				xmotor->updatePositionByOffset(offsetPos);
				// xmotor->updatePositionByDelta(offsetPos * 10);
				m_longRunStep = 5;
				tickCount = 0;
			}
		}
		break;

		case 5:
		{
			text.Format(_T("Moving to right end ... %d/%d"), posX, targetPos);
			showStatus(text);

			if (xmotor->isEndOfUpperPosition() || ++tickCount > 30) {
				xmotor->controlStop();
				m_longRunStep = 6;
				tickCount = 0;
			}
		}
		break;

		case 6:
		{
			if (++tickCount > 20) {
				targetPos = m_startPosX;
				offsetPos = targetPos - posX;
				xmotor->updatePositionByOffset(offsetPos);
				// xmotor->updatePositionByDelta(offsetPos * 10);
				m_longRunStep = 7;
			}
		}
		break;

		case 7:
		{
			text.Format(_T("Moving to x started ... %d/%d"), posX, targetPos);
			showStatus(text);

			if (abs(xmotor->getPosition() - m_startPosX) <= 1 || ++tickCount > 30) {
				xmotor->controlStop();
				m_longRunStep = 8;
				tickCount = 0;
			}
		}
		break;

		case 8:
		{
			m_longRunStep = (m_useStageZ ? 8 : 14);

			if (++tickCount > 20) {
				targetPos = zmotor->getRangeMin();
				offsetPos = targetPos - posZ ;
				zmotor->updatePositionByOffset(offsetPos);
				// zmotor->updatePositionByDelta(offsetPos * 10);
				m_longRunStep = 9;
				tickCount = 0;
			}
		}
		break;

		case 9:
		{
			text.Format(_T("Moving to front end ... %d/%d"), posZ, targetPos);
			showStatus(text);

			if (zmotor->isEndOfLowerPosition() || ++tickCount > 30) {
				zmotor->controlStop();
				m_longRunStep = 10;
				tickCount = 0;
			}
		}
		break;

		case 10:
		{
			if (++tickCount > 20) {
				targetPos = zmotor->getRangeMax();
				offsetPos = targetPos - posZ ;
				zmotor->updatePositionByOffset(offsetPos);
				// zmotor->updatePositionByDelta(offsetPos*10);
				m_longRunStep = 11;
				tickCount = 0;
			}
		}
		break;

		case 11:
		{
			text.Format(_T("Moving to rear end ... %d/%d"), posZ, targetPos);
			showStatus(text);

			if (zmotor->isEndOfUpperPosition() || ++tickCount > 30) {
				zmotor->controlStop();
				m_longRunStep = 12;
				tickCount = 0;
			}
		}
		break;

		case 12:
		{
			if (++tickCount > 20) {
				targetPos = m_startPosZ;
				offsetPos = targetPos - posZ;
				zmotor->updatePositionByOffset(offsetPos);
				// zmotor->updatePositionByDelta(offsetPos * 10);
				m_longRunStep = 13;
				tickCount = 0;
			}
		}
		break;

		case 13:
		{
			text.Format(_T("Moving to z started ... %d/%d"), posZ, targetPos);
			showStatus(text);

			if (abs(zmotor->getPosition() - m_startPosZ) <= 1 || ++tickCount > 30) {
				zmotor->controlStop();
				m_longRunStep = 14;
				tickCount = 0;
			}
		}
		break;

		case 14:
		{
			m_longRunStep = (m_useStageY ? 14 : 20);

			if (++tickCount > 20) {
				targetPos = ymotor->getRangeMin();
				offsetPos = targetPos - posY;
				ymotor->updatePositionByOffset(offsetPos);
				m_longRunStep = 15;
				tickCount = 0;
			}
		}
		break;

		case 15:
		{
			text.Format(_T("Moving to lower end ... %d/%d"), posY, targetPos);
			showStatus(text);

			if (posY <= 0 || ymotor->isEndOfLowerPosition() || ++tickCount > 50) {
				ymotor->controlStop();
				m_longRunStep = 16;
				tickCount = 0;
			}
		}
		break;

		case 16:
		{
			if (++tickCount > 30) {
				targetPos = ymotor->getRangeMax();
				offsetPos = targetPos - posY;
				ymotor->updatePositionByOffset(offsetPos);
				m_longRunStep = 17;
				tickCount = 0;
			}
		}
		break;

		case 17:
		{
			text.Format(_T("Moving to upper end ... %d/%d"), posY, targetPos);
			showStatus(text);

			if (posY >= 48000 || ymotor->isEndOfUpperPosition() || ++tickCount > 50) {
				ymotor->controlStop();
				m_longRunStep = 18;
				tickCount = 0;
			}
		}
		break;

		case 18:
		{
			if (++tickCount > 30) {
				targetPos = m_startPosY;
				offsetPos = targetPos - posY;
				ymotor->updatePositionByOffset(offsetPos);
				m_longRunStep = 19;
				tickCount = 0;
			}
		}
		break;

		case 19:
		{
			text.Format(_T("Moving to y started ... %d/%d"), posY, targetPos);
			showStatus(text);

			if (abs(ymotor->getPosition() - m_startPosY) <= 100 || ++tickCount > 50) {
				ymotor->controlStop();
				m_longRunStep = 20;
				tickCount = 0;
			}
		}
		break;

		case 20:
		{
			m_longRunStep = (m_useScanning ? 20 : 21);

			if (!GlobalSettings::useOctEnable()) {
				m_longRunStep = 23;
				break;
			}
			if (++tickCount > 50) {
				showStatus(_T("Start scan preview..."));

				OctScanMeasure measure;
				measure.getPattern().setScanSpeed(ScanSpeed::Faster);
				measure.getPattern().setup(PatternName::MacularLine, 512, 1, 6.0f, 6.0f);
				Controller::startScan2(measure, previewView.getCallbackFunction());
				m_longRunStep = 21;
				tickCount = 0;
			}
		}
		break;

		case 21:
		{
			m_longRunStep = (m_useOptimizing ? 21 : 23);

			if (++tickCount > 30) {
				showStatus(_T("Start optimizing..."));

				Controller::startAutoOptimize(false);
				m_longRunStep = 22;
				tickCount = 0;
			}
		}
		break;

		case 22:
		{
			if (++tickCount > 30) {
				if (!Controller::isAutoOptimizing()) {
					showStatus(_T("Optimization completed"));
					m_longRunStep = 23;
					tickCount = 0;

					if (Controller::isScanning()) {
						Controller::closeScan(false);
					}
				}
			}
		}
		break;

		case 23:
		{
			m_longRunStep = (m_useColorFundus ? 23 : 24);

			if (!GlobalSettings::useFundusEnable() && !GlobalSettings::useFundusFILR_Enable()) {
				m_longRunStep = 24;
				break;
			}
			if (++tickCount > 30) {
				showStatus(_T("Taking color fundus photo..."));

				OctFundusMeasure measure;
				Controller::takeColorFundus(measure, nullptr, nullptr,
					colorView.getImageCallbackFunction(),
					colorView.getFrameCallbackFunction(),
					false);
				m_longRunStep = 24;
				tickCount = 0;
			}
		}
		break;

		case 24:
		{
			m_longRunStep = (m_useChinrest ? 24 : 30);

			if (++tickCount > 30) {
				showStatus(_T("Chinrest moving up..."));

				Controller::getMainBoard()->moveChinrestUp();
				m_longRunStep = 25;
				tickCount = 0;
			}
		}
		break;

		case 25:
		{
			if (tickCount++ < 200) {
				if (Controller::getMainBoard()->isChinrestAtHighLimit()) {
					Controller::getMainBoard()->stopChinrest();

					showStatus(_T("Chinrest at upper limit"));
					m_longRunStep = 26;
					tickCount = 0;
				}
			}
			else {
				showStatus(_T("Chinrest moving up failed!"));
				OnBnClickedButtonPause();
			}
		}
		break;

		case 26:
		{
			if (++tickCount > 20) {
				showStatus(_T("Chinrest moving down..."));

				Controller::getMainBoard()->moveChinrestDown();
				m_longRunStep = 27;
				tickCount = 0;
			}
		}
		break;

		case 27:
		{
			if (tickCount++ < 200) {
				if (Controller::getMainBoard()->isChinrestAtLowLimit()) {
					Controller::getMainBoard()->stopChinrest();

					showStatus(_T("Chinrest at lower limit"));
					m_longRunStep = 28;
					tickCount = 0;
				}
			}
			else {
				showStatus(_T("Chinrest moving down failed!"));
				OnBnClickedButtonPause();
			}
		}
		break;

		case 28:
		{
			if (++tickCount > 20) {
				showStatus(_T("Chinrest moving center..."));

				Controller::getMainBoard()->moveChinrestUp();
				m_longRunStep = 29;
				tickCount = 0;
			}
		}
		break;

		case 29:
		{
			if (++tickCount > 50) {
				showStatus(_T("Chinrest at center"));

				Controller::getMainBoard()->stopChinrest();
				m_longRunStep = 30;
				tickCount = 0;
			}
		}
		break;

		case 30:
		{
			showStatus(_T("Longrun operations completed"));

			if (++tickCount > 25) {
				if (++m_currCount > m_stopAfter) {
					OnBnClickedButtonPause();
				}
				else {
					m_longRunStep = 1;
					tickCount = 0;
				}
			}
		}
		break;

		default:
			break;
	};

	CDialogEx::OnTimer(nIDEvent);
}


void DlgLongRunTest::OnBnClickedButtonStart()
{
	// TODO: Add your control notification handler code here
	CString text;
	editStopAfter.GetWindowTextW(text);
	m_stopAfter = max(_ttoi(text), 1);
	m_currCount = 1;
	m_startTime = CTime::GetCurrentTime();
	m_longRunTest = true;

	prepareSystem();

	m_longRunStep = 1;
	SetTimer(1, 100, NULL);
	return;
}


void DlgLongRunTest::OnBnClickedButtonPause()
{
	// TODO: Add your control notification handler code here
	if (m_longRunTest) {
		KillTimer(1);
		m_longRunTest = false;

		pauseSystem();
	}
	else {
		SetTimer(1, 100, NULL);
		m_longRunTest = true;
	}
	return;
}


void DlgLongRunTest::OnBnClickedCheckStageX()
{
	// TODO: Add your control notification handler code here
	m_useStageX = IsDlgButtonChecked(IDC_CHECK_STAGE_X) ? false : true;
	CheckDlgButton(IDC_CHECK_STAGE_X, m_useStageX);
}


void DlgLongRunTest::OnBnClickedCheckStageY()
{
	// TODO: Add your control notification handler code here
	m_useStageY = IsDlgButtonChecked(IDC_CHECK_STAGE_Y) ? false : true;
	CheckDlgButton(IDC_CHECK_STAGE_Y, m_useStageY);
}


void DlgLongRunTest::OnBnClickedCheckStageZ()
{
	// TODO: Add your control notification handler code here
	m_useStageZ = IsDlgButtonChecked(IDC_CHECK_STAGE_Z) ? false : true;
	CheckDlgButton(IDC_CHECK_STAGE_Z, m_useStageZ);
}


void DlgLongRunTest::OnBnClickedCheckScanning()
{
	// TODO: Add your control notification handler code here
	m_useScanning = IsDlgButtonChecked(IDC_CHECK_SCANNING) ? false : true;
	CheckDlgButton(IDC_CHECK_SCANNING, m_useScanning);
}


void DlgLongRunTest::OnBnClickedCheckOptimizing()
{
	// TODO: Add your control notification handler code here
	m_useOptimizing = IsDlgButtonChecked(IDC_CHECK_OPTIMIZING) ? false : true;
	CheckDlgButton(IDC_CHECK_OPTIMIZING, m_useOptimizing);
}


void DlgLongRunTest::OnBnClickedCheckColorFundus()
{
	// TODO: Add your control notification handler code here
	m_useColorFundus = IsDlgButtonChecked(IDC_CHECK_COLOR_FUNDUS) ? false : true;
	CheckDlgButton(IDC_CHECK_COLOR_FUNDUS, m_useColorFundus);
}


void DlgLongRunTest::OnBnClickedCheckChinrest()
{
	// TODO: Add your control notification handler code here
	m_useChinrest = IsDlgButtonChecked(IDC_CHECK_CHINREST) ? false : true;
	CheckDlgButton(IDC_CHECK_CHINREST, m_useChinrest);
}



void DlgLongRunTest::OnBnClickedButtonFocus()
{
	// TODO: Add your control notification handler code here
	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);
	return;
}


void DlgLongRunTest::OnBnClickedButtonMotor()
{
	// TODO: Add your control notification handler code here
	if (pDlgMotor.get() == nullptr) {
		pDlgMotor = make_unique<DlgMotorControl>();
		pDlgMotor->Create(IDD_DLGMOTORCONTROL, this);
	}
	pDlgMotor->ShowWindow(SW_SHOW);
	return;
}


void DlgLongRunTest::OnBnClickedButtonStage2()
{
	// TODO: Add your control notification handler code here
	if (pDlgStage.get() == nullptr) {
		pDlgStage = make_unique<DlgStageControl>();
		pDlgStage->Create(IDD_DLGSTAGECONTROL, this);
	}
	pDlgStage->ShowWindow(SW_SHOW);
	return;
}


void DlgLongRunTest::OnBnClickedButtonLight()
{
	// TODO: Add your control notification handler code here
	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);
	return;
}


void DlgLongRunTest::OnBnClickedButtonCamera()
{
	// TODO: Add your control notification handler code here
	if (pDlgCamera.get() == nullptr) {
		pDlgCamera = make_unique<DlgCameraControl>();
		pDlgCamera->Create(IDD_DLGCAMERACONTROL, this);
	}
	pDlgCamera->ShowWindow(SW_SHOW);
	return;
}


void DlgLongRunTest::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);
	return;
}


