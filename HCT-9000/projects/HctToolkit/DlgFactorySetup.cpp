// DlgFactorySetup.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgFactorySetup.h"
#include "afxdialogex.h"

#include "OctSystem2.h"
#include "OctDevice2.h"
#include "OctConfig2.h"
#include "CppUtil2.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;

// DlgFactorySetup dialog

IMPLEMENT_DYNAMIC(DlgFactorySetup, CDialogEx)

DlgFactorySetup::DlgFactorySetup(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGFACTORYSETUP, pParent)
{

}


DlgFactorySetup::~DlgFactorySetup()
{
}


void DlgFactorySetup::loadControls(void)
{
	DeviceSettings* dset = SystemProfile::getDeviceSettings();

	CString text;
	text.Format(_T("%d"), dset->getRetinaIrLed()->getValue());
	editRetinaLed.SetWindowText(text);
	text.Format(_T("%d"), dset->getRetinaIrLedBright()->getValue());
	editRetinaLedBright.SetWindowText(text);
	text.Format(_T("%d"), dset->getCorneaIrLed()->getValue());
	editCorneaLed.SetWindowText(text);
	text.Format(_T("%d"), dset->getKeratoRingLed()->getValue());
	editKerRingLed.SetWindowText(text);
	text.Format(_T("%d"), dset->getKeratoFocusLed()->getValue());
	editKerFocusLed.SetWindowText(text);
	text.Format(_T("%d"), dset->getSplitFocusLed()->getValue());
	editSplitLed.SetWindowText(text);
	text.Format(_T("%d"), dset->getWorkingDotsLed()->getValue());
	editWorkingDotsLed.SetWindowText(text);
	text.Format(_T("%d"), dset->getWorkingDot2Led()->getValue());
	editWorkingDot2Led.SetWindowText(text);
	text.Format(_T("%d"), dset->getExtFixationLed()->getValue());
	editExtFixLed.SetWindowTextW(text);

	CameraSettings* cset = SystemProfile::getCameraSettings();
	RetinaSettings* rset = SystemProfile::getRetinaSettings();
	CorneaSettings* aset = SystemProfile::getCorneaSettings();
	FundusSettings* fset = SystemProfile::getFundusSettings();
	FixationSettings* xset = SystemProfile::getFixationSettings();
	SystemSettings* sset = SystemProfile::getSystemSettings();
	PatternSettings* pset = SystemProfile::getPatternSettings();

	// text.Format(_T("%d"), fset->getAutoFlashLevelNormal());
	// editFlashNormal.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashLevelMin());
	editFlashMin.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashLevelMax());
	editFlashMax.SetWindowTextW(text);

	/*
	text.Format(_T("%.2f"), fset->getAutoFlashSizeFactor());
	editFlashFactor.SetWindowTextW(text);
	text.Format(_T("%.2f"), fset->getAutoFlashSizeFactor(1));
	editFlashFactor2.SetWindowTextW(text);
	text.Format(_T("%.2f"), fset->getAutoFlashSizeFactor(2));
	editFlashFactor3.SetWindowTextW(text);
	*/
	text.Format(_T("%d"), fset->getAutoFlashSizeBase(0));
	editFlashBase1.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashSizeBase(1));
	editFlashBase2.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashSizeBase(2));
	editFlashBase3.SetWindowTextW(text);

	text.Format(_T("%d"), fset->getAutoFlashSizeHigh(0));
	editFlashHigh1.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashSizeHigh(1));
	editFlashHigh2.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashSizeHigh(2));
	editFlashHigh3.SetWindowTextW(text);

	text.Format(_T("%d"), fset->getAutoFlashFocusInt(0));
	editFlashInt1.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashFocusInt(1));
	editFlashInt2.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashFocusInt(2));
	editFlashInt3.SetWindowTextW(text);

	//text.Format(_T("%d"), 60); // fset->getAutoFlashDefLevel(0));
	//editFlashLevel1.SetWindowTextW(text);
	//text.Format(_T("%d"), 40); // fset->getAutoFlashDefLevel(1));
	//editFlashLevel2.SetWindowTextW(text);
	//text.Format(_T("%d"), 30); // fset->getAutoFlashDefLevel(2));
	//editFlashLevel3.SetWindowTextW(text);

	text.Format(_T("%d"), fset->getAutoFlashDefLevel(0));
	editFlashLevel1.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashDefLevel(1));
	editFlashLevel2.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashDefLevel(2));
	editFlashLevel3.SetWindowTextW(text);

	text.Format(_T("%d"), fset->getAutoFlashSplitFocus(0));
	editFlashSplit1.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashSplitFocus(1));
	editFlashSplit2.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashSplitFocus(2));
	editFlashSplit3.SetWindowTextW(text);

	text.Format(_T("%d"), fset->getAutoFlashRetinaIr(0));
	editFlashRetina1.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashRetinaIr(1));
	editFlashRetina2.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashRetinaIr(2));
	editFlashRetina3.SetWindowTextW(text);

	text.Format(_T("%d"), fset->getAutoFlashWorkDot1(0));
	editFlashWdot11.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashWorkDot1(1));
	editFlashWdot12.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashWorkDot1(2));
	editFlashWdot13.SetWindowTextW(text);

	text.Format(_T("%d"), fset->getAutoFlashWorkDot2(0));
	editFlashWdot21.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashWorkDot2(1));
	editFlashWdot22.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getAutoFlashWorkDot2(2));
	editFlashWdot23.SetWindowTextW(text);

	CheckDlgButton(IDC_CHECK_FUNDUS_FLASH, fset->useAutoFlashLevel());

	text.Format(_T("%.2f"), pset->retinaPatternScale(0).first);
	editPatScaleX1.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->retinaPatternScale(0).second);
	editPatScaleY1.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->retinaPatternScale(1).first);
	editPatScaleX2.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->retinaPatternScale(1).second);
	editPatScaleY2.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->retinaPatternScale(2).first);
	editPatScaleX3.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->retinaPatternScale(2).second);
	editPatScaleY3.SetWindowTextW(text);

	text.Format(_T("%.2f"), pset->corneaPatternScale(0).first);
	editPatScaleX4.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->corneaPatternScale(0).second);
	editPatScaleY4.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->corneaPatternScale(1).first);
	editPatScaleX5.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->corneaPatternScale(1).second);
	editPatScaleY5.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->corneaPatternScale(2).first);
	editPatScaleX6.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->corneaPatternScale(2).second);
	editPatScaleY6.SetWindowTextW(text);

	text.Format(_T("%.3f"), pset->topographyPatternScale(0).first);
	editPatScaleX7.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternScale(0).second);
	editPatScaleY7.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternScale(1).first);
	editPatScaleX8.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternScale(1).second);
	editPatScaleY8.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternScale(2).first);
	editPatScaleX9.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternScale(2).second);
	editPatScaleY9.SetWindowTextW(text);

	text.Format(_T("%.2f"), pset->retinaPatternOffset(0).first);
	editPatOffsetX1.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->retinaPatternOffset(0).second);
	editPatOffsetY1.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->retinaPatternOffset(1).first);
	editPatOffsetX2.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->retinaPatternOffset(1).second);
	editPatOffsetY2.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->retinaPatternOffset(2).first);
	editPatOffsetX3.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->retinaPatternOffset(2).second);
	editPatOffsetY3.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->corneaPatternOffset(0).first);
	editPatOffsetX4.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->corneaPatternOffset(0).second);
	editPatOffsetY4.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->corneaPatternOffset(1).first);
	editPatOffsetX5.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->corneaPatternOffset(1).second);
	editPatOffsetY5.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->corneaPatternOffset(2).first);
	editPatOffsetX6.SetWindowTextW(text);
	text.Format(_T("%.2f"), pset->corneaPatternOffset(2).second);
	editPatOffsetY6.SetWindowTextW(text);

	text.Format(_T("%.3f"), pset->topographyPatternOffset(0).first);
	editPatOffsetX7.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternOffset(0).second);
	editPatOffsetY7.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternOffset(1).first);
	editPatOffsetX8.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternOffset(1).second);
	editPatOffsetY8.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternOffset(2).first);
	editPatOffsetX9.SetWindowTextW(text);
	text.Format(_T("%.3f"), pset->topographyPatternOffset(2).second);
	editPatOffsetY9.SetWindowTextW(text);

	text.Format(_T("%d"), pset->getReferenceRangeLowerSize());
	editReferLower.SetWindowTextW(text);
	text.Format(_T("%d"), pset->getReferenceRangeUpperSize());
	editReferUpper.SetWindowTextW(text);

	float gain;
	int csel = 0;

	gain = cset->getRetinaCamera()->analogGain();
	csel = (gain >= 10.0f ? 4 : (int)log2(gain));
	cmbRetinaAgain.SetCurSel(csel);
	gain = cset->getRetinaCamera()->analogGain(1);
	csel = (gain >= 10.0f ? 4 : (int)log2(gain));
	cmbRetinaAgain2.SetCurSel(csel);
	gain = cset->getRetinaCamera()->analogGain(2);
	csel = (gain >= 10.0f ? 4 : (int)log2(gain));
	cmbRetinaAgain3.SetCurSel(csel);

	gain = cset->getCorneaCamera()->analogGain();
	csel = (gain >= 10.0f ? 4 : (int)log2(gain));
	cmbCorneaAgain.SetCurSel(csel);
	if (GlobalSettings::useFundusFILR_Enable()) {
		cmbColorAgain.EnableWindow(false);
	}
	else {
		gain = cset->getFundusCamera()->analogGain();
		csel = (gain >= 10.0f ? 4 : (int)log2(gain));
		cmbColorAgain.SetCurSel(csel);
	}

	text.Format(_T("%.1f"), cset->getRetinaCamera()->digitalGain());
	editRetinaDgain.SetWindowTextW(text);
	text.Format(_T("%.1f"), cset->getRetinaCamera()->digitalGain(1));
	editRetinaDgain2.SetWindowTextW(text);
	text.Format(_T("%.1f"), cset->getRetinaCamera()->digitalGain(2));
	editRetinaDgain3.SetWindowTextW(text);

	text.Format(_T("%d"), rset->getCenterPointOfROI().first);
	editRetinaRoiX.SetWindowTextW(text);
	text.Format(_T("%d"), rset->getCenterPointOfROI().second);
	editRetinaRoiY.SetWindowTextW(text);
	text.Format(_T("%d"), rset->getRadiusOfROI());
	editRetinaRoiRadius.SetWindowTextW(text);

	text.Format(_T("%d"), rset->getCenterPointOfMask().first);
	editRetinaMaskCenterX.SetWindowTextW(text);
	text.Format(_T("%d"), rset->getCenterPointOfMask().second);
	editRetinaMaskCenterY.SetWindowTextW(text);
	text.Format(_T("%d"), rset->getCenterMaskSize());
	editRetinaMaskSize.SetWindowTextW(text);

	CheckDlgButton(IDC_CHECK_RETINA_ROI, rset->useROI());
	CheckDlgButton(IDC_CHECK_RETINA_MASK, rset->useCenterMask());

	text.Format(_T("%.1f"), rset->getEnhanceClipLimit());
	editClipLimit.SetWindowTextW(text);
	text.Format(_T("%d"), rset->getEnhanceClipScalar());
	editClipScalar.SetWindowTextW(text);

	text.Format(_T("%.1f"), cset->getCorneaCamera()->digitalGain());
	editCorneaDgain.SetWindowTextW(text);

	text.Format(_T("%d"), rset->getSplitCenter().first);
	editSplitCentX.SetWindowTextW(text);
	text.Format(_T("%d"), rset->getSplitCenter().second);
	editSplitCentY.SetWindowTextW(text);

	text.Format(_T("%d"), aset->getPixelsPerMM());
	editCorneaPixelsMM.SetWindowTextW(text);
	text.Format(_T("%.1f"), aset->getSmallPupilSize());
	editCorneaSmallPupilSize.SetWindowTextW(text);
	 
	text.Format(_T("%.1f"), cset->getFundusCamera()->digitalGain());
	editColorDgain.SetWindowTextW(text);

	text.Format(_T("%d"), fset->getCenterPointOfROI().first);
	editColorRoiX.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCenterPointOfROI().second);
	editColorRoiY.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getRadiusOfROI());
	editColorRoiRadius.SetWindowTextW(text);
	CheckDlgButton(IDC_CHECK_COLOR_ROI, fset->useROICoordinates());

	text.Format(_T("%lf"), fset->getWhiteBalanceParameters()[0]);
	editWbParam1.SetWindowTextW(text);
	text.Format(_T("%lf"), fset->getWhiteBalanceParameters()[1]);
	editWbParam2.SetWindowTextW(text);
	CheckDlgButton(IDC_CHECK_FUNDUS_WB, fset->useWhiteBalance());

	text.Format(_T("%d"), fset->getCorrectionCoordsX()[0]);
	editRadialParamlX1.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsX()[1]);
	editRadialParamlX2.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsX()[2]);
	editRadialParamlX3.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsX()[3]);
	editRadialParamlX4.SetWindowTextW(text);
	CheckDlgButton(IDC_CHECK_FUNDUS_LEVEL, fset->useColorCorrection());

	text.Format(_T("%d"), fset->getCorrectionCoordsX_FILR()[0]);
	editRadialParamlX1_FILR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsX_FILR()[1]);
	editRadialParamlX2_FILR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsX_FILR()[2]);
	editRadialParamlX3_FILR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsX_FILR()[3]);
	editRadialParamlX4_FILR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsX_FILR()[4]);
	editRadialParamlX5_FILR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsX_FILR()[5]);
	editRadialParamlX6_FILR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsX_FILR()[6]);
	editRadialParamlX7_FILR.SetWindowTextW(text);

	text.Format(_T("%.1f"), fset->radialCorrectionRatio());
	editRadialRatio.SetWindowTextW(text);
	CheckDlgButton(IDC_CHECK_FUNDUS_RADIAL, fset->useRadialCorrection());

	text.Format(_T("%.1f"), fset->autoBright());
	editAutoBright.SetWindowTextW(text);
	text.Format(_T("%.1f"), fset->autoContrast());
	editAutoContrast.SetWindowTextW(text);
	text.Format(_T("%.2f"), fset->autoUB());
	editAutoUB.SetWindowTextW(text);
	text.Format(_T("%.2f"), fset->autoVR());
	editAutoVR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->edgeKernelSize());
	editEdgeKernel.SetWindowTextW(text);

	text.Format(_T("%d"), fset->getCorrectionCoordsY()[0]);
	editRadialParamlY1.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsY()[1]);
	editRadialParamlY2.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsY()[2]);
	editRadialParamlY3.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsY()[3]);
	editRadialParamlY4.SetWindowTextW(text);

	text.Format(_T("%d"), fset->getCorrectionCoordsY_FILR()[0]);
	editRadialParamlY1_FILR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsY_FILR()[1]);
	editRadialParamlY2_FILR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsY_FILR()[2]);
	editRadialParamlY3_FILR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsY_FILR()[3]);
	editRadialParamlY4_FILR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsY_FILR()[4]);
	editRadialParamlY5_FILR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsY_FILR()[5]);
	editRadialParamlY6_FILR.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getCorrectionCoordsY_FILR()[6]);
	editRadialParamlY7_FILR.SetWindowTextW(text);

	text.Format(_T("%d"), fset->getRemoveReflection1());
	editRemoveLight1.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getRemoveReflection2());
	editRemoveLight2.SetWindowTextW(text);
	text.Format(_T("%d"), fset->getRemoveReflection3());
	editRemoveLight3.SetWindowTextW(text);

	CheckDlgButton(IDC_CHECK_REMOVE_LIGHT, fset->useRemoveReflectionLight());

	text.Format(_T("%d"), xset->getCenterOD().first);
	editOdFoveaX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getCenterOD().second);
	editOdFoveaY.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getFundusOD().first);
	editOdFundusX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getFundusOD().second);
	editOdFundusY.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getScanDiskOD().first);
	editOdDiscX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getScanDiskOD().second);
	editOdDiscY.SetWindowTextW(text);

	text.Format(_T("%d"), xset->getCenterOS().first);
	editOsFoveaX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getCenterOS().second);
	editOsFoveaY.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getFundusOS().first);
	editOsFundusX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getFundusOS().second);
	editOsFundusY.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getScanDiskOS().first);
	editOsDiscX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getScanDiskOS().second);
	editOsDiscY.SetWindowTextW(text);

	text.Format(_T("%d"), xset->getLeftSideOD(0).first);
	editOdLeftX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getLeftSideOD(0).second);
	editOdLeftY.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getLeftSideOD(1).first);
	editOdLupX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getLeftSideOD(1).second);
	editOdLupY.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getLeftSideOD(2).first);
	editOdLdnX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getLeftSideOD(2).second);
	editOdLdnY.SetWindowTextW(text);

	text.Format(_T("%d"), xset->getLeftSideOS(0).first);
	editOsLeftX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getLeftSideOS(0).second);
	editOsLeftY.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getLeftSideOS(1).first);
	editOsLupX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getLeftSideOS(1).second);
	editOsLupY.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getLeftSideOS(2).first);
	editOsLdnX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getLeftSideOS(2).second);
	editOsLdnY.SetWindowTextW(text);

	text.Format(_T("%d"), xset->getRightSideOD(0).first);
	editOdRightX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getRightSideOD(0).second);
	editOdRightY.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getRightSideOD(1).first);
	editOdRupX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getRightSideOD(1).second);
	editOdRupY.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getRightSideOD(2).first);
	editOdRdnX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getRightSideOD(2).second);
	editOdRdnY.SetWindowTextW(text);

	text.Format(_T("%d"), xset->getRightSideOS(0).first);
	editOsRightX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getRightSideOS(0).second);
	editOsRightY.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getRightSideOS(1).first);
	editOsRupX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getRightSideOS(1).second);
	editOsRupY.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getRightSideOS(2).first);
	editOsRdnX.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getRightSideOS(2).second);
	editOsRdnY.SetWindowTextW(text);

	CheckDlgButton(IDC_CHECK_INT_FIX_BLINK, xset->useLcdBlinkOn());
	text.Format(_T("%d"), xset->getBrightness());
	editFixBright.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getBlinkPeriod());
	editFixPeriod.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getBlinkOnTime());
	editFixOnTime.SetWindowTextW(text);
	text.Format(_T("%d"), xset->getFixationType());
	editIntFixType.SetWindowTextW(text);
	return;
}


void DlgFactorySetup::saveControls(void)
{
	DeviceSettings* dset = SystemProfile::getDeviceSettings();

	CString text;
	int value, x, y;
	float fvalue;

	editRetinaLed.GetWindowText(text);
	value = min(max(_ttoi(text), 0), 100);
	dset->getRetinaIrLed()->setValue(value);
	editRetinaLedBright.GetWindowText(text);
	value = min(max(_ttoi(text), 0), 100);
	dset->getRetinaIrLedBright()->setValue(value);
	editCorneaLed.GetWindowText(text);
	value = min(max(_ttoi(text), 0), 100);
	dset->getCorneaIrLed()->setValue(value);
	editKerRingLed.GetWindowText(text);
	value = min(max(_ttoi(text), 0), 100);
	dset->getKeratoRingLed()->setValue(value);
	editKerFocusLed.GetWindowText(text);
	value = min(max(_ttoi(text), 0), 100);
	dset->getKeratoFocusLed()->setValue(value);
	editSplitLed.GetWindowText(text);
	value = min(max(_ttoi(text), 0), 100);
	dset->getSplitFocusLed()->setValue(value);
	editWorkingDotsLed.GetWindowText(text);
	value = min(max(_ttoi(text), 0), 100);
	dset->getWorkingDotsLed()->setValue(value);
	editWorkingDot2Led.GetWindowText(text);
	value = min(max(_ttoi(text), 0), 100);
	dset->getWorkingDot2Led()->setValue(value);
	editExtFixLed.GetWindowText(text);
	value = min(max(_ttoi(text), 0), 100);
	dset->getExtFixationLed()->setValue(value);

	CameraSettings* cset = SystemProfile::getCameraSettings();
	RetinaSettings* rset = SystemProfile::getRetinaSettings();
	CorneaSettings* aset = SystemProfile::getCorneaSettings();
	FundusSettings* fset = SystemProfile::getFundusSettings();
	FixationSettings* xset = SystemProfile::getFixationSettings();
	SystemSettings* sset = SystemProfile::getSystemSettings();
	PatternSettings* pset = SystemProfile::getPatternSettings();

	// editFlashNormal.GetWindowText(text);
	// value = min(max(_ttoi(text), 0), 100);
	fset->setAutoFlashLevelNormal(value);
	editFlashMin.GetWindowText(text);
	value = min(max(_ttoi(text), 0), 100);
	fset->setAutoFlashLevelMin(value);
	editFlashMax.GetWindowText(text);
	value = min(max(_ttoi(text), 0), 100);
	fset->setAutoFlashLevelMax(value);

	/*
	editFlashFactor.GetWindowText(text);
	fvalue = min(max((float)_ttof(text), 0.0f), 10.0f);
	fset->setAutoFlashSizeFactor(fvalue);
	editFlashFactor2.GetWindowText(text);
	fvalue = min(max((float)_ttof(text), 0.0f), 10.0f);
	fset->setAutoFlashSizeFactor(fvalue, 1);
	editFlashFactor3.GetWindowText(text);
	fvalue = min(max((float)_ttof(text), 0.0f), 10.0f);
	fset->setAutoFlashSizeFactor(fvalue, 2);
	*/

	editFlashBase1.GetWindowTextW(text);
	fset->setAutoFlashSizeBase(_ttoi(text));
	editFlashBase2.GetWindowTextW(text);
	fset->setAutoFlashSizeBase(_ttoi(text), 1);
	editFlashBase3.GetWindowTextW(text);
	fset->setAutoFlashSizeBase(_ttoi(text), 2);

	editFlashHigh1.GetWindowTextW(text);
	fset->setAutoFlashSizeHigh(_ttoi(text));
	editFlashHigh2.GetWindowTextW(text);
	fset->setAutoFlashSizeHigh(_ttoi(text), 1);
	editFlashHigh3.GetWindowTextW(text);
	fset->setAutoFlashSizeHigh(_ttoi(text), 2);

	editFlashInt1.GetWindowTextW(text);
	fset->setAutoFlashFocusInt(_ttoi(text));
	editFlashInt2.GetWindowTextW(text);
	fset->setAutoFlashFocusInt(_ttoi(text), 1);
	editFlashInt3.GetWindowTextW(text);
	fset->setAutoFlashFocusInt(_ttoi(text), 2);

	editFlashLevel1.GetWindowTextW(text);
	fset->setAutoFlashDefLevel(_ttoi(text));
	editFlashLevel2.GetWindowTextW(text);
	fset->setAutoFlashDefLevel(_ttoi(text), 1);
	editFlashLevel3.GetWindowTextW(text);
	fset->setAutoFlashDefLevel(_ttoi(text), 2);

	editFlashSplit1.GetWindowTextW(text);
	fset->setAutoFlashSplitFocus(_ttoi(text), 0);
	editFlashSplit2.GetWindowTextW(text);
	fset->setAutoFlashSplitFocus(_ttoi(text), 1);
	editFlashSplit3.GetWindowTextW(text);
	fset->setAutoFlashSplitFocus(_ttoi(text), 2);

	editFlashRetina1.GetWindowTextW(text);
	fset->setAutoFlashRetinaIr(_ttoi(text), 0);
	editFlashRetina2.GetWindowTextW(text);
	fset->setAutoFlashRetinaIr(_ttoi(text), 1);
	editFlashRetina3.GetWindowTextW(text);
	fset->setAutoFlashRetinaIr(_ttoi(text), 2);

	editFlashWdot11.GetWindowTextW(text);
	fset->setAutoFlashWorkDot1(_ttoi(text), 0);
	editFlashWdot12.GetWindowTextW(text);
	fset->setAutoFlashWorkDot1(_ttoi(text), 1);
	editFlashWdot13.GetWindowTextW(text);
	fset->setAutoFlashWorkDot1(_ttoi(text), 2);

	editFlashWdot21.GetWindowTextW(text);
	fset->setAutoFlashWorkDot2(_ttoi(text), 0);
	editFlashWdot22.GetWindowTextW(text);
	fset->setAutoFlashWorkDot2(_ttoi(text), 1);
	editFlashWdot23.GetWindowTextW(text);
	fset->setAutoFlashWorkDot2(_ttoi(text), 2);

	fset->useAutoFlashLevel(true, IsDlgButtonChecked(IDC_CHECK_FUNDUS_FLASH) ? true : false);

	{
		float x, y;
		editPatScaleX1.GetWindowText(text);
		x = min(max((float)_ttof(text), 0.1f), 2.0f);
		editPatScaleY1.GetWindowText(text);
		y = min(max((float)_ttof(text), 0.1f), 2.0f);
		pset->retinaPatternScale(0) = std::pair<float, float>(x, y);

		editPatScaleX2.GetWindowText(text);
		x = min(max((float)_ttof(text), 0.1f), 2.0f);
		editPatScaleY2.GetWindowText(text);
		y = min(max((float)_ttof(text), 0.1f), 2.0f);
		pset->retinaPatternScale(1) = std::pair<float, float>(x, y);

		editPatScaleX3.GetWindowText(text);
		x = min(max((float)_ttof(text), 0.1f), 2.0f);
		editPatScaleY3.GetWindowText(text);
		y = min(max((float)_ttof(text), 0.1f), 2.0f);
		pset->retinaPatternScale(2) = std::pair<float, float>(x, y);

		editPatScaleX4.GetWindowText(text);
		x = min(max((float)_ttof(text), 0.1f), 2.0f);
		editPatScaleY4.GetWindowText(text);
		y = min(max((float)_ttof(text), 0.1f), 2.0f);
		pset->corneaPatternScale(0) = std::pair<float, float>(x, y);

		editPatScaleX5.GetWindowText(text);
		x = min(max((float)_ttof(text), 0.1f), 2.0f);
		editPatScaleY5.GetWindowText(text);
		y = min(max((float)_ttof(text), 0.1f), 2.0f);
		pset->corneaPatternScale(1) = std::pair<float, float>(x, y);

		editPatScaleX6.GetWindowText(text);
		x = min(max((float)_ttof(text), 0.1f), 2.0f);
		editPatScaleY6.GetWindowText(text);
		y = min(max((float)_ttof(text), 0.1f), 2.0f);
		pset->corneaPatternScale(2) = std::pair<float, float>(x, y);

		editPatScaleX7.GetWindowText(text);
		x = min(max((float)_ttof(text), 0.1f), 2.0f);
		editPatScaleY7.GetWindowText(text);
		y = min(max((float)_ttof(text), 0.1f), 2.0f);
		pset->topographyPatternScale(0) = std::pair<float, float>(x, y);

		editPatScaleX8.GetWindowText(text);
		x = min(max((float)_ttof(text), 0.1f), 2.0f);
		editPatScaleY8.GetWindowText(text);
		y = min(max((float)_ttof(text), 0.1f), 2.0f);
		pset->topographyPatternScale(1) = std::pair<float, float>(x, y);

		editPatScaleX9.GetWindowText(text);
		x = min(max((float)_ttof(text), 0.1f), 2.0f);
		editPatScaleY9.GetWindowText(text);
		y = min(max((float)_ttof(text), 0.1f), 2.0f);
		pset->topographyPatternScale(2) = std::pair<float, float>(x, y);

		editPatOffsetX1.GetWindowText(text);
		x = min(max((float)_ttof(text), -10.0f), +10.0f);
		editPatOffsetY1.GetWindowText(text);
		y = min(max((float)_ttof(text), -10.0f), +10.0f);
		pset->retinaPatternOffset(0) = std::pair<float, float>(x, y);

		editPatOffsetX2.GetWindowText(text);
		x = min(max((float)_ttof(text), -10.0f), +10.0f);
		editPatOffsetY2.GetWindowText(text);
		y = min(max((float)_ttof(text), -10.0f), +10.0f);
		pset->retinaPatternOffset(1) = std::pair<float, float>(x, y);

		editPatOffsetX3.GetWindowText(text);
		x = min(max((float)_ttof(text), -10.0f), +10.0f);
		editPatOffsetY3.GetWindowText(text);
		y = min(max((float)_ttof(text), -10.0f), +10.0f);
		pset->retinaPatternOffset(2) = std::pair<float, float>(x, y);

		editPatOffsetX4.GetWindowText(text);
		x = min(max((float)_ttof(text), -10.0f), +10.0f);
		editPatOffsetY4.GetWindowText(text);
		y = min(max((float)_ttof(text), -10.0f), +10.0f);
		pset->corneaPatternOffset(0) = std::pair<float, float>(x, y);

		editPatOffsetX5.GetWindowText(text);
		x = min(max((float)_ttof(text), -10.0f), +10.0f);
		editPatOffsetY5.GetWindowText(text);
		y = min(max((float)_ttof(text), -10.0f), +10.0f);
		pset->corneaPatternOffset(1) = std::pair<float, float>(x, y);

		editPatOffsetX6.GetWindowText(text);
		x = min(max((float)_ttof(text), -10.0f), +10.0f);
		editPatOffsetY6.GetWindowText(text);
		y = min(max((float)_ttof(text), -10.0f), +10.0f);
		pset->corneaPatternOffset(2) = std::pair<float, float>(x, y);

		editPatOffsetX7.GetWindowText(text);
		x = min(max((float)_ttof(text), -10.0f), +10.0f);
		editPatOffsetY7.GetWindowText(text);
		y = min(max((float)_ttof(text), -10.0f), +10.0f);
		pset->topographyPatternOffset(0) = std::pair<float, float>(x, y);

		editPatOffsetX8.GetWindowText(text);
		x = min(max((float)_ttof(text), -10.0f), +10.0f);
		editPatOffsetY8.GetWindowText(text);
		y = min(max((float)_ttof(text), -10.0f), +10.0f);
		pset->topographyPatternOffset(1) = std::pair<float, float>(x, y);

		editPatOffsetX9.GetWindowText(text);
		x = min(max((float)_ttof(text), -10.0f), +10.0f);
		editPatOffsetY9.GetWindowText(text);
		y = min(max((float)_ttof(text), -10.0f), +10.0f);
		pset->topographyPatternOffset(2) = std::pair<float, float>(x, y);
	}

	editReferLower.GetWindowTextW(text);
	pset->setReferenceRangeLowerSize(_ttoi(text));
	editReferUpper.GetWindowTextW(text);
	pset->setReferenceRangeUpperSize(_ttoi(text));

	float gain;
	int csel = 0;

	csel = cmbRetinaAgain.GetCurSel();
	gain = (float)pow(2, csel);
	gain = (gain > 10.0f ? 10.0f : gain);
	cset->getRetinaCamera()->analogGain() = gain;

	csel = cmbRetinaAgain2.GetCurSel();
	gain = (float)pow(2, csel);
	gain = (gain > 10.0f ? 10.0f : gain);
	cset->getRetinaCamera()->analogGain(1) = gain;

	csel = cmbRetinaAgain3.GetCurSel();
	gain = (float)pow(2, csel);
	gain = (gain > 10.0f ? 10.0f : gain);
	cset->getRetinaCamera()->analogGain(2) = gain;

	csel = cmbCorneaAgain.GetCurSel();
	gain = (float)pow(2, csel);
	gain = (gain > 10.0f ? 10.0f : gain);
	cset->getCorneaCamera()->analogGain() = gain;

	csel = cmbColorAgain.GetCurSel();
	gain = (float)pow(2, csel);
	gain = (gain > 10.0f ? 10.0f : gain);
	cset->getFundusCamera()->analogGain() = gain;

	editRetinaDgain.GetWindowTextW(text);
	fvalue = min(max((float)_ttof(text), 1.0f), 8.0f);
	cset->getRetinaCamera()->digitalGain() = fvalue;

	editRetinaDgain2.GetWindowTextW(text);
	fvalue = min(max((float)_ttof(text), 1.0f), 8.0f);
	cset->getRetinaCamera()->digitalGain(1) = fvalue;

	editRetinaDgain3.GetWindowTextW(text);
	fvalue = min(max((float)_ttof(text), 1.0f), 8.0f);
	cset->getRetinaCamera()->digitalGain(2) = fvalue;

	editRetinaRoiX.GetWindowTextW(text);
	x = min(max(_ttoi(text), 0), 640);
	editRetinaRoiY.GetWindowTextW(text);
	y = min(max(_ttoi(text), 0), 480);
	rset->setCenterPointOfROI(std::pair<int, int>(x, y));
	editRetinaRoiRadius.GetWindowTextW(text);
	value = min(max(_ttoi(text), 0), 480);
	rset->setRadiusOfROI(value);

	editRetinaMaskCenterX.GetWindowTextW(text);
	x = min(max(_ttoi(text), 0), 640);
	editRetinaMaskCenterY.GetWindowTextW(text);
	y = min(max(_ttoi(text), 0), 480);
	rset->setCenterPointOfMask(std::pair<int, int>(x, y));
	editRetinaMaskSize.GetWindowTextW(text);
	rset->setCenterMaskSize(_ttoi(text));

	rset->useROI(true, IsDlgButtonChecked(IDC_CHECK_RETINA_ROI) ? true : false);
	rset->useCenterMask(true, IsDlgButtonChecked(IDC_CHECK_RETINA_MASK) ? true : false);

	editClipLimit.GetWindowTextW(text);
	rset->setEnhanceClipLimit((float)_ttof(text));
	editClipScalar.GetWindowTextW(text);
	rset->setEnhanceClipScalar(_ttoi(text));

	editCorneaDgain.GetWindowTextW(text);
	fvalue = min(max((float)_ttof(text), 1.0f), 8.0f);
	cset->getCorneaCamera()->digitalGain() = fvalue;

	editSplitCentX.GetWindowTextW(text);
	x = min(max(_ttoi(text), 0), 640);
	editSplitCentY.GetWindowTextW(text);
	y = min(max(_ttoi(text), 0), 480);
	rset->setSplitCenter(std::pair<int, int>(x, y));

	editCorneaPixelsMM.GetWindowTextW(text);
	value = min(max(_ttoi(text), CORNEA_CAMERA_PIXELS_PER_MM_MIN), CORNEA_CAMERA_PIXELS_PER_MM_MAX);
	aset->setPixelsPerMM(value);
	editCorneaSmallPupilSize.GetWindowTextW(text);
	fvalue = min(max((float)_ttof(text), CORNEA_SMALL_PUPIL_SIZE_MIN), CORNEA_SMALL_PUPIL_SIZE_MAX);
	aset->setSmallPupilSize(fvalue);

	editColorDgain.GetWindowTextW(text);
	fvalue = min(max((float)_ttof(text), 1.0f), 8.0f);
	cset->getFundusCamera()->digitalGain() = fvalue;

	editColorRoiX.GetWindowTextW(text);
	x = min(max(_ttoi(text), 0), 3840);
	editColorRoiY.GetWindowTextW(text);
	y = min(max(_ttoi(text), 0), 2440);
	fset->setCenterPointOfROI(std::pair<int, int>(x, y));
	editColorRoiRadius.GetWindowTextW(text);
	value = min(max(_ttoi(text), 0), 2440);
	fset->setRadiusOfROI(value);

	fset->useROICoordinates(true, IsDlgButtonChecked(IDC_CHECK_COLOR_ROI) ? true : false);

	auto wbs = std::vector<double>();
	editWbParam1.GetWindowTextW(text);
	wbs.push_back(_ttof(text));
	editWbParam2.GetWindowTextW(text);
	wbs.push_back(_ttof(text));
	fset->setWhiteBalanceParameters(wbs);

	fset->useWhiteBalance(true, IsDlgButtonChecked(IDC_CHECK_FUNDUS_WB) ? true : false);

	auto radXs = std::vector<int>();
	editRadialParamlX1.GetWindowTextW(text);
	radXs.push_back(_ttoi(text));
	editRadialParamlX2.GetWindowTextW(text);
	radXs.push_back(_ttoi(text));
	editRadialParamlX3.GetWindowTextW(text);
	radXs.push_back(_ttoi(text));
	editRadialParamlX4.GetWindowTextW(text);
	radXs.push_back(_ttoi(text));
	fset->setCorrectionCoordsX(radXs);

	auto radYs = std::vector<int>();
	editRadialParamlY1.GetWindowTextW(text);
	radYs.push_back(_ttoi(text));
	editRadialParamlY2.GetWindowTextW(text);
	radYs.push_back(_ttoi(text));
	editRadialParamlY3.GetWindowTextW(text);
	radYs.push_back(_ttoi(text));
	editRadialParamlY4.GetWindowTextW(text);
	radYs.push_back(_ttoi(text));
	fset->setCorrectionCoordsY(radYs);

	fset->useColorCorrection(true, IsDlgButtonChecked(IDC_CHECK_FUNDUS_LEVEL) ? true : false);

	auto radXs_FILR = std::vector<int>();
	editRadialParamlX1_FILR.GetWindowTextW(text);
	radXs_FILR.push_back(_ttoi(text));
	editRadialParamlX2_FILR.GetWindowTextW(text);
	radXs_FILR.push_back(_ttoi(text));
	editRadialParamlX3_FILR.GetWindowTextW(text);
	radXs_FILR.push_back(_ttoi(text));
	editRadialParamlX4_FILR.GetWindowTextW(text);
	radXs_FILR.push_back(_ttoi(text));
	editRadialParamlX5_FILR.GetWindowTextW(text);
	radXs_FILR.push_back(_ttoi(text));
	editRadialParamlX6_FILR.GetWindowTextW(text);
	radXs_FILR.push_back(_ttoi(text));
	editRadialParamlX7_FILR.GetWindowTextW(text);
	radXs_FILR.push_back(_ttoi(text));
	fset->setCorrectionCoordsX_FILR(radXs_FILR);

	auto radYs_FILR = std::vector<int>();
	editRadialParamlY1_FILR.GetWindowTextW(text);
	radYs_FILR.push_back(_ttoi(text));
	editRadialParamlY2_FILR.GetWindowTextW(text);
	radYs_FILR.push_back(_ttoi(text));
	editRadialParamlY3_FILR.GetWindowTextW(text);
	radYs_FILR.push_back(_ttoi(text));
	editRadialParamlY4_FILR.GetWindowTextW(text);
	radYs_FILR.push_back(_ttoi(text));
	editRadialParamlY5_FILR.GetWindowTextW(text);
	radYs_FILR.push_back(_ttoi(text));
	editRadialParamlY6_FILR.GetWindowTextW(text);
	radYs_FILR.push_back(_ttoi(text));
	editRadialParamlY7_FILR.GetWindowTextW(text);
	radYs_FILR.push_back(_ttoi(text));
	fset->setCorrectionCoordsY_FILR(radYs_FILR);

	editRadialRatio.GetWindowTextW(text);
	fvalue = min(max((float)_ttof(text), 0.0f), 1.0f);
	fset->setRadialCorrectionRatio(fvalue);

	fset->useRadialCorrection(true, IsDlgButtonChecked(IDC_CHECK_FUNDUS_RADIAL) ? true : false);

	editRemoveLight1.GetWindowTextW(text);
	fset->setRemoveReflection1(_ttoi(text));
	editRemoveLight2.GetWindowTextW(text);
	fset->setRemoveReflection2(_ttoi(text));
	editRemoveLight3.GetWindowTextW(text);
	fset->setRemoveReflection3(_ttoi(text));

	fset->useRemoveReflectionLight(true, IsDlgButtonChecked(IDC_CHECK_REMOVE_LIGHT) ? true : false);

	editAutoBright.GetWindowTextW(text);
	fvalue = min(max((float)_ttof(text), -200.0f), 200.0f);
	fset->setAutoBright(fvalue);
	editAutoContrast.GetWindowTextW(text);
	fvalue = min(max((float)_ttof(text), -200.0f), 200.0f);
	fset->setAutoContrast(fvalue);
	editAutoUB.GetWindowTextW(text);
	fvalue = min(max((float)_ttof(text), -10.0f), 10.0f);
	fset->setAutoUB(fvalue);
	editAutoVR.GetWindowTextW(text);
	fvalue = min(max((float)_ttof(text), -10.0f), 10.0f);
	fset->setAutoVR(fvalue);

	editEdgeKernel.GetWindowTextW(text);
	value = min(max(_ttoi(text), 0), 99);
	fset->setEdgeKernelSize(value);

	int row, col;
	editOdFoveaX.GetWindowTextW(text);
	row = min(max(_ttoi(text), 0), 199);
	editOdFoveaY.GetWindowTextW(text);
	col = min(max(_ttoi(text), 0), 199);
	xset->setCenterOD(std::pair<int, int>(row, col));
	editOdFundusX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOdFundusY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setFundusOD(std::pair<int, int>(row, col));
	editOdDiscX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOdDiscY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setScanDiskOD(std::pair<int, int>(row, col));

	editOsFoveaX.GetWindowTextW(text);
	row = min(max(_ttoi(text), 0), 199);
	editOsFoveaY.GetWindowTextW(text);
	col = min(max(_ttoi(text), 0), 199);
	xset->setCenterOS(std::pair<int, int>(row, col));
	editOsFundusX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOsFundusY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setFundusOS(std::pair<int, int>(row, col));
	editOsDiscX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOsDiscY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setScanDiskOS(std::pair<int, int>(row, col));

	editOdLeftX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOdLeftY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setLeftSideOD(0, std::pair<int, int>(row, col));
	editOdLupX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOdLupY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setLeftSideOD(1, std::pair<int, int>(row, col));
	editOdLdnX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOdLdnY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setLeftSideOD(2, std::pair<int, int>(row, col));

	editOsLeftX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOsLeftY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setLeftSideOS(0, std::pair<int, int>(row, col));
	editOsLupX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOsLupY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setLeftSideOS(1, std::pair<int, int>(row, col));
	editOsLdnX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOsLdnY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setLeftSideOS(2, std::pair<int, int>(row, col));

	editOdRightX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOdRightY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setRightSideOD(0, std::pair<int, int>(row, col));
	editOdRupX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOdRupY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setRightSideOD(1, std::pair<int, int>(row, col));
	editOdRdnX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOdRdnY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setRightSideOD(2, std::pair<int, int>(row, col));

	editOsRightX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOsRightY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setRightSideOS(0, std::pair<int, int>(row, col));
	editOsRupX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOsRupY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setRightSideOS(1, std::pair<int, int>(row, col));
	editOsRdnX.GetWindowTextW(text);
	row = min(max(_ttoi(text), -199), 199);
	editOsRdnY.GetWindowTextW(text);
	col = min(max(_ttoi(text), -199), 199);
	xset->setRightSideOS(2, std::pair<int, int>(row, col));

	xset->useLcdBlinkOn(true, IsDlgButtonChecked(IDC_CHECK_INT_FIX_BLINK) ? true : false);

	editFixBright.GetWindowTextW(text);
	value = min(max(_ttoi(text), 0), 100);
	xset->setBrightness(value);
	editFixPeriod.GetWindowTextW(text);
	value = min(max(_ttoi(text), 0), 99999);
	xset->setBlinkPeriod(value);
	editFixOnTime.GetWindowTextW(text);
	value = min(max(_ttoi(text), 0), 99999);
	xset->setBlinkOnTime(value);
	editIntFixType.GetWindowTextW(text);
	value = min(max(_ttoi(text), 0), 4);
	xset->setFixationType(value);
	return;
}


void DlgFactorySetup::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Control(pDX, IDC_COMBO_COLOR_AGAIN, cmdColorAgain);
	//  DDX_Control(pDX, IDC_COMBO_CORNEA_AGAIN, cmdCorneaAgain);
	DDX_Control(pDX, IDC_COMBO_RETINA_AGAIN, cmbRetinaAgain);
	DDX_Control(pDX, IDC_EDIT_COLOR_DGAIN, editColorDgain);
	DDX_Control(pDX, IDC_EDIT_COLOR_ROI_CENT_X, editColorRoiX);
	DDX_Control(pDX, IDC_EDIT_COLOR_ROI_CENT_Y, editColorRoiY);
	DDX_Control(pDX, IDC_EDIT_COLOR_ROI_RADIUS, editColorRoiRadius);
	DDX_Control(pDX, IDC_EDIT_CORNEA_IR_DGAIN, editCorneaDgain);
	DDX_Control(pDX, IDC_EDIT_CORNEA_IR_LED, editCorneaLed);
	DDX_Control(pDX, IDC_EDIT_INT_FIX_BRIGHT, editFixBright);
	DDX_Control(pDX, IDC_EDIT_INT_FIX_ONTIME, editFixOnTime);
	DDX_Control(pDX, IDC_EDIT_INT_FIX_PERIOD, editFixPeriod);
	DDX_Control(pDX, IDC_EDIT_KER_FOCUS_LED, editKerFocusLed);
	DDX_Control(pDX, IDC_EDIT_KER_RING_LED, editKerRingLed);
	DDX_Control(pDX, IDC_EDIT_OD_DISC_X, editOdDiscX);
	DDX_Control(pDX, IDC_EDIT_OD_DISC_Y, editOdDiscY);
	DDX_Control(pDX, IDC_EDIT_OD_FOVEA_X, editOdFoveaX);
	DDX_Control(pDX, IDC_EDIT_OD_FOVEA_Y, editOdFoveaY);
	DDX_Control(pDX, IDC_EDIT_OD_FUNDUS_X, editOdFundusX);
	DDX_Control(pDX, IDC_EDIT_OD_FUNDUS_Y, editOdFundusY);
	DDX_Control(pDX, IDC_EDIT_OD_LDN_X, editOdLdnX);
	DDX_Control(pDX, IDC_EDIT_OD_LDN_Y, editOdLdnY);
	DDX_Control(pDX, IDC_EDIT_OD_LEFT_X, editOdLeftX);
	DDX_Control(pDX, IDC_EDIT_OD_LEFT_Y, editOdLeftY);
	DDX_Control(pDX, IDC_EDIT_OD_LUP_X, editOdLupX);
	DDX_Control(pDX, IDC_EDIT_OD_LUP_Y, editOdLupY);
	DDX_Control(pDX, IDC_EDIT_OD_RDN_X, editOdRdnX);
	//  DDX_Control(pDX, IDC_EDIT_OD_RDN_Y, editOdRndY);
	DDX_Control(pDX, IDC_EDIT_OD_RDN_Y, editOdRdnY);
	DDX_Control(pDX, IDC_EDIT_OD_RIGHT_X, editOdRightX);
	DDX_Control(pDX, IDC_EDIT_OD_RIGHT_Y, editOdRightY);
	DDX_Control(pDX, IDC_EDIT_OD_RUP_X, editOdRupX);
	DDX_Control(pDX, IDC_EDIT_OD_RUP_Y, editOdRupY);
	DDX_Control(pDX, IDC_EDIT_OS_DISC_X, editOsDiscX);
	DDX_Control(pDX, IDC_EDIT_OS_DISC_Y, editOsDiscY);
	DDX_Control(pDX, IDC_EDIT_OS_FOVEA_X, editOsFoveaX);
	DDX_Control(pDX, IDC_EDIT_OS_FOVEA_Y, editOsFoveaY);
	DDX_Control(pDX, IDC_EDIT_OS_FUNDUS_X, editOsFundusX);
	DDX_Control(pDX, IDC_EDIT_OS_FUNDUS_Y, editOsFundusY);
	DDX_Control(pDX, IDC_EDIT_OS_LDN_X, editOsLdnX);
	DDX_Control(pDX, IDC_EDIT_OS_LDN_Y, editOsLdnY);
	DDX_Control(pDX, IDC_EDIT_OS_LEFT_X, editOsLeftX);
	DDX_Control(pDX, IDC_EDIT_OS_LEFT_Y, editOsLeftY);
	DDX_Control(pDX, IDC_EDIT_OS_LUP_X, editOsLupX);
	DDX_Control(pDX, IDC_EDIT_OS_LUP_Y, editOsLupY);
	DDX_Control(pDX, IDC_EDIT_OS_RDN_X, editOsRdnX);
	DDX_Control(pDX, IDC_EDIT_OS_RDN_Y, editOsRdnY);
	DDX_Control(pDX, IDC_EDIT_OS_RIGHT_X, editOsRightX);
	DDX_Control(pDX, IDC_EDIT_OS_RIGHT_Y, editOsRightY);
	DDX_Control(pDX, IDC_EDIT_OS_RUP_X, editOsRupX);
	DDX_Control(pDX, IDC_EDIT_OS_RUP_Y, editOsRupY);
	//  DDX_Control(pDX, IDC_EDIT_RADIAL_PARAM1_X, editRadialParam1);
	DDX_Control(pDX, IDC_EDIT_RADIAL_PARAM1_X, editRadialParamlX1);
	DDX_Control(pDX, IDC_EDIT_RADIAL_PARAM1_Y, editRadialParamlY1);
	DDX_Control(pDX, IDC_EDIT_RADIAL_PARAM2_X, editRadialParamlX2);
	DDX_Control(pDX, IDC_EDIT_RADIAL_PARAM2_Y, editRadialParamlY2);
	DDX_Control(pDX, IDC_EDIT_RADIAL_PARAM3_X, editRadialParamlX3);
	DDX_Control(pDX, IDC_EDIT_RADIAL_PARAM3_Y, editRadialParamlY3);
	DDX_Control(pDX, IDC_EDIT_RADIAL_PARAM4_X, editRadialParamlX4);
	DDX_Control(pDX, IDC_EDIT_RADIAL_PARAM4_Y, editRadialParamlY4);
	DDX_Control(pDX, IDC_EDIT_RETINA_DGAIN, editRetinaDgain);
	DDX_Control(pDX, IDC_EDIT_RETINA_IR_LED, editRetinaLed);
	//  DDX_Control(pDX, IDC_EDIT_RETINA_ROI_CENT_X, editRetinaCentX);
	//  DDX_Control(pDX, IDC_EDIT_RETINA_ROI_CENT_Y, editRetinaCentY);
	//  DDX_Control(pDX, IDC_EDIT_RETINA_ROI_RADIUS, editRetinaRadius);
	DDX_Control(pDX, IDC_EDIT_SPLIT_FOCUS_CENT_X, editSplitCentX);
	DDX_Control(pDX, IDC_EDIT_SPLIT_FOCUS_CENT_Y, editSplitCentY);
	DDX_Control(pDX, IDC_EDIT_SPLIT_FOCUS_LED, editSplitLed);
	DDX_Control(pDX, IDC_EDIT_WB_PARAM1, editWbParam1);
	DDX_Control(pDX, IDC_EDIT_WB_PARAM2, editWbParam2);
	DDX_Control(pDX, IDC_EDIT_WORKING_DOT_LED, editWorkingDotsLed);
	DDX_Control(pDX, IDC_EDIT_RETINA_ROI_CENT_X, editRetinaRoiX);
	DDX_Control(pDX, IDC_EDIT_RETINA_ROI_CENT_Y, editRetinaRoiY);
	DDX_Control(pDX, IDC_EDIT_RETINA_ROI_RADIUS, editRetinaRoiRadius);
	DDX_Control(pDX, IDC_COMBO_COLOR_AGAIN, cmbColorAgain);
	DDX_Control(pDX, IDC_COMBO_CORNEA_AGAIN, cmbCorneaAgain);
	DDX_Control(pDX, IDC_EDIT_RETINA_IR_LED_BRIGHT, editRetinaLedBright);
	// DDX_Control(pDX, IDC_EDIT_FLAHS_NORMAL, editFlashNormal);
	DDX_Control(pDX, IDC_EDIT_FLASH_MAX, editFlashMax);
	DDX_Control(pDX, IDC_EDIT_FLASH_MIN, editFlashMin);
	DDX_Control(pDX, IDC_EDIT_AUTO_BRIGHT, editAutoBright);
	DDX_Control(pDX, IDC_EDIT_AUTO_CONTRAST, editAutoContrast);
	DDX_Control(pDX, IDC_EDIT_AUTO_UB, editAutoUB);
	DDX_Control(pDX, IDC_EDIT_AUTO_VR, editAutoVR);
	DDX_Control(pDX, IDC_EDIT_EDGE_KERNEL, editEdgeKernel);
	DDX_Control(pDX, IDC_EDIT_RADIAL_RATIO, editRadialRatio);
	DDX_Control(pDX, IDC_EDIT_SCALE_X1, editPatScaleX1);
	DDX_Control(pDX, IDC_EDIT_SCALE_X2, editPatScaleX2);
	DDX_Control(pDX, IDC_EDIT_SCALE_X3, editPatScaleX3);
	DDX_Control(pDX, IDC_EDIT_SCALE_X4, editPatScaleX4);
	DDX_Control(pDX, IDC_EDIT_SCALE_X5, editPatScaleX5);
	DDX_Control(pDX, IDC_EDIT_SCALE_X6, editPatScaleX6);
	DDX_Control(pDX, IDC_EDIT_SCALE_X7, editPatScaleX7);
	DDX_Control(pDX, IDC_EDIT_SCALE_X8, editPatScaleX8);
	DDX_Control(pDX, IDC_EDIT_SCALE_X9, editPatScaleX9);

	DDX_Control(pDX, IDC_EDIT_SCALE_Y1, editPatScaleY1);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y2, editPatScaleY2);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y3, editPatScaleY3);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y4, editPatScaleY4);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y5, editPatScaleY5);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y6, editPatScaleY6);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y7, editPatScaleY7);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y8, editPatScaleY8);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y9, editPatScaleY9);

	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_X1, editPatOffsetX1);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_X2, editPatOffsetX2);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_X3, editPatOffsetX3);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_X4, editPatOffsetX4);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_X5, editPatOffsetX5);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_X6, editPatOffsetX6);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_X7, editPatOffsetX7);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_X8, editPatOffsetX8);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_X9, editPatOffsetX9);

	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_Y1, editPatOffsetY1);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_Y2, editPatOffsetY2);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_Y3, editPatOffsetY3);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_Y4, editPatOffsetY4);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_Y5, editPatOffsetY5);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_Y6, editPatOffsetY6);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_Y7, editPatOffsetY7);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_Y8, editPatOffsetY8);
	DDX_Control(pDX, IDC_EDIT_PATTERN_OFFSET_Y9, editPatOffsetY9);

	DDX_Control(pDX, IDC_EDIT_WORKING_DOT_LED2, editWorkingDot2Led);
	DDX_Control(pDX, IDC_EDIT_EXT_FIXATION_LED, editExtFixLed);
	DDX_Control(pDX, IDC_COMBO_RETINA_AGAIN2, cmbRetinaAgain2);
	DDX_Control(pDX, IDC_COMBO_RETINA_AGAIN3, cmbRetinaAgain3);
	DDX_Control(pDX, IDC_EDIT_RETINA_DGAIN2, editRetinaDgain2);
	DDX_Control(pDX, IDC_EDIT_RETINA_DGAIN3, editRetinaDgain3);
	DDX_Control(pDX, IDC_EDIT_FLASH_BASE1, editFlashBase1);
	DDX_Control(pDX, IDC_EDIT_FLASH_BASE2, editFlashBase2);
	DDX_Control(pDX, IDC_EDIT_FLASH_BASE3, editFlashBase3);
	DDX_Control(pDX, IDC_EDIT_FLASH_HIGH1, editFlashHigh1);
	//  DDX_Control(pDX, IDC_EDIT_FLASH_HIGH2, editFlash2);
	DDX_Control(pDX, IDC_EDIT_FLASH_HIGH2, editFlashHigh2);
	DDX_Control(pDX, IDC_EDIT_FLASH_HIGH3, editFlashHigh3);
	DDX_Control(pDX, IDC_EDIT_FLASH_INT1, editFlashInt1);
	DDX_Control(pDX, IDC_EDIT_FLASH_INT2, editFlashInt2);
	DDX_Control(pDX, IDC_EDIT_FLASH_INT3, editFlashInt3);
	DDX_Control(pDX, IDC_EDIT_FLASH_LEVEL1, editFlashLevel1);
	DDX_Control(pDX, IDC_EDIT_FLASH_LEVEL2, editFlashLevel2);
	DDX_Control(pDX, IDC_EDIT_FLASH_LEVEL3, editFlashLevel3);
	DDX_Control(pDX, IDC_EDIT_REFER_LOWER, editReferLower);
	DDX_Control(pDX, IDC_EDIT_REFER_UPPER, editReferUpper);
	DDX_Control(pDX, IDC_EDIT_RETINA_MASK_SIZE, editRetinaMaskSize);
	DDX_Control(pDX, IDC_EDIT_FLASH_RETINA1, editFlashRetina1);
	DDX_Control(pDX, IDC_EDIT_FLASH_RETINA2, editFlashRetina2);
	DDX_Control(pDX, IDC_EDIT_FLASH_RETINA3, editFlashRetina3);
	DDX_Control(pDX, IDC_EDIT_FLASH_SPLIT1, editFlashSplit1);
	DDX_Control(pDX, IDC_EDIT_FLASH_SPLIT2, editFlashSplit2);
	DDX_Control(pDX, IDC_EDIT_FLASH_SPLIT3, editFlashSplit3);
	DDX_Control(pDX, IDC_EDIT_FLASH_WDOT11, editFlashWdot11);
	DDX_Control(pDX, IDC_EDIT_FLASH_WDOT12, editFlashWdot12);
	DDX_Control(pDX, IDC_EDIT_FLASH_WDOT13, editFlashWdot13);
	DDX_Control(pDX, IDC_EDIT_FLASH_WDOT21, editFlashWdot21);
	DDX_Control(pDX, IDC_EDIT_FLASH_WDOT22, editFlashWdot22);
	DDX_Control(pDX, IDC_EDIT_FLASH_WDOT23, editFlashWdot23);
	DDX_Control(pDX, IDC_INT_FIX_TYPE, editIntFixType);
	DDX_Control(pDX, IDC_EDIT_RETINA_MASK_CENT_X, editRetinaMaskCenterX);
	DDX_Control(pDX, IDC_EDIT_RETINA_MASK_CENT_Y, editRetinaMaskCenterY);
	DDX_Control(pDX, IDC_EDIT_CORNEA_PIXELS_MM, editCorneaPixelsMM);
	DDX_Control(pDX, IDC_EDIT_CORNEA_SMALL_PUPIL_SIZE, editCorneaSmallPupilSize);
	DDX_Control(pDX, IDC_EDIT_RETINA_CLIPLIMIT, editClipLimit);
	DDX_Control(pDX, IDC_EDIT_RETINA_CLIPSCALAR, editClipScalar);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM1_X, editRadialParamlX1_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM2_X, editRadialParamlX2_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM3_X, editRadialParamlX3_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM4_X, editRadialParamlX4_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM5_X, editRadialParamlX5_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM6_X, editRadialParamlX6_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM7_X, editRadialParamlX7_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM1_Y, editRadialParamlY1_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM2_Y, editRadialParamlY2_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM3_Y, editRadialParamlY3_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM4_Y, editRadialParamlY4_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM5_Y, editRadialParamlY5_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM6_Y, editRadialParamlY6_FILR);
	DDX_Control(pDX, IDC_EDIT_FILR_RADIAL_PARAM7_Y, editRadialParamlY7_FILR);
	DDX_Control(pDX, IDC_EDIT_REMOVE_LIGHT1, editRemoveLight1);
	DDX_Control(pDX, IDC_EDIT_REMOVE_LIGHT2, editRemoveLight2);
	DDX_Control(pDX, IDC_EDIT_REMOVE_LIGHT3, editRemoveLight3);
}


BEGIN_MESSAGE_MAP(DlgFactorySetup, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &DlgFactorySetup::OnBnClickedButtonLoad)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &DlgFactorySetup::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &DlgFactorySetup::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, &DlgFactorySetup::OnBnClickedButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, &DlgFactorySetup::OnBnClickedButtonImport)
	ON_EN_CHANGE(IDC_EDIT_SPLIT_FOCUS_LED, &DlgFactorySetup::OnChangeEditSplitFocusLed)
	ON_EN_CHANGE(IDC_EDIT_WORKING_DOT_LED, &DlgFactorySetup::OnChangeEditWorkingDotLed)
	ON_EN_CHANGE(IDC_EDIT_WORKING_DOT_LED2, &DlgFactorySetup::OnChangeEditWorkingDotLed2)
	ON_EN_CHANGE(IDC_EDIT_RETINA_IR_LED, &DlgFactorySetup::OnChangeEditRetinaIrLed)
	ON_EN_CHANGE(IDC_EDIT_OD_FOVEA_X, &DlgFactorySetup::OnEnChangeEditOdFoveaX)
	ON_EN_CHANGE(IDC_EDIT_OD_FOVEA_Y, &DlgFactorySetup::OnEnChangeEditOdFoveaY)
	ON_EN_CHANGE(IDC_EDIT_RETINA_ROI_CENT_X, &DlgFactorySetup::OnEnChangeEditRetinaRoiCentX)
	ON_EN_CHANGE(IDC_EDIT_RETINA_ROI_CENT_Y, &DlgFactorySetup::OnEnChangeEditRetinaRoiCentY)
END_MESSAGE_MAP()


// DlgFactorySetup message handlers




void DlgFactorySetup::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnClose();
}


BOOL DlgFactorySetup::PreTranslateMessage(MSG* pMsg)
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


BOOL DlgFactorySetup::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	loadControls();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void DlgFactorySetup::OnBnClickedButtonLoad()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Loader::loadSetupProfile(false, false)) {
		loadControls();
		AfxMessageBox(_T("Factory settings loaded from mainboard!"));
	}
	else {
		AfxMessageBox(_T("Failed to load Factory settings from mainboard!"));
	}
	EndWaitCursor();
	return;
}


void DlgFactorySetup::OnBnClickedButtonSave()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	saveControls();
	if (Loader::saveSetupProfile(false, false)) {
		Loader::applySystemProfile(true);
		AfxMessageBox(_T("Factory settings saved to mainboard!"));
	}
	else {
		AfxMessageBox(_T("Failed to save Factory settings to mainboard!"));
	}
	EndWaitCursor();
	return;
}


void DlgFactorySetup::OnBnClickedButtonExport()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	saveControls();
	if (Loader::saveSetupProfile(false, true)) {
		Loader::applySystemProfile(true);
		AfxMessageBox(_T("Factory settings saved to config file!"));
	}
	else {
		AfxMessageBox(_T("Failed to save Factory settings to config file!"));
	}
	EndWaitCursor();
	return;
}


void DlgFactorySetup::OnBnClickedButtonImport()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Loader::loadSetupProfile(true, true)) {
		loadControls();
		AfxMessageBox(_T("Factory settings loaded from config file!"));
	}
	else {
		AfxMessageBox(_T("Failed to load Factory settings from config file!"));
	}
	EndWaitCursor();
	return;
}



void DlgFactorySetup::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);

}



void DlgFactorySetup::OnChangeEditSplitFocusLed()
{
	CString text;
	GetDlgItemText(IDC_EDIT_SPLIT_FOCUS_LED, text);
	if (text.IsEmpty()) {
		return;
	}

	int value = _ttoi(text);
	CString val1, val2;
	val1.Format(_T("%d"), (int)(value * 0.9f));
	val2.Format(_T("%d"), (int)(value * 0.8f));

	editFlashSplit1.SetWindowTextW(text);
	editFlashSplit2.SetWindowTextW(val1);
	editFlashSplit3.SetWindowTextW(val2);
}


void DlgFactorySetup::OnChangeEditWorkingDotLed()
{
	CString text;
	GetDlgItemText(IDC_EDIT_WORKING_DOT_LED, text);
	if (text.IsEmpty()) {
		return;
	}

	int value = _ttoi(text);
	CString val1, val2;
	val1.Format(_T("%d"), (int)(value * 0.9f));
	val2.Format(_T("%d"), (int)(value * 0.8f));

	editFlashWdot11.SetWindowTextW(text);
	editFlashWdot12.SetWindowTextW(val1);
	editFlashWdot13.SetWindowTextW(val2);
}


void DlgFactorySetup::OnChangeEditWorkingDotLed2()
{
	CString text;
	GetDlgItemText(IDC_EDIT_WORKING_DOT_LED2, text);
	if (text.IsEmpty()) {
		return;
	}

	int value = _ttoi(text);
	CString val1, val2;
	val1.Format(_T("%d"), (int)(value * 0.9f));
	val2.Format(_T("%d"), (int)(value * 0.8f));

	editFlashWdot21.SetWindowTextW(text);
	editFlashWdot22.SetWindowTextW(val1);
	editFlashWdot23.SetWindowTextW(val2);
}


void DlgFactorySetup::OnChangeEditRetinaIrLed()
{
	CString text;
	GetDlgItemText(IDC_EDIT_RETINA_IR_LED, text);
	if (text.IsEmpty()) {
		return;
	}

	editFlashRetina1.SetWindowTextW(text);
	editFlashRetina2.SetWindowTextW(text);
	editFlashRetina3.SetWindowTextW(text);
}


void DlgFactorySetup::OnEnChangeEditOdFoveaX()
{
	CString text;
	editOdFoveaX.GetWindowTextW(text);

	editOsFoveaX.SetWindowTextW(text);
	return;
}


void DlgFactorySetup::OnEnChangeEditOdFoveaY()
{
	CString text;
	editOdFoveaY.GetWindowTextW(text);

	editOsFoveaY.SetWindowTextW(text);
	return;
}


void DlgFactorySetup::OnEnChangeEditRetinaRoiCentX()
{
	CString text;
	editRetinaRoiX.GetWindowTextW(text);

	editRetinaMaskCenterX.SetWindowTextW(text);
	return;
}


void DlgFactorySetup::OnEnChangeEditRetinaRoiCentY()
{
	CString text;
	editRetinaRoiY.GetWindowTextW(text);

	editRetinaMaskCenterY.SetWindowTextW(text);
	return;
}

BOOL IsExist(CString path)
{
	WIN32_FIND_DATA findData;

	ZeroMemory(&findData, sizeof(findData));

	BOOL ok = FALSE;
	HANDLE hExist = FindFirstFile(path, &findData);
	if (hExist == INVALID_HANDLE_VALUE)
	{
		ok = FALSE;
	}
	else
		ok = TRUE;

	FindClose(hExist);

	return ok;
}
