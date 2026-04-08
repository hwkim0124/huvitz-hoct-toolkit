// DlgCameraControl.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgCameraControl.h"
#include "afxdialogex.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace std;

// DlgCameraControl dialog

IMPLEMENT_DYNAMIC(DlgCameraControl, CDialogEx)

DlgCameraControl::DlgCameraControl(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGCAMERACONTROL, pParent)
{

}

DlgCameraControl::~DlgCameraControl()
{
}


void DlgCameraControl::initControls(void)
{
	MainBoard* board = Controller::getMainBoard();
	retinaIr = board->getRetinaIrLed();
	corneaIr = board->getCorneaIrLed();
	kerFocus = board->getKeratoFocusLed();
	kerRing = board->getKeratoRingLed();

	retinaIr->setControls(&sldRetina, &editRetina);
	corneaIr->setControls(&sldCornea, &editCornea);
	kerFocus->setControls(&sldFocus, &editFocus);
	kerRing->setControls(&sldRing, &editRing);

	corneaCam = board->getCorneaCamera();
	retinaCam = board->getRetinaCamera();

	updateCorneaAgain(true);
	updateCorneaDgain(true);
	updateRetinaAgain(true);
	updateRetinaDgain(true);
	return;
}


void DlgCameraControl::updateCorneaAgain(bool reset)
{
	int csel = 0;
	float gain;
	if (reset) {
		gain = corneaCam->getAnalogGain();
		csel = (gain >= 10.0f ? 4 : (int) log2(gain));
		cmbCornea.SetCurSel(csel);
	}
	else {
		csel = cmbCornea.GetCurSel();
		gain = (float) pow(2, csel);
		gain = (gain > 10.0f ? 10.0f : gain);
		corneaCam->setAnalogGain((float)gain);
	}
	return;
}


void DlgCameraControl::updateCorneaDgain(bool reset)
{
	float gain;
	CString cstr;
	if (reset) {
		gain = corneaCam->getDigitalGain();
		cstr.Format(_T("%.1f"), gain);
		SetDlgItemText(IDC_EDIT_CORNEA_DGAIN, cstr);
	}
	else {
		GetDlgItemText(IDC_EDIT_CORNEA_DGAIN, cstr);
		gain = (float) _ttof(cstr);
		corneaCam->setDigitalGain(gain);
	}
	return;
}


void DlgCameraControl::updateRetinaAgain(bool reset)
{
	int csel = 0;
	float gain;
	if (reset) {
		gain = retinaCam->getAnalogGain();
		csel = (gain >= 10.0f ? 4 : (int)log2(gain));
		cmbRetina.SetCurSel(csel);
	}
	else {
		csel = cmbRetina.GetCurSel();
		gain = (float)pow(2, csel);
		gain = (gain > 10.0f ? 10.0f : gain);
		retinaCam->setAnalogGain((float)gain);
	}
	return;
}


void DlgCameraControl::updateRetinaDgain(bool reset)
{
	float gain;
	CString cstr;
	if (reset) {
		gain = retinaCam->getDigitalGain();
		cstr.Format(_T("%.1f"), gain);
		SetDlgItemText(IDC_EDIT_RETINA_DGAIN, cstr);
	}
	else {
		GetDlgItemText(IDC_EDIT_RETINA_DGAIN, cstr);
		gain = (float)_ttof(cstr);
		retinaCam->setDigitalGain(gain);
	}
	return;
}


void DlgCameraControl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_CORNEA, editCornea);
	DDX_Control(pDX, IDC_EDIT_CORNEA_DGAIN, editCorneaDgain);
	DDX_Control(pDX, IDC_EDIT_FOCUS, editFocus);
	DDX_Control(pDX, IDC_EDIT_RETINA, editRetina);
	DDX_Control(pDX, IDC_EDIT_RETINA_DGAIN, editRetinaDgain);
	DDX_Control(pDX, IDC_EDIT_RING, editRing);
	DDX_Control(pDX, IDC_SLIDER_CORNEA, sldCornea);
	DDX_Control(pDX, IDC_SLIDER_FOCUS, sldFocus);
	DDX_Control(pDX, IDC_SLIDER_RETINA, sldRetina);
	DDX_Control(pDX, IDC_SLIDER_RING, sldRing);
	DDX_Control(pDX, IDC_COMBO_CORNEA_AGAIN, cmbCornea);
	DDX_Control(pDX, IDC_COMBO_RETINA_AGAIN, cmbRetina);
}


BEGIN_MESSAGE_MAP(DlgCameraControl, CDialogEx)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_CORNEA, &DlgCameraControl::OnReleasedcaptureSliderCornea)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_FOCUS, &DlgCameraControl::OnReleasedcaptureSliderFocus)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_RETINA, &DlgCameraControl::OnReleasedcaptureSliderRetina)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_RING, &DlgCameraControl::OnReleasedcaptureSliderRing)
	ON_BN_CLICKED(IDC_BUTTON_CORNEA_ON, &DlgCameraControl::OnBnClickedButtonCorneaOn)
	ON_BN_CLICKED(IDC_BUTTON_CORNEA_OFF, &DlgCameraControl::OnBnClickedButtonCorneaOff)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_ON, &DlgCameraControl::OnBnClickedButtonRetinaOn)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_OFF, &DlgCameraControl::OnBnClickedButtonRetinaOff)
	ON_BN_CLICKED(IDC_BUTTON_RING_STEP1, &DlgCameraControl::OnBnClickedButtonRingStep1)
	ON_BN_CLICKED(IDC_BUTTON_RING_STEP2, &DlgCameraControl::OnBnClickedButtonRingStep2)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS_STEP1, &DlgCameraControl::OnBnClickedButtonFocusStep1)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS_STEP2, &DlgCameraControl::OnBnClickedButtonFocusStep2)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_COMBO_CORNEA_AGAIN, &DlgCameraControl::OnSelchangeComboCorneaAgain)
	ON_CBN_SELCHANGE(IDC_COMBO_RETINA_AGAIN, &DlgCameraControl::OnSelchangeComboRetinaAgain)
	ON_BN_CLICKED(IDC_BUTTON_CORNEA_CH_ON, &DlgCameraControl::OnBnClickedButtonCorneaChOn)
	ON_BN_CLICKED(IDC_BUTTON_CORNEA_CH_OFF, &DlgCameraControl::OnBnClickedButtonCorneaChOff)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_CH_ON, &DlgCameraControl::OnBnClickedButtonRetinaChOn)
	ON_BN_CLICKED(IDC_BUTTON_RETINA_CH_OFF, &DlgCameraControl::OnBnClickedButtonRetinaChOff)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &DlgCameraControl::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &DlgCameraControl::OnBnClickedButtonSave)
END_MESSAGE_MAP()


// DlgCameraControl message handlers


BOOL DlgCameraControl::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgCameraControl::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL)
		{
			int ctrlID = pFocused->GetDlgCtrlID();
			if (ctrlID == editCornea.GetDlgCtrlID()) {
				corneaIr->updateIntensityByEdit();
			}
			else if (ctrlID == editCorneaDgain.GetDlgCtrlID()) {
				updateCorneaDgain(false);
			}
			else if (ctrlID == editRetina.GetDlgCtrlID()) {
				retinaIr->updateIntensityByEdit();
			}
			else if (ctrlID == editRetinaDgain.GetDlgCtrlID()) {
				updateRetinaDgain(false);
			}
			else if (ctrlID == editFocus.GetDlgCtrlID()) {
				kerFocus->updateIntensityByEdit();
			}
			else if (ctrlID == editRing.GetDlgCtrlID()) {
				kerRing->updateIntensityByEdit();
			}
		}
		return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgCameraControl::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	MainBoard* board = Controller::getMainBoard();
	retinaIr = board->getRetinaIrLed();
	corneaIr = board->getCorneaIrLed();
	kerFocus = board->getKeratoFocusLed();
	kerRing = board->getKeratoRingLed();

	retinaIr->setControls();
	corneaIr->setControls();
	kerFocus->setControls();
	kerRing->setControls();

	// To recycle the contents of the window after closing. 
	ShowWindow(SW_HIDE);

	// CDialogEx::OnClose();
}


void DlgCameraControl::OnReleasedcaptureSliderCornea(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	corneaIr->updateIntensity(sldCornea.GetPos());
	*pResult = 0;
}


void DlgCameraControl::OnReleasedcaptureSliderFocus(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	kerFocus->updateIntensity(sldFocus.GetPos());
	*pResult = 0;
}


void DlgCameraControl::OnReleasedcaptureSliderRetina(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	retinaIr->updateIntensity(sldRetina.GetPos());
	*pResult = 0;
}


void DlgCameraControl::OnReleasedcaptureSliderRing(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	kerRing->updateIntensity(sldRing.GetPos());
	*pResult = 0;
}


void DlgCameraControl::OnBnClickedButtonCorneaOn()
{
	// TODO: Add your control notification handler code here
	corneaIr->updateIntensity(50);
}


void DlgCameraControl::OnBnClickedButtonCorneaOff()
{
	// TODO: Add your control notification handler code here
	corneaIr->updateIntensity(0);
}


void DlgCameraControl::OnBnClickedButtonRetinaOn()
{
	// TODO: Add your control notification handler code here
	retinaIr->updateIntensity(50);
}


void DlgCameraControl::OnBnClickedButtonRetinaOff()
{
	// TODO: Add your control notification handler code here
	retinaIr->updateIntensity(0);
}


void DlgCameraControl::OnBnClickedButtonRingStep1()
{
	// TODO: Add your control notification handler code here
	kerRing->updateIntensityByOffset(-5);
}


void DlgCameraControl::OnBnClickedButtonRingStep2()
{
	// TODO: Add your control notification handler code here
	kerRing->updateIntensityByOffset(+5);
}


void DlgCameraControl::OnBnClickedButtonFocusStep1()
{
	// TODO: Add your control notification handler code here
	kerFocus->updateIntensityByOffset(-5);
}


void DlgCameraControl::OnBnClickedButtonFocusStep2()
{
	// TODO: Add your control notification handler code here
	kerFocus->updateIntensityByOffset(+5);
}


void DlgCameraControl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow) {
		initControls();
	}
	return;
}


void DlgCameraControl::OnSelchangeComboCorneaAgain()
{
	// TODO: Add your control notification handler code here
	updateCorneaAgain(false);
}


void DlgCameraControl::OnSelchangeComboRetinaAgain()
{
	// TODO: Add your control notification handler code here
	updateRetinaAgain(false);
}


void DlgCameraControl::OnBnClickedButtonCorneaChOn()
{
	// TODO: Add your control notification handler code here
	Controller::getMainBoard()->getCorneaIrCamera()->play();
}


void DlgCameraControl::OnBnClickedButtonCorneaChOff()
{
	// TODO: Add your control notification handler code here
	Controller::getMainBoard()->getCorneaIrCamera()->pause();
}


void DlgCameraControl::OnBnClickedButtonRetinaChOn()
{
	// TODO: Add your control notification handler code here
	Controller::getMainBoard()->getRetinaIrCamera()->play();
}


void DlgCameraControl::OnBnClickedButtonRetinaChOff()
{
	// TODO: Add your control notification handler code here
	Controller::getMainBoard()->getRetinaIrCamera()->pause();
}


void DlgCameraControl::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	OnClose();
}


void DlgCameraControl::OnBnClickedButtonSave()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	retinaCam->saveSystemParameters();
	corneaCam->saveSystemParameters();
	EndWaitCursor();
	return;
}
