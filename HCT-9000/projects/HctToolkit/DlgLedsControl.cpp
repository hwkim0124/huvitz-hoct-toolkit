// DlgLedsControl.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgLedsControl.h"
#include "afxdialogex.h"
#include "OctSystem2.h"

#include <string>

using namespace OctSystem;
using namespace OctDevice;
using namespace std;
// DlgLedsControl dialog

IMPLEMENT_DYNAMIC(DlgLedsControl, CDialogEx)

DlgLedsControl::DlgLedsControl(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGLEDSCONTROL, pParent)
{

}

DlgLedsControl::~DlgLedsControl()
{
}


void DlgLedsControl::initControls(void)
{
	MainBoard* board = Controller::getMainBoard();
	board->getCorneaIrLed()->setControls(&sldCorneaIR, &editCorneaIR);
	board->getRetinaIrLed()->setControls(&sldRetinaIR, &editRetinaIR);
	board->getKeratoFocusLed()->setControls(&sldKerFocus, &editKerFocus);
	board->getKeratoRingLed()->setControls(&sldKerRing, &editKerRing);
	board->getFundusFlashLed()->setControls(&sldFundusFlash, &editFundusFlash);
	board->getSplitFocusLed()->setControls(&sldSplitFocus, &editSplitFocus);
	board->getWorkingDotsLed()->setControls(&sldWdots, &editWdots);
	board->getExtFixationLed()->setControls(&sldExtFix, &editExtFix);
	board->getOctSldLed()->setControls(&sldOctSld, &editOctSld);
	return;
}


void DlgLedsControl::updateCorneaIr(void)
{
	CString str;
	editCorneaIR.GetWindowText(str);
	if (str.GetLength() > 0) {
		int pos = _ttoi(str);
		Controller::getMainBoard()->getCorneaIrLed()->setIntensity(pos);
		pos = Controller::getMainBoard()->getCorneaIrLed()->getIntensity();
		sldCorneaIR.SetPos(pos);
		str.Format(_T("%d"), pos);
		editCorneaIR.SetWindowTextW(str);
		editCorneaIR.SetSel(0, -1, FALSE);
	}
	return;
}


void DlgLedsControl::updateKerFocus(void)
{
	CString str;
	editKerFocus.GetWindowText(str);
	if (str.GetLength() > 0) {
		int pos = _ttoi(str);
		Controller::getMainBoard()->getKeratoFocusLed()->setIntensity(pos);
		pos = Controller::getMainBoard()->getKeratoFocusLed()->getIntensity();
		sldKerFocus.SetPos(pos);
		str.Format(_T("%d"), pos);
		editKerFocus.SetWindowTextW(str);
		editKerFocus.SetSel(0, -1, FALSE);
	}
	return;
}


void DlgLedsControl::updateKerRing(void)
{
	CString str;
	editKerRing.GetWindowText(str);
	if (str.GetLength() > 0) {
		int pos = _ttoi(str);
		Controller::getMainBoard()->getKeratoRingLed()->setIntensity(pos);
		pos = Controller::getMainBoard()->getKeratoRingLed()->getIntensity();
		sldKerRing.SetPos(pos);
		str.Format(_T("%d"), pos);
		editKerRing.SetWindowTextW(str);
		editKerRing.SetSel(0, -1, FALSE);
	}
	return;
}


void DlgLedsControl::updateRetinaIr(void)
{
	CString str;
	editRetinaIR.GetWindowText(str);
	if (str.GetLength() > 0) {
		int pos = _ttoi(str);
		Controller::getMainBoard()->getRetinaIrLed()->setIntensity(pos);
		pos = Controller::getMainBoard()->getRetinaIrLed()->getIntensity();
		sldRetinaIR.SetPos(pos);
		str.Format(_T("%d"), pos);
		editRetinaIR.SetWindowTextW(str);
		editRetinaIR.SetSel(0, -1, FALSE);
	}
	return;
}


void DlgLedsControl::updateSplitFocus(void)
{
	CString str;
	editSplitFocus.GetWindowText(str);
	if (str.GetLength() > 0) {
		int pos = _ttoi(str);
		Controller::getMainBoard()->getSplitFocusLed()->setIntensity(pos);
		pos = Controller::getMainBoard()->getSplitFocusLed()->getIntensity();
		sldSplitFocus.SetPos(pos);
		str.Format(_T("%d"), pos);
		editSplitFocus.SetWindowTextW(str);
		editSplitFocus.SetSel(0, -1, FALSE);
	}
	return;
}


void DlgLedsControl::updateWorkingDots(void)
{
	CString str;
	editWdots.GetWindowText(str);
	if (str.GetLength() > 0) {
		int pos = _ttoi(str);
		Controller::getMainBoard()->getWorkingDotsLed()->setIntensity(pos);
		pos = Controller::getMainBoard()->getWorkingDotsLed()->getIntensity();
		sldWdots.SetPos(pos);
		str.Format(_T("%d"), pos);
		editWdots.SetWindowTextW(str);
		editWdots.SetSel(0, -1, FALSE);
	}
	return;
}


void DlgLedsControl::updateOctSld(void)
{
	CString str;
	editOctSld.GetWindowText(str);
	if (str.GetLength() > 0) {
		int pos = _ttoi(str);
		Controller::getMainBoard()->getOctSldLed()->setIntensity(pos);
		pos = Controller::getMainBoard()->getOctSldLed()->getIntensity();
		sldOctSld.SetPos(pos);
		str.Format(_T("%d"), pos);
		editOctSld.SetWindowTextW(str);
		editOctSld.SetSel(0, -1, FALSE);
	}
	return;
}


void DlgLedsControl::updateExtFixation(void)
{
	CString str;
	editExtFix.GetWindowText(str);
	if (str.GetLength() > 0) {
		int pos = _ttoi(str);
		Controller::getMainBoard()->getExtFixationLed()->setIntensity(pos);
		pos = Controller::getMainBoard()->getExtFixationLed()->getIntensity();
		sldExtFix.SetPos(pos);
		str.Format(_T("%d"), pos);
		editExtFix.SetWindowTextW(str);
		editExtFix.SetSel(0, -1, FALSE);
	}
	return;
}


void DlgLedsControl::updateFundusFlash(void)
{
	CString str;
	editFundusFlash.GetWindowText(str);
	if (str.GetLength() > 0) {
		int pos = _ttoi(str);
		Controller::getMainBoard()->getFundusFlashLed()->setIntensity(pos);
		pos = Controller::getMainBoard()->getFundusFlashLed()->getIntensity();
		sldFundusFlash.SetPos(pos);
		str.Format(_T("%d"), pos);
		editFundusFlash.SetWindowTextW(str);
		editFundusFlash.SetSel(0, -1, FALSE);
	}
	return;
}


void DlgLedsControl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_EXT_FIX, sldExtFix);
	DDX_Control(pDX, IDC_SLIDER_FUNDUS_FLASH, sldFundusFlash);
	DDX_Control(pDX, IDC_SLIDER_KER_FOCUS, sldKerFocus);
	DDX_Control(pDX, IDC_SLIDER_KER_RING, sldKerRing);
	DDX_Control(pDX, IDC_SLIDER_OCT_SLD, sldOctSld);
	DDX_Control(pDX, IDC_SLIDER_RETINA_IR, sldRetinaIR);
	DDX_Control(pDX, IDC_SLIDER_SPLIT_FOCUS, sldSplitFocus);
	DDX_Control(pDX, IDC_SLIDER_WDOTS, sldWdots);
	DDX_Control(pDX, IDC_SLIDER_CORNEA_IR, sldCorneaIR);
	DDX_Control(pDX, IDC_EDIT_CORNEA_IR, editCorneaIR);
	DDX_Control(pDX, IDC_EDIT_EXT_FIX, editExtFix);
	DDX_Control(pDX, IDC_EDIT_FUNDUS_FLASH, editFundusFlash);
	DDX_Control(pDX, IDC_EDIT_KER_FOCUS, editKerFocus);
	DDX_Control(pDX, IDC_EDIT_KER_RING, editKerRing);
	DDX_Control(pDX, IDC_EDIT_OCT_SLD, editOctSld);
	DDX_Control(pDX, IDC_EDIT_RETINA_IR, editRetinaIR);
	DDX_Control(pDX, IDC_EDIT_SPLIT_FOCUS, editSplitFocus);
	DDX_Control(pDX, IDC_EDIT_WDOTS, editWdots);
}


BEGIN_MESSAGE_MAP(DlgLedsControl, CDialogEx)
	ON_WM_CLOSE()
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_CORNEA_IR, &DlgLedsControl::OnReleasedcaptureSliderCorneaIr)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_EXT_FIX, &DlgLedsControl::OnReleasedcaptureSliderExtFix)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_FUNDUS_FLASH, &DlgLedsControl::OnReleasedcaptureSliderFundusFlash)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_KER_FOCUS, &DlgLedsControl::OnReleasedcaptureSliderKerFocus)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_KER_RING, &DlgLedsControl::OnReleasedcaptureSliderKerRing)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_OCT_SLD, &DlgLedsControl::OnReleasedcaptureSliderOctSld)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_RETINA_IR, &DlgLedsControl::OnReleasedcaptureSliderRetinaIr)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SPLIT_FOCUS, &DlgLedsControl::OnReleasedcaptureSliderSplitFocus)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_WDOTS, &DlgLedsControl::OnReleasedcaptureSliderWdots)
ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// DlgLedsControl message handlers


void DlgLedsControl::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	// To recycle the contents of the window after closing. 
	ShowWindow(SW_HIDE);

	// CDialogEx::OnClose();
}


BOOL DlgLedsControl::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void DlgLedsControl::OnReleasedcaptureSliderCorneaIr(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editCorneaIR.SetWindowTextW(to_wstring(sldCorneaIR.GetPos()).c_str());
	updateCorneaIr();
	*pResult = 0;
}


void DlgLedsControl::OnReleasedcaptureSliderExtFix(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editExtFix.SetWindowTextW(to_wstring(sldExtFix.GetPos()).c_str());
	updateExtFixation();
	*pResult = 0;
}


void DlgLedsControl::OnReleasedcaptureSliderFundusFlash(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editFundusFlash.SetWindowTextW(to_wstring(sldFundusFlash.GetPos()).c_str());
	updateFundusFlash();
	*pResult = 0;
}


void DlgLedsControl::OnReleasedcaptureSliderKerFocus(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editKerFocus.SetWindowTextW(to_wstring(sldKerFocus.GetPos()).c_str());
	updateKerFocus();
	*pResult = 0;
}


void DlgLedsControl::OnReleasedcaptureSliderKerRing(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editKerRing.SetWindowTextW(to_wstring(sldKerRing.GetPos()).c_str());
	updateKerRing();
	*pResult = 0;
}


void DlgLedsControl::OnReleasedcaptureSliderOctSld(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editOctSld.SetWindowTextW(to_wstring(sldOctSld.GetPos()).c_str());
	updateOctSld();
	*pResult = 0;
}


void DlgLedsControl::OnReleasedcaptureSliderRetinaIr(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editRetinaIR.SetWindowTextW(to_wstring(sldRetinaIR.GetPos()).c_str());
	updateRetinaIr();
	*pResult = 0;
}


void DlgLedsControl::OnReleasedcaptureSliderSplitFocus(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editSplitFocus.SetWindowTextW(to_wstring(sldSplitFocus.GetPos()).c_str());
	updateSplitFocus();
	*pResult = 0;
}


void DlgLedsControl::OnReleasedcaptureSliderWdots(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	editWdots.SetWindowTextW(to_wstring(sldWdots.GetPos()).c_str());
	updateWorkingDots();
	*pResult = 0;
}


BOOL DlgLedsControl::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)) 
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL) {
			if (pFocused->GetDlgCtrlID() == editCorneaIR.GetDlgCtrlID()) {
				updateCorneaIr();
			}
			else if (pFocused->GetDlgCtrlID() == editExtFix.GetDlgCtrlID()) {
				updateExtFixation();
			}
			else if (pFocused->GetDlgCtrlID() == editFundusFlash.GetDlgCtrlID()) {
				updateFundusFlash();
			}
			else if (pFocused->GetDlgCtrlID() == editKerFocus.GetDlgCtrlID()) {
				updateKerFocus();
			}
			else if (pFocused->GetDlgCtrlID() == editKerRing.GetDlgCtrlID()) {
				updateKerRing();
			}
			else if (pFocused->GetDlgCtrlID() == editOctSld.GetDlgCtrlID()) {
				updateOctSld();
			}
			else if (pFocused->GetDlgCtrlID() == editRetinaIR.GetDlgCtrlID()) {
				updateRetinaIr();
			}
			else if (pFocused->GetDlgCtrlID() == editSplitFocus.GetDlgCtrlID()) {
				updateSplitFocus();
			}
			else if (pFocused->GetDlgCtrlID() == editWdots.GetDlgCtrlID()) {
				updateWorkingDots();
			}
		}

		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}



void DlgLedsControl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow) {
		initControls();
	}
}
