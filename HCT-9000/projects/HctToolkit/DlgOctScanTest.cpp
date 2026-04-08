// DlgOctScanTest.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgOctScanTest.h"
#include "afxdialogex.h"

#include "OctSystem2.h"

using namespace OctSystem;
using namespace std;

// DlgOctScanTest dialog

IMPLEMENT_DYNAMIC(DlgOctScanTest, CDialogEx)

DlgOctScanTest::DlgOctScanTest(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGOCTSCANTEST, pParent)
{

}


DlgOctScanTest::~DlgOctScanTest()
{
}


void DlgOctScanTest::initControls(void)
{
	corneaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(15, 10, 15 + 320, 10 + 240), this);
	retinaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(15, 10 + 240 + 32, 15 + 320, 10 + 240 + 32 + 240), this);

	corneaView.setDisplaySize(320, 240);
	retinaView.setDisplaySize(320, 240);

	previewView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(350, 10, 350 + 1024, 10 + 512), this);
	previewView.setDisplaySize(1024, 512);

	cmbPattern.SetCurSel(0);
	cmbRange.SetCurSel(0);
	cmbPoints.SetCurSel(0);
	return;
}


void DlgOctScanTest::controlCameras(bool play)
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


void DlgOctScanTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PATTERN, cmbPattern);
	DDX_Control(pDX, IDC_COMBO_POINTS, cmbPoints);
	DDX_Control(pDX, IDC_COMBO_RANGE, cmbRange);
	DDX_Control(pDX, IDC_BUTTON_START_SCAN, btnStartScan);
}


BEGIN_MESSAGE_MAP(DlgOctScanTest, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_START_SCAN, &DlgOctScanTest::OnClickedButtonStartScan)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS, &DlgOctScanTest::OnBnClickedButtonFocus)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA, &DlgOctScanTest::OnBnClickedButtonCamera)
	ON_BN_CLICKED(IDC_BUTTON_MOTOR, &DlgOctScanTest::OnBnClickedButtonMotor)
	ON_BN_CLICKED(IDC_BUTTON_LIGHT, &DlgOctScanTest::OnBnClickedButtonLight)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &DlgOctScanTest::OnBnClickedButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_IMAGE, &DlgOctScanTest::OnBnClickedButtonSaveImage)
	ON_BN_CLICKED(IDC_BUTTON_MEASURE, &DlgOctScanTest::OnBnClickedButtonMeasure)
END_MESSAGE_MAP()


// DlgOctScanTest message handlers


BOOL DlgOctScanTest::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	initControls();
	controlCameras(true);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgOctScanTest::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgOctScanTest::OnClose()
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


void DlgOctScanTest::OnClickedButtonStartScan()
{
	// TODO: Add your control notification handler code here
	if (Controller::isScanning()) {
		Controller::closeScan(false);
		btnStartScan.SetWindowTextW(L"Start Scan");
	}
	else {
		Controller::setPreviewImageCallback(previewView.getCallbackFunction());
		if (Controller::startScan(EyeSide::OD, PatternDomain::Macular, PatternType::HorzLine, 
			1024, 1)) {
			btnStartScan.SetWindowTextW(L"Close Scan");
		}
	}
	return;
}


void DlgOctScanTest::OnBnClickedButtonMeasure()
{
	// TODO: Add your control notification handler code here
	OnClickedButtonStartScan();
}



void DlgOctScanTest::OnBnClickedButtonFocus()
{
	// TODO: Add your control notification handler code here
	if (pDlgFocus.get() == nullptr) {
		pDlgFocus = make_unique<DlgFocusControl>();
		pDlgFocus->Create(IDD_DLGFOCUSCONTROL, this);
	}
	pDlgFocus->ShowWindow(SW_SHOW);
	return;
}


void DlgOctScanTest::OnBnClickedButtonCamera()
{
	// TODO: Add your control notification handler code here
	if (pDlgCamera.get() == nullptr) {
		pDlgCamera = make_unique<DlgCameraControl>();
		pDlgCamera->Create(IDD_DLGCAMERACONTROL, this);
	}
	pDlgCamera->ShowWindow(SW_SHOW);
	return;
}


void DlgOctScanTest::OnBnClickedButtonMotor()
{
	// TODO: Add your control notification handler code here
	if (pDlgMotor.get() == nullptr) {
		pDlgMotor = make_unique<DlgMotorControl>();
		pDlgMotor->Create(IDD_DLGMOTORCONTROL, this);
	}
	pDlgMotor->ShowWindow(SW_SHOW);
	return;
}


void DlgOctScanTest::OnBnClickedButtonLight()
{
	// TODO: Add your control notification handler code here
	if (pDlgLight.get() == nullptr) {
		pDlgLight = make_unique<DlgLightControl>();
		pDlgLight->Create(IDD_DLGLIGHTCONTROL, this);
	}
	pDlgLight->ShowWindow(SW_SHOW);
	return;
}


void DlgOctScanTest::OnBnClickedButtonExit()
{
	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);
	return;
}


void DlgOctScanTest::OnBnClickedButtonSaveImage()
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

