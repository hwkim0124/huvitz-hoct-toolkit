// EyeScapeVd.h : Implementation of the CEyeScapeVd class



// CEyeScapeVd implementation

// code generated on 2019년 2월 26일 화요일, 오후 4:50

#include "stdafx.h"
#include "EyeScapeVd.h"
IMPLEMENT_DYNAMIC(CEyeScapeVd, CRecordset)

CEyeScapeVd::CEyeScapeVd(CDatabase* pdb)
	: CRecordset(pdb)
{
	m_id = L"";
	m_vn = L"";
	m_dop;
	m_dr = L"";
	m_s = L"";
	m_t = L"";
	m_memo = L"";
	m_dx = L"";
	m_dx1 = L"";
	m_dx2 = L"";
	m_Addendum = "";
	m_CPTCategory = L"";
	m_ICD9CodeNum = L"";
	m_ICD9CodeDescrip = L"";
	m_ClinicalFindings = "";
	m_ComparativeData = "";
	m_ClinicalManagement = "";
	m_DocsName = L"";
	m_StudyID = 0;
	m_StudyInstanceUID = L"";
	m_Hidden = L"";
	m_nFields = 21;
	m_nDefaultType = dynaset;
}

void CEyeScapeVd::SetConnectInfo(CString ci)
{
	m_ConnectInfo = ci + _T("DriverId=281;FIL=MS Access;MaxBufferSize=2048;PageTimeout=5;UID=admin;");
	//return _T("FILEDSN=esc_odbc.dsn;DBQ=E:\\HOCT-9000\\Source\\Repos\\huvitz-oct-9000\\HCT-9000\\Win32\\Release\\esc.mdb;DriverId=281;FIL=MS Access;MaxBufferSize=2048;PageTimeout=5;UID=admin;");
}

//#error Security Issue: The connection string may contain a password
// The connection string below may contain plain text passwords and/or
// other sensitive information. Please remove the #error after reviewing
// the connection string for any security related issues. You may want to
// store the password in some other form or use a different user authentication.
CString CEyeScapeVd::GetDefaultConnect()
{
	//return _T("DSN=ESC_DSN;DBQ=E:\\huvitz\\nidek_export\\backup2\\esc.mdb;DriverId=281;FIL=MS Access;MaxBufferSize=2048;PageTimeout=5;UID=admin;");
	return m_ConnectInfo;
}

CString CEyeScapeVd::GetDefaultSQL()
{
	return _T("[vd]");
}

void CEyeScapeVd::DoFieldExchange(CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
// Macros such as RFX_Text() and RFX_Int() are dependent on the
// type of the member variable, not the type of the field in the database.
// ODBC will try to automatically convert the column value to the requested type
	RFX_Text(pFX, _T("[id]"), m_id);
	RFX_Text(pFX, _T("[vn]"), m_vn);
	RFX_Date(pFX, _T("[dop]"), m_dop);
	RFX_Text(pFX, _T("[dr]"), m_dr);
	RFX_Text(pFX, _T("[s]"), m_s);
	RFX_Text(pFX, _T("[t]"), m_t);
	RFX_Text(pFX, _T("[memo]"), m_memo);
	RFX_Text(pFX, _T("[dx]"), m_dx);
	RFX_Text(pFX, _T("[dx1]"), m_dx1);
	RFX_Text(pFX, _T("[dx2]"), m_dx2);
	RFX_Text(pFX, _T("[Addendum]"), m_Addendum);
	RFX_Text(pFX, _T("[CPTCategory]"), m_CPTCategory);
	RFX_Text(pFX, _T("[ICD9CodeNum]"), m_ICD9CodeNum);
	RFX_Text(pFX, _T("[ICD9CodeDescrip]"), m_ICD9CodeDescrip);
	RFX_Text(pFX, _T("[ClinicalFindings]"), m_ClinicalFindings);
	RFX_Text(pFX, _T("[ComparativeData]"), m_ComparativeData);
	RFX_Text(pFX, _T("[ClinicalManagement]"), m_ClinicalManagement);
	RFX_Text(pFX, _T("[DocsName]"), m_DocsName);
	RFX_Long(pFX, _T("[StudyID]"), m_StudyID);
	RFX_Text(pFX, _T("[StudyInstanceUID]"), m_StudyInstanceUID);
	RFX_Text(pFX, _T("[Hidden]"), m_Hidden);

}
/////////////////////////////////////////////////////////////////////////////
// CEyeScapeVd diagnostics

#ifdef _DEBUG
void CEyeScapeVd::AssertValid() const
{
	CRecordset::AssertValid();
}

void CEyeScapeVd::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG


