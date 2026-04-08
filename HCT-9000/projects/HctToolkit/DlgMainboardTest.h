#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include <memory>
#include <string>
#include <vector>

#include "UsbLoopBack.h"
#include "StepMotorTest.h"

#include "OctSystem2.h"
#include "OctDevice2.h"

using namespace OctSystem;
using namespace OctDevice;

// DlgMainboardTest dialog

class DlgMainboardTest : public CDialogEx
{
	DECLARE_DYNAMIC(DlgMainboardTest)

public:
	DlgMainboardTest(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgMainboardTest();

protected:
	void initUsbLoopBack(void);
	bool startUsbLoopBack(void);
	void cancelUsbLoopBack(void);
	void callbackUsbLoopBackDownload(unsigned int addr, int present, int block, int total, bool isError);
	void callbackUsbLoopBackComplete(bool success);

	void initPIStatus(void);
	bool startPIStatusTest(void);
	void cancelPIStatusTest(void);
	bool reportPIStatusTest(void);
	void updatePIStatus(std::uint32_t status);

	void initStepMotorTest(void);
	bool startStepMotorTest(void);
	void cancelStepMotorTest(void);
	void callbackStepMotorMoving(int stage, int present, int target, bool arrived);
	void callbackStepMotorComplete(bool success);

	void initStageMotorTest(void);
	bool startStageMotorTest(void);
	void cancelStageMotorTest(void);
	void reportStageMotorTest(void);

	void initLcdFixationTest(void);
	bool updateLcdFixation(void);

	void initSldInterfaceTest(void);
	bool updateSldInterface(void);
	void startSldInterfaceTest(void);
	void cancelSldInterfaceTest(void);

	void initGalvanoScanTest(void);
	bool startGalvanoScan(void);
	void cancelGalvanoScan(void);


protected:
	UsbLoopBackDownloadCallback _clbUsbLoopBackDownload;
	UsbLoopBackCompleteCallback _clbUsbLoopBackComplete;

	StepMotorMovingCallback _clbStepMotorMoving;
	StepMotorCompleteCallback _clbStepMotorComplete;

	bool _stageMoving;
	StageMotorType _stageType;
	bool _isStageX;
	bool _isStageCw;
	bool _turnOnSld = false;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGMAINBOARDTEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	CEdit editUsbDataSize;
	CEdit editUsbRepeat;
	CEdit editUsbStatus;
	afx_msg void OnBnClickedUsbLoopbackStart();
	afx_msg void OnBnClickedMpiStart();
	afx_msg void OnBnClickedUsbCopy();
	CComboBox cmbStepMotor;
//	CEdit editStepRepeat;
	CEdit editStepStatus;
	CEdit editStepRepeat;
	afx_msg void OnBnClickedStepStart();
	afx_msg void OnBnClickedStageStart();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedStageX();
	afx_msg void OnBnClickedStageZ();
	afx_msg void OnBnClickedStageCw();
	afx_msg void OnBnClickedStageCcw();
	CEdit editLcdBlink;
	CEdit editLcdBright;
	CEdit editLcdOnTime;
	CEdit editLcdPeriod;
	CEdit editSldEpd;
	CEdit editSldIpd;
	CEdit editSldSld;
	CEdit editSldTemp;
	afx_msg void OnBnClickedLcdCross();
	afx_msg void OnBnClickedLcdGreen();
	afx_msg void OnBnClickedLcdUpdate();
	afx_msg void OnBnClickedSldOn();
	afx_msg void OnBnClickedSldOff();
	afx_msg void OnBnClickedSldUpdate();
	afx_msg void OnBnClickedCameraOpen();
	afx_msg void OnBnClickedScanSld();
//	CComboBox cmdScanPattern;
//	CComboBox cmdScanRange;
	CEdit editScanOffsetX;
	CEdit editScanOffsetY;
	CComboBox cmbScanPattern;
	CComboBox cmbScanRange;
	afx_msg void OnBnClickedScanStart();
	afx_msg void OnBnClickedCameraLed();
};
