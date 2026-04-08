#pragma once
#include "afxwin.h"

#include <memory>
#include <string>

#include "OctSystem2.h"
#include "OctConfig2.h"
#include "CppUtil2.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


// DlgSysCalibration dialog

class DlgSysCalibration : public CDialogEx
{
	DECLARE_DYNAMIC(DlgSysCalibration)

public:
	DlgSysCalibration(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgSysCalibration();

protected:
	void loadControls(void);
	void saveControls(void);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGSYSCALIBRATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit editCorAgain;
	CEdit editCorDgain;
	CEdit editCorDisp1;
	CEdit editCorDisp2;
	CEdit editCorDisp3;
	CEdit editCorPos;
	CEdit editFunMinusLens;
	CEdit editFunNoLens;
	CEdit editFunPlusLens;
	CEdit editIrFocus;
	CEdit editPupNormal;
	CEdit editPupSmall;
	CEdit editQuickIn;
	CEdit editQuickOut;
	CEdit editRetAgain;
	CEdit editRetDgain;
	CEdit editRetDisp1;
	CEdit editRetDisp2;
	CEdit editRetDisp3;
	CEdit editRetPos;
	CEdit editScanFocus;
	CEdit editTopographyFocus;
	CEdit editScanMinusLens;
	CEdit editScanNoLens;
	CEdit editScanPlusLens;
	CEdit editEpdMax;
	CEdit editEpdMin;
	CEdit editSldHigh;
	CEdit editImMax;
	CEdit editImMin;
	CEdit editIsMax;
	CEdit editIsMin;
	CEdit editSldLow1;
	CEdit editSldLow2;
	CEdit editSldRsi;
	CEdit editSldSN;
	CEdit editSpectCal1;
	CEdit editSpectCal2;
	CEdit editSpectCal3;
	CEdit editSpectCal4;
	CEdit editSpectSN;
	CEdit editSplitIn;
	CEdit editSplitOut;
	CEdit editSysSN;
	CEdit editXBase;
	CEdit editXEncNear;
	CEdit editXEncOffset;
	CEdit editXFccw;
	CEdit editXFcw;
	CEdit editXNccw;
	CEdit editXNcw;
	CEdit editXOffset;
	CEdit editXRange;
	CEdit editYBase;
	CEdit editYEncNear;
	CEdit editYEncOffset;
	CEdit editYFccw;
	CEdit editYFcw;
	CEdit editYNccw;
	CEdit editYNcw;
	CEdit editYOffset;
	CEdit editYRange;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnBnClickedButtonSave();
	CEdit editChecksum;
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonSldLoad();
	afx_msg void OnBnClickedButtonSldUpdate();
	afx_msg void OnBnClickedButtonSldGet();
	afx_msg void OnBnClickedButtonUpdate();
	CEdit editPolPos;
	afx_msg void OnBnClickedCheckUartDebug();
	CEdit editIntFixOffsetX;
	CEdit editIntFixOffsetY;
	afx_msg void OnBnClickedButtonExport2();
	afx_msg void OnBnClickedButtonImport2();
	afx_msg void OnBnClickedButtonFactory();
};
