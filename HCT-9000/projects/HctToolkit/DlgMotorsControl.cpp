// DlgMotorsControl.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgMotorsControl.h"
#include "afxdialogex.h"
#include "OctSystem2.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace std;
// DlgMotorsControl dialog

IMPLEMENT_DYNAMIC(DlgMotorsControl, CDialogEx)

DlgMotorsControl::DlgMotorsControl(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGMOTORSCONTROL, pParent)
{

}


DlgMotorsControl::~DlgMotorsControl()
{
}


void DlgMotorsControl::initControls(void)
{
	MainBoard* board = Controller::getMainBoard();
	board->getOctFocusMotor()->setControls(&sldOctFocus, &editOctFocus);
	board->getPolarizationMotor()->setControls(&sldPolar, &editPolar);
	board->getReferenceMotor()->setControls(&sldRefer, &editRefer);
	board->getFundusFocusMotor()->setControls(&sldFundusFocus, &editFundusFocus);
	board->getOctSampleMotor()->setControls(&sldSample, &editSample);
	board->getDiopterSelMotor()->setControls(&sldDioptSel, &editDioptSel);
	board->getSplitFocusMotor()->setControls(&sldSplitFocus, &editSplitFocus);
	board->getOctCompensationMotor()->setControls(&sldOctCompen, &editOctCompen);
	board->getPupilMaskMotor()->setControls(&sldPupilMask, &editPupilMask);
	return;
}


void DlgMotorsControl::updateOctFocus(bool edit)
{
	if (edit) {
		CString str;
		editOctFocus.GetWindowText(str);
		if (str.GetLength() > 0) {
			int pos = _ttoi(str);
			pos = (pos > sldOctFocus.GetRangeMax() ? sldOctFocus.GetRangeMax() : pos);
			pos = (pos < sldOctFocus.GetRangeMin() ? sldOctFocus.GetRangeMin() : pos);
			sldOctFocus.SetPos(pos);
			str.Format(_T("%d"), pos);
			editOctFocus.SetWindowTextW(str);
			editOctFocus.SetSel(0, -1, FALSE);

			Controller::getMainBoard()->getOctFocusMotor()->controlMove(pos);
		}
	}
	else {
		int pos = sldOctFocus.GetPos();
		editOctFocus.SetWindowTextW(to_wstring(pos).c_str());
		//Controller::getOctFocusMotor()->controlMove(pos);
	}
	return;
}


void DlgMotorsControl::updateRefer(bool edit)
{
	if (edit) {
		CString str;
		editRefer.GetWindowText(str);
		if (str.GetLength() > 0) {
			int pos = _ttoi(str);
			pos = (pos > sldRefer.GetRangeMax() ? sldRefer.GetRangeMax() : pos);
			pos = (pos < sldRefer.GetRangeMin() ? sldRefer.GetRangeMin() : pos);
			sldRefer.SetPos(pos);
			str.Format(_T("%d"), pos);
			editRefer.SetWindowTextW(str);
			editRefer.SetSel(0, -1, FALSE);

			Controller::getMainBoard()->getReferenceMotor()->controlMove(pos);
		}
	}
	else {
		int pos = sldRefer.GetPos();
		editRefer.SetWindowTextW(to_wstring(pos).c_str());
		//Controller::getReferenceMotor()->controlMove(pos);
	}
	return;
}


void DlgMotorsControl::updatePolar(bool edit)
{
	if (edit) {
		CString str;
		editPolar.GetWindowText(str);
		if (str.GetLength() > 0) {
			int pos = _ttoi(str);
			pos = (pos > sldPolar.GetRangeMax() ? sldPolar.GetRangeMax() : pos);
			pos = (pos < sldPolar.GetRangeMin() ? sldPolar.GetRangeMin() : pos);
			sldPolar.SetPos(pos);
			str.Format(_T("%d"), pos);
			editPolar.SetWindowTextW(str);
			editPolar.SetSel(0, -1, FALSE);

			Controller::getMainBoard()->getPolarizationMotor()->controlMove(pos);
		}
	}
	else {
		int pos = sldPolar.GetPos();
		editPolar.SetWindowTextW(to_wstring(pos).c_str());
		//Controller::getPolarizationMotor()->controlMove(pos);
	}
	return;
}


void DlgMotorsControl::updateFundusFocus(bool edit)
{
	if (edit) {
		CString str;
		editFundusFocus.GetWindowText(str);
		if (str.GetLength() > 0) {
			int pos = _ttoi(str);
			pos = (pos > sldFundusFocus.GetRangeMax() ? sldFundusFocus.GetRangeMax() : pos);
			pos = (pos < sldFundusFocus.GetRangeMin() ? sldFundusFocus.GetRangeMin() : pos);
			sldFundusFocus.SetPos(pos);
			str.Format(_T("%d"), pos);
			editFundusFocus.SetWindowTextW(str);
			editFundusFocus.SetSel(0, -1, FALSE);

			Controller::getMainBoard()->getFundusFocusMotor()->controlMove(pos);
		}
	}
	else {
		int pos = sldFundusFocus.GetPos();
		editFundusFocus.SetWindowTextW(to_wstring(pos).c_str());
		//Controller::getFundusFocusMotor()->controlMove(pos);
	}
	return;
}


void DlgMotorsControl::updateSplitFocus(bool edit)
{
	if (edit) {
		CString str;
		editSplitFocus.GetWindowText(str);
		if (str.GetLength() > 0) {
			int pos = _ttoi(str);
			pos = (pos > sldSplitFocus.GetRangeMax() ? sldSplitFocus.GetRangeMax() : pos);
			pos = (pos < sldSplitFocus.GetRangeMin() ? sldSplitFocus.GetRangeMin() : pos);
			sldSplitFocus.SetPos(pos);
			str.Format(_T("%d"), pos);
			editSplitFocus.SetWindowTextW(str);
			editSplitFocus.SetSel(0, -1, FALSE);

			Controller::getMainBoard()->getSplitFocusMotor()->controlMove(pos);
		}
	}
	else {
		int pos = sldSplitFocus.GetPos();
		editSplitFocus.SetWindowTextW(to_wstring(pos).c_str());
		//Controller::getSplitFocusMotor()->controlMove(pos);
	}
	return;
}


void DlgMotorsControl::updateSample(bool edit)
{
	if (edit) {
		CString str;
		editSample.GetWindowText(str);
		if (str.GetLength() > 0) {
			int pos = _ttoi(str);
			pos = (pos > sldSample.GetRangeMax() ? sldSample.GetRangeMax() : pos);
			pos = (pos < sldSample.GetRangeMin() ? sldSample.GetRangeMin() : pos);
			sldSample.SetPos(pos);
			str.Format(_T("%d"), pos);
			editSample.SetWindowTextW(str);
			editSample.SetSel(0, -1, FALSE);

			Controller::getMainBoard()->getOctSampleMotor()->controlMove(pos);
		}
	}
	else {
		int pos = sldSample.GetPos();
		editSample.SetWindowTextW(to_wstring(pos).c_str());
		//Controller::getOctSampleMotor()->controlMove(pos);
	}
	return;
}


void DlgMotorsControl::updateDioptSel(bool edit)
{
	if (edit) {
		CString str;
		editDioptSel.GetWindowText(str);
		if (str.GetLength() > 0) {
			int pos = _ttoi(str);
			pos = (pos > sldDioptSel.GetRangeMax() ? sldDioptSel.GetRangeMax() : pos);
			pos = (pos < sldDioptSel.GetRangeMin() ? sldDioptSel.GetRangeMin() : pos);
			sldDioptSel.SetPos(pos);
			str.Format(_T("%d"), pos);
			editDioptSel.SetWindowTextW(str);
			editDioptSel.SetSel(0, -1, FALSE);

			Controller::getMainBoard()->getDiopterSelMotor()->controlMove(pos);
		}
	}
	else {
		int pos = sldDioptSel.GetPos();
		editDioptSel.SetWindowTextW(to_wstring(pos).c_str());
		//Controller::getDiopterSelMotor()->controlMove(pos);
	}
	return;
}


void DlgMotorsControl::updateOctCompen(bool edit)
{
	if (edit) {
		CString str;
		editOctCompen.GetWindowText(str);
		if (str.GetLength() > 0) {
			int pos = _ttoi(str);
			pos = (pos > sldOctCompen.GetRangeMax() ? sldOctCompen.GetRangeMax() : pos);
			pos = (pos < sldOctCompen.GetRangeMin() ? sldOctCompen.GetRangeMin() : pos);
			sldOctCompen.SetPos(pos);
			str.Format(_T("%d"), pos);
			editOctCompen.SetWindowTextW(str);
			editOctCompen.SetSel(0, -1, FALSE);

			Controller::getMainBoard()->getOctCompensationMotor()->controlMove(pos);
		}
	}
	else {
		int pos = sldOctCompen.GetPos();
		editOctCompen.SetWindowTextW(to_wstring(pos).c_str());
		//Controller::getOctCompensationMotor()->controlMove(pos);
	}
	return;
}


void DlgMotorsControl::updatePupilMask(bool edit)
{
	if (edit) {
		CString str;
		editPupilMask.GetWindowText(str);
		if (str.GetLength() > 0) {
			int pos = _ttoi(str);
			pos = (pos > sldPupilMask.GetRangeMax() ? sldPupilMask.GetRangeMax() : pos);
			pos = (pos < sldPupilMask.GetRangeMin() ? sldPupilMask.GetRangeMin() : pos);
			sldPupilMask.SetPos(pos);
			str.Format(_T("%d"), pos);
			editPupilMask.SetWindowTextW(str);
			editPupilMask.SetSel(0, -1, FALSE);

			Controller::getMainBoard()->getPupilMaskMotor()->controlMove(pos);
		}
	}
	else {
		int pos = sldPupilMask.GetPos();
		editPupilMask.SetWindowTextW(to_wstring(pos).c_str());
		//Controller::getPupilMaskMotor()->controlMove(pos);
	}
	return;
}


void DlgMotorsControl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DIOPT_SEL, editDioptSel);
	DDX_Control(pDX, IDC_EDIT_FUNDUS_FOCUS, editFundusFocus);
	//  DDX_Control(pDX, IDC_EDIT_OCT_COMPEN, editOctCompens);
	DDX_Control(pDX, IDC_EDIT_OCT_COMPEN, editOctCompen);
	DDX_Control(pDX, IDC_EDIT_OCT_FOCUS, editOctFocus);
	DDX_Control(pDX, IDC_EDIT_POLAR, editPolar);
	DDX_Control(pDX, IDC_EDIT_REFER, editRefer);
	DDX_Control(pDX, IDC_EDIT_SAMPLE, editSample);
	DDX_Control(pDX, IDC_EDIT_SPLIT_FOCUS, editSplitFocus);
	DDX_Control(pDX, IDC_SLIDER_DIOPT_SEL, sldDioptSel);
	DDX_Control(pDX, IDC_SLIDER_FUNDUS_FOCUS, sldFundusFocus);
	DDX_Control(pDX, IDC_SLIDER_OCT_COMPEN, sldOctCompen);
	DDX_Control(pDX, IDC_SLIDER_OCT_FOCUS, sldOctFocus);
	DDX_Control(pDX, IDC_SLIDER_POLAR, sldPolar);
	DDX_Control(pDX, IDC_SLIDER_REFER, sldRefer);
	DDX_Control(pDX, IDC_SLIDER_SAMPLE, sldSample);
	DDX_Control(pDX, IDC_SLIDER_SPLIT_FOCUS, sldSplitFocus);
	DDX_Control(pDX, IDC_SLIDER_PUPIL_MASK, sldPupilMask);
	DDX_Control(pDX, IDC_EDIT_PUPIL_MASK, editPupilMask);
}


BEGIN_MESSAGE_MAP(DlgMotorsControl, CDialogEx)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_DIOPT_SEL, &DlgMotorsControl::OnReleasedcaptureSliderDioptSel)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_FUNDUS_FOCUS, &DlgMotorsControl::OnReleasedcaptureSliderFundusFocus)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_OCT_COMPEN, &DlgMotorsControl::OnReleasedcaptureSliderOctCompen)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_OCT_FOCUS, &DlgMotorsControl::OnReleasedcaptureSliderOctFocus)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_POLAR, &DlgMotorsControl::OnReleasedcaptureSliderPolar)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_REFER, &DlgMotorsControl::OnReleasedcaptureSliderRefer)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SAMPLE, &DlgMotorsControl::OnReleasedcaptureSliderSample)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SPLIT_FOCUS, &DlgMotorsControl::OnReleasedcaptureSliderSplitFocus)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_PUPIL_MASK, &DlgMotorsControl::OnReleasedcaptureSliderPupilMask)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// DlgMotorsControl message handlers


void DlgMotorsControl::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	// To recycle the contents of the window after closing. 
	ShowWindow(SW_HIDE);

	// CDialogEx::OnClose();
}


BOOL DlgMotorsControl::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	// initControls();


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgMotorsControl::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL) {
			if (pFocused->GetDlgCtrlID() == editOctFocus.GetDlgCtrlID()) {
				updateOctFocus();
			}
			else if (pFocused->GetDlgCtrlID() == editPolar.GetDlgCtrlID()) {
				updatePolar();
			}
			else if (pFocused->GetDlgCtrlID() == editRefer.GetDlgCtrlID()) {
				updateRefer();
			}
			else if (pFocused->GetDlgCtrlID() == editFundusFocus.GetDlgCtrlID()) {
				updateFundusFocus();
			}
			else if (pFocused->GetDlgCtrlID() == editSample.GetDlgCtrlID()) {
				updateSample();
			}
			else if (pFocused->GetDlgCtrlID() == editDioptSel.GetDlgCtrlID()) {
				updateDioptSel();
			}
			else if (pFocused->GetDlgCtrlID() == editOctCompen.GetDlgCtrlID()) {
				updateOctCompen();
			}
			else if (pFocused->GetDlgCtrlID() == editSplitFocus.GetDlgCtrlID()) {
				updateSplitFocus();
			}
			else if (pFocused->GetDlgCtrlID() == editPupilMask.GetDlgCtrlID()) {
				updatePupilMask();
			}
		}
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgMotorsControl::OnReleasedcaptureSliderDioptSel(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editDioptSel.SetWindowTextW(to_wstring(sldDioptSel.GetPos()).c_str());
	updateDioptSel();
	*pResult = 0;
}


void DlgMotorsControl::OnReleasedcaptureSliderFundusFocus(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editFundusFocus.SetWindowTextW(to_wstring(sldFundusFocus.GetPos()).c_str());
	updateFundusFocus();
	*pResult = 0;
}


void DlgMotorsControl::OnReleasedcaptureSliderOctCompen(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editOctCompen.SetWindowTextW(to_wstring(sldOctCompen.GetPos()).c_str());
	updateOctCompen();
	*pResult = 0;
}


void DlgMotorsControl::OnReleasedcaptureSliderOctFocus(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editOctFocus.SetWindowTextW(to_wstring(sldOctFocus.GetPos()).c_str());
	updateOctFocus();
	*pResult = 0;
}


void DlgMotorsControl::OnReleasedcaptureSliderPolar(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editPolar.SetWindowTextW(to_wstring(sldPolar.GetPos()).c_str());
	updatePolar();
	*pResult = 0;
}


void DlgMotorsControl::OnReleasedcaptureSliderRefer(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editRefer.SetWindowTextW(to_wstring(sldRefer.GetPos()).c_str());
	updateRefer();
	*pResult = 0;
}


void DlgMotorsControl::OnReleasedcaptureSliderSample(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editSample.SetWindowTextW(to_wstring(sldSample.GetPos()).c_str());
	updateSample();
	*pResult = 0;
}


void DlgMotorsControl::OnReleasedcaptureSliderSplitFocus(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editSplitFocus.SetWindowTextW(to_wstring(sldSplitFocus.GetPos()).c_str());
	updateSplitFocus();
	*pResult = 0;
}


void DlgMotorsControl::OnReleasedcaptureSliderPupilMask(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editPupilMask.SetWindowTextW(to_wstring(sldPupilMask.GetPos()).c_str());
	updatePupilMask();
	*pResult = 0;
}


void DlgMotorsControl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow) {
		initControls();
	}
}
