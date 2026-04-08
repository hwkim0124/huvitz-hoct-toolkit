#pragma once

#include "DlgFocusControl.h"
#include "DlgCameraControl.h"
#include "DlgMotorControl.h"
#include "DlgLightControl.h"
#include "DlgColorCameraControl.h"
#include "DlgOriginalImage.h"
#include "DlgOriginalLive.h"

#include "ViewCorneaCamera.h"
#include "ViewRetinaCamera.h"
#include "ViewColorCamera.h"

#include <memory>
#include "afxwin.h"
#include "afxcmn.h"

#include "OctSystem2.h"

// DlgColorCameraTest dialog

class DlgColorCameraTest : public CDialogEx
{
	DECLARE_DYNAMIC(DlgColorCameraTest)

public:
	DlgColorCameraTest(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgColorCameraTest();

protected:
	void initControls(void);
	void controlIrCameras(bool play);

	void updateFundusFocus(void);
	void updateFundusFlash(void);
	void callbackEyeSideEvent(bool od);

	void saveROIData(const std::string& outputFilename, const std::vector<std::string>& imageFilenames);
protected:
	ViewCorneaCamera corneaView;
	ViewRetinaCamera retinaView;
	ViewColorCamera colorView;
	bool isPlaying = false;

	JoystickEventCallback cbJoystick;
	EyeSideEventCallback cbEyeSide;

	std::unique_ptr<DlgFocusControl> pDlgFocus;
	std::unique_ptr<DlgCameraControl> pDlgCamera;
	std::unique_ptr<DlgMotorControl> pDlgMotor;
	std::unique_ptr<DlgLightControl> pDlgLight;
	std::unique_ptr<DlgColorCameraControl> pDlgColor;
	std::unique_ptr<DlgOriginalImage> pDlgOriginal;
	std::unique_ptr<DlgOriginalLive> pDlgLive;

	OctDevice::FundusFocusMotor* fundusMotor;
	OctDevice::FlashLed* flashLed;
	OctDevice::ColorCamera* colorCamera;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGCOLORCAMERATEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonConfigSave();
	afx_msg void OnBnClickedButtonConfigLoad();
	afx_msg void OnBnClickedButtonFocus();
	afx_msg void OnBnClickedButtonCamera();
	afx_msg void OnBnClickedButtonMotor();
	afx_msg void OnBnClickedButtonOriginal();
	afx_msg void OnBnClickedButtonLeds();
	afx_msg void OnBnClickedButtonColor();
	afx_msg void OnBnClickedButtonSaveRaw();
	afx_msg void OnBnClickedButtonSaveImage();
	afx_msg void OnBnClickedButtonCapture();
	afx_msg void OnBnClickedButtonLive();
	afx_msg void OnBnClickedButtonColorExit();
	afx_msg void OnBnClickedButtonLoadRaw();
	CComboBox cmbAgain;
	CComboBox cmbInfixCol;
	CComboBox cmbInfixRow;
	CEdit editDgain;
	CEdit editFlash;
	CSliderCtrl sldFlash;
	CSliderCtrl sldFundus;
	CEdit editFundus;
	CEdit editDiopt;
	afx_msg void OnReleasedcaptureSliderFundus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderFlash(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSelchangeComboAgain();
	afx_msg void OnBnClickedButtonFundusStep1();
	afx_msg void OnBnClickedButtonFundusStep2();
	afx_msg void OnBnClickedButtonFundusStep3();
	afx_msg void OnBnClickedButtonFundusStep4();
	afx_msg void OnBnClickedButtonFundusOrigin();
	afx_msg void OnBnClickedButtonFlashStep1();
	afx_msg void OnBnClickedButtonFlashStep2();
	afx_msg void OnBnClickedButtonIntfixOn();
	afx_msg void OnBnClickedButtonIntfixOff();
	afx_msg void OnBnClickedRadioNormal();
	afx_msg void OnBnClickedRadioSmall();
	afx_msg void OnNMCustomdrawSliderFlash(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeComboIntfixRow();
	afx_msg void OnCbnSelchangeComboIntfixCol();
	afx_msg void OnNMCustomdrawSliderFundus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonAwbCalc();
	afx_msg void OnBnClickedButtonAwbInit();
	afx_msg void OnBnClickedCheckAwb();
	afx_msg void OnBnClickedButtonIntfixOd();
	afx_msg void OnBnClickedButtonIntfixOs();
	afx_msg void OnBnClickedCheckRadial();
	afx_msg void OnBnClickedButtonExport();
	afx_msg void OnBnClickedButtonExportResult();
	afx_msg void OnBnClickedButtonExportData();
	afx_msg void OnBnClickedCheckMaskRoi();
	afx_msg void OnBnClickedButtonWhites();
	afx_msg void OnBnClickedButtonBlacks();
	afx_msg void OnBnClickedButtonPixelDetect();
	afx_msg void OnBnClickedButtonPixelCorrect();
	CEdit editBrightB;
	CEdit editBrightG;
	CEdit editBrightR;
	CEdit editDarkB;
	CEdit editDarkG;
	CEdit editDarkR;
	CEdit editLightRadius;
	CEdit editShadowRadius;
	CEdit editBorderRadius;
	CEdit editThreshold;
	afx_msg void OnBnClickedButtonBpLoad();
	afx_msg void OnBnClickedButtonBpSave();
	afx_msg void OnBnClickedCheckAlignGuide();
	afx_msg void OnBnClickedButtonRetinaPreset1();
	afx_msg void OnBnClickedButtonRetinaPreset2();
	afx_msg void OnBnClickedButtonRetinaPreset3();
	afx_msg void OnBnClickedButtonOriginlive();
	afx_msg void OnBnClickedButtonApplyRemovalLight();
	afx_msg void OnBnClickedCheckRemovalLight();
	afx_msg void OnBnClickedCheckRemoveGuide();
	afx_msg void OnEnChangeEditThreshold();
	afx_msg void OnBnClickedButtonBlackspotAlign();
	afx_msg void OnBnClickedButtonBlackspotCapture();
	afx_msg void OnBnClickedButtonLedAlign();
	afx_msg void OnBnClickedButtonRoiXUp();
	afx_msg void OnBnClickedButtonRoiXDown();
	afx_msg void OnBnClickedButtonRoiYUp();
	afx_msg void OnBnClickedButtonRoiYDown();
	afx_msg void OnBnClickedButtonRoiRadiusDown();
	afx_msg void OnBnClickedButtonRoiRadiusUp();
	afx_msg void OnBnClickedButtonUpdateColorRoi();
	afx_msg void OnBnClickedButtonFundusCalibration();
};
