#pragma once
#include "afxwin.h"
#include "stdafx.h"

// DlgFactorySetup dialog

class DlgFactorySetup : public CDialogEx
{
	DECLARE_DYNAMIC(DlgFactorySetup)

public:
	DlgFactorySetup(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgFactorySetup();

	void loadControls(void);
	void saveControls(void);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGFACTORYSETUP };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	CComboBox cmdColorAgain;
//	CComboBox cmdCorneaAgain;
	CComboBox cmbRetinaAgain;
	CEdit editColorDgain;
	CEdit editColorRoiX;
	CEdit editColorRoiY;
	CEdit editColorRoiRadius;
	CEdit editCorneaDgain;
	CEdit editCorneaLed;
	CEdit editFixBright;
	CEdit editFixOnTime;
	CEdit editFixPeriod;
	CEdit editKerFocusLed;
	CEdit editKerRingLed;
	CEdit editOdDiscX;
	CEdit editOdDiscY;
	CEdit editOdFoveaX;
	CEdit editOdFoveaY;
	CEdit editOdFundusX;
	CEdit editOdFundusY;
	CEdit editOdLdnX;
	CEdit editOdLdnY;
	CEdit editOdLeftX;
	CEdit editOdLeftY;
	CEdit editOdLupX;
	CEdit editOdLupY;
	CEdit editOdRdnX;
//	CEdit editOdRndY;
	CEdit editOdRdnY;
	CEdit editOdRightX;
	CEdit editOdRightY;
	CEdit editOdRupX;
	CEdit editOdRupY;
	CEdit editOsDiscX;
	CEdit editOsDiscY;
	CEdit editOsFoveaX;
	CEdit editOsFoveaY;
	CEdit editOsFundusX;
	CEdit editOsFundusY;
	CEdit editOsLdnX;
	CEdit editOsLdnY;
	CEdit editOsLeftX;
	CEdit editOsLeftY;
	CEdit editOsLupX;
	CEdit editOsLupY;
	CEdit editOsRdnX;
	CEdit editOsRdnY;
	CEdit editOsRightX;
	CEdit editOsRightY;
	CEdit editOsRupX;
	CEdit editOsRupY;
//	CEdit editRadialParam1;
	CEdit editRadialParamlX1;
	CEdit editRadialParamlY1;
	CEdit editRadialParamlX2;
	CEdit editRadialParamlY2;
	CEdit editRadialParamlX3;
	CEdit editRadialParamlY3;
	CEdit editRadialParamlX4;
	CEdit editRadialParamlY4;
	CEdit editRetinaDgain;
	CEdit editRetinaLed;
//	CEdit editRetinaCentX;
//	CEdit editRetinaCentY;
//	CEdit editRetinaRadius;
	CEdit editSplitCentX;
	CEdit editSplitCentY;
	CEdit editSplitLed;
	CEdit editWbParam1;
	CEdit editWbParam2;
	CEdit editWorkingDotsLed;
	CEdit editRetinaRoiX;
	CEdit editRetinaRoiY;
	CEdit editRetinaRoiRadius;
	afx_msg void OnBnClickedButtonLoad();
	CComboBox cmbColorAgain;
	CComboBox cmbCorneaAgain;
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	CEdit editRetinaLedBright;
	CEdit editFlashNormal;
	CEdit editFlashMax;
	CEdit editFlashMin;
	CEdit editAutoBright;
	CEdit editAutoContrast;
	CEdit editAutoUB;
	CEdit editAutoVR;
	CEdit editEdgeKernel;
	CEdit editRadialRatio;
	CEdit editPatScaleX1;
	CEdit editPatScaleX2;
	CEdit editPatScaleX3;
	CEdit editPatScaleX4;
	CEdit editPatScaleX5;
	CEdit editPatScaleX6;
	CEdit editPatScaleX7;
	CEdit editPatScaleX8;
	CEdit editPatScaleX9;
	CEdit editPatScaleY1;
	CEdit editPatScaleY2;
	CEdit editPatScaleY3;
	CEdit editPatScaleY4;
	CEdit editPatScaleY5;
	CEdit editPatScaleY6;
	CEdit editPatScaleY7;
	CEdit editPatScaleY8;
	CEdit editPatScaleY9;
	CEdit editPatOffsetX1;
	CEdit editPatOffsetX2;
	CEdit editPatOffsetX3;
	CEdit editPatOffsetX4;
	CEdit editPatOffsetX5;
	CEdit editPatOffsetX6;
	CEdit editPatOffsetX7;
	CEdit editPatOffsetX8;
	CEdit editPatOffsetX9;
	CEdit editPatOffsetY1;
	CEdit editPatOffsetY2;
	CEdit editPatOffsetY3;
	CEdit editPatOffsetY4;
	CEdit editPatOffsetY5;
	CEdit editPatOffsetY6;
	CEdit editPatOffsetY7;
	CEdit editPatOffsetY8;
	CEdit editPatOffsetY9;
	CEdit editWorkingDot2Led;
	CEdit editExtFixLed;
	afx_msg void OnBnClickedButtonExport();
	afx_msg void OnBnClickedButtonImport();
	CComboBox cmbRetinaAgain2;
	CComboBox cmbRetinaAgain3;
	CEdit editRetinaDgain2;
	CEdit editRetinaDgain3;
	CEdit editFlashBase1;
	CEdit editFlashBase2;
	CEdit editFlashBase3;
	CEdit editFlashHigh1;
//	CEdit editFlash2;
	CEdit editFlashHigh2;
	CEdit editFlashHigh3;
	CEdit editFlashInt1;
	CEdit editFlashInt2;
	CEdit editFlashInt3;
	CEdit editFlashLevel1;
	CEdit editFlashLevel2;
	CEdit editFlashLevel3;
	CEdit editReferLower;
	CEdit editReferUpper;
	CEdit editRetinaMaskSize;
	CEdit editFlashRetina1;
	CEdit editFlashRetina2;
	CEdit editFlashRetina3;
	CEdit editFlashSplit1;
	CEdit editFlashSplit2;
	CEdit editFlashSplit3;
	CEdit editFlashWdot11;
	CEdit editFlashWdot12;
	CEdit editFlashWdot13;
	CEdit editFlashWdot21;
	CEdit editFlashWdot22;
	CEdit editFlashWdot23;
	CEdit editIntFixType;
	CEdit editRetinaMaskCenterX;
	CEdit editRetinaMaskCenterY;
	afx_msg void OnChangeEditSplitFocusLed();
	afx_msg void OnChangeEditWorkingDotLed();
	afx_msg void OnChangeEditWorkingDotLed2();
	afx_msg void OnChangeEditRetinaIrLed();
	CEdit editCorneaPixelsMM;
	CEdit editCorneaSmallPupilSize;
	CEdit editClipLimit;
	CEdit editClipScalar;
	afx_msg void OnEnChangeEditRadialParam4X();
	CEdit editRadialParamlX1_FILR;
	CEdit editRadialParamlX2_FILR;
	CEdit editRadialParamlX3_FILR;
	CEdit editRadialParamlX4_FILR;
	CEdit editRadialParamlX5_FILR;
	CEdit editRadialParamlX6_FILR;
	CEdit editRadialParamlX7_FILR;
	CEdit editRadialParamlY1_FILR;
	CEdit editRadialParamlY2_FILR;
	CEdit editRadialParamlY3_FILR;
	CEdit editRadialParamlY4_FILR;
	CEdit editRadialParamlY5_FILR;
	CEdit editRadialParamlY6_FILR;
	CEdit editRadialParamlY7_FILR;
	CEdit editRemoveLight1;
	CEdit editRemoveLight2;
	CEdit editRemoveLight3;
	afx_msg void OnEnChangeEditOdFoveaX();
	afx_msg void OnEnChangeEditOdFoveaY();
	afx_msg void OnEnChangeEditRetinaRoiCentX();
	afx_msg void OnEnChangeEditRetinaRoiCentY();
};
