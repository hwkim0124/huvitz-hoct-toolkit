// DlgIrCameraTest.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgIrCameraTest.h"
#include "afxdialogex.h"

#include "OctSystem2.h"

using namespace OctSystem;
using namespace std;

// DlgIrCameraTest dialog

IMPLEMENT_DYNAMIC(DlgIrCameraTest, CDialogEx)

DlgIrCameraTest::DlgIrCameraTest(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGIRCAMERATEST, pParent)
{

}

DlgIrCameraTest::~DlgIrCameraTest()
{
}

void DlgIrCameraTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Control(pDX, IDC_EDIT_STATUS, m_editStatus);
	DDX_Control(pDX, IDC_BUTTON_START_CORNEA, btnStartCornea);
	DDX_Control(pDX, IDC_BUTTON_START_RETINA, btnStartRetina);
	DDX_Control(pDX, IDC_CHECK_OVERLAY_CORNEA, chkOverlayCornea);
	DDX_Control(pDX, IDC_CHECK_OVERLAY_RETINA, chkOverlayRetina);
}


BEGIN_MESSAGE_MAP(DlgIrCameraTest, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_START_CORNEA, &DlgIrCameraTest::OnClickedButtonStartCornea)
	ON_BN_CLICKED(IDC_BUTTON_START_RETINA, &DlgIrCameraTest::OnClickedButtonStartRetina)
	ON_BN_CLICKED(IDC_CHECK_OVERLAY_CORNEA, &DlgIrCameraTest::OnClickedCheckOverlayCornea)
	ON_BN_CLICKED(IDC_CHECK_OVERLAY_RETINA, &DlgIrCameraTest::OnClickedCheckOverlayRetina)
	ON_BN_CLICKED(IDC_BUTTON_LEDS, &DlgIrCameraTest::OnClickedButtonLeds)
	ON_BN_CLICKED(IDC_BUTTON_MOTORS, &DlgIrCameraTest::OnClickedButtonMotors)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_CONFIG, &DlgIrCameraTest::OnClickedButtonLoadConfig)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CONFIG, &DlgIrCameraTest::OnClickedButtonSaveConfig)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// DlgIrCameraTest message handlers


BOOL DlgIrCameraTest::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	// Create CStatic control.
	// https://msdn.microsoft.com/en-us/library/kaw7w663.aspx
	corneaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(15, 10, 15+640, 10+480), this);
	retinaView.Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, CRect(680, 10, 680+640, 10+480), this);

	using namespace OctSystem;
	Controller::getMainBoard()->getCorneaIrCamera()->setCallback(corneaView.getCallbackFunction());
	Controller::getMainBoard()->getRetinaIrCamera()->setCallback(retinaView.getCallbackFunction());

	chkOverlayCornea.SetCheck(TRUE);
	chkOverlayRetina.SetCheck(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void DlgIrCameraTest::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class


	CDialogEx::OnOK();
}


void DlgIrCameraTest::OnClickedButtonStartCornea()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	using namespace OctSystem;
	if (Controller::getMainBoard()->getCorneaIrCamera()->isPlaying()) {
		Controller::getMainBoard()->getCorneaIrCamera()->pause();
		btnStartCornea.SetWindowTextW(L"Start");
	}
	else {
		Controller::getMainBoard()->getCorneaIrCamera()->play();
		btnStartCornea.SetWindowTextW(L"Stop");
	}
	EndWaitCursor();
}


void DlgIrCameraTest::OnClickedButtonStartRetina()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	using namespace OctSystem;
	if (Controller::getMainBoard()->getRetinaIrCamera()->isPlaying()) {
		Controller::getMainBoard()->getRetinaIrCamera()->pause();
		btnStartRetina.SetWindowTextW(L"Start");
	}
	else {
		Controller::getMainBoard()->getRetinaIrCamera()->play();
		btnStartRetina.SetWindowTextW(L"Stop");
	}
	EndWaitCursor();
}


void DlgIrCameraTest::OnClickedCheckOverlayCornea()
{
	// TODO: Add your control notification handler code here
	if (chkOverlayCornea.GetCheck() == BST_CHECKED) {
		chkOverlayCornea.SetCheck(0);
		corneaView.setOverlay(false);
	}
	else {
		chkOverlayCornea.SetCheck(1);
		corneaView.setOverlay(true);
	}
	return;
}


void DlgIrCameraTest::OnClickedCheckOverlayRetina()
{
	// TODO: Add your control notification handler code here
	if (chkOverlayRetina.GetCheck() == BST_CHECKED) {
		chkOverlayRetina.SetCheck(0);
		retinaView.setOverlay(false);
	}
	else {
		chkOverlayRetina.SetCheck(1);
		retinaView.setOverlay(true);
	}
	return;
}


void DlgIrCameraTest::OnClickedButtonLeds()
{
	// TODO: Add your control notification handler code here
	if (pDlgLedsCtrl.get() == nullptr) {
		pDlgLedsCtrl = make_unique<DlgLedsControl>();
		pDlgLedsCtrl->Create(IDD_DLGLEDSCONTROL, this);
	}
	pDlgLedsCtrl->ShowWindow(SW_SHOW);
}


void DlgIrCameraTest::OnClickedButtonMotors()
{
	// TODO: Add your control notification handler code here
	if (pDlgMotorsCtrl.get() == nullptr) {
		pDlgMotorsCtrl = make_unique<DlgMotorsControl>();
		pDlgMotorsCtrl->Create(IDD_DLGMOTORSCONTROL, this);
	}
	pDlgMotorsCtrl->ShowWindow(SW_SHOW);
}


void DlgIrCameraTest::OnClickedButtonLoadConfig()
{
	// TODO: Add your control notification handler code here
	Controller::loadConfiguration();
}


void DlgIrCameraTest::OnClickedButtonSaveConfig()
{
	// TODO: Add your control notification handler code here
	Controller::saveConfiguration();
}


void DlgIrCameraTest::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	using namespace OctSystem;
	Controller::getMainBoard()->getCorneaIrCamera()->pause();
	Controller::getMainBoard()->getRetinaIrCamera()->pause();

	if (pDlgLedsCtrl.get() != nullptr) {
		pDlgLedsCtrl->DestroyWindow();
	}
	if (pDlgMotorsCtrl.get() != nullptr) {
		pDlgMotorsCtrl->DestroyWindow();
	}

	CDialogEx::OnClose();
}


BOOL DlgIrCameraTest::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
