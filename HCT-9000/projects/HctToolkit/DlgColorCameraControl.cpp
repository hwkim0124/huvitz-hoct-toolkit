// DlgColorCameraControl.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgColorCameraControl.h"
#include "afxdialogex.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace std;

// DlgColorCameraControl dialog

IMPLEMENT_DYNAMIC(DlgColorCameraControl, CDialogEx)

DlgColorCameraControl::DlgColorCameraControl(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGCOLORCAMERACONTROL, pParent)
{
	camera = Controller::getMainBoard()->getColorCamera();
}


DlgColorCameraControl::~DlgColorCameraControl()
{
}


void DlgColorCameraControl::initControls(void)
{
	CString cstr;

	cstr.Format(_T("%d"), camera->getROI_X_Start());
	editStartX.SetWindowTextW(cstr);
	cstr.Format(_T("%d"), camera->getROI_Y_Start());
	editStartY.SetWindowTextW(cstr);
	cstr.Format(_T("%d"), camera->getROI_X_Width());
	editWidth.SetWindowTextW(cstr);
	cstr.Format(_T("%d"), camera->getROI_Y_Height());
	editHeight.SetWindowTextW(cstr);

	if (GlobalSettings::useFundusFILR_Enable()) {
		cmbRate.EnableWindow(false);
		editExposCount.EnableWindow(false);
		editStrobeDelay.EnableWindow(false);
		editStrobeAct.EnableWindow(false);

		cstr.Format(_T("%d"), camera->getExposureOffCount());
		editExposOffCount.SetWindowTextW(cstr);

		cstr.Format(_T("%.2f"), camera->getDigitalGain());
		editDgain.SetWindowTextW(cstr);

		cmbAgain.EnableWindow(false);

		cmbFrameSize.EnableWindow(false);
		cmbFlip.EnableWindow(false);

	}
	else {
		cmbRate.SetCurSel(camera->getROI_DecimationRatio(true));

		cstr.Format(_T("%d"), camera->getExposureCount());
		editExposCount.SetWindowTextW(cstr);
		cstr.Format(_T("%d"), camera->getExposureOffCount());
		editExposOffCount.SetWindowTextW(cstr);
		cstr.Format(_T("%d"), camera->getStrobeDelay());
		editStrobeDelay.SetWindowTextW(cstr);
		cstr.Format(_T("%d"), camera->getStrobeActTime());
		editStrobeAct.SetWindowTextW(cstr);

		cmbAgain.SetCurSel(camera->getAnalogGain(true));
		cstr.Format(_T("%.2f"), camera->getDigitalGain());
		editDgain.SetWindowTextW(cstr);

		cmbFrameSize.SetCurSel(camera->getFrameSize(true));
	}
	cmbShutter.SetCurSel(camera->getShutterMode(true));
	cmbFlip.SetCurSel(camera->getFlipMode(true));
	cmbCapture.SetCurSel(camera->getCaptureMode(true));
	return;
}


void DlgColorCameraControl::readControls(void)
{
	CString cstr;

	if (!GlobalSettings::useOctEnable()) {
		unsigned int offset, size;

		editStartX.GetWindowTextW(cstr);
		offset = _ttoi(cstr);
		editWidth.GetWindowTextW(cstr);
		size = _ttoi(cstr);

		if (offset + size > 5472) {
			camera->setROI_X_Width(3648);
			camera->setROI_X_Start(912);
		}
		else {
			camera->setROI_X_Width(size);
			camera->setROI_X_Start(offset);
		}


		editStartY.GetWindowTextW(cstr);
		offset = _ttoi(cstr);
		editHeight.GetWindowTextW(cstr);
		size = _ttoi(cstr);
		if (offset + size > 3648) {
			camera->setROI_Y_Height(3000);
			camera->setROI_Y_Start(648);
		}
		else {
			camera->setROI_Y_Height(size);
			camera->setROI_Y_Start(offset);
		}
		cstr.Format(_T("%d"), camera->getROI_X_Start());
		editStartX.SetWindowTextW(cstr);
		cstr.Format(_T("%d"), camera->getROI_Y_Start());
		editStartY.SetWindowTextW(cstr);
		cstr.Format(_T("%d"), camera->getROI_X_Width());
		editWidth.SetWindowTextW(cstr);
		cstr.Format(_T("%d"), camera->getROI_Y_Height());
		editHeight.SetWindowTextW(cstr);


	}
	else {
		editStartX.GetWindowTextW(cstr);
		camera->setROI_X_Start(_ttoi(cstr));
		editStartY.GetWindowTextW(cstr);
		camera->setROI_Y_Start(_ttoi(cstr));
		editWidth.GetWindowTextW(cstr);
		camera->setROI_X_Width(_ttoi(cstr));
		editHeight.GetWindowTextW(cstr);
		camera->setROI_Y_Height(_ttoi(cstr));
	}

	camera->setROI_DecimationRatio(cmbRate.GetCurSel(), true);

	editExposCount.GetWindowTextW(cstr);
	camera->setExposureCount(_ttoi(cstr));
	editExposOffCount.GetWindowTextW(cstr);
	camera->setExposureOffCount(_ttoi(cstr));
	editStrobeDelay.GetWindowTextW(cstr);
	camera->setStrobeDelay(_ttoi(cstr));
	editStrobeAct.GetWindowTextW(cstr);
	camera->setStrobeActTime(_ttoi(cstr));
	camera->setStrobeMode(1);

	camera->setAnalogGain(cmbAgain.GetCurSel(), true);

	editDgain.GetWindowTextW(cstr);
	camera->setDigitalGain((float)_ttof(cstr));

	camera->setFrameSize(cmbFrameSize.GetCurSel(), true);
	camera->setShutterMode(cmbShutter.GetCurSel(), true);
	camera->setFlipMode(cmbFlip.GetCurSel(), true);
	camera->setCaptureMode(cmbCapture.GetCurSel(), true);
	return;
}


void DlgColorCameraControl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_AGAIN, cmbAgain);
	DDX_Control(pDX, IDC_COMBO_CAPTURE, cmbCapture);
	DDX_Control(pDX, IDC_COMBO_FLIP, cmbFlip);
	DDX_Control(pDX, IDC_COMBO_FRAMESIZE, cmbFrameSize);
	DDX_Control(pDX, IDC_COMBO_RATE, cmbRate);
	DDX_Control(pDX, IDC_COMBO_SHUTTER, cmbShutter);
	DDX_Control(pDX, IDC_EDIT_DGAIN, editDgain);
	DDX_Control(pDX, IDC_EDIT_EXPOS_COUNT, editExposCount);
	DDX_Control(pDX, IDC_EDIT_EXPOS_OFFCOUNT, editExposOffCount);
	DDX_Control(pDX, IDC_EDIT_STROBE_ACT, editStrobeAct);
	DDX_Control(pDX, IDC_EDIT_STROBE_DELAY, editStrobeDelay);
	DDX_Control(pDX, IDC_EDIT_X_START, editStartX);
	DDX_Control(pDX, IDC_EDIT_X_WIDTH, editWidth);
	DDX_Control(pDX, IDC_EDIT_Y_HEIGHT, editHeight);
	DDX_Control(pDX, IDC_EDIT_Y_START, editStartY);
}


BEGIN_MESSAGE_MAP(DlgColorCameraControl, CDialogEx)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_COMBO_FRAMESIZE, &DlgColorCameraControl::OnSelchangeComboFramesize)
	ON_CBN_SELCHANGE(IDC_COMBO_RATE, &DlgColorCameraControl::OnSelchangeComboRate)
	ON_CBN_SELCHANGE(IDC_COMBO_SHUTTER, &DlgColorCameraControl::OnSelchangeComboShutter)
	ON_CBN_SELCHANGE(IDC_COMBO_FLIP, &DlgColorCameraControl::OnSelchangeComboFlip)
	ON_CBN_SELCHANGE(IDC_COMBO_AGAIN, &DlgColorCameraControl::OnSelchangeComboAgain)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &DlgColorCameraControl::OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT, &DlgColorCameraControl::OnBnClickedButtonDefault)
	ON_CBN_SELCHANGE(IDC_COMBO_CAPTURE, &DlgColorCameraControl::OnCbnSelchangeComboCapture)
END_MESSAGE_MAP()


// DlgColorCameraControl message handlers


BOOL DlgColorCameraControl::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	initControls();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgColorCameraControl::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL)
		{
			int ctrlID = pFocused->GetDlgCtrlID();
			//readControls();
			//initControls();
			OnBnClickedButtonApply();
		}
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgColorCameraControl::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnClose();
}


void DlgColorCameraControl::OnSelchangeComboFramesize()
{
	// TODO: Add your control notification handler code here
	camera->setFrameSize(cmbFrameSize.GetCurSel(), true);
	OnBnClickedButtonApply();
}


void DlgColorCameraControl::OnSelchangeComboRate()
{
	// TODO: Add your control notification handler code here
	camera->setROI_DecimationRatio(cmbRate.GetCurSel(), true);
	OnBnClickedButtonApply();
}


void DlgColorCameraControl::OnSelchangeComboShutter()
{
	// TODO: Add your control notification handler code here
	camera->setShutterMode(cmbShutter.GetCurSel(), true);
	OnBnClickedButtonApply();
}


void DlgColorCameraControl::OnSelchangeComboFlip()
{
	// TODO: Add your control notification handler code here
	camera->setFlipMode(cmbFlip.GetCurSel(), true);
	OnBnClickedButtonApply();
}


void DlgColorCameraControl::OnSelchangeComboAgain()
{
	// TODO: Add your control notification handler code here
	camera->setAnalogGain(cmbAgain.GetCurSel(), true);
	OnBnClickedButtonApply();
}


void DlgColorCameraControl::OnCbnSelchangeComboCapture()
{
	// TODO: Add your control notification handler code here
	camera->setCaptureMode(cmbCapture.GetCurSel(), true);
	OnBnClickedButtonApply();
}


void DlgColorCameraControl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow) {
		initControls();
	}
	return;
}


void DlgColorCameraControl::OnBnClickedButtonApply()
{
	// TODO: Add your control notification handler code here
	readControls();
	camera->updateParameters();
	// initControls();
	return;
}


void DlgColorCameraControl::OnBnClickedButtonDefault()
{
	// TODO: Add your control notification handler code here
	camera->setDefaultParameters(false);
	initControls();
	return;
}


