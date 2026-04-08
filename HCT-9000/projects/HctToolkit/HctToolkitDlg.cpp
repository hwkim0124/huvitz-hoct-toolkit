
// HctToolkitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "HctToolkitDlg.h"
#include "afxdialogex.h"

#include "DlgMainboardTest.h"
#include "DlgLongRunTest.h"
#include "DlgScanPattern.h"
#include "DlgSpectrometerCalibration.h"
#include "DlgDispersionCompensation.h"
#include "DlgKeratoCalibration.h"
#include "DlgColorCameraTest.h"
#include "DlgSignalTester.h"
#include "DlgSysCalibration.h"
#include "DlgFactorySetup.h"
#include "DlgAngioPattern.h"
#include "DlgUserLogin.h"
#include "DlgTopographyPattern.h"
#include "DlgSystemSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CHctToolkitDlg dialog



CHctToolkitDlg::CHctToolkitDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_HCTTOOLKIT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHctToolkitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHctToolkitDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_OCT_SCAN_TEST, &CHctToolkitDlg::OnClickedButtonOctScanTest)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SPECTRO_CALIB, &CHctToolkitDlg::OnBnClickedButtonSpectroCalib)
	ON_BN_CLICKED(IDC_BUTTON_DISPERSION, &CHctToolkitDlg::OnBnClickedButtonDispersion)
	ON_BN_CLICKED(IDC_BUTTON_KERATO, &CHctToolkitDlg::OnBnClickedButtonKerato)
	ON_BN_CLICKED(IDC_BUTTON_COLOR, &CHctToolkitDlg::OnBnClickedButtonColor)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CHctToolkitDlg::OnBnClickedButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_SYSTEM, &CHctToolkitDlg::OnBnClickedButtonSaveSystem)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_SYSTEM, &CHctToolkitDlg::OnBnClickedButtonLoadSystem)
	ON_BN_CLICKED(IDC_BUTTON_RESET_USB, &CHctToolkitDlg::OnBnClickedButtonResetUsb)
	ON_BN_CLICKED(IDC_BUTTON_SIGNAL, &CHctToolkitDlg::OnBnClickedButtonSignal)
	ON_BN_CLICKED(IDC_BUTTON_SYS_CALIB, &CHctToolkitDlg::OnBnClickedButtonSysCalib)
	ON_BN_CLICKED(IDC_BUTTON_MAINBOARD, &CHctToolkitDlg::OnBnClickedButtonMainboard)
	ON_BN_CLICKED(IDC_BUTTON_RESET_SYSTEM, &CHctToolkitDlg::OnBnClickedButtonResetSystem)
	ON_BN_CLICKED(IDC_BUTTON_LONGRUN, &CHctToolkitDlg::OnBnClickedButtonLongrun)
	ON_BN_CLICKED(IDC_BUTTON_FACTORY_SETUP, &CHctToolkitDlg::OnBnClickedButtonFactorySetup)
	ON_BN_CLICKED(IDC_BUTTON_PACKAGE_MODE, &CHctToolkitDlg::OnBnClickedButtonPackageMode)
	ON_BN_CLICKED(IDC_BUTTON_ANGIO_PATTERN, &CHctToolkitDlg::OnBnClickedButtonAngioPattern)
	ON_BN_CLICKED(IDC_BUTTON_RESET_USB_DRIVER, &CHctToolkitDlg::OnBnClickedButtonResetUsbDriver)
	ON_BN_CLICKED(IDC_BUTTON_TOPOGRAPHY_PATTERN, &CHctToolkitDlg::OnBnClickedButtonTopographyPattern)
	ON_WM_QUERYENDSESSION()
	ON_BN_CLICKED(IDC_BUTTON_SYSTEM_SETTINGS, &CHctToolkitDlg::OnBnClickedButtonSystemSettings)
END_MESSAGE_MAP()


// CHctToolkitDlg message handlers

BOOL CHctToolkitDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	try {
		SetUnhandledExceptionFilter(nullptr);

		// Disable Windows Error Reporting restart
		typedef BOOL(WINAPI *PRERR)(DWORD);
		PRERR DisableWER = (PRERR)GetProcAddress(
			GetModuleHandle(L"kernel32"), "SetErrorMode");
		if (DisableWER != nullptr) {
			DisableWER(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
		}
	}
	catch (...) {
	}
	
	// Add "About..." menu item to system menu.
	m_hMutex = CreateMutexA(NULL, FALSE, "hct-toolkit-mutex");
	DWORD dwMutexWaitResult = WaitForSingleObject(m_hMutex, 0);
	if (dwMutexWaitResult != WAIT_OBJECT_0) {
		CDialogEx::OnClose();
		return FALSE;
	}

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CString version;
	version = L"Device Calibrator Ver.";
	version += atow(OctSystem::Loader::getToolkitVersion()).c_str();
	SetWindowTextW(version);
	
	// OctSystem::Normative::initNormative();
	// return TRUE;  // return TRUE  unless you set the focus to a control

	// Check ID and PASSWORD
	// checkPermission();

	// TODO: Add extra initialization here
	GlobalSettings::applyEngineerModeSettings();
	if (OctSystem::Loader::initializeSystem()) {
		version += L" (Firmware Ver.";
		version += atow(OctSystem::Loader::getFirmwareVersion()).c_str();
		version += L", FPGA Ver.";
		version += atow(OctSystem::Loader::getFPGAVersion()).c_str();
		version += L")";

		if (GlobalSettings::useOctEnable()) {
			if (!GlobalSettings::useFundusEnable() && !GlobalSettings::useFundusFILR_Enable()) {	// HOCT-1
				GetDlgItem(IDC_BUTTON_COLOR)->EnableWindow(FALSE);
			}
		}
		else {																						// HFC-1
			GetDlgItem(IDC_BUTTON_OCT_SCAN_TEST)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_SIGNAL)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_DISPERSION)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_SPECTRO_CALIB)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_ANGIO_PATTERN)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_TOPOGRAPHY_PATTERN)->EnableWindow(FALSE);
		}
		if (OctSystem::Loader::getNumberOfWarnings() > 0) {
			AfxMessageBox(_T("Warnings occurred during initialization!\r\nCheck the log file."));
		}

		//firmware update alarm (3.11)

		/*auto versionStr = atow(OctSystem::Loader::getFirmwareVersion()).c_str();

		int n1 = versionStr[0] - L'0';
		int n2 = versionStr[3] - L'0';
		int n3 = versionStr[4] - L'0';

		if (n1 >= 3) {
			auto versionNum = n1 * 100 + n2 * 10 + n3;
			if (versionNum < 311) {
				AfxMessageBox(_T("Firmware Update Required!"));
			}
		}*/
	}
	else {
		AfxMessageBox(_T("System initialization failed!"));
		OctSystem::Loader::releaseSystem();
	}

	SetWindowTextW(version);
	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CHctToolkitDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHctToolkitDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHctToolkitDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CHctToolkitDlg::OnBnClickedButtonMainboard()
{
	// TODO: Add your control notification handler code here
	DlgMainboardTest dlg;
	dlg.DoModal();
	return;
}


void CHctToolkitDlg::OnBnClickedButtonLongrun()
{
	// TODO: Add your control notification handler code here
	DlgLongRunTest dlg;
	dlg.DoModal();
	return;
}


void CHctToolkitDlg::OnClickedButtonOctScanTest()
{
	// TODO: Add your control notification handler code here
	DlgScanPattern dlg;
	dlg.DoModal();
}


void CHctToolkitDlg::OnBnClickedButtonSpectroCalib()
{
	// TODO: Add your control notification handler code here
	DlgSpectrometerCalibration dlg;
	dlg.DoModal();
}


void CHctToolkitDlg::OnBnClickedButtonDispersion()
{
	// TODO: Add your control notification handler code here
	DlgDispersionCompensation dlg;
	dlg.DoModal();
}


void CHctToolkitDlg::OnBnClickedButtonKerato()
{
	// TODO: Add your control notification handler code here
	DlgKeratoCalibration dlg;
	dlg.DoModal();
	return;
}


void CHctToolkitDlg::OnBnClickedButtonColor()
{
	// TODO: Add your control notification handler code here
	DlgColorCameraTest dlg;
	dlg.DoModal();
	return;
}


void CHctToolkitDlg::OnBnClickedButtonSignal()
{
	// TODO: Add your control notification handler code here
	DlgSignalTester dlg;
	dlg.DoModal();
	return;
}


void CHctToolkitDlg::OnBnClickedButtonSysCalib()
{
	// TODO: Add your control notification handler code here
	DlgSysCalibration dlg;
	dlg.DoModal();
	return;
}


void CHctToolkitDlg::OnBnClickedButtonFactorySetup()
{
	// TODO: Add your control notification handler code here
	DlgFactorySetup dlg;
	dlg.DoModal();
	return;
}


BOOL CHctToolkitDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialogEx::DestroyWindow();
}


void CHctToolkitDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here

	return;
}


void CHctToolkitDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	OctSystem::Loader::releaseSystem();
	CloseHandle(m_hMutex);
	CDialogEx::OnClose();
}


void CHctToolkitDlg::OnBnClickedButtonExit()
{
	// TODO: Add your control notification handler code here
	// CDialogEx::OnOK();
	SendMessage(WM_CLOSE, NULL, NULL);
}


void CHctToolkitDlg::OnBnClickedButtonSaveSystem()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	int ret = AfxMessageBox(_T("The current System configuration will be stored in mainboard\r\n Are you sure?"), MB_OKCANCEL);
	if (ret == IDOK) {
		if (Loader::saveConfiguration()) {
			AfxMessageBox(_T("System configuration saved!"));
		}
		else {
			AfxMessageBox(_T("Failed to save System configuration!"));
		}
	}
	EndWaitCursor();
	return;
}


void CHctToolkitDlg::OnBnClickedButtonLoadSystem()
{
	// TODO: Add your control notification handler code here	
	BeginWaitCursor();
	int ret = AfxMessageBox(_T("System configuration will be reloaded\r\n Are you sure?"), MB_OKCANCEL);
	if (ret == IDOK) {
		if (Loader::loadConfiguration()) {
			AfxMessageBox(_T("System configuration loaded!"));
		}
		else {
			AfxMessageBox(_T("Failed to load System configuration!"));
		}
	}
	EndWaitCursor();
	return;
}


void CHctToolkitDlg::OnBnClickedButtonResetUsb()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Controller::getMainBoard()->openFTDIdevices(true)) {
		AfxMessageBox(_T("Reset USB channel ok!"));
	}
	else {
		AfxMessageBox(_T("Reset USB channel failed!"));
	}
	EndWaitCursor();
	return;
}


void CHctToolkitDlg::OnBnClickedButtonResetUsbDriver()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Controller::getMainBoard()->openFTDIdevices(true, true)) {
		AfxMessageBox(_T("Reset FTDI driver ok!"));
	}
	else {
		AfxMessageBox(_T("Reset FTDI driver failed!"));
	}
	EndWaitCursor();
	return;
}


void CHctToolkitDlg::OnBnClickedButtonResetSystem()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Controller::getMainBoard()->rebootSystem()) {
		AfxMessageBox(_T("Reboot system ok!, it will take about 30 secs..."));
		Loader::releaseSystem();
		Sleep(30000);
		if (!Loader::initializeSystem()) {
			AfxMessageBox(_T("System re-initialization failed!"));
		}
	}
	else {
		AfxMessageBox(_T("Reboot system failed!"));
	}
	EndWaitCursor();
	return;
}


void CHctToolkitDlg::OnBnClickedButtonPackageMode()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	Controller::getMainBoard()->performPackagingMode();
	EndWaitCursor();
	return;
}


void CHctToolkitDlg::OnBnClickedButtonAngioPattern()
{
	// TODO: Add your control notification handler code here
	DlgAngioPattern dlg;
	dlg.DoModal();
}


void CHctToolkitDlg::checkPermission()
{
	DlgUserLogin dlg;
	if (dlg.DoModal() == IDCANCEL) {
		if (!dlg.isLoggedOn()) {
			PostMessage(WM_QUIT);
		}
	}
	return;
}


void CHctToolkitDlg::OnBnClickedButtonTopographyPattern()
{
	// TODO: Add your control notification handler code here
	DlgTopographyPattern dlg;
	dlg.DoModal();
}


BOOL CHctToolkitDlg::OnQueryEndSession()
{
	if (!CDialogEx::OnQueryEndSession())
		return FALSE;

	// TODO:  Add your specialized query end session code here
		return TRUE;
}


void CHctToolkitDlg::OnBnClickedButtonSystemSettings()
{
	// TODO: Add your control notification handler code here
	DlgSystemSettings dlg;
	dlg.DoModal();
}
