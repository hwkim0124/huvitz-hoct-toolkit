#include "stdafx.h"
#include "PatientDatabase.h"
#include "DatabaseUtil.h"
#include "Patient.h"
#include "Exam.h"
#include "WorkList.h"
#include "Physician.h"
#include "Operator.h"
#include "Race.h"
#include "ScanResult.h"
#include "ScanPattern.h"
#include "FundusResult.h"
#include "Device.h"
#include "DiagnosisItem.h"
#include "DiagnosisGroup.h"
#include "UserAccount.h"
#include "FetchPropertyPatient.h"
#include "FetchPropertyExam.h"
#include "FetchPropertyScanResult.h"
#include "FetchPropertyFundusResult.h"
#include <locale>
#include <codecvt>

#include "CppUtil2.h"


using namespace PatientDB;
using namespace CppUtil;

PatientDatabase::PatientDatabase()
{
	m_dbName = "HCT_DB.db";
}

PatientDatabase::PatientDatabase(string dbName)
{
	m_dbName = dbName;
}

PatientDatabase::~PatientDatabase()
{
	disconnectDB();
}

bool PatientDatabase::connectDB()
{
	CString filename = (CString)m_dbName.c_str();
	if (!PathFileExists(filename))
	{
		CString s;
		s.Format(_T("Database file is not exist : %s\n"), filename);
		AfxMessageBox(s);
		return false;
	}
	
	if (sqlite3_open(m_dbName.c_str(), &m_db) != SQLITE_OK)
	{
		CString s;
		s.Format(_T("Database open failed : %s\n"), filename);
		AfxMessageBox(s);
		return false;
	}

	// Add columns for old version
	if (!correctExamColumns()) {
		CString s;
		s.Format(_T("Database correctExamColumns() failed : %s\n"), filename);
		AfxMessageBox(s);
		return false;
	}
	// Add columns for old version
	if (!correctPatientColumns()) {
		CString s;
		s.Format(_T("Database correctPatientColumns() failed : %s\n"), filename);
		AfxMessageBox(s);
		return false;
	}

	return true;
}

void PatientDatabase::disconnectDB()
{
	sqlite3_close(m_db);
}

bool PatientDatabase::execQuery(const char* szQuery)
{
	char *szErrMsg;

	if (sqlite3_exec(m_db, szQuery, NULL, NULL, &szErrMsg) != SQLITE_OK)
	{
		return false;
	}

	return true;
}

void PatientDatabase::showErrorMsg(const char *szError)
{
	char szLog[128] = {0};
	sprintf_s(szLog, "%s\nerrmsg = %s\next_errcode = %d",
		szError, sqlite3_errmsg(m_db), sqlite3_extended_errcode(m_db));
	
	AfxMessageBox(CString(szLog));

	return;
}

void PatientDatabase::updateRaceList()
{
	//sqlite3_stmt *stmt;
	//
	//// mixed
	//if (sqlite3_prepare16_v2(m_db, L"SELECT * FROM RaceTbl WHERE Name = \"Mixed\"", -1, &stmt, NULL)
	//	== SQLITE_OK) {
	//	if (sqlite3_step(stmt) != SQLITE_ROW) {
	//		Race newRace;

	//		newRace.setIndex(7);
	//		newRace.setName(L"Mixed");
	//		newRace.setDisplayOrder(7);
	//		newRace.setStatus(Status::USE);

	//		insertRace(newRace);
	//	}
	//}

	//sqlite3_reset(stmt);
	//sqlite3_finalize(stmt);

	//return;

	sqlite3_stmt *stmt;

	//mixed -> other
	if (sqlite3_prepare16_v2(m_db, L"SELECT rowid FROM RaceTbl WHERE Name = \"Mixed\"", -1, &stmt, NULL)
		== SQLITE_OK) {
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			int rowid = sqlite3_column_int(stmt, 0);
			sqlite3_finalize(stmt);

			sqlite3_stmt *update_stmt;
			if (sqlite3_prepare16_v2(m_db, L"UPDATE RaceTbl SET Name = \"Other\" WHERE rowid = ?", -1, &update_stmt, NULL)
				== SQLITE_OK) {
				sqlite3_bind_int(update_stmt, 1, rowid);
				sqlite3_step(update_stmt);
				sqlite3_finalize(update_stmt);
			}
		}
		else {
			sqlite3_finalize(stmt);

			sqlite3_stmt *stmt_other;
			if (sqlite3_prepare16_v2(m_db, L"SELECT 1 FROM RaceTbl WHERE Name = \"Other\"", -1, &stmt_other, NULL)
				== SQLITE_OK) {
				if (sqlite3_step(stmt_other) != SQLITE_ROW) {
					Race newRace;
					newRace.setIndex(7);
					newRace.setName(L"Other");
					newRace.setDisplayOrder(7);
					newRace.setStatus(Status::USE);
					insertRace(newRace);
				}
				sqlite3_finalize(stmt_other);
			}
		}
	}
}

// Insert
bool PatientDatabase::insertPatient(Patient &patient)
{
	bool ret = true;
	sqlite3_stmt *stmt;
	const wchar_t *query = L"INSERT INTO PatientTbl (Patient_IDS, First_Name, Middle_Name, Last_Name, Gender, Birth_Date, Refract_OD, Refract_OS, Race_ID, Physician_ID, Operator_ID, Description, Create_Time, Update_Time, Need_Transfer, Status, Modality, Accession_Number, Study_Description, Series_Description) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	if (sqlite3_prepare16_v2(m_db, query, -1, &stmt, NULL) == SQLITE_OK)
	{
		sqlite3_bind_text16(stmt, 1, patient.getPatientId().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 2, patient.getFirstName().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 3, patient.getMiddleName().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 4, patient.getLastName().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 5, int(patient.getGender()));
		sqlite3_bind_text16(stmt, 6, patient.getBirthDate().Format(L"%Y-%m-%d %H:%M:%S"), -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 7, patient.getRefractOD());
		sqlite3_bind_double(stmt, 8, patient.getRefractOS());
		sqlite3_bind_int(stmt, 9, patient.getRaceID());
		sqlite3_bind_int(stmt, 10, patient.getPhysicianIdx());
		sqlite3_bind_int(stmt, 11, patient.getOperatorIdx());
		sqlite3_bind_text16(stmt, 12, patient.getDescription().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 13, patient.getCreateTime().Format(L"%Y-%m-%d %H:%M:%S"), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 14, patient.getUpdateTime().Format(L"%Y-%m-%d %H:%M:%S"), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 15, patient.getNeedTransfer());
		sqlite3_bind_int(stmt, 16, int(patient.getStatus()));
		sqlite3_bind_text16(stmt, 17, patient.getModality().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 18, patient.getAccessionNumber().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 19, patient.getStudyDescription().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 20, patient.getSeriesDescription().c_str(), -1, SQLITE_STATIC);

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			showErrorMsg("Insert Patient Failed");
			ret = false;
		}
	}
	else
	{
		LogW() << "Insert Patient Failed - 1";
		ret = false;
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return ret;
}

bool PatientDatabase::insertExam(Exam &exam)
{
	bool ret = true;
	sqlite3_stmt *stmt;
	const wchar_t *query = L"INSERT INTO ExamTbl (Exam_Date, Patient_ID, Diagnosis, Comment, File_Path, Status, Modality, Scheduled_Date, Scheduled_Time, Study_InstanceUID) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	if (sqlite3_prepare16_v2(m_db, query, -1, &stmt, NULL) == SQLITE_OK)
	{
		sqlite3_bind_text16(stmt, 1, exam.getExamDate().Format(L"%Y-%m-%d %H:%M:%S"), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, exam.getPatientIdx());
		sqlite3_bind_text16(stmt, 3, exam.getDiagnosis().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 4, exam.getComment().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 5, exam.getFilePath().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 6, (int)exam.getStatus());
		sqlite3_bind_text16(stmt, 7, exam.getModality().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 8, exam.getScheduledDate().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 9, exam.getScheduledTime().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 10, exam.getStudyInstanceUID().c_str(), -1, SQLITE_STATIC);


		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			LogW() << "Insert Exam Failed - 1";
			//showErrorMsg("Insert Exam Failed");
			ret = false;
		}
	}
	else
	{
		LogW() << "Insert Exam Failed - 1";
		ret = false;
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	
	return ret;
}

bool PatientDatabase::insertScanPattern(ScanPattern &pattern)
{
	bool ret = true;
	sqlite3_stmt *stmt;
	const wchar_t *query = L"INSERT INTO ScanPatternTbl (Code, Name, Version, Scan_Type, Status) VALUES (?, ?, ?, ?, ?)";

	if (sqlite3_prepare16_v2(m_db, query, -1, &stmt, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, pattern.getCode());
		sqlite3_bind_text16(stmt, 2, pattern.getName().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 3, pattern.getVersion().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 4, (int)pattern.getScanType());
		sqlite3_bind_int(stmt, 5, (int)pattern.getStatus());

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			showErrorMsg("Insert ScanPattern Failed");
			ret = false;
		}
	}
	else
	{
		LogW() << "Insert ScanPattern Failed - 1";
		ret = false;
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return ret;
}

bool PatientDatabase::insertScanResult(ScanResult &result)
{
	bool ret = true;
	sqlite3_stmt *stmt;
	const wchar_t *query = L"INSERT INTO ScanResultTbl (Exam_ID, Pattern_ID, Device_ID, Measure_Time, Eye_Side, Fixation_Target, SSI, SQI, A_scans, B_scans, Scan_Width, Scan_Height, Scan_Direction, X_Offset, Y_Offset, Baseline, File_Path, Status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	if (sqlite3_prepare16_v2(m_db, query, -1, &stmt, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, result.getExamIdx());
		sqlite3_bind_int(stmt, 2, result.getPatternIdx());
		sqlite3_bind_int(stmt, 3, result.getDeviceIdx());
		sqlite3_bind_text16(stmt, 4, result.getMeasureTime().Format(L"%Y-%m-%d %H:%M:%S"), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 5, (int)result.getEyeSide());
		sqlite3_bind_int(stmt, 6, (int)result.getFixationTarget());
		sqlite3_bind_int(stmt, 7, result.getSSI());
		sqlite3_bind_int(stmt, 8, result.getSQI());
		sqlite3_bind_int(stmt, 9, result.getAScans());
		sqlite3_bind_int(stmt, 10, result.getBScans());
		sqlite3_bind_double(stmt, 11, result.getScanWidth());
		sqlite3_bind_double(stmt, 12, result.getScanHeight());
		sqlite3_bind_int(stmt, 13, (int)result.getScanDirection());
		sqlite3_bind_double(stmt, 14, result.getXOffset());
		sqlite3_bind_double(stmt, 15, result.getYOffset());
		sqlite3_bind_int(stmt, 16, result.getBaseLine());
		sqlite3_bind_text16(stmt, 17, result.getFilePath().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 18, (int)result.getStatus());

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			showErrorMsg("Insert ScanResult Failed");
			ret = false;
		}
	}
	else
	{
		LogW() << "Insert ScanResult Failed - 1";
		ret = false;
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return ret;
}

bool PatientDatabase::insertFundusResult(FundusResult &result)
{
	bool ret = true;
	sqlite3_stmt *stmt;
	const wchar_t *query = L"INSERT INTO FundusResultTbl (Exam_ID, Device_ID, Measure_Time, Eye_Side, Fixation_Target, Flash_Level, Shutter_Speed, ISO, Panorma, Stereo, File_Path, Status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
	
	if (sqlite3_prepare16_v2(m_db, query, -1, &stmt, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, result.getExamIdx());
		sqlite3_bind_int(stmt, 2, result.getDeviceIdx());
		sqlite3_bind_text16(stmt, 3, result.getMeasureTime().Format(L"%Y-%m-%d %H:%M:%S"), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 4, (int)result.getEyeSide());
		sqlite3_bind_int(stmt, 5, (int)result.getFixationTarget());
		sqlite3_bind_int(stmt, 6, result.getFlashLevel());
		sqlite3_bind_int(stmt, 7, result.getShutterSpeed());
		sqlite3_bind_int(stmt, 8, result.getISO());
		sqlite3_bind_int(stmt, 9, result.getPanorma());
		sqlite3_bind_int(stmt, 10, result.getStereo());
		sqlite3_bind_text16(stmt, 11, result.getFilePath().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 12, (int)result.getStatus());

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			showErrorMsg("Insert FundusResult Failed");
			ret = false;
		}
	}
	else
	{
		LogW() << "Insert FundusResult Failed - 1";
		ret = false;
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return ret;
}

bool PatientDatabase::insertPhysician(Physician &physician)
{
	if (!connectDB())
	{
		return false;
	}

	//setup converter
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	std::string name = converter.to_bytes(physician.getName());
	std::string desc = converter.to_bytes(physician.getDescription());
	Status status = physician.getStatus();

	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "INSERT INTO PhysicianTbl (Name, Description, Status) VALUES ('%s', '%s', %d)",
		name.c_str(),
		desc.c_str(),
		status);

	if (!execQuery((const char*)szQuery))
	{
		disconnectDB();
		return false;
	}


	disconnectDB();
	return true;
}

bool PatientDatabase::insertOperator(Operator &oper)
{
	if (!connectDB()) {
		return false;
	}

	//setup converter
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	std::string name = converter.to_bytes(oper.getName());
	std::string desc = converter.to_bytes(oper.getDescription());
	Status status = oper.getStatus();

	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "INSERT INTO OperatorTbl (Name, Description, Status) VALUES ('%s', '%s', %d)",
		name.c_str(),
		desc.c_str(),
		status);

	if (!execQuery((const char*)szQuery))
	{
		disconnectDB();
		return false;
	}


	disconnectDB();
	return true;
}

//bool PatientDatabase::insertDiagnosisGroup(DiagnosisGroup &group)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "INSERT INTO DiagnosisGroupTbl (Name, Display_Order, Status) VALUES ('%s', %d, %d)",
//		group.getName().c_str(),
//		group.getDisplayOrder(),
//		group.getStatus());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//
//	disconnectDB();
//	return true;
//}
//
//bool PatientDatabase::insertDiagnosisItem(DiagnosisItem &item)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "INSERT INTO DiagnosisItemTbl (Name, Group_ID, Display_Order, Status) VALUES ('%s', %d, %d, %d)",
//		item.getName().c_str(),
//		item.getGroupIdx(),
//		item.getDisplayOrder(),
//		item.getStatus());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//
//	disconnectDB();
//	return true;
//}
//
bool PatientDatabase::insertRace(Race &race)
{
	std::wstring wstrName = race.getName();
	std::string strName(wstrName.begin(), wstrName.end());

	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "INSERT INTO RaceTbl (Name, Display_Order, Status) VALUES ('%s', %d, %d)",
		strName.c_str(),
		race.getDisplayOrder(),
		race.getStatus());

	if (!execQuery((const char*)szQuery)) {
		return false;
	}

	return true;
}
//
//bool PatientDatabase::insertDevice(Device &device)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "INSERT INTO DeviceTbl (Name, Description, IP_Addr, Port, Status) VALUES ('%s', '%s, '%s', %d, %d)",
//		device.getName().c_str(),
//		device.getDescription().c_str(),
//		device.getIpAddress().c_str(),
//		device.getPort(),
//		device.getStatus());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//
//	disconnectDB();
//	return true;
//}
//
//bool PatientDatabase::insertUserAccount(UserAccount &user)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "INSERT INTO UserAccountTbl (User_ID, User_Password, Type, Create_Time, Login_Time, Logout_Time, Email_Address, Status) VALUES ('%s', '%s', %d, '%s', '%s', '%s', '%s', %d)",
//		user.getUserId().c_str(),
//		user.getPassword().c_str(),
//		user.getPermission(),
//		DatabaseUtil::getDateTimeString(user.getCreateTime()).c_str(),
//		DatabaseUtil::getDateTimeString(user.getLoginTime()).c_str(),
//		DatabaseUtil::getDateTimeString(user.getLogoutTime()).c_str(),
//		user.getEmailAddress().c_str(),
//		user.getStatus());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//
//	disconnectDB();
//	return true;
//}

// Update
bool PatientDatabase::updatePatient(Patient &patient)
{
	bool ret = true;
	sqlite3_stmt *stmt;
	const wchar_t *query = L"UPDATE PatientTbl SET Patient_IDS=?, First_Name=?, Middle_Name=?, Last_Name=?, Gender=?, Birth_Date=?, Refract_OD=?, Refract_OS=?, Race_ID=?, Physician_ID=?, Operator_ID=?, Description=?, Create_Time=?, Update_Time=?, Need_Transfer=?, Status=?, Modality=?, Accession_Number=?, Study_Description=?, Series_Description=? WHERE Serial_ID=?";

	if (sqlite3_prepare16_v2(m_db, query, -1, &stmt, NULL) == SQLITE_OK)
	{
		sqlite3_bind_text16(stmt, 1, patient.getPatientId().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 2, patient.getFirstName().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 3, patient.getMiddleName().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 4, patient.getLastName().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 5, int(patient.getGender()));
		sqlite3_bind_text16(stmt, 6, patient.getBirthDate().Format(L"%Y-%m-%d %H:%M:%S"), -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 7, patient.getRefractOD());
		sqlite3_bind_double(stmt, 8, patient.getRefractOS());
		sqlite3_bind_int(stmt, 9, patient.getRaceID());
		sqlite3_bind_int(stmt, 10, patient.getPhysicianIdx());
		sqlite3_bind_int(stmt, 11, patient.getOperatorIdx());
		sqlite3_bind_text16(stmt, 12, patient.getDescription().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 13, patient.getCreateTime().Format(L"%Y-%m-%d %H:%M:%S"), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 14, patient.getUpdateTime().Format(L"%Y-%m-%d %H:%M:%S"), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 15, patient.getNeedTransfer());
		sqlite3_bind_int(stmt, 16, int(patient.getStatus()));
		sqlite3_bind_text16(stmt, 17, patient.getModality().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 18, patient.getAccessionNumber().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 19, patient.getStudyDescription().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 20, patient.getSeriesDescription().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 21, int(patient.getIndex()));

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			showErrorMsg("Update Patient Failed");
			ret = false;
		}
	}
	else
	{
		ret = false;
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}

bool PatientDatabase::updateExam(Exam &exam)
{
	bool ret = true;
	sqlite3_stmt *stmt;
	const wchar_t *query = L"UPDATE ExamTbl SET Exam_Date=?, Patient_ID=?, Diagnosis=?, Comment=?, File_Path=?, Status=?, Modality=?, Scheduled_Date=?, Scheduled_Time=?, Study_InstanceUID=? WHERE Serial_ID=?";

	if (sqlite3_prepare16_v2(m_db, query, -1, &stmt, NULL) == SQLITE_OK)
	{
		sqlite3_bind_text16(stmt, 1, exam.getExamDate().Format(L"%Y-%m-%d %H:%M:%S"), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, exam.getPatientIdx());
		sqlite3_bind_text16(stmt, 3, exam.getDiagnosis().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 4, exam.getComment().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 5, exam.getFilePath().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 6, (int)exam.getStatus());
		sqlite3_bind_text16(stmt, 7, exam.getModality().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 8, exam.getScheduledDate().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 9, exam.getScheduledTime().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text16(stmt, 10, exam.getStudyInstanceUID().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 11, exam.getIndex());

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			showErrorMsg("Update Exam Failed");
			ret = false;
		}
	}
	else
	{
		ret = false;
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return ret;
}

//bool PatientDatabase::updateScanPattern(ScanPattern &pattern)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "UPDATE ScanPatternTbl SET Code='%d', Name='%s', Version='%s', Scan_Type=%d, Status=%d WHERE Serial_ID=%d",
//		pattern.getCode(),
//		pattern.getName().c_str(),
//		pattern.getVersion().c_str(),
//		pattern.getScanType(),
//		pattern.getStatus(),
//		pattern.getIndex());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//	disconnectDB();
//	return true;
//}
//bool PatientDatabase::updateScanResult(ScanResult &result)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "UPDATE ScanResultTbl SET Exam_ID=%d, Pattern_ID=%d, Device_ID=%d, Measure_Time='%s', Eye_Side=%d, SSI=%d, SQI=%d, A_scans=%d, B_scans=%d, Scan_Width=%f, Scan_Height=%f, Scan_Direction=%d, X_Offset=%f, Y_Offset=%f, Baseline=%d, File_Path='%s', Status=%d WHERE Serial_ID=%d",
//		result.getExamIdx(),
//		result.getPatternIdx(),
//		result.getDeviceIdx(),
//		DatabaseUtil::getDateTimeString(result.getMeasureTime()).c_str(),
//		result.getEyeSide(),
//		result.getSSI(),
//		result.getSQI(),
//		result.getAScans(),
//		result.getBScans(),
//		result.getScanWidth(),
//		result.getScanHeight(),
//		result.getScanDirection(),
//		result.getXOffset(),
//		result.getYOffset(),
//		result.getBaseLine(),
//		result.getFilePath().c_str(),
//		result.getStatus(),
//		result.getIndex());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//	disconnectDB();
//	return true;
//}
//
//bool PatientDatabase::updateFundusResult(FundusResult &result)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "UPDATE FundusResultTbl SET Exam_ID=%d, Device_ID=%d, Measure_Time='%s', Eye_Side=%d, Flash_Level=%d, Shutter_Speed=%d, ISO=%d, Panorma=%d, Stereo=%d, File_Path='%s', Status=%d WHERE Serial_ID=%d",
//		result.getExamIdx(),
//		result.getDeviceIdx(),
//		DatabaseUtil::getDateTimeString(result.getMeasureTime()).c_str(),
//		result.getEyeSide(),
//		result.getFlashLevel(),
//		result.getShutterSpeed(),
//		result.getISO(),
//		result.getPanorma(),
//		result.getStereo(),
//		result.getFilePath().c_str(),
//		result.getStatus(),
//		result.getIndex());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//	disconnectDB();
//	return true;
//}
//
//bool PatientDatabase::updatePhysician(Physician &physician)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "UPDATE PhysicianTbl SET Name='%s', Description='%s', Status=%d WHERE Serial_ID=%d",
//		physician.getName().c_str(),
//		physician.getDescription().c_str(),
//		physician.getStatus(),
//		physician.getIndex());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//	disconnectDB();
//	return true;
//}
//
//bool PatientDatabase::updateOperator(Operator &oper)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "UPDATE OperatorTbl SET Name='%s', Description='%s', Status=%d WHERE Serial_ID=%d",
//		oper.getName().c_str(),
//		oper.getDescription().c_str(),
//		oper.getStatus(),
//		oper.getIndex());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//	disconnectDB();
//	return true;
//}
//
//bool PatientDatabase::updateDiagnosisGroup(DiagnosisGroup &group)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "UPDATE DiagnosisGroupTbl SET Name='%s', Display_Order=%d, Status=%d WHERE Serial_ID=%d",
//		group.getName().c_str(),
//		group.getDisplayOrder(),
//		group.getStatus(),
//		group.getIndex());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//	disconnectDB();
//	return true;
//}
//
//bool PatientDatabase::updateDiagnosisItem(DiagnosisItem &item)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "UPDATE DiagnosisItemTbl SET Name='%s', Group_ID=%d, Display_Order=%d, Status=%d WHERE Serial_ID=%d",
//		item.getName().c_str(),
//		item.getGroupIdx(),
//		item.getDisplayOrder(),
//		item.getStatus(),
//		item.getIndex());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//	disconnectDB();
//	return true;
//}
//
bool PatientDatabase::updateRace(Race &race)
{
	if (!connectDB())
	{
		return false;
	}

	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "UPDATE RaceTbl SET Name='%s', Display_Order=%d, Status=%d WHERE Serial_ID=%d",
		race.getName().c_str(),
		race.getDisplayOrder(),
		race.getStatus(),
		race.getIndex());

	if (!execQuery((const char*)szQuery))
	{
		disconnectDB();
		return false;
	}

	disconnectDB();
	return true;
}
//
//bool PatientDatabase::updateDevice(Device &device)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "UPDATE DeviceTbl SET Name='%s', Description='%s', IP_Addr='%s', Port=%d, Status=%d WHERE Serial_ID=%d",
//		device.getName().c_str(),
//		device.getDescription().c_str(),
//		device.getIpAddress().c_str(),
//		device.getPort(),
//		device.getStatus(),
//		device.getIndex());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//	disconnectDB();
//	return true;
//}
//
//bool PatientDatabase::updateUserAccount(UserAccount &user)
//{
//	if (!connectDB())
//	{
//		return false;
//	}
//
//	char szQuery[2048] = { 0 };
//	sprintf_s(szQuery, "UPDATE UserAccountTbl SET User_ID='%s', User_Password='%s', Type=%d, Create_Time='%s', Login_Time='%s', Logout_Time='%s', Email_Address='%s', Status=%d WHERE Serial_ID=%d",
//		user.getUserId().c_str(),
//		user.getPassword().c_str(),
//		user.getPermission(),
//		DatabaseUtil::getDateTimeString(user.getCreateTime()).c_str(),
//		DatabaseUtil::getDateTimeString(user.getLoginTime()).c_str(),
//		DatabaseUtil::getDateTimeString(user.getLogoutTime()).c_str(),
//		user.getEmailAddress().c_str(),
//		user.getStatus(),
//		user.getIndex());
//
//	if (!execQuery((const char*)szQuery))
//	{
//		disconnectDB();
//		return false;
//	}
//
//	disconnectDB();
//	return true;
//}

// Delete
bool PatientDatabase::deletePatient(long index)
{
	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "DELETE FROM PatientTbl WHERE Serial_ID = %d", index);

	bool ret = execQuery((const char*)szQuery);
	if (!ret)
	{
		showErrorMsg("Delete Patient Failed");
	}

	return ret;
}

bool PatientDatabase::deleteExam(long index)
{
	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "DELETE FROM ExamTbl WHERE Serial_ID = %d", index);

	bool ret = execQuery((const char*)szQuery);
	if (!ret)
	{
		showErrorMsg("Delete Exam Failed");
	}

	return ret;
}

bool PatientDatabase::deleteScanPattern(long index)
{
	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "DELETE FROM ScanPatternTbl WHERE Serial_ID = %d", index);

	bool ret = execQuery((const char*)szQuery);
	if (!ret)
	{
		showErrorMsg("Delete ScanPattern Failed");
	}

	return ret;
}

bool PatientDatabase::deleteScanResult(long index)
{
	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "DELETE FROM ScanResultTbl WHERE Serial_ID = %d", index);

	bool ret = execQuery((const char*)szQuery);
	if (!ret)
	{
		showErrorMsg("Delete ScanResult Failed");
	}

	return ret;
}

bool PatientDatabase::deleteFundusResult(long index)
{
	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "DELETE FROM FundusResultTbl WHERE Serial_ID = %d", index);

	bool ret = execQuery((const char*)szQuery);
	if (!ret)
	{
		showErrorMsg("Delete FundusResult Failed");
	}

	return ret;
}

bool PatientDatabase::deletePhysician(long index)
{
	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "DELETE FROM PhysicianTbl WHERE Serial_ID = %d", index);

	bool ret = execQuery((const char*)szQuery);
	if (!ret)
	{
		showErrorMsg("Delete Physician Failed");
	}

	sprintf_s(szQuery, "DELETE FROM sqlite_sequence WHERE name = '%s'", "PhysicianTbl");
	ret = execQuery((const char*)szQuery);
	if (!ret) {
		showErrorMsg("Delete Operator Failed");
	}

	return ret;
}

bool PatientDatabase::deleteOperator(long index)
{
	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "DELETE FROM OperatorTbl WHERE Serial_ID = %d", index);

	bool ret = execQuery((const char*)szQuery);
	if (!ret) {
		showErrorMsg("Delete Operator Failed");
	}

	sprintf_s(szQuery, "DELETE FROM sqlite_sequence WHERE name = '%s'", "OperatorTbl");
	ret = execQuery((const char*)szQuery);
	if (!ret) {
		showErrorMsg("Delete Operator Failed");
	}

	return ret;
}

bool PatientDatabase::deleteDiagnosisGroup(long index)
{
	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "DELETE FROM DiagnosisGroupTbl WHERE Serial_ID = %d", index);

	bool ret = execQuery((const char*)szQuery);
	if (!ret)
	{
		showErrorMsg("Delete DiagnosisGroup Failed");
	}

	return ret;
}

bool PatientDatabase::deleteDiagnosisItem(long index)
{
	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "DELETE FROM DiagnosisItemTbl WHERE Serial_ID = %d", index);

	bool ret = execQuery((const char*)szQuery);
	if (!ret)
	{
		showErrorMsg("Delete DiagnosisItem Failed");
	}

	return ret;
}

bool PatientDatabase::deleteRace(long index)
{
	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "DELETE FROM RaceTbl WHERE Serial_ID = %d", index);

	bool ret = execQuery((const char*)szQuery);
	if (!ret)
	{
		showErrorMsg("Delete Race Failed");
	}

	return ret;
}

bool PatientDatabase::deleteDevice(long index)
{
	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "DELETE FROM DeviceTbl WHERE Serial_ID = %d", index);

	bool ret = execQuery((const char*)szQuery);
	if (!ret)
	{
		showErrorMsg("Delete Device Failed");
	}

	return ret;
}

bool PatientDatabase::deleteUserAccount(long index)
{
	char szQuery[2048] = { 0 };
	sprintf_s(szQuery, "DELETE FROM UserAccountTbl WHERE Serial_ID = %d", index);

	bool ret = execQuery((const char*)szQuery);
	if (!ret)
	{
		showErrorMsg("Delete UserAccount Failed");
	}

	return ret;
}


// Get List
bool PatientDatabase::fetchPatientList(vector <Patient> *list, wstring query)
{
	sqlite3_stmt *stmt;

	if (sqlite3_prepare16_v2(m_db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			Patient temp;

			temp.setIndex(sqlite3_column_int(stmt, 0));
			temp.setPatientID((const wchar_t*)sqlite3_column_text16(stmt, 1));
			temp.setFirstName((const wchar_t*)sqlite3_column_text16(stmt, 2));
			temp.setMiddleName((const wchar_t*)sqlite3_column_text16(stmt, 3));
			temp.setLastName((const wchar_t*)sqlite3_column_text16(stmt, 4));
			temp.setGender((Gender)sqlite3_column_int(stmt, 5));
			temp.setBirthDate(DatabaseUtil::getDateTime(sqlite3_column_text(stmt, 6)));
			temp.setRefractOD(sqlite3_column_double(stmt, 7));
			temp.setRefractOS(sqlite3_column_double(stmt, 8));
			temp.setRaceID(sqlite3_column_int(stmt, 9));
			temp.setPhysicianIdx(sqlite3_column_int(stmt, 10));
			temp.setOperatorIdx(sqlite3_column_int(stmt, 11));
			temp.setDescription((const wchar_t*)sqlite3_column_text16(stmt, 12));
			temp.setCreateTime(DatabaseUtil::getDateTime(sqlite3_column_text(stmt, 13)));
			temp.setUpdateTime(DatabaseUtil::getDateTime(sqlite3_column_text(stmt, 14)));
			temp.setNeedTransfer(sqlite3_column_int(stmt, 15));
			temp.setStatus((Status)sqlite3_column_int(stmt, 16));
			if ((const wchar_t*)sqlite3_column_text16(stmt, 17) != NULL) {
				temp.setModality((const wchar_t*)sqlite3_column_text16(stmt, 17));
			}
			if ((const wchar_t*)sqlite3_column_text16(stmt, 18) != NULL) {
				temp.setAccessionNumber((const wchar_t*)sqlite3_column_text16(stmt, 18));
			}
			if ((const wchar_t*)sqlite3_column_text16(stmt, 19) != NULL) {
				temp.setStudyDescription((const wchar_t*)sqlite3_column_text16(stmt, 19));
			}
			if ((const wchar_t*)sqlite3_column_text16(stmt, 20) != NULL) {
				temp.setSeriesDescription((const wchar_t*)sqlite3_column_text16(stmt, 20));
			}

			list->push_back(temp);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}

bool PatientDatabase::fetchPatientList(vector <Patient> *list, FetchPropertyPatient &prop)
{
	wstring queryString = L"SELECT * FROM PatientTbl" + prop.getPatientCondition();

	return fetchPatientList(list, queryString);
}

bool PatientDatabase::fetchExamList(vector <Exam> *list, FetchPropertyExam &prop)
{
	sqlite3_stmt *stmt;
	wstring queryString = L"SELECT * FROM ExamTbl" + prop.getExamCondition();

	bool bRet = true;

	if (sqlite3_prepare16_v2(m_db, queryString.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
		bRet = false;
		goto FINISH;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		Exam temp;

		wchar_t* pStrTmp = nullptr;

		// index
		temp.setIndex(sqlite3_column_int(stmt, 0));

		// exam date
		temp.setExamDate(DatabaseUtil::getDateTime(sqlite3_column_text(stmt, 1)));

		// patient index
		temp.setPatientIdx(sqlite3_column_int(stmt, 2));

		// diagnosis
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 3);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setDiagnosis(pStrTmp);

		// comment
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 4);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setComment(pStrTmp);

		// file path
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 5);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setFilePath(pStrTmp);

		// status
		temp.setStatus((Status)sqlite3_column_int(stmt, 6));

		// modality
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 7);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setModality(pStrTmp);

		// scheduled date
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 8);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setScheduledDate(pStrTmp);

		// scheduled time
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 9);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setScheduledTime(pStrTmp);

		// uid
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 10);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setStudyInstanceUID(pStrTmp);

		list->push_back(temp);
	}

FINISH:
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return bRet;
}

bool PatientDatabase::fetchExamList(vector <Exam> *list, wstring query)
{
	sqlite3_stmt *stmt;

	bool bRet = true;

	if (sqlite3_prepare16_v2(m_db, query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
		bRet = false;
		goto FINISH;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		Exam temp;

		wchar_t* pStrTmp = nullptr;

		// index
		temp.setIndex(sqlite3_column_int(stmt, 0));

		// exam date
		temp.setExamDate(DatabaseUtil::getDateTime(sqlite3_column_text(stmt, 1)));

		// patient index
		temp.setPatientIdx(sqlite3_column_int(stmt, 2));

		// diagnosis
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 3);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setDiagnosis(pStrTmp);

		// comment
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 4);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setComment(pStrTmp);

		// file path
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 5);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setFilePath(pStrTmp);

		// status
		temp.setStatus((Status)sqlite3_column_int(stmt, 6));

		// modality
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 7);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setModality(pStrTmp);

		// scheduled date
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 8);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setScheduledDate(pStrTmp);

		// scheduled time
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 9);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setScheduledTime(pStrTmp);

		// uid
		pStrTmp = (wchar_t*)sqlite3_column_text16(stmt, 10);
		if (pStrTmp == nullptr) {
			bRet = false;
			goto FINISH;
		}
		temp.setStudyInstanceUID(pStrTmp);

		list->push_back(temp);
	}

FINISH:
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return bRet;
}

bool PatientDatabase::fetchExamIdList(vector <int> *list, FetchPropertyExam &prop)
{
	sqlite3_stmt *stmt;
	wstring queryString = L"SELECT * FROM ExamTbl" + prop.getExamCondition();

	bool bRet = true;

	if (sqlite3_prepare16_v2(m_db, queryString.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
		bRet = false;
		goto FINISH;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		// index
		list->push_back(sqlite3_column_int(stmt, 0));
	}

FINISH:
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return bRet;
}

bool PatientDatabase::fetchScanPatternList(vector <ScanPattern> *list)
{
	sqlite3_stmt *stmt;

	if (sqlite3_prepare16_v2(m_db, L"SELECT * FROM ScanPatternTbl", -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			ScanPattern temp;

			temp.setIndex(sqlite3_column_int(stmt, 0));
			temp.setCode(sqlite3_column_int(stmt, 1));
			temp.setName((const wchar_t*)sqlite3_column_text16(stmt, 2));
			temp.setVersion((const wchar_t*)sqlite3_column_text16(stmt, 3));
			temp.setScanType((ScanType)sqlite3_column_int(stmt, 4));
			temp.setStatus((Status)sqlite3_column_int(stmt, 5));

			list->push_back(temp);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}


bool PatientDatabase::fetchScanResultList(vector <ScanResult> *list, FetchPropertyScanResult &prop)
{
	sqlite3_stmt *stmt;
	wstring queryString = L"SELECT * FROM ScanResultTbl" + prop.getCondition();

	if (sqlite3_prepare16_v2(m_db, queryString.c_str(), -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			ScanResult temp;

			temp.setIndex(sqlite3_column_int(stmt, 0));
			temp.setExamIdx(sqlite3_column_int(stmt, 1));
			temp.setPatternIdx(sqlite3_column_int(stmt, 2));
			temp.setDeviceIdx(sqlite3_column_int(stmt, 3));
			temp.setMeasureTime(DatabaseUtil::getDateTime(sqlite3_column_text(stmt, 4)));
			temp.setEyeSide(sqlite3_column_int(stmt, 5));
			temp.setFixationTarget(sqlite3_column_int(stmt, 6));
			temp.setSSI(sqlite3_column_int(stmt, 7));
			temp.setSQI(sqlite3_column_int(stmt, 8));
			temp.setAScans(sqlite3_column_int(stmt, 9));
			temp.setBScans(sqlite3_column_int(stmt, 10));
			temp.setScanWidth(sqlite3_column_double(stmt, 11));
			temp.setScanHeight(sqlite3_column_double(stmt, 12));
			temp.setScanDirection((Direction)sqlite3_column_int(stmt, 13));
			temp.setXOffset(sqlite3_column_double(stmt, 14));
			temp.setYOffset(sqlite3_column_double(stmt, 15));
			temp.setBaseLine(sqlite3_column_int(stmt, 16));
			temp.setFilePath((const wchar_t*)sqlite3_column_text16(stmt, 17));
			temp.setStatus((Status)sqlite3_column_int(stmt, 18));

			list->push_back(temp);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}

bool PatientDatabase::fetchFundusResultList(vector <FundusResult> *list, FetchPropertyFundusResult &prop)
{
	sqlite3_stmt *stmt;
	wstring queryString = L"SELECT * FROM FundusResultTbl" + prop.getCondition();

	if (sqlite3_prepare16_v2(m_db, queryString.c_str(), -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			FundusResult temp;

			temp.setIndex(sqlite3_column_int(stmt, 0));
			temp.setExamIdx(sqlite3_column_int(stmt, 1));
			temp.setDeviceIdx(sqlite3_column_int(stmt, 2));
			temp.setMeasureTime(DatabaseUtil::getDateTime(sqlite3_column_text(stmt, 3)));
			temp.setEyeSide(sqlite3_column_int(stmt, 4));
			temp.setFixationTarget(sqlite3_column_int(stmt, 5));
			temp.setFlashLevel(sqlite3_column_int(stmt, 6));
			temp.setShutterSpeed(sqlite3_column_int(stmt, 7));
			temp.setISO(sqlite3_column_int(stmt, 8));
			temp.setPanorma(sqlite3_column_int(stmt, 9));
			temp.setStereo(sqlite3_column_int(stmt, 10));
			temp.setFilePath((const wchar_t*)sqlite3_column_text16(stmt, 11));
			temp.setStatus((Status)sqlite3_column_int(stmt, 12));

			list->push_back(temp);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}

bool PatientDatabase::fetchPhysicianList(vector <Physician> *list)
{
	sqlite3_stmt *stmt;
	if (sqlite3_prepare16_v2(m_db, L"SELECT * FROM PhysicianTbl", -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			Physician temp;

			temp.setIndex(sqlite3_column_int(stmt, 0));
			temp.setName((const wchar_t*)sqlite3_column_text16(stmt, 1));
			temp.setDescription((const wchar_t*)sqlite3_column_text16(stmt, 2));
			temp.setStatus((Status)sqlite3_column_int(stmt, 3));

			list->push_back(temp);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}

bool PatientDatabase::fetchOperatorList(vector <Operator> *list)
{
	sqlite3_stmt *stmt;
	if (sqlite3_prepare16_v2(m_db, L"SELECT * FROM OperatorTbl", -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			Operator temp;

			temp.setIndex(sqlite3_column_int(stmt, 0));
			temp.setName((const wchar_t*)sqlite3_column_text16(stmt, 1));
			temp.setDescription((const wchar_t*)sqlite3_column_text16(stmt, 2));
			temp.setStatus((Status)sqlite3_column_int(stmt, 3));

			list->push_back(temp);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}

bool PatientDatabase::fetchDiagnosisGroupList(vector <DiagnosisGroup> *list)
{
	sqlite3_stmt *stmt;
	if (sqlite3_prepare16_v2(m_db, L"SELECT * FROM DiagnosisGroupTbl", -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			DiagnosisGroup temp;

			temp.setIndex(sqlite3_column_int(stmt, 0));
			temp.setName((const wchar_t*)sqlite3_column_text16(stmt, 1));
			temp.setDisplayOrder(sqlite3_column_int(stmt, 2));
			temp.setStatus((Status)sqlite3_column_int(stmt, 3));

			list->push_back(temp);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}

bool PatientDatabase::fetchDiagnosisItemList(vector <DiagnosisItem> *list, long groupIndex)
{
	sqlite3_stmt *stmt;
	wstring queryString = L"SELECT * FROM DiagnosisItemTbl Where Group_ID = " + to_wstring(groupIndex);

	if (sqlite3_prepare16_v2(m_db, queryString.c_str(), -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			DiagnosisItem temp;

			temp.setIndex(sqlite3_column_int(stmt, 0));
			temp.setName((const wchar_t*)sqlite3_column_text16(stmt, 1));
			temp.setGroupIdx(sqlite3_column_int(stmt, 2));
			temp.setDisplayOrder(sqlite3_column_int(stmt, 3));
			temp.setStatus((Status)sqlite3_column_int(stmt, 4));

			list->push_back(temp);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}

bool PatientDatabase::fetchRaceList(vector <Race> *list)
{
	sqlite3_stmt *stmt;
	if (sqlite3_prepare16_v2(m_db, L"SELECT * FROM RaceTbl", -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			Race temp;

			temp.setIndex(sqlite3_column_int(stmt, 0));
			temp.setName((const wchar_t*)sqlite3_column_text16(stmt, 1));
			temp.setDisplayOrder(sqlite3_column_int(stmt, 2));
			temp.setStatus((Status)sqlite3_column_int(stmt, 3));

			list->push_back(temp);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}

bool PatientDatabase::fetchDeviceList(vector <Device> *list)
{
	sqlite3_stmt *stmt;
	if (sqlite3_prepare16_v2(m_db, L"SELECT * FROM DeviceTbl", -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			Device temp;

			temp.setIndex(sqlite3_column_int(stmt, 0));
			temp.setName((const wchar_t*)sqlite3_column_text16(stmt, 1));
			temp.setDescription((const wchar_t*)sqlite3_column_text16(stmt, 2));
			temp.setIpAddress((const wchar_t*)sqlite3_column_text16(stmt, 3));
			temp.setPort(sqlite3_column_int(stmt, 4));
			temp.setStatus((Status)sqlite3_column_int(stmt, 5));

			list->push_back(temp);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}

bool PatientDatabase::fetchUserAccountList(vector <UserAccount> *list)
{
	sqlite3_stmt *stmt;
	if (sqlite3_prepare16_v2(m_db, L"SELECT * FROM UserAccountTbl", -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			UserAccount temp;

			temp.setIndex(sqlite3_column_int(stmt, 0));
			temp.setUserId((const wchar_t*)sqlite3_column_text16(stmt, 1));
			temp.setPassword((const wchar_t*)sqlite3_column_text16(stmt, 2));
			temp.setPermission((Permission)sqlite3_column_int(stmt, 3));
			temp.setCreateTime(DatabaseUtil::getDateTime(sqlite3_column_text(stmt, 4)));
			temp.setLoginTime(DatabaseUtil::getDateTime(sqlite3_column_text(stmt, 5)));
			temp.setLogoutTime(DatabaseUtil::getDateTime(sqlite3_column_text(stmt, 6)));
			temp.setEmailAddress((const wchar_t*)sqlite3_column_text16(stmt, 7));
			temp.setStatus((Status)sqlite3_column_int(stmt, 8));

			list->push_back(temp);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}

int PatientDatabase::getPatientListSize()
{
	sqlite3_stmt *stmt;
	int temp = 0;
	bool result = false;

	const wchar_t *query = L"SELECT COUNT(*) FROM PatientTbl";

	if (sqlite3_prepare16_v2(m_db, query, -1, &stmt, NULL) == SQLITE_OK);
	{
		result = (sqlite3_step(stmt) == SQLITE_ROW);
		if (result) {
			temp = sqlite3_column_int(stmt, 0);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return temp;
}

bool PatientDatabase::getLastExam(Exam* exam, long patientIndex)
{
	sqlite3_stmt *stmt;
	bool result = false;
	
	wstring queryString = L"SELECT * FROM ExamTbl WHERE Patient_ID=" + to_wstring(patientIndex) + L" ORDER BY Exam_Date DESC LIMIT 1";

	if (sqlite3_prepare16_v2(m_db, queryString.c_str(), -1, &stmt, NULL) == SQLITE_OK)
	{
		result = (sqlite3_step(stmt) == SQLITE_ROW);

		if (result == true)
		{
			exam->setIndex(sqlite3_column_int(stmt, 0));
			exam->setExamDate(DatabaseUtil::getDateTime(sqlite3_column_text(stmt, 1)));
			exam->setPatientIdx(sqlite3_column_int(stmt, 2));
			exam->setDiagnosis((const wchar_t*)sqlite3_column_text16(stmt, 5));
			exam->setComment((const wchar_t*)sqlite3_column_text16(stmt, 6));
			exam->setFilePath((const wchar_t*)sqlite3_column_text16(stmt, 7));
			exam->setStatus((Status)sqlite3_column_int(stmt, 8));
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return result;
}

bool PatientDatabase::getLastSerialIndex(string tableName, long *ret)
{
	sqlite3_stmt *stmt;
	bool result = false;

	string queryString = "SELECT seq FROM sqlite_sequence WHERE name='" + tableName + "'";

	if (sqlite3_prepare_v2(m_db, queryString.c_str(), -1, &stmt, NULL) == SQLITE_OK)
	{
		result = (sqlite3_step(stmt) == SQLITE_ROW);

		if (result == true)
		{
			*ret = sqlite3_column_int(stmt, 0);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return result;
}

bool PatientDatabase::isScanResultExistInCondition(FetchPropertyScanResult &prop)
{
	sqlite3_stmt *stmt;
	wstring queryString = L"SELECT * FROM ScanResultTbl" + prop.getCondition();

	bool bResult = false;
	if (sqlite3_prepare16_v2(m_db, queryString.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			bResult = true;
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return bResult;
}

bool PatientDatabase::isFundusResultInCondition(FetchPropertyFundusResult &prop)
{
	sqlite3_stmt *stmt;
	wstring queryString = L"SELECT * FROM FundusResultTbl" + prop.getCondition();

	bool bResult = false;
	if (sqlite3_prepare16_v2(m_db, queryString.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			bResult = true;
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return bResult;
}

bool PatientDatabase::correctExamColumns()
{
	bool bRet;

	sqlite3_stmt *stmt;
	const wchar_t *query = L"UPDATE ExamTbl SET Exam_Date=?, Patient_ID=?, Diagnosis=?, Comment=?, File_Path=?, Status=?, Modality=?, Scheduled_Date=?, Scheduled_Time=?, Study_InstanceUID=? WHERE Serial_ID=?";

	if (sqlite3_prepare16_v2(m_db, query, -1, &stmt, NULL) == SQLITE_OK) {
		bRet = true;
		goto FINISH;
	}

	sqlite3_stmt *stmt_addMissedColumn;
	const wchar_t *query_addMissedColumn;

	// add
	query_addMissedColumn = L"ALTER TABLE ExamTbl ADD 'Modality' varchar(32)";
	if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
		bRet = false;
		goto FINISH;
	}
	if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
		bRet = false;
		goto FINISH;
	}

	query_addMissedColumn = L"ALTER TABLE ExamTbl ADD 'Scheduled_Date' varchar(32)";
	if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
		bRet = false;
		goto FINISH;
	}
	if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
		bRet = false;
		goto FINISH;
	}

	query_addMissedColumn = L"ALTER TABLE ExamTbl ADD 'Scheduled_Time' varchar(32)";
	if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
		bRet = false;
		goto FINISH;
	}
	if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
		bRet = false;
		goto FINISH;
	}

	query_addMissedColumn = L"ALTER TABLE ExamTbl ADD 'Study_InstanceUID' varchar(128)";
	if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
		bRet = false;
		goto FINISH;
	}
	if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
		bRet = false;
		goto FINISH;
	}

	// update
	query_addMissedColumn = L"UPDATE ExamTbl SET 'Modality' = ''";
	if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
		bRet = false;
		goto FINISH;
	}
	if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
		bRet = false;
		goto FINISH;
	}

	query_addMissedColumn = L"UPDATE ExamTbl SET 'Scheduled_Date' = ''";
	if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
		bRet = false;
		goto FINISH;
	}
	if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
		bRet = false;
		goto FINISH;
	}

	query_addMissedColumn = L"UPDATE ExamTbl SET 'Scheduled_Time' = ''";
	if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
		bRet = false;
		goto FINISH;
	}
	if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
		bRet = false;
		goto FINISH;
	}

	query_addMissedColumn = L"UPDATE ExamTbl SET 'Study_InstanceUID' =''";
	if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
		bRet = false;
		goto FINISH;
	}
	if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
		bRet = false;
		goto FINISH;
	}

	bRet = sqlite3_prepare16_v2(m_db, query, -1, &stmt, NULL) == SQLITE_OK;

FINISH:
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return bRet;
}

bool PatientDatabase::correctPatientColumns()
{
	bool bRet;

	sqlite3_stmt *stmt;
	const wchar_t *query = L"UPDATE PatientTbl SET Patient_IDS=?, First_Name=?, Middle_Name=?, Last_Name=?, Gender=?, Birth_Date=?, Refract_OD=?, Refract_OS=?, Race_ID=?, Physician_ID=?, Operator_ID=?, Description=?, Create_Time=?, Update_Time=?, Need_Transfer=?, Status=?, Modality=?, Accession_Number=?, Study_Description=?, Series_Description=? WHERE Serial_ID=?";

	if (sqlite3_prepare16_v2(m_db, query, -1, &stmt, NULL) == SQLITE_OK) {
		bRet = true;
		goto FINISH;
	}

	sqlite3_stmt *stmt_addMissedColumn;
	const wchar_t *query_addMissedColumn;

	// modality
	do {
		// add
		query_addMissedColumn = L"ALTER TABLE PatientTbl ADD 'Modality' varchar(32)";
		if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
			break;
		}
		if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
			break;
		}

		// update
		query_addMissedColumn = L"UPDATE PatientTbl SET 'Modality' = ''";
		if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
			break;
		}
		if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
			break;
		}
	} while (false);

	// accession number
	do {
		// add
		query_addMissedColumn = L"ALTER TABLE PatientTbl ADD 'Accession_Number' varchar(32)";
		if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
			break;
		}
		if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
			break;
		}

		// update
		query_addMissedColumn = L"UPDATE PatientTbl SET 'Accession_Number' = ''";
		if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
			break;
		}
		if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
			break;
		}
	} while (false);

	// study description
	do {
		// add
		query_addMissedColumn = L"ALTER TABLE PatientTbl ADD 'Study_Description' varchar(32)";
		if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
			break;
		}
		if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
			break;
		}

		// update
		query_addMissedColumn = L"UPDATE PatientTbl SET 'Study_Description' = ''";
		if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
			break;
		}
		if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
			break;
		}
	} while (false);

	// series description
	do {
		// add
		query_addMissedColumn = L"ALTER TABLE PatientTbl ADD 'Series_Description' varchar(32)";
		if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
			break;
		}
		if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
			break;
		}

		// update
		query_addMissedColumn = L"UPDATE PatientTbl SET 'Series_Description' = ''";
		if (sqlite3_prepare16_v2(m_db, query_addMissedColumn, -1, &stmt_addMissedColumn, NULL) != SQLITE_OK) {
			break;
		}
		if (sqlite3_step(stmt_addMissedColumn) != SQLITE_DONE) {
			break;
		}
	} while (false);

	bRet = sqlite3_prepare16_v2(m_db, query, -1, &stmt, NULL) == SQLITE_OK;

FINISH:
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return bRet;
}

bool PatientDatabase::fetchStitchableFundusResultList(wstring patient_ids, wstring measureDate, int eyeSide, vector<FundusStitchResult> *list)
{
	sqlite3_stmt *stmt;

	wstring queryString = L"select Serial_ID, Patient_ID, Exam_ID, Eye_Side, Measure_Time, Fixation_Target, File_Path, Exam_Path from FundusResultTbl as FRT, (select ET.Serial_ID as SerialId, ET.Patient_ID as Patient_ID, ET.File_Path as Exam_Path from ExamTbl as ET, (select * from PatientTbl where patient_ids = '" + patient_ids + L"') as PatientResult where PatientResult.Serial_ID = ET.Patient_ID) as ExamResult where FRT.Exam_ID = ExamResult.SerialId and NOT FRT.Fixation_Target = 10 and NOT FRT.Fixation_Target = 20 and NOT FRT.Fixation_Target = 30 and NOT FRT.Fixation_Target = 99 AND DATE(Measure_Time) = '" + measureDate + L"' AND Eye_Side = " + to_wstring(eyeSide);

	if (sqlite3_prepare16_v2(m_db, queryString.c_str(), -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			FundusStitchResult temp;

			temp.setSerialID(sqlite3_column_int(stmt, 0));
			temp.setPatientID(sqlite3_column_int(stmt, 1));
			temp.setExamID(sqlite3_column_int(stmt, 2));
			temp.setDeviceID(0);
			temp.setMeasureTime(DatabaseUtil::getDateTime(sqlite3_column_text(stmt, 4)));
			temp.setEyeSide(sqlite3_column_int(stmt, 3));
			temp.setExamPath((const wchar_t*)sqlite3_column_text16(stmt, 7));
			temp.setResultPath((const wchar_t*)sqlite3_column_text16(stmt, 6));
			temp.setFixationTarget(sqlite3_column_int(stmt, 5));
			list->push_back(temp);
		}
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return true;
}