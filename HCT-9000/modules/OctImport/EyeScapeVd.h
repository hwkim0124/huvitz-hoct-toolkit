// EyeScapeVd.h : Declaration of the CEyeScapeVd

#pragma once

// code generated on 2019년 2월 26일 화요일, 오후 4:50

class CEyeScapeVd : public CRecordset
{
public:
	CEyeScapeVd(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CEyeScapeVd)

// Field/Param Data

// The string types below (if present) reflect the actual data type of the
// database field - CStringA for ANSI datatypes and CStringW for Unicode
// datatypes. This is to prevent the ODBC driver from performing potentially
// unnecessary conversions.  If you wish, you may change these members to
// CString types and the ODBC driver will perform all necessary conversions.
// (Note: You must use an ODBC driver version that is version 3.5 or greater
// to support both Unicode and these conversions).

	CStringW	m_id;
	CStringW	m_vn;
	CTime	m_dop;
	CStringW	m_dr;
	CStringW	m_s;
	CStringW	m_t;
	CStringW	m_memo;
	CStringW	m_dx;
	CStringW	m_dx1;
	CStringW	m_dx2;
	CStringA	m_Addendum;
	CStringW	m_CPTCategory;
	CStringW	m_ICD9CodeNum;
	CStringW	m_ICD9CodeDescrip;
	CStringA	m_ClinicalFindings;
	CStringA	m_ComparativeData;
	CStringA	m_ClinicalManagement;
	CStringW	m_DocsName;
	long	m_StudyID;
	CStringW	m_StudyInstanceUID;
	CStringW	m_Hidden;

// Overrides
	// Wizard generated virtual function overrides
	public:

	CString		m_ConnectInfo;
	void SetConnectInfo(CString ci);

	virtual CString GetDefaultConnect();	// Default connection string

	virtual CString GetDefaultSQL(); 	// default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);	// RFX support

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};


