// DlgStageControl.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgStageControl.h"
#include "afxdialogex.h"


using namespace OctSystem;
using namespace OctDevice;
using namespace std;

// DlgStageControl dialog

IMPLEMENT_DYNAMIC(DlgStageControl, CDialogEx)

DlgStageControl::DlgStageControl(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGSTAGECONTROL, pParent)
{

}

DlgStageControl::~DlgStageControl()
{
}


void DlgStageControl::initControls(void)
{
	MainBoard* board = Controller::getMainBoard();
	xMotor = board->getStageXMotor();
	yMotor = board->getStageYMotor();
	zMotor = board->getStageZMotor();

	xMotor->setControls(&sldStageX, &editStageX);
	yMotor->setControls(&sldStageY, &editStageY);
	zMotor->setControls(&sldStageZ, &editStageZ);
	return;
}


void DlgStageControl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_STAGE_X, editStageX);
	DDX_Control(pDX, IDC_EDIT_STAGE_Y, editStageY);
	DDX_Control(pDX, IDC_EDIT_STAGE_Z, editStageZ);
	DDX_Control(pDX, IDC_SLIDER_STAGE_X, sldStageX);
	DDX_Control(pDX, IDC_SLIDER_STAGE_Y, sldStageY);
	DDX_Control(pDX, IDC_SLIDER_STAGE_Z, sldStageZ);
}


BEGIN_MESSAGE_MAP(DlgStageControl, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_STAGE_X, &DlgStageControl::OnReleasedcaptureSliderStageX)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_STAGE_Y, &DlgStageControl::OnReleasedcaptureSliderStageY)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_STAGE_Z, &DlgStageControl::OnReleasedcaptureSliderStageZ)
	ON_BN_CLICKED(IDC_BUTTON_X_STEP1, &DlgStageControl::OnBnClickedButtonXStep1)
	ON_BN_CLICKED(IDC_BUTTON_X_STEP2, &DlgStageControl::OnBnClickedButtonXStep2)
	ON_BN_CLICKED(IDC_BUTTON_X_STEP3, &DlgStageControl::OnBnClickedButtonXStep3)
	ON_BN_CLICKED(IDC_BUTTON_X_STEP4, &DlgStageControl::OnBnClickedButtonXStep4)
	ON_BN_CLICKED(IDC_BUTTON_Y_STEP1, &DlgStageControl::OnBnClickedButtonYStep1)
	ON_BN_CLICKED(IDC_BUTTON_Y_STEP2, &DlgStageControl::OnBnClickedButtonYStep2)
	ON_BN_CLICKED(IDC_BUTTON_Y_STEP3, &DlgStageControl::OnBnClickedButtonYStep3)
	ON_BN_CLICKED(IDC_BUTTON_Y_STEP4, &DlgStageControl::OnBnClickedButtonYStep4)
	ON_BN_CLICKED(IDC_BUTTON_Y_ORIGIN, &DlgStageControl::OnBnClickedButtonYOrigin)
	ON_BN_CLICKED(IDC_BUTTON_Z_STEP1, &DlgStageControl::OnBnClickedButtonZStep1)
	ON_BN_CLICKED(IDC_BUTTON_Z_STEP2, &DlgStageControl::OnBnClickedButtonZStep2)
	ON_BN_CLICKED(IDC_BUTTON_Z_STEP3, &DlgStageControl::OnBnClickedButtonZStep3)
	ON_BN_CLICKED(IDC_BUTTON_Z_STEP4, &DlgStageControl::OnBnClickedButtonZStep4)
	ON_BN_CLICKED(IDC_BUTTON_Z_ORIGIN, &DlgStageControl::OnBnClickedButtonZOrigin)
	ON_BN_CLICKED(IDC_BUTTON_X_ORIGIN, &DlgStageControl::OnBnClickedButtonXOrigin)
END_MESSAGE_MAP()


// DlgStageControl message handlers


void DlgStageControl::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	MainBoard* board = Controller::getMainBoard();
	xMotor = board->getStageXMotor();
	yMotor = board->getStageYMotor();
	zMotor = board->getStageZMotor();

	xMotor->setControls();
	yMotor->setControls();
	zMotor->setControls();

	ShowWindow(SW_HIDE);

	// CDialogEx::OnClose();
}


BOOL DlgStageControl::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgStageControl::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL)
		{
			int ctrlID = pFocused->GetDlgCtrlID();
			if (ctrlID == editStageX.GetDlgCtrlID()) {
				xMotor->updatePositionByEdit();
			}
			else if (ctrlID == editStageY.GetDlgCtrlID()) {
				yMotor->updatePositionByEdit();
			}
			else if (ctrlID == editStageZ.GetDlgCtrlID()) {
				zMotor->updatePositionByEdit();
			}
		}
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgStageControl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow) {
		initControls();
	}
	return;
}


void DlgStageControl::OnReleasedcaptureSliderStageX(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	xMotor->updatePosition(sldStageX.GetPos());
	*pResult = 0;
}


void DlgStageControl::OnReleasedcaptureSliderStageY(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	yMotor->updatePosition(sldStageY.GetPos());
	*pResult = 0;
}


void DlgStageControl::OnReleasedcaptureSliderStageZ(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	zMotor->updatePosition(sldStageZ.GetPos());
	*pResult = 0;
}


void DlgStageControl::OnBnClickedButtonXStep1()
{
	// TODO: Add your control notification handler code here
	xMotor->updatePositionByOffset(-5);
}


void DlgStageControl::OnBnClickedButtonXStep2()
{
	// TODO: Add your control notification handler code here
	xMotor->updatePositionByOffset(-1);
}


void DlgStageControl::OnBnClickedButtonXStep3()
{
	// TODO: Add your control notification handler code here
	xMotor->updatePositionByOffset(+1);
}


void DlgStageControl::OnBnClickedButtonXStep4()
{
	// TODO: Add your control notification handler code here
	xMotor->updatePositionByOffset(+5);
}


void DlgStageControl::OnBnClickedButtonXOrigin()
{
	// TODO: Add your control notification handler code here
	xMotor->updatePosition(0);
}


void DlgStageControl::OnBnClickedButtonYStep1()
{
	// TODO: Add your control notification handler code here
	yMotor->updatePositionByOffset(-250);
}


void DlgStageControl::OnBnClickedButtonYStep2()
{
	// TODO: Add your control notification handler code here
	yMotor->updatePositionByOffset(-50);
}


void DlgStageControl::OnBnClickedButtonYStep3()
{
	// TODO: Add your control notification handler code here
	yMotor->updatePositionByOffset(+50);
}


void DlgStageControl::OnBnClickedButtonYStep4()
{
	// TODO: Add your control notification handler code here
	yMotor->updatePositionByOffset(+250);
}


void DlgStageControl::OnBnClickedButtonYOrigin()
{
	// TODO: Add your control notification handler code here
	yMotor->updatePosition(0);
}


void DlgStageControl::OnBnClickedButtonZStep1()
{
	// TODO: Add your control notification handler code here
	zMotor->updatePositionByOffset(-5);
}


void DlgStageControl::OnBnClickedButtonZStep2()
{
	// TODO: Add your control notification handler code here
	zMotor->updatePositionByOffset(-1);

}


void DlgStageControl::OnBnClickedButtonZStep3()
{
	// TODO: Add your control notification handler code here
	zMotor->updatePositionByOffset(+1);

}


void DlgStageControl::OnBnClickedButtonZStep4()
{
	// TODO: Add your control notification handler code here
	zMotor->updatePositionByOffset(+5);

}


void DlgStageControl::OnBnClickedButtonZOrigin()
{
	// TODO: Add your control notification handler code here
	zMotor->updatePosition(0);

}

