// DlgSystemSettings.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "afxdialogex.h"
#include "DlgSystemSettings.h"
#include "DiscTestConfig.h"

#include "OctSystem2.h"
#include "OctDevice2.h"
#include "OctConfig2.h"
#include "CppUtil2.h"
#include "DiscTestConfig.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


// DlgSystemSettings dialog

IMPLEMENT_DYNAMIC(DlgSystemSettings, CDialogEx)

DlgSystemSettings::DlgSystemSettings(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLGSYSTEMSETTINGS, pParent)
{

}

DlgSystemSettings::~DlgSystemSettings()
{
}

void DlgSystemSettings::initControls(void)
{
}

void DlgSystemSettings::loadControls(void)
{
	SystemSettings* sset = SystemProfile::getSystemSettings();
	CString text;

	text.Empty();
	if (sset->serialNumber().size() > 0) {
		text = CString(atow(sset->serialNumber()).c_str());
	}
	editSerialNo.SetWindowTextW(text);

	cmbModelType.AddString(_T("HOCT-1F"));
	cmbModelType.AddString(_T("HOCT-1"));

#ifdef INTEGRATED_VERSION
	cmbModelType.AddString(_T("HOCT-1F_NV"));
	cmbModelType.AddString(_T("HOCT-1_NV"));
	cmbModelType.AddString(_T("HOCT-1FA"));
	cmbModelType.AddString(_T("HOCT-1A"));
	cmbModelType.AddString(_T("HOCT-1FA_NV"));
	cmbModelType.AddString(_T("HOCT-1A_NV"));
	cmbModelType.AddString(_T("HFC-1"));
#endif

	auto type = sset->getModelType();
	cmbModelType.SetCurSel(type);

	type = sset->getOctGrabberType();
	int value = (int)(type == (int)OctGrabberType::MIL_FRAME_GRABBER);
	CheckDlgButton(IDC_RADIO_OCT_GRAB1, value);
	value = (int)(type == (int)OctGrabberType::USB_LINE_CAMERA);
	CheckDlgButton(IDC_RADIO_OCT_GRAB2, value);
	if (type != (int)OctGrabberType::USB_LINE_CAMERA && type != (int)OctGrabberType::MIL_FRAME_GRABBER) {
		CheckDlgButton(IDC_RADIO_OCT_GRAB1, 1);
		CheckDlgButton(IDC_RADIO_OCT_GRAB2, 0);
		_grabberType = 0;
	}

	type = sset->getOctLineCameraMode();
	CheckDlgButton(IDC_RADIO_OCT_LINE1, (int)(type == (int)OctLineCameraMode::LINE_CAMERA_68K));
	CheckDlgButton(IDC_RADIO_OCT_LINE2, (int)(type == (int)OctLineCameraMode::LINE_CAMERA_80K_TEST));
	CheckDlgButton(IDC_RADIO_OCT_LINE3, (int)(type == (int)OctLineCameraMode::LINE_CAMERA_80K));
	if (type != (int)OctLineCameraMode::LINE_CAMERA_80K_TEST && type != (int)OctLineCameraMode::LINE_CAMERA_80K) {
		CheckDlgButton(IDC_RADIO_OCT_LINE1, 1);
	}

	text.Format(_T("%d"), sset->getTriggerForePadd(0));
	editForePadd1.SetWindowTextW(text);
	text.Format(_T("%d"), sset->getTriggerForePadd(1));
	editForePadd2.SetWindowTextW(text);
	text.Format(_T("%d"), sset->getTriggerForePadd(2));
	editForePadd3.SetWindowTextW(text);

	text.Format(_T("%d"), sset->getTriggerPostPadd(0));
	editPostPadd1.SetWindowTextW(text);
	text.Format(_T("%d"), sset->getTriggerPostPadd(1));
	editPostPadd2.SetWindowTextW(text);
	text.Format(_T("%d"), sset->getTriggerPostPadd(2));
	editPostPadd3.SetWindowTextW(text);

	type = sset->getSldFaultDetection();
	CheckDlgButton(IDC_RADIO_SLD_FAULT1, (int)(type != 1));
	CheckDlgButton(IDC_RADIO_SLD_FAULT2, (int)(type == 1));

	text.Format(_T("%d"), sset->getSldFaultTimer());
	editFaultTimer.SetWindowTextW(text);
	text.Format(_T("%d"), sset->getSldFaultThreshold());
	editFaultThresh.SetWindowTextW(text);

	type = sset->getRetinaTrackingSpeed();
	CheckDlgButton(IDC_RADIO_RETINA_TRACK1, (int)(type == (int)RetinaTrackingSpeed::TRACK_CAMERA_15FPS));
	CheckDlgButton(IDC_RADIO_RETINA_TRACK2, (int)(type != (int)RetinaTrackingSpeed::TRACK_CAMERA_15FPS));
	
	changeDiscTestStauts();
	UpdateData(TRUE);

	return;
}

void DlgSystemSettings::saveControls(void)
{
	SystemSettings* sset = SystemProfile::getSystemSettings();
	CString text;

	editSerialNo.GetWindowTextW(text);
	std::wstring wstr(text);
	std::string sstr = wtoa(wstr);
	sset->setSerialNumber(sstr);

	int sel = cmbModelType.GetCurSel();
	sset->setModelType(sel);

	if (_grabberType == 1) {
		sset->setOctGrabberType((int)OctGrabberType::USB_LINE_CAMERA);
	}
	else {
		sset->setOctGrabberType((int)OctGrabberType::MIL_FRAME_GRABBER);
	}

	if (_lineCameraMode == 1) {
		sset->setOctLineCameraMode((int)OctLineCameraMode::LINE_CAMERA_80K_TEST);
	}
	else if (_lineCameraMode == 2) {
		sset->setOctLineCameraMode((int)OctLineCameraMode::LINE_CAMERA_80K);
	}
	else {
		sset->setOctLineCameraMode((int)OctLineCameraMode::LINE_CAMERA_68K);
	}
	
	editForePadd1.GetWindowTextW(text);
	if (text.GetLength() > 0) {
		sset->setTriggerForePadd(0, _ttoi(text));
	}
	editForePadd2.GetWindowTextW(text);
	if (text.GetLength() > 0) {
		sset->setTriggerForePadd(1, _ttoi(text));
	}
	editForePadd3.GetWindowTextW(text);
	if (text.GetLength() > 0) {
		sset->setTriggerForePadd(2, _ttoi(text));
	}

	editPostPadd1.GetWindowTextW(text);
	if (text.GetLength() > 0) {
		sset->setTriggerPostPadd(0, _ttoi(text));
	}
	editPostPadd2.GetWindowTextW(text);
	if (text.GetLength() > 0) {
		sset->setTriggerPostPadd(1, _ttoi(text));
	}
	editPostPadd3.GetWindowTextW(text);
	if (text.GetLength() > 0) {
		sset->setTriggerPostPadd(2, _ttoi(text));
	}

	if (_faultDetection == 1) {
		sset->setSldFaultDetection(1);
	}
	else {
		sset->setSldFaultDetection(0);
	}

	editFaultTimer.GetWindowTextW(text);
	if (text.GetLength() > 0) {
		sset->setSldFaultTimer(_ttoi(text));
	}
	else {
		sset->setSldFaultTimer(0);
	}

	editFaultThresh.GetWindowTextW(text);
	if (text.GetLength() > 0) {
		sset->setSldFaultThreshold(_ttoi(text));
	}
	else {
		sset->setSldFaultThreshold(0);
	}

	if (_retinaTracking == 1) {
		sset->setRetinaTrackingSpeed((int)RetinaTrackingSpeed::TRACK_CAMERA_30FPS);
	}
	else {
		sset->setRetinaTrackingSpeed((int)RetinaTrackingSpeed::TRACK_CAMERA_15FPS);
	}
	return;
}

BOOL DlgSystemSettings::IsExist(CString path)
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

void DlgSystemSettings::updateOctGrabberType(UINT value)
{
	if (value == IDC_RADIO_OCT_GRAB1) {
		CheckDlgButton(IDC_RADIO_OCT_LINE1, true);
		CheckDlgButton(IDC_RADIO_OCT_LINE2, false);
		CheckDlgButton(IDC_RADIO_OCT_LINE3, false);

		GetDlgItem(IDC_RADIO_OCT_LINE2)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_OCT_LINE3)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_RADIO_OCT_LINE2)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_OCT_LINE3)->EnableWindow(TRUE);
	}

	UpdateData(TRUE);
}

void DlgSystemSettings::updateOctLineCameraMode(UINT value)
{
	UpdateData(TRUE);
}

void DlgSystemSettings::updateSldFaultDetection(UINT value)
{
	UpdateData(TRUE);
}

void DlgSystemSettings::updateRetinaTrackingSpeed(UINT value)
{
	UpdateData(TRUE);
}

void DlgSystemSettings::changeDiscTestStauts(void)
{
	CString text;

	if (DiscTest::IsEnabled()) {
		text.Format(_T("On"));
	}
	else {
		text.Format(_T("Off"));
	}
	editDiscTestStatus.SetWindowTextW(text);
}

void DlgSystemSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SERIAL_NO, editSerialNo);
	DDX_Control(pDX, IDC_COMBO_MODELTYPE, cmbModelType);
	DDX_Radio(pDX, IDC_RADIO_OCT_GRAB1, _grabberType);
	DDX_Radio(pDX, IDC_RADIO_OCT_LINE1, _lineCameraMode);
	DDX_Radio(pDX, IDC_RADIO_SLD_FAULT1, _faultDetection);
	DDX_Radio(pDX, IDC_RADIO_RETINA_TRACK1, _retinaTracking);
	DDX_Control(pDX, IDC_EDIT_FAULT_THRESH, editFaultThresh);
	DDX_Control(pDX, IDC_EDIT_FAULT_TIMER, editFaultTimer);
	DDX_Control(pDX, IDC_EDIT_DISC_TEST_STATUS, editDiscTestStatus);
	DDX_Control(pDX, IDC_EDIT_TRIGGER_FORE_PADD1, editForePadd1);
	DDX_Control(pDX, IDC_EDIT_TRIGGER_FORE_PADD2, editForePadd2);
	DDX_Control(pDX, IDC_EDIT_TRIGGER_FORE_PADD3, editForePadd3);
	DDX_Control(pDX, IDC_EDIT_TRIGGER_POST_PADD1, editPostPadd1);
	DDX_Control(pDX, IDC_EDIT_TRIGGER_POST_PADD2, editPostPadd2);
	DDX_Control(pDX, IDC_EDIT_TRIGGER_POST_PADD3, editPostPadd3);
}


BEGIN_MESSAGE_MAP(DlgSystemSettings, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, &DlgSystemSettings::OnBnClickedButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, &DlgSystemSettings::OnBnClickedButtonImport)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &DlgSystemSettings::OnBnClickedButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &DlgSystemSettings::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_CHINA, &DlgSystemSettings::OnBnClickedButtonApplyChina)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_CHINA, &DlgSystemSettings::OnBnClickedButtonDeleteChina)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_DISCTEST, &DlgSystemSettings::OnBnClickedButtonApplyDisctest)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_DISCTEST, &DlgSystemSettings::OnBnClickedButtonDeleteDisctest)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_OCT_GRAB1, IDC_RADIO_OCT_GRAB2, DlgSystemSettings::updateOctGrabberType)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_OCT_LINE1, IDC_RADIO_OCT_LINE3, DlgSystemSettings::updateOctLineCameraMode)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_SLD_FAULT1, IDC_RADIO_SLD_FAULT2, DlgSystemSettings::updateSldFaultDetection)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_RETINA_TRACK1, IDC_RADIO_RETINA_TRACK2, DlgSystemSettings::updateRetinaTrackingSpeed)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &DlgSystemSettings::OnBnClickedButtonClose)
END_MESSAGE_MAP()


// DlgSystemSettings message handlers


void DlgSystemSettings::OnBnClickedButtonExport()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	saveControls();
	if (Loader::saveSetupProfile(false, true)) {
		Loader::applySystemProfile(true);
		AfxMessageBox(_T("System settings saved to config file!"));
	}
	else {
		AfxMessageBox(_T("Failed to save System settings to config file!"));
	}
	EndWaitCursor();
	return;
}


void DlgSystemSettings::OnBnClickedButtonImport()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Loader::loadSetupProfile(true, true)) {
		loadControls();
		AfxMessageBox(_T("System settings loaded from config file!"));
	}
	else {
		AfxMessageBox(_T("Failed to load System settings from config file!"));
	}
	EndWaitCursor();
	return;
}


void DlgSystemSettings::OnBnClickedButtonLoad()
{
	// TODO: Add your control notification handler code here
	BeginWaitCursor();
	if (Loader::loadSetupProfile(false, false)) {
		loadControls();
		AfxMessageBox(_T("System settings loaded from mainboard!"));
	}
	else {
		AfxMessageBox(_T("Failed to load System settings from mainboard!"));
	}
	EndWaitCursor();
	return;
}


void DlgSystemSettings::OnBnClickedButtonSave()
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


void DlgSystemSettings::OnBnClickedButtonApplyChina()
{
	// TODO: Add your control notification handler code here
	CRegKey cKeyValue;
	wstring modelTypePath = L"SOFTWARE\\CodeGenerator\\CodeGenerator_";
	wstring modelType = L"ModelType";

	if (cKeyValue.Open(HKEY_CURRENT_USER, modelTypePath.c_str()) == ERROR_SUCCESS) {
		cKeyValue.SetDWORDValue(modelType.c_str(), (DWORD)1);
		cKeyValue.Close();
		AfxMessageBox(_T("Apply China version successfully!"));
	}
	else {
		if (ERROR_SUCCESS != cKeyValue.Create(HKEY_CURRENT_USER, modelTypePath.c_str())) {
			AfxMessageBox(_T("Failed to Apply China version"));
		}
		cKeyValue.SetDWORDValue(modelType.c_str(), (DWORD)1);
		cKeyValue.Close();
		AfxMessageBox(_T("Apply China version successfully!"));
	}

	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	std::string path = wtoa(buffer);

	string temp1 = path + "\\" + "UserConfig.ini";
	string temp2 = path + "\\" + "HCT_DB_Sys.db";
	CString filePath1;
	CString filePath2;

	filePath1 = temp1.c_str();
	filePath2 = temp2.c_str();

	if (IsExist(filePath1)) {
		DeleteFile(filePath1.operator LPCWSTR());
	}

	if (IsExist(filePath2)) {
		DeleteFile(filePath2.operator LPCWSTR());
	}

	return;
}


void DlgSystemSettings::OnBnClickedButtonDeleteChina()
{
	// TODO: Add your control notification handler code here
	CRegKey cKeyValue;

	wstring modelTypePath = L"SOFTWARE\\CodeGenerator\\CodeGenerator_";
	wstring modelType = L"ModelType";

	if (cKeyValue.Open(HKEY_CURRENT_USER, modelTypePath.c_str()) == ERROR_SUCCESS) {
		cKeyValue.DeleteValue(modelType.c_str());
		AfxMessageBox(_T("Delete China version successfully!"));
	}
	else {
		AfxMessageBox(_T("Failed to Delete China version"));
	}

	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	std::string path = wtoa(buffer);

	string temp1 = path + "\\" + "UserConfig.ini";
	string temp2 = path + "\\" + "HCT_DB_Sys.db";
	CString filePath1;
	CString filePath2;

	filePath1 = temp1.c_str();
	filePath2 = temp2.c_str();

	if (IsExist(filePath1)) {
		DeleteFile(filePath1.operator LPCWSTR());
	}

	if (IsExist(filePath2)) {
		DeleteFile(filePath2.operator LPCWSTR());
	}

	return;
}

void DlgSystemSettings::OnBnClickedButtonApplyDisctest()
{
	CRegKey key;
	wstring path = L"SOFTWARE\\CodeGenerator\\DiscTestOption";
	wstring value = L"DiscTestEnabled";

	if (key.Create(HKEY_CURRENT_USER, path.c_str()) == ERROR_SUCCESS) {
		if (key.SetDWORDValue(value.c_str(), (DWORD)1) == ERROR_SUCCESS) {
			AfxMessageBox(_T("Apply Disc Test successfully!"));
		}
		else {
			AfxMessageBox(_T("Failed to Apply Disc Test"));
		}
		key.Close();
	}

	changeDiscTestStauts();
}

void DlgSystemSettings::OnBnClickedButtonDeleteDisctest()
{
	CRegKey key;
	wstring path = L"SOFTWARE\\CodeGenerator\\DiscTestOption";
	wstring value = L"DiscTestEnabled";

	if (key.Open(HKEY_CURRENT_USER, path.c_str()) == ERROR_SUCCESS) {
		if (key.DeleteValue(value.c_str()) == ERROR_SUCCESS) {
			AfxMessageBox(_T("Delete Disc Test successfully!"));
		}
		else {
			AfxMessageBox(_T("Failed to Delete Disc Test"));
		}
		key.Close();
	}
	else {
		AfxMessageBox(_T("Failed to Delete Disc Test"));
	}

	changeDiscTestStauts();
}


BOOL DlgSystemSettings::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	loadControls();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL DlgSystemSettings::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	return CDialogEx::PreTranslateMessage(pMsg);
}


void DlgSystemSettings::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnClose();
}


void DlgSystemSettings::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	SendMessage(WM_CLOSE, NULL, NULL);
}