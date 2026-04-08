// DlgMotorControl.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgMotorControl.h"
#include "afxdialogex.h"

#include "OctSystem2.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace std;

// DlgMotorControl dialog

IMPLEMENT_DYNAMIC(DlgMotorControl, CDialogEx)

DlgMotorControl::DlgMotorControl(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGMOTORCONTROL, pParent)
{

}

DlgMotorControl::~DlgMotorControl()
{
}


void DlgMotorControl::initControls(void)
{
	MainBoard* board = Controller::getMainBoard();
	sample = board->getOctSampleMotor();
	funDiopt = board->getFundusDiopterMotor();
	octDiopt = board->getOctDiopterMotor();
	split = board->getSplitFocusMotor();
	pupil = board->getPupilMaskMotor();
	// compen = board->getOctCompensationMotor();

	sample->setControls(&sldMirror, &editMirror);
	funDiopt->setControls(&sldDiopt, &editDiopt);
	octDiopt->setControls(&sldDiopt2, &editDiopt2);
	split->setControls(&sldSplit, &editSplit);
	pupil->setControls(&sldPupil, &editPupil);
	//compen->setControls(&sldCompen, &editCompen);

	cmbMirror.SetCurSel(0);
	cmbDiopt.SetCurSel(0);
	cmbDiopt2.SetCurSel(0);
	cmbPupil.SetCurSel(0);
	cmbSplit.SetCurSel(0);
	return;
}


void DlgMotorControl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	// DDX_Control(pDX, IDC_SLIDER_COMPEN, sldCompen);
	DDX_Control(pDX, IDC_SLIDER_DIOPT, sldDiopt);
	DDX_Control(pDX, IDC_SLIDER_MIRROR2, sldMirror);
	DDX_Control(pDX, IDC_SLIDER_PUPIL, sldPupil);
	DDX_Control(pDX, IDC_SLIDER_SPLIT, sldSplit);
	// DDX_Control(pDX, IDC_EDIT_COMPEN, editCompen);
	DDX_Control(pDX, IDC_EDIT_DIOPT, editDiopt);
	DDX_Control(pDX, IDC_EDIT_MIRROR2, editMirror);
	DDX_Control(pDX, IDC_EDIT_PUPIL, editPupil);
	DDX_Control(pDX, IDC_EDIT_SPLIT, editSplit);
	DDX_Control(pDX, IDC_COMBO_DIOPT, cmbDiopt);
	DDX_Control(pDX, IDC_COMBO_MIRROR, cmbMirror);
	DDX_Control(pDX, IDC_COMBO_PUPIL, cmbPupil);
	DDX_Control(pDX, IDC_COMBO_SPLIT, cmbSplit);
	DDX_Control(pDX, IDC_COMBO_DIOPT2, cmbDiopt2);
	DDX_Control(pDX, IDC_EDIT_DIOPT2, editDiopt2);
	DDX_Control(pDX, IDC_SLIDER_DIOPT2, sldDiopt2);
}


BEGIN_MESSAGE_MAP(DlgMotorControl, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_MIRROR2, &DlgMotorControl::OnReleasedcaptureSliderMirror)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SPLIT, &DlgMotorControl::OnReleasedcaptureSliderSplit)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_PUPIL, &DlgMotorControl::OnReleasedcaptureSliderPupil)
	// ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_COMPEN, &DlgMotorControl::OnReleasedcaptureSliderCompen)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_DIOPT, &DlgMotorControl::OnReleasedcaptureSliderDiopt)
	ON_BN_CLICKED(IDC_BUTTON_MIRROR_IN, &DlgMotorControl::OnBnClickedButtonMirrorIn)
	ON_BN_CLICKED(IDC_BUTTON_MIRROR_OUT, &DlgMotorControl::OnBnClickedButtonMirrorOut)
	ON_BN_CLICKED(IDC_BUTTON_MIRROR_SET, &DlgMotorControl::OnBnClickedButtonMirrorSet)
	ON_BN_CLICKED(IDC_BUTTON_DIOPT_MINUS, &DlgMotorControl::OnBnClickedButtonDioptMinus)
	ON_BN_CLICKED(IDC_BUTTON_DIOPT_ZERO, &DlgMotorControl::OnBnClickedButtonDioptZero)
	ON_BN_CLICKED(IDC_BUTTON_DiOPT_PLUS, &DlgMotorControl::OnBnClickedButtonDioptPlus)
	ON_BN_CLICKED(IDC_BUTTON_DIOPT_SET, &DlgMotorControl::OnBnClickedButtonDioptSet)
	ON_BN_CLICKED(IDC_BUTTON_SPLIT_STEP1, &DlgMotorControl::OnBnClickedButtonSplitStep1)
	ON_BN_CLICKED(IDC_BUTTON_SPLIT_STEP2, &DlgMotorControl::OnBnClickedButtonSplitStep2)
	// ON_BN_CLICKED(IDC_BUTTON_SPLIT_ZERO, &DlgMotorControl::OnBnClickedButtonSplitZero)
	// ON_BN_CLICKED(IDC_BUTTON_SPLIT_ORIGIN, &DlgMotorControl::OnBnClickedButtonSplitOrigin)
	ON_BN_CLICKED(IDC_BUTTON_PUPIL_NORMAL, &DlgMotorControl::OnBnClickedButtonPupilNormal)
	ON_BN_CLICKED(IDC_BUTTON_PUPIL_SMALL, &DlgMotorControl::OnBnClickedButtonPupilSmall)
	ON_BN_CLICKED(IDC_BUTTON_PUPIL_SET, &DlgMotorControl::OnBnClickedButtonPupilSet)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &DlgMotorControl::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &DlgMotorControl::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_SPLIT_SET, &DlgMotorControl::OnBnClickedButtonSplitSet)
	ON_BN_CLICKED(IDC_BUTTON_DIOPT_SET2, &DlgMotorControl::OnBnClickedButtonDioptSet2)
	ON_BN_CLICKED(IDC_BUTTON_DIOPT_MINUS2, &DlgMotorControl::OnBnClickedButtonDioptMinus2)
	ON_BN_CLICKED(IDC_BUTTON_DIOPT_ZERO2, &DlgMotorControl::OnBnClickedButtonDioptZero2)
	ON_BN_CLICKED(IDC_BUTTON_DiOPT_PLUS2, &DlgMotorControl::OnBnClickedButtonDioptPlus2)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_DIOPT2, &DlgMotorControl::OnReleasedcaptureSliderDiopt2)
END_MESSAGE_MAP()


// DlgMotorControl message handlers


BOOL DlgMotorControl::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgMotorControl::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL)
		{
			int ctrlID = pFocused->GetDlgCtrlID();
			if (ctrlID == editMirror.GetDlgCtrlID()) {
				sample->updatePositionByEdit();
			}
			else if (ctrlID == editDiopt.GetDlgCtrlID()) {
				funDiopt->updatePositionByEdit();
			}
			else if (ctrlID == editSplit.GetDlgCtrlID()) {
				split->updatePositionByEdit();
			}
			else if (ctrlID == editPupil.GetDlgCtrlID()) {
				pupil->updatePositionByEdit();
			}
			else if (ctrlID == editDiopt2.GetDlgCtrlID()) {
				octDiopt->updatePositionByEdit();
			}
		}
		return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgMotorControl::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	MainBoard* board = Controller::getMainBoard();
	sample = board->getOctSampleMotor();
	funDiopt = board->getFundusDiopterMotor();
	octDiopt = board->getOctDiopterMotor();
	split = board->getSplitFocusMotor();
	pupil = board->getPupilMaskMotor();
	// compen = board->getOctCompensationMotor();

	sample->setControls();
	funDiopt->setControls();
	octDiopt->setControls();
	split->setControls();
	pupil->setControls();
	//compen->setControls(&sldCompen, &editCompen);

	// To recycle the contents of the window after closing. 
	ShowWindow(SW_HIDE);

	// CDialogEx::OnClose();
}


void DlgMotorControl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow) {
		initControls();
	}
	return;
}


void DlgMotorControl::OnReleasedcaptureSliderMirror(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	sample->updatePosition(sldMirror.GetPos());
	*pResult = 0;
}


void DlgMotorControl::OnReleasedcaptureSliderSplit(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	split->updatePosition(sldSplit.GetPos());
	*pResult = 0;
}


void DlgMotorControl::OnReleasedcaptureSliderPupil(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	pupil->updatePosition(sldPupil.GetPos());
	*pResult = 0;
}

/*
void DlgMotorControl::OnReleasedcaptureSliderCompen(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	compen->updatePosition(sldCompen.GetPos());
	*pResult = 0;
}
*/


void DlgMotorControl::OnReleasedcaptureSliderDiopt(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	funDiopt->updatePosition(sldDiopt.GetPos());
	*pResult = 0;
}



void DlgMotorControl::OnReleasedcaptureSliderDiopt2(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	octDiopt->updatePosition(sldDiopt2.GetPos());
	*pResult = 0;
}



void DlgMotorControl::OnBnClickedButtonMirrorIn()
{
	// TODO: Add your control notification handler code here
	sample->updatePositionToMirrorIn();
}


void DlgMotorControl::OnBnClickedButtonMirrorOut()
{
	// TODO: Add your control notification handler code here
	sample->updatePositionToMirrorOut();
}


void DlgMotorControl::OnBnClickedButtonMirrorSet()
{
	// TODO: Add your control notification handler code here
	int curSel = cmbMirror.GetCurSel();
	if (curSel == 0) {
		sample->setCurrentPositionAsMirrorIn();
	}
	else {
		sample->setCurrentPositionAsMirrorOut();
	}
}


void DlgMotorControl::OnBnClickedButtonDioptMinus()
{
	// TODO: Add your control notification handler code here
	funDiopt->updatePositionToMinusLens();
}


void DlgMotorControl::OnBnClickedButtonDioptZero()
{
	// TODO: Add your control notification handler code here
	funDiopt->updatePositionToZeroLens();
}


void DlgMotorControl::OnBnClickedButtonDioptPlus()
{
	// TODO: Add your control notification handler code here
	funDiopt->updatePositionToPlusLens();
}


void DlgMotorControl::OnBnClickedButtonDioptSet()
{
	// TODO: Add your control notification handler code here
	int curSel = cmbDiopt.GetCurSel();

	if (curSel == 0) {
		funDiopt->setCurrentPositionAsMinusLens();
	}
	else if (curSel == 1) {
		funDiopt->setCurrentPositionAsZeroLens();
	}
	else {
		funDiopt->setCurrentPositionAsPlusLens();
	}
}


void DlgMotorControl::OnBnClickedButtonSplitStep1()
{
	// TODO: Add your control notification handler code here
	split->updatePositionToMirrorIn();
}


void DlgMotorControl::OnBnClickedButtonSplitStep2()
{
	// TODO: Add your control notification handler code here
	split->updatePositionToMirrorOut();
}


void DlgMotorControl::OnBnClickedButtonSplitSet()
{
	// TODO: Add your control notification handler code here
	int curSel = cmbSplit.GetCurSel();
	if (curSel == 0) {
		split->setCurrentPositionAsMirrorIn();
	}
	else {
		split->setCurrentPositionAsMirrorOut();
	}
}


void DlgMotorControl::OnBnClickedButtonPupilNormal()
{
	// TODO: Add your control notification handler code here
	pupil->updatePositionToNormal();
}


void DlgMotorControl::OnBnClickedButtonPupilSmall()
{
	// TODO: Add your control notification handler code here
	pupil->updatePositionToSmall();
}


void DlgMotorControl::OnBnClickedButtonPupilSet()
{
	// TODO: Add your control notification handler code here
	int curSel = cmbPupil.GetCurSel();
	if (curSel == 0) {
		pupil->setCurrentPositionAsNormal();
	}
	else {
		pupil->setCurrentPositionAsSmall();
	}
}


void DlgMotorControl::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	OnClose();
}


void DlgMotorControl::OnBnClickedButtonSave()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	sample->saveSystemParameters();
	split->saveSystemParameters();
	pupil->saveSystemParameters();
	funDiopt->saveSystemParameters();
	octDiopt->saveSystemParameters();
	EndWaitCursor();
	return;
}


void DlgMotorControl::OnBnClickedButtonDioptSet2()
{
	// TODO: Add your control notification handler code here
	int curSel = cmbDiopt2.GetCurSel();

	if (curSel == 0) {
		octDiopt->setCurrentPositionAsMinusLens();
	}
	else if (curSel == 1) {
		octDiopt->setCurrentPositionAsZeroLens();
	}
	else {
		octDiopt->setCurrentPositionAsPlusLens();
	}
}


void DlgMotorControl::OnBnClickedButtonDioptMinus2()
{
	// TODO: Add your control notification handler code here
	octDiopt->updatePositionToMinusLens();
}


void DlgMotorControl::OnBnClickedButtonDioptZero2()
{
	// TODO: Add your control notification handler code here
	octDiopt->updatePositionToZeroLens();
}


void DlgMotorControl::OnBnClickedButtonDioptPlus2()
{
	// TODO: Add your control notification handler code here
	octDiopt->updatePositionToPlusLens();
}


void DlgMotorControl::blackSpotAlign()
{
	split->updatePositionToMirrorOut();
	sample->updatePositionToMirrorOut();

	return;
}