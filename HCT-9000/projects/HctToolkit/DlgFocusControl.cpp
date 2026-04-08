#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgFocusControl.h"
#include "afxdialogex.h"

#include "OctSystem2.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace std;


// DlgLedsControl dialog

IMPLEMENT_DYNAMIC(DlgFocusControl, CDialogEx)

DlgFocusControl::DlgFocusControl(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGFOCUSCONTROL, pParent)
{

}


DlgFocusControl::~DlgFocusControl()
{
}


void DlgFocusControl::initControls(void)
{
	MainBoard* board = Controller::getMainBoard();
	focusMotor = board->getOctFocusMotor();
	polarMotor = board->getPolarizationMotor();
	referMotor = board->getReferenceMotor();
	fundusMotor = board->getFundusFocusMotor();

	focusMotor->setControls(&sldFocus, &editFocus, &editFocusDiopt);
	polarMotor->setControls(&sldPolar, &editPolar, &editPolarDegree);
	referMotor->setControls(&sldRefer, &editRefer);
	fundusMotor->setControls(&sldFundus, &editFundus, &editFundusDiopt);

	SetDlgItemText(IDC_EDIT_REFER_STEP, _T("100"));

	useRefCornea = false;
	useFocusTopo = false;

	return;
}


void DlgFocusControl::resetControls(void)
{
	MainBoard* board = Controller::getMainBoard();
	focusMotor = board->getOctFocusMotor();
	polarMotor = board->getPolarizationMotor();
	referMotor = board->getReferenceMotor();
	fundusMotor = board->getFundusFocusMotor();

	focusMotor->setControls(NULL, NULL, NULL);
	polarMotor->setControls(NULL, NULL, NULL);
	referMotor->setControls(NULL, NULL);
	fundusMotor->setControls(NULL, NULL, NULL);
}


void DlgFocusControl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SLIDER_FOCUS, sldFocus);
	DDX_Control(pDX, IDC_SLIDER_FUNDUS, sldFundus);
	DDX_Control(pDX, IDC_SLIDER_POLAR, sldPolar);
	DDX_Control(pDX, IDC_SLIDER_REFER, sldRefer);
	DDX_Control(pDX, IDC_EDIT_FOCUS, editFocus);
	DDX_Control(pDX, IDC_EDIT_FOCUS_DIOPT, editFocusDiopt);
	DDX_Control(pDX, IDC_EDIT_FUNDUS, editFundus);
	DDX_Control(pDX, IDC_EDIT_POLAR, editPolar);
	DDX_Control(pDX, IDC_EDIT_POLAR_DEGREE, editPolarDegree);
	DDX_Control(pDX, IDC_EDIT_FUNDUS_DIOPT, editFundusDiopt);
	DDX_Control(pDX, IDC_EDIT_REFER, editRefer);
}


BEGIN_MESSAGE_MAP(DlgFocusControl, CDialogEx)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_REFER, &DlgFocusControl::OnReleasedcaptureSliderRefer)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_POLAR, &DlgFocusControl::OnReleasedcaptureSliderPolar)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_FUNDUS, &DlgFocusControl::OnReleasedcaptureSliderFundus)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_FOCUS, &DlgFocusControl::OnReleasedcaptureSliderFocus)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS_STEP1, &DlgFocusControl::OnBnClickedButtonFocusStep1)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS_STEP2, &DlgFocusControl::OnBnClickedButtonFocusStep2)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS_STEP3, &DlgFocusControl::OnBnClickedButtonFocusStep3)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS_STEP4, &DlgFocusControl::OnBnClickedButtonFocusStep4)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS_ZERO, &DlgFocusControl::OnBnClickedButtonFocusZero)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS_ORIGIN, &DlgFocusControl::OnBnClickedButtonFocusOrigin)
	ON_BN_CLICKED(IDC_BUTTON_POLAR_STEP1, &DlgFocusControl::OnBnClickedButtonPolarStep1)
	ON_BN_CLICKED(IDC_BUTTON_POLAR_STEP2, &DlgFocusControl::OnBnClickedButtonPolarStep2)
	ON_BN_CLICKED(IDC_BUTTON_POLAR_STEP3, &DlgFocusControl::OnBnClickedButtonPolarStep3)
	ON_BN_CLICKED(IDC_BUTTON_POLAR_STEP4, &DlgFocusControl::OnBnClickedButtonPolarStep4)
	ON_BN_CLICKED(IDC_BUTTON_POLAR_ZERO, &DlgFocusControl::OnBnClickedButtonPolarZero)
	ON_BN_CLICKED(IDC_BUTTON_POLAR_ORIGIN, &DlgFocusControl::OnBnClickedButtonPolarOrigin)
	ON_BN_CLICKED(IDC_BUTTON_FUNDUS_STEP1, &DlgFocusControl::OnBnClickedButtonFundusStep1)
	ON_BN_CLICKED(IDC_BUTTON_FUNDUS_STEP2, &DlgFocusControl::OnBnClickedButtonFundusStep2)
	ON_BN_CLICKED(IDC_BUTTON_FUNDUS_STEP3, &DlgFocusControl::OnBnClickedButtonFundusStep3)
	ON_BN_CLICKED(IDC_BUTTON_FUNDUS_STEP4, &DlgFocusControl::OnBnClickedButtonFundusStep4)
	ON_BN_CLICKED(IDC_BUTTON_FUNDUS_ZERO, &DlgFocusControl::OnBnClickedButtonFundusZero)
	ON_BN_CLICKED(IDC_BUTTON_FUNDUS_ORIGIN, &DlgFocusControl::OnBnClickedButtonFundusOrigin)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON_REFER_ORIGIN, &DlgFocusControl::OnBnClickedButtonReferOrigin)
	ON_BN_CLICKED(IDC_CHECK_REF_CORNEA, &DlgFocusControl::OnBnClickedCheckRefCornea)
	ON_BN_CLICKED(IDC_BUTTON_REFER_ZERO, &DlgFocusControl::OnBnClickedButtonReferZero)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &DlgFocusControl::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &DlgFocusControl::OnBnClickedButtonClose)
	ON_EN_CHANGE(IDC_EDIT_REFER, &DlgFocusControl::OnEnChangeEditRefer)
	ON_BN_CLICKED(IDC_BUTTON_REFER_STEP1, &DlgFocusControl::OnBnClickedButtonReferStep1)
	ON_BN_CLICKED(IDC_BUTTON_REFER_STEP2, &DlgFocusControl::OnBnClickedButtonReferStep2)
	ON_BN_CLICKED(IDC_CHECK_FOCUS_TOPO, &DlgFocusControl::OnBnClickedCheckFocusTopo)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_BOARD, &DlgFocusControl::OnBnClickedButtonSaveBoard)
END_MESSAGE_MAP()


BOOL DlgFocusControl::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgFocusControl::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL) 
		{
			int ctrlID = pFocused->GetDlgCtrlID();
			if (ctrlID == editFocus.GetDlgCtrlID()) {
				focusMotor->updatePositionByEdit();
			}
			else if (ctrlID == editFocusDiopt.GetDlgCtrlID()) {
				focusMotor->updatePositionByDiopt();
			}
			else if (ctrlID == editFundus.GetDlgCtrlID()) {
				fundusMotor->updatePositionByEdit();
			}
			else if (ctrlID == editFundusDiopt.GetDlgCtrlID()) {
				fundusMotor->updatePositionByDiopt();
			}
			else if (ctrlID == editPolar.GetDlgCtrlID()) {
				polarMotor->updatePositionByEdit();
			}
			else if (ctrlID == editPolarDegree.GetDlgCtrlID()) {
				polarMotor->updatePositionByDegree();
			}
			else if (ctrlID == editRefer.GetDlgCtrlID()) {
				referMotor->updatePositionByEdit();
			}
		}
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgFocusControl::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	// To recycle the contents of the window after closing. 
	ShowWindow(SW_HIDE);
	
	// CDialogEx::OnClose();
}


void DlgFocusControl::OnReleasedcaptureSliderRefer(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	referMotor->updatePosition(sldRefer.GetPos());
	*pResult = 0;
}


void DlgFocusControl::OnReleasedcaptureSliderPolar(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	polarMotor->updatePosition(sldPolar.GetPos());
	*pResult = 0;
}


void DlgFocusControl::OnReleasedcaptureSliderFundus(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	fundusMotor->updatePosition(sldFundus.GetPos());
	*pResult = 0;
}


void DlgFocusControl::OnReleasedcaptureSliderFocus(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	focusMotor->updatePosition(sldFocus.GetPos());
	*pResult = 0;
}


void DlgFocusControl::OnBnClickedButtonFocusStep1()
{
	// TODO: Add your control notification handler code here
	focusMotor->updateDiopterByOffset(+1.0f);
}


void DlgFocusControl::OnBnClickedButtonFocusStep2()
{
	// TODO: Add your control notification handler code here
	focusMotor->updateDiopterByOffset(+0.5f);
}


void DlgFocusControl::OnBnClickedButtonFocusStep3()
{
	// TODO: Add your control notification handler code here
	focusMotor->updateDiopterByOffset(-0.5f);
}


void DlgFocusControl::OnBnClickedButtonFocusStep4()
{
	// TODO: Add your control notification handler code here
	focusMotor->updateDiopterByOffset(-1.0f);
}


void DlgFocusControl::OnBnClickedButtonFocusZero()
{
	if (useFocusTopo) {
		focusMotor->setCurrentPositionAsTopogrphyDiopter();
	}
	else {
		focusMotor->setCurrentPositionAsZeroDiopter();
	}
}


void DlgFocusControl::OnBnClickedButtonFocusOrigin()
{
	if (useFocusTopo) {
		focusMotor->updateTopographyDiopter();
	}
	else {
		focusMotor->updateDiopter(0.0f);
	}
}


void DlgFocusControl::OnBnClickedButtonPolarStep1()
{
	// TODO: Add your control notification handler code here
	polarMotor->updateDegreeByOffset(-10.0f);
}


void DlgFocusControl::OnBnClickedButtonPolarStep2()
{
	// TODO: Add your control notification handler code here
	polarMotor->updateDegreeByOffset(-5.0f);
}


void DlgFocusControl::OnBnClickedButtonPolarStep3()
{
	// TODO: Add your control notification handler code here
	polarMotor->updateDegreeByOffset(+5.0f);
}


void DlgFocusControl::OnBnClickedButtonPolarStep4()
{
	// TODO: Add your control notification handler code here
	polarMotor->updateDegreeByOffset(+10.0f);
}


void DlgFocusControl::OnBnClickedButtonPolarZero()
{
	// TODO: Add your control notification handler code here
	polarMotor->setCurrentPositionAsZeroDegree();
}


void DlgFocusControl::OnBnClickedButtonPolarOrigin()
{
	// TODO: Add your control notification handler code here
	polarMotor->updateDegree(0.0);
}


void DlgFocusControl::OnBnClickedButtonFundusStep1()
{
	// TODO: Add your control notification handler code here
	fundusMotor->updateDiopterByOffset(+1.0f);
}


void DlgFocusControl::OnBnClickedButtonFundusStep2()
{
	// TODO: Add your control notification handler code here
	fundusMotor->updateDiopterByOffset(+0.5f);
}


void DlgFocusControl::OnBnClickedButtonFundusStep3()
{
	// TODO: Add your control notification handler code here
	fundusMotor->updateDiopterByOffset(-0.5f);
}


void DlgFocusControl::OnBnClickedButtonFundusStep4()
{
	// TODO: Add your control notification handler code here
	fundusMotor->updateDiopterByOffset(-1.0f);
}


void DlgFocusControl::OnBnClickedButtonFundusZero()
{
	// TODO: Add your control notification handler code here
	fundusMotor->setCurrentPositionAsZeroDiopter();
}


void DlgFocusControl::OnBnClickedButtonFundusOrigin()
{
	// TODO: Add your control notification handler code here
	fundusMotor->updateDiopter(0.0f);
}


void DlgFocusControl::OnBnClickedButtonReferZero()
{
	// TODO: Add your control notification handler code here
	if (useRefCornea) {
		referMotor->setCurrentPositionAsCorneaOrigin();
	}
	else {
		referMotor->setCurrentPositionAsRetinaOrigin();
	}
	return;
}


void DlgFocusControl::OnBnClickedButtonReferOrigin()
{
	// TODO: Add your control notification handler code here
	if (useRefCornea) {
		referMotor->updatePositionToCorneaOrigin();
	}
	else {
		referMotor->updatePositionToRetinaOrigin();
	}
	return;
}


void DlgFocusControl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow) {
		initControls();
	}
	else {
		resetControls();
	}
	return;
}


void DlgFocusControl::OnBnClickedCheckRefCornea()
{
	// TODO: Add your control notification handler code here
	useRefCornea = (IsDlgButtonChecked(IDC_CHECK_REF_CORNEA) ? true : false);
	return;
}


void DlgFocusControl::OnBnClickedButtonSave()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	focusMotor->saveSystemParameters();
	referMotor->saveSystemParameters();
	polarMotor->saveSystemParameters();
	fundusMotor->saveSystemParameters();
	EndWaitCursor();
	return;
}


void DlgFocusControl::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);
	return;
}


void DlgFocusControl::OnEnChangeEditRefer()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void DlgFocusControl::OnBnClickedButtonReferStep1()
{
	// TODO: Add your control notification handler code here
	CString text;
	GetDlgItemText(IDC_EDIT_REFER_STEP, text);
	if (!text.IsEmpty()) {
		int value = _ttoi(text) * -1;
		referMotor->updatePositionByOffset(value);
	}
}


void DlgFocusControl::OnBnClickedButtonReferStep2()
{
	// TODO: Add your control notification handler code here
	CString text;
	GetDlgItemText(IDC_EDIT_REFER_STEP, text);
	if (!text.IsEmpty()) {
		int value = _ttoi(text) * +1;
		referMotor->updatePositionByOffset(value);
	}
}


void DlgFocusControl::OnBnClickedCheckFocusTopo()
{
	useFocusTopo = (IsDlgButtonChecked(IDC_CHECK_FOCUS_TOPO) ? true : false);
	return;
}

void DlgFocusControl::MoveReferenceToCorneaOrigin() {

	referMotor->updatePositionToCorneaOrigin();
	return;
}

void DlgFocusControl::initControlsWithoutShowWindow() {
	initControls();
	return;
}


void DlgFocusControl::isTopography(bool isTopo) {
	useFocusTopo = isTopo;
	return;
}

void DlgFocusControl::OnBnClickedButtonSaveBoard()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	OnBnClickedButtonSave();
	if (!Controller::getMainBoard()->saveSystemConfiguration(true)) {
		AfxMessageBox(_T("Failed to save System calibration!"));
	}
	else {
		AfxMessageBox(_T("System calibration saved to mainboard!"));
	}
	EndWaitCursor();
	return;
}
