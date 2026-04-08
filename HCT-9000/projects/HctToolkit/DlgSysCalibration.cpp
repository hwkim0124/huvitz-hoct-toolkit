// DlgSysCalibration.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgSysCalibration.h"
#include "afxdialogex.h"


// DlgSysCalibration dialog

IMPLEMENT_DYNAMIC(DlgSysCalibration, CDialogEx)

DlgSysCalibration::DlgSysCalibration(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGSYSCALIBRATION, pParent)
{
}


DlgSysCalibration::~DlgSysCalibration()
{
}


void DlgSysCalibration::loadControls(void)
{
	SysCal_st* pst = SystemConfig::getSysCalibData();

	CString text;
	text.Format(_T("%d"), pst->SysSerial.ColorSensorSN);
	editSysSN.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SysSerial.SpectroSN);
	editSpectSN.SetWindowTextW(text);

	//std::string sstr;
	//std::int8_t ascii;
	//for (int i = 0; i < 8; i++) {
	//	ascii = pst->SysSerial.SLD_SN[i];
	//	if (ascii < 0 || ascii > 128) {
	//		ascii = char('0');
	//	}
	//	sstr += ascii;
	//}
	//text = CString(atow(sstr).c_str());
	text.Format(_T("%S"), pst->SysSerial.SLD_SN);
	editSldSN.SetWindowTextW(text);

	text.Format(_T("%d"), pst->REF_RetinaPos);
	editRetPos.SetWindowTextW(text);
	text.Format(_T("%d"), pst->REF_CorneaPos);
	editCorPos.SetWindowTextW(text);
	text.Format(_T("%d"), pst->PolarizationPos);
	editPolPos.SetWindowTextW(text);

	text.Format(_T("%d"), pst->Diopter_Cal.IR_focus_zeroD_pos);
	editIrFocus.SetWindowTextW(text);
	text.Format(_T("%d"), pst->Diopter_Cal.Scan_focus_zeroD_pos);
	editScanFocus.SetWindowTextW(text);
	text.Format(_T("%d"), pst->Diopter_Cal.Scan_focus_topography_pos);
	editTopographyFocus.SetWindowTextW(text);

	text.Format(_T("%.2f"), pst->Galvano_Xcal.Galvano_offset);
	editXOffset.SetWindowTextW(text);
	text.Format(_T("%.2f"), pst->Galvano_Xcal.Galvano_Range);
	editXRange.SetWindowTextW(text);
	text.Format(_T("%.2f"), pst->Galvano_Ycal.Galvano_offset);
	editYOffset.SetWindowTextW(text);
	text.Format(_T("%.2f"), pst->Galvano_Ycal.Galvano_Range);
	editYRange.SetWindowTextW(text);

	text.Format(_T("%d"), pst->QuickReturnMirrorCal.InPos);
	editQuickIn.SetWindowTextW(text);
	text.Format(_T("%d"), pst->QuickReturnMirrorCal.OutPos);
	editQuickOut.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SplitFocusMirrorCal.InPos);
	editSplitIn.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SplitFocusMirrorCal.OutPos);
	editSplitOut.SetWindowTextW(text);

	text.Format(_T("%d"), pst->PupilMaskCal.NormalPupilPos);
	editPupNormal.SetWindowTextW(text);
	text.Format(_T("%d"), pst->PupilMaskCal.SmallPupilPos);
	editPupSmall.SetWindowTextW(text);

	text.Format(_T("%d"), pst->IntFixationOffset.xoffset);
	editIntFixOffsetX.SetWindowTextW(text);
	text.Format(_T("%d"), pst->IntFixationOffset.yoffset);
	editIntFixOffsetY.SetWindowTextW(text);

	text.Format(_T("%d"), pst->FdiopterCompLensPosCal.MinusLensPos);
	editFunMinusLens.SetWindowTextW(text);
	text.Format(_T("%d"), pst->FdiopterCompLensPosCal.PlusLensPos);
	editFunPlusLens.SetWindowTextW(text);
	text.Format(_T("%d"), pst->FdiopterCompLensPosCal.NoLensPos);
	editFunNoLens.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SdiopterCompLensPosCal.MinusLensPos);
	editScanMinusLens.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SdiopterCompLensPosCal.PlusLensPos);
	editScanPlusLens.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SdiopterCompLensPosCal.NoLensPos);
	editScanNoLens.SetWindowTextW(text);

	text.Format(_T("%lf"), pst->RetinaDispersion.a1);
	editRetDisp1.SetWindowTextW(text);
	text.Format(_T("%lf"), pst->RetinaDispersion.a2);
	editRetDisp2.SetWindowTextW(text);
	text.Format(_T("%lf"), pst->RetinaDispersion.a3);
	editRetDisp3.SetWindowTextW(text);

	text.Format(_T("%lf"), pst->CorneaDispersion.a1);
	editCorDisp1.SetWindowTextW(text);
	text.Format(_T("%lf"), pst->CorneaDispersion.a2);
	editCorDisp2.SetWindowTextW(text);
	text.Format(_T("%lf"), pst->CorneaDispersion.a3);
	editCorDisp3.SetWindowTextW(text);

	text.Format(_T("%e"), pst->SpectroCal.a1);
	editSpectCal1.SetWindowTextW(text);
	text.Format(_T("%e"), pst->SpectroCal.a2);
	editSpectCal2.SetWindowTextW(text);
	text.Format(_T("%e"), pst->SpectroCal.a3);
	editSpectCal3.SetWindowTextW(text);
	text.Format(_T("%e"), pst->SpectroCal.a4);
	editSpectCal4.SetWindowTextW(text);

	text.Format(_T("%d"), pst->SLD_Param.IM_MAX);
	editImMax.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SLD_Param.IM_MIN);
	editImMin.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SLD_Param.IS_MAX);
	editIsMax.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SLD_Param.IS_MIN);
	editIsMin.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SLD_Param.DN_REF_EPD_MAX);
	editEpdMax.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SLD_Param.DN_REF_EPD_MIN);
	editEpdMin.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SLD_Param.RmonHighCode);
	editSldHigh.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SLD_Param.RmonLowCode1);
	editSldLow1.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SLD_Param.RmonLowCode2);
	editSldLow2.SetWindowTextW(text);
	text.Format(_T("%d"), pst->SLD_Param.RsiCode);
	editSldRsi.SetWindowTextW(text);

	text.Format(_T("%d"), pst->IRCamParam.CorneaAgain);
	editCorAgain.SetWindowTextW(text);
	text.Format(_T("%d"), pst->IRCamParam.CorneaDgain);
	editCorDgain.SetWindowTextW(text);
	text.Format(_T("%d"), pst->IRCamParam.RetinaAgain);
	editRetAgain.SetWindowTextW(text);
	text.Format(_T("%d"), pst->IRCamParam.RetinaDgain);
	editRetDgain.SetWindowTextW(text);

	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_x.duty_base);
	editXBase.SetWindowTextW(text);
	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_x.duty_fcw);
	editXFcw.SetWindowTextW(text);
	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_x.duty_fccw);
	editXFccw.SetWindowTextW(text);
	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_x.duty_ncw);
	editXNcw.SetWindowTextW(text);
	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_x.duty_nccw);
	editXNccw.SetWindowTextW(text);
	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_x.enc_near);
	editXEncNear.SetWindowTextW(text);
	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_x.enc_offs);
	editXEncOffset.SetWindowTextW(text);

	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_z.duty_base);
	editYBase.SetWindowTextW(text);
	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_z.duty_fcw);
	editYFcw.SetWindowTextW(text);
	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_z.duty_fccw);
	editYFccw.SetWindowTextW(text);
	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_z.duty_ncw);
	editYNcw.SetWindowTextW(text);
	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_z.duty_nccw);
	editYNccw.SetWindowTextW(text);
	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_z.enc_near);
	editYEncNear.SetWindowTextW(text);
	text.Format(_T("%d"), pst->AutoStage_Cal.dcm_setup_z.enc_offs);
	editYEncOffset.SetWindowTextW(text);

	text.Format(_T("%d"), pst->chksum);
	editChecksum.SetWindowTextW(text);

	bool flag = Controller::getMainBoard()->isUartDebugEnabled();
	CheckDlgButton(IDC_CHECK_UART_DEBUG, flag);
	return;
}


void DlgSysCalibration::saveControls(void)
{
	SysCal_st* pst = SystemConfig::getSysCalibData();

	CString text;
	editSysSN.GetWindowTextW(text);
	pst->SysSerial.ColorSensorSN = _ttoi(text);
	editSpectSN.GetWindowTextW(text);
	pst->SysSerial.SpectroSN = _ttoi(text);
	editSldSN.GetWindowTextW(text);

	std::wstring wstr(text);
	if (!wstr.empty()) {
		std::string sstr = wtoa(wstr);
		int size = min((int)sstr.size(), 8);
		for (int i = 0; i < size; i++) {
			pst->SysSerial.SLD_SN[i] = sstr[i];
		}
	}
	else {
		for (int i = 0; i < 8; i++) {
			pst->SysSerial.SLD_SN[i] = ' ';
		}
	}

	editRetPos.GetWindowTextW(text);
	pst->REF_RetinaPos = _ttoi(text);
	editCorPos.GetWindowTextW(text);
	pst->REF_CorneaPos = _ttoi(text);
	editPolPos.GetWindowTextW(text);
	pst->PolarizationPos = _ttoi(text);

	editIrFocus.GetWindowTextW(text);
	pst->Diopter_Cal.IR_focus_zeroD_pos = _ttoi(text);
	editScanFocus.GetWindowTextW(text);
	pst->Diopter_Cal.Scan_focus_zeroD_pos = _ttoi(text);
	editTopographyFocus.GetWindowTextW(text);
	pst->Diopter_Cal.Scan_focus_topography_pos = _ttoi(text);

	editXOffset.GetWindowTextW(text);
	pst->Galvano_Xcal.Galvano_offset = (float)_ttof(text);
	editXRange.GetWindowTextW(text);
	pst->Galvano_Xcal.Galvano_Range = (float)_ttof(text);
	editYOffset.GetWindowTextW(text);
	pst->Galvano_Ycal.Galvano_offset = (float)_ttof(text);
	editYRange.GetWindowTextW(text);
	pst->Galvano_Ycal.Galvano_Range = (float)_ttof(text);

	editQuickIn.GetWindowTextW(text);
	pst->QuickReturnMirrorCal.InPos = _ttoi(text);
	editQuickOut.GetWindowTextW(text);
	pst->QuickReturnMirrorCal.OutPos = _ttoi(text);
	editSplitIn.GetWindowTextW(text);
	pst->SplitFocusMirrorCal.InPos = _ttoi(text);
	editSplitOut.GetWindowTextW(text);
	pst->SplitFocusMirrorCal.OutPos = _ttoi(text);

	editPupNormal.GetWindowTextW(text);
	pst->PupilMaskCal.NormalPupilPos = _ttoi(text);
	editPupSmall.GetWindowTextW(text);
	pst->PupilMaskCal.SmallPupilPos = _ttoi(text);

	editIntFixOffsetX.GetWindowTextW(text);
	pst->IntFixationOffset.xoffset = _ttoi(text);
	editIntFixOffsetY.GetWindowTextW(text);
	pst->IntFixationOffset.yoffset = _ttoi(text);

	editFunMinusLens.GetWindowTextW(text);
	pst->FdiopterCompLensPosCal.MinusLensPos = _ttoi(text);
	editFunPlusLens.GetWindowTextW(text);
	pst->FdiopterCompLensPosCal.PlusLensPos = _ttoi(text);
	editFunNoLens.GetWindowTextW(text);
	pst->FdiopterCompLensPosCal.NoLensPos = _ttoi(text);
	editScanMinusLens.GetWindowTextW(text);
	pst->SdiopterCompLensPosCal.MinusLensPos = _ttoi(text);
	editScanPlusLens.GetWindowTextW(text);
	pst->SdiopterCompLensPosCal.PlusLensPos = _ttoi(text);
	editScanNoLens.GetWindowTextW(text);
	pst->SdiopterCompLensPosCal.NoLensPos = _ttoi(text);

	editRetDisp1.GetWindowTextW(text);
	pst->RetinaDispersion.a1 = _ttof(text);
	editRetDisp2.GetWindowTextW(text);
	pst->RetinaDispersion.a2 = _ttof(text);
	editRetDisp3.GetWindowTextW(text);
	pst->RetinaDispersion.a3 = _ttof(text);

	editCorDisp1.GetWindowTextW(text);
	pst->CorneaDispersion.a1 = _ttof(text);
	editCorDisp2.GetWindowTextW(text);
	pst->CorneaDispersion.a2 = _ttof(text);
	editCorDisp3.GetWindowTextW(text);
	pst->CorneaDispersion.a3 = _ttof(text);

	editSpectCal1.GetWindowTextW(text);
	pst->SpectroCal.a1 = _ttof(text);
	editSpectCal2.GetWindowTextW(text);
	pst->SpectroCal.a2 = _ttof(text);
	editSpectCal3.GetWindowTextW(text);
	pst->SpectroCal.a3 = _ttof(text);
	editSpectCal4.GetWindowTextW(text);
	pst->SpectroCal.a4 = _ttof(text);

	editImMax.GetWindowTextW(text);
	pst->SLD_Param.IM_MAX = _ttoi(text);
	editImMin.GetWindowTextW(text);
	pst->SLD_Param.IM_MIN = _ttoi(text);
	editIsMax.GetWindowTextW(text);
	pst->SLD_Param.IS_MAX = _ttoi(text);
	editIsMin.GetWindowTextW(text);
	pst->SLD_Param.IS_MIN = _ttoi(text);
	editEpdMax.GetWindowTextW(text);
	pst->SLD_Param.DN_REF_EPD_MAX = _ttoi(text);
	editEpdMin.GetWindowTextW(text);
	pst->SLD_Param.DN_REF_EPD_MIN = _ttoi(text);
	editSldHigh.GetWindowTextW(text);
	pst->SLD_Param.RmonHighCode = _ttoi(text);
	editSldLow1.GetWindowTextW(text);
	pst->SLD_Param.RmonLowCode1 = _ttoi(text);
	editSldLow2.GetWindowTextW(text);
	pst->SLD_Param.RmonLowCode2 = _ttoi(text);
	editSldRsi.GetWindowTextW(text);
	pst->SLD_Param.RsiCode = _ttoi(text);

	editCorAgain.GetWindowTextW(text);
	pst->IRCamParam.CorneaAgain = _ttoi(text);
	editCorDgain.GetWindowTextW(text);
	pst->IRCamParam.CorneaDgain = _ttoi(text);
	editRetAgain.GetWindowTextW(text);
	pst->IRCamParam.RetinaAgain = _ttoi(text);
	editRetDgain.GetWindowTextW(text);
	pst->IRCamParam.RetinaDgain = _ttoi(text);

	editXBase.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_x.duty_base = _ttoi(text);
	editXFcw.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_x.duty_fcw = _ttoi(text);
	editXFccw.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_x.duty_fccw = _ttoi(text);
	editXNcw.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_x.duty_ncw = _ttoi(text);
	editXNccw.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_x.duty_nccw = _ttoi(text);
	editXEncNear.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_x.enc_near = _ttoi(text);
	editXEncOffset.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_x.enc_offs = _ttoi(text);

	editYBase.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_z.duty_base = _ttoi(text);
	editYFcw.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_z.duty_fcw = _ttoi(text);
	editYFccw.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_z.duty_fccw = _ttoi(text);
	editYNcw.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_z.duty_ncw = _ttoi(text);
	editYNccw.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_z.duty_nccw = _ttoi(text);
	editYEncNear.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_z.enc_near = _ttoi(text);
	editYEncOffset.GetWindowTextW(text);
	pst->AutoStage_Cal.dcm_setup_z.enc_offs = _ttoi(text);
	
	return;
}


void DlgSysCalibration::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_COR_AGAIN, editCorAgain);
	DDX_Control(pDX, IDC_EDIT_COR_DGAIN, editCorDgain);
	DDX_Control(pDX, IDC_EDIT_COR_DISP1, editCorDisp1);
	DDX_Control(pDX, IDC_EDIT_COR_DISP2, editCorDisp2);
	DDX_Control(pDX, IDC_EDIT_COR_DISP3, editCorDisp3);
	DDX_Control(pDX, IDC_EDIT_COR_POS, editCorPos);
	DDX_Control(pDX, IDC_EDIT_FUN_MINUS_LENS, editFunMinusLens);
	DDX_Control(pDX, IDC_EDIT_FUN_NO_LENS, editFunNoLens);
	DDX_Control(pDX, IDC_EDIT_FUN_PLUS_LENS, editFunPlusLens);
	DDX_Control(pDX, IDC_EDIT_IR_FOCUS, editIrFocus);
	DDX_Control(pDX, IDC_EDIT_PUP_NORMAL, editPupNormal);
	DDX_Control(pDX, IDC_EDIT_PUP_SMALL, editPupSmall);
	DDX_Control(pDX, IDC_EDIT_QUICK_IN, editQuickIn);
	DDX_Control(pDX, IDC_EDIT_QUICK_OUT, editQuickOut);
	DDX_Control(pDX, IDC_EDIT_RET_AGAIN, editRetAgain);
	DDX_Control(pDX, IDC_EDIT_RET_DGAIN, editRetDgain);
	DDX_Control(pDX, IDC_EDIT_RET_DISP1, editRetDisp1);
	DDX_Control(pDX, IDC_EDIT_RET_DISP2, editRetDisp2);
	DDX_Control(pDX, IDC_EDIT_RET_DISP3, editRetDisp3);
	DDX_Control(pDX, IDC_EDIT_RET_POS, editRetPos);
	DDX_Control(pDX, IDC_EDIT_SCAN_FOCUS, editScanFocus);
	DDX_Control(pDX, IDC_EDIT_TOPOGRAPHY_FOCUS, editTopographyFocus);
	DDX_Control(pDX, IDC_EDIT_SCAN_MINUS_LENS, editScanMinusLens);
	DDX_Control(pDX, IDC_EDIT_SCAN_NO_LENS, editScanNoLens);
	DDX_Control(pDX, IDC_EDIT_SCAN_PLUS_LENS, editScanPlusLens);
	DDX_Control(pDX, IDC_EDIT_SLD_EPD_MAX, editEpdMax);
	DDX_Control(pDX, IDC_EDIT_SLD_EPD_MIN, editEpdMin);
	DDX_Control(pDX, IDC_EDIT_SLD_HIGH, editSldHigh);
	DDX_Control(pDX, IDC_EDIT_SLD_IM_MAX, editImMax);
	DDX_Control(pDX, IDC_EDIT_SLD_IM_MIN, editImMin);
	DDX_Control(pDX, IDC_EDIT_SLD_IS_MAX, editIsMax);
	DDX_Control(pDX, IDC_EDIT_SLD_IS_MIN, editIsMin);
	DDX_Control(pDX, IDC_EDIT_SLD_LOW1, editSldLow1);
	DDX_Control(pDX, IDC_EDIT_SLD_LOW2, editSldLow2);
	DDX_Control(pDX, IDC_EDIT_SLD_RSI, editSldRsi);
	DDX_Control(pDX, IDC_EDIT_SLD_SN, editSldSN);
	DDX_Control(pDX, IDC_EDIT_SPECT_CAL1, editSpectCal1);
	DDX_Control(pDX, IDC_EDIT_SPECT_CAL2, editSpectCal2);
	DDX_Control(pDX, IDC_EDIT_SPECT_CAL3, editSpectCal3);
	DDX_Control(pDX, IDC_EDIT_SPECT_CAL4, editSpectCal4);
	DDX_Control(pDX, IDC_EDIT_SPECT_SN, editSpectSN);
	DDX_Control(pDX, IDC_EDIT_SPLIT_IN, editSplitIn);
	DDX_Control(pDX, IDC_EDIT_SPLIT_OUT, editSplitOut);
	DDX_Control(pDX, IDC_EDIT_SYS_SN, editSysSN);
	DDX_Control(pDX, IDC_EDIT_X_BASE, editXBase);
	DDX_Control(pDX, IDC_EDIT_X_ENC_NEAR, editXEncNear);
	DDX_Control(pDX, IDC_EDIT_X_ENC_OFFSET, editXEncOffset);
	DDX_Control(pDX, IDC_EDIT_X_FCCW, editXFccw);
	DDX_Control(pDX, IDC_EDIT_X_FCW, editXFcw);
	DDX_Control(pDX, IDC_EDIT_X_NCCW, editXNccw);
	DDX_Control(pDX, IDC_EDIT_X_NCW, editXNcw);
	DDX_Control(pDX, IDC_EDIT_X_OFFSET, editXOffset);
	DDX_Control(pDX, IDC_EDIT_X_RANGE, editXRange);
	DDX_Control(pDX, IDC_EDIT_Y_BASE, editYBase);
	DDX_Control(pDX, IDC_EDIT_Y_ENC_NEAR, editYEncNear);
	DDX_Control(pDX, IDC_EDIT_Y_ENC_OFFSET, editYEncOffset);
	DDX_Control(pDX, IDC_EDIT_Y_FCCW, editYFccw);
	DDX_Control(pDX, IDC_EDIT_Y_FCW, editYFcw);
	DDX_Control(pDX, IDC_EDIT_Y_NCCW, editYNccw);
	DDX_Control(pDX, IDC_EDIT_Y_NCW, editYNcw);
	DDX_Control(pDX, IDC_EDIT_Y_OFFSET, editYOffset);
	DDX_Control(pDX, IDC_EDIT_Y_RANGE, editYRange);
	DDX_Control(pDX, IDC_EDIT_CHECKSUM, editChecksum);
	DDX_Control(pDX, IDC_EDIT_POL_POS, editPolPos);
	DDX_Control(pDX, IDC_EDIT_INTFIX_OFFSET_X, editIntFixOffsetX);
	DDX_Control(pDX, IDC_EDIT_INTFIX_OFFSET_Y, editIntFixOffsetY);
}


BEGIN_MESSAGE_MAP(DlgSysCalibration, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &DlgSysCalibration::OnBnClickedButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &DlgSysCalibration::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &DlgSysCalibration::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_SLD_LOAD, &DlgSysCalibration::OnBnClickedButtonSldLoad)
	ON_BN_CLICKED(IDC_BUTTON_SLD_UPDATE, &DlgSysCalibration::OnBnClickedButtonSldUpdate)
	ON_BN_CLICKED(IDC_BUTTON_SLD_GET, &DlgSysCalibration::OnBnClickedButtonSldGet)
	// ON_BN_CLICKED(IDC_BUTTON_UPDATE, &DlgSysCalibration::OnBnClickedButtonUpdate)
	ON_BN_CLICKED(IDC_CHECK_UART_DEBUG, &DlgSysCalibration::OnBnClickedCheckUartDebug)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT2, &DlgSysCalibration::OnBnClickedButtonExport2)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT2, &DlgSysCalibration::OnBnClickedButtonImport2)
	ON_BN_CLICKED(IDC_BUTTON_Factory, &DlgSysCalibration::OnBnClickedButtonFactory)
END_MESSAGE_MAP()


// DlgSysCalibration message handlers


BOOL DlgSysCalibration::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		CWnd* pFocused = GetFocus();
		if (pMsg->wParam == VK_RETURN && pFocused != NULL) {
		}
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


BOOL DlgSysCalibration::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialogEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


BOOL DlgSysCalibration::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	loadControls();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void DlgSysCalibration::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	CDialogEx::OnClose();
}


void DlgSysCalibration::OnBnClickedButtonLoad()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (!Controller::getMainBoard()->loadSystemConfiguration()) {
		AfxMessageBox(_T("Failed to load System calibration!"));
	}
	else {
		Loader::applyConfiguration();
		loadControls();
		AfxMessageBox(_T("System calibration loaded from mainboard!"));
	}
	EndWaitCursor();
	return;
}


void DlgSysCalibration::OnBnClickedButtonUpdate()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	saveControls();
	if (!Controller::getMainBoard()->saveSystemConfiguration(false)) {
		AfxMessageBox(_T("Failed to update System calibration!"));
	}
	EndWaitCursor();
	return;
}


void DlgSysCalibration::OnBnClickedButtonSave()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	saveControls();
	if (!Controller::getMainBoard()->saveSystemConfiguration(true)) {
		AfxMessageBox(_T("Failed to save System calibration!"));
	}
	else {
		AfxMessageBox(_T("System calibration saved to mainboard!"));
	}
	EndWaitCursor();
	return;
}

void DlgSysCalibration::OnBnClickedButtonExport2()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	saveControls();
	if (!Controller::getMainBoard()->exportSystemConfiguration()) {
		AfxMessageBox(_T("Failed to export System calibration!"));
	}
	else {
		AfxMessageBox(_T("System calibration exported to config file!"));
	}
	EndWaitCursor();
	return;
}


void DlgSysCalibration::OnBnClickedButtonImport2()
{
	BeginWaitCursor();
	if (!Controller::getMainBoard()->importSystemConfiguration()) {
		AfxMessageBox(_T("Failed to import System calibration!"));
	}
	else {
		Loader::applyConfiguration();
		loadControls();
		AfxMessageBox(_T("System calibration imported from config file!"));
	}
	EndWaitCursor();
}



void DlgSysCalibration::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);
}


void DlgSysCalibration::OnBnClickedButtonSldLoad()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	OctSldLed* pSldLed = Controller::getMainBoard()->getOctSldLed();
	
	CString text;
	editSldHigh.GetWindowTextW(text);
	pSldLed->setHighCode(_ttoi(text));
	editSldLow1.GetWindowTextW(text);
	pSldLed->setLowCode1(_ttoi(text));
	editSldLow2.GetWindowTextW(text);
	pSldLed->setLowCode2(_ttoi(text));
	editSldRsi.GetWindowTextW(text);
	pSldLed->setRsiCode(_ttoi(text));
	EndWaitCursor();
	return;
}


void DlgSysCalibration::OnBnClickedButtonSldUpdate()
{
	// TODO: Add your control notification handler code here
	OctSldLed* pSldLed = Controller::getMainBoard()->getOctSldLed();

	BeginWaitCursor();
	if (!pSldLed->updateParametersToSldMemory()) {
		AfxMessageBox(_T("Failed to update SLD parameters!"));
	}
	EndWaitCursor();
	return;
}


void DlgSysCalibration::OnBnClickedButtonSldGet()
{
	// TODO: Add your control notification handler code here
	OctSldLed* pSldLed = Controller::getMainBoard()->getOctSldLed();

	BeginWaitCursor();
	if (!pSldLed->fetchParametersToMainMemory()) {
		AfxMessageBox(_T("Failed to fetch SLD parameters!"));
	}
	else {
		if (!Controller::getMainBoard()->loadSystemConfiguration()) {
			AfxMessageBox(_T("Failed to load System calibration!"));
		}
		else {
			loadControls();
		}
	}
	EndWaitCursor();
	return;
}


void DlgSysCalibration::OnBnClickedCheckUartDebug()
{
	// TODO: Add your control notification handler code here
	bool flag = (IsDlgButtonChecked(IDC_CHECK_UART_DEBUG) ? true : false);
	
	if (!Controller::getMainBoard()->setUartDebugEnabled(flag)) {
		AfxMessageBox(_T("Uart debug mode setting failed!"));
	}
	return;
}


void DlgSysCalibration::OnBnClickedButtonFactory()
{
	SystemConfig::resetToDefaultValues();
	loadControls();
	AfxMessageBox(_T("System calibration data has been Reset!"));
	return;
}
