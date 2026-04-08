#pragma once
#include <string>
#include "stdafx.h"
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class Patient;
	class Exam;
	class ScanPattern;
	class ScanResult;
	class FundusResult;
	class FundusStitchResult;
	class Physician;
	class Operator;
	class DiagnosisGroup;
	class DiagnosisItem;
	class Race;
	class Device;
	class UserAccount;
	class FetchPropertyPatient;
	class FetchPropertyExam;
	class FetchPropertyScanResult;
	class FetchPropertyFundusResult;


	class PATIENTDB_DLL_API PatientDatabase
	{
	public:
		PatientDatabase();
		PatientDatabase(string dbName);
		virtual ~PatientDatabase();

	private:
		string		m_dbName;
		sqlite3		*m_db;

	public:
		bool		connectDB();
		void		disconnectDB();
		bool		execQuery(const char* szQuery);
		void		showErrorMsg(const char* szError);
		void		updateRaceList();

		// Insert
		bool		insertPatient(Patient &patient);
		bool		insertExam(Exam &exam);
		bool		insertScanPattern(ScanPattern &pattern);
		bool		insertScanResult(ScanResult &result);
		bool		insertFundusResult(FundusResult &result);
		bool		insertPhysician(Physician &physician);
		bool		insertOperator(Operator &oper);
//		bool		insertDiagnosisGroup(DiagnosisGroup &group);
//		bool		insertDiagnosisItem(DiagnosisItem &item);
		bool		insertRace(Race &race);
//		bool		insertDevice(Device &device);
//		bool		insertUserAccount(UserAccount &user);

		// Update
		bool		updatePatient(Patient &patient);
		bool		updateExam(Exam &exam);
//		bool		updateScanPattern(ScanPattern &pattern);
//		bool		updateScanResult(ScanResult &result);
//		bool		updateFundusResult(FundusResult &result);
//		bool		updatePhysician(Physician &physician);
//		bool		updateOperator(Operator &oper);
//		bool		updateDiagnosisGroup(DiagnosisGroup &group);
//		bool		updateDiagnosisItem(DiagnosisItem &item);
		bool		updateRace(Race &race);
//		bool		updateDevice(Device &device);
//		bool		updateUserAccount(UserAccount &user);

		// Delete
		bool		deletePatient(long index);
		bool		deleteExam(long index);
		bool		deleteScanPattern(long index);
		bool		deleteScanResult(long index);
		bool		deleteFundusResult(long index);
		bool		deletePhysician(long index);
		bool		deleteOperator(long index);
		bool		deleteDiagnosisGroup(long index);
		bool		deleteDiagnosisItem(long index);
		bool		deleteRace(long index);
		bool		deleteDevice(long index);
		bool		deleteUserAccount(long index);

		//bool		deletePatient(Patient &pat);
		//bool		deleteExam(Exam &exam);
		//bool		deleteScanPattern(ScanPattern &pattern);
		//bool		deleteScanResult(ScanResult &result);
		//bool		deleteFundusResult(FundusResult &result);
		//bool		deletePhysician(Physician &physician);
		//bool		deleteOperator(Operator &oper);
		//bool		deleteDiagnosisGroup(DiagnosisGroup &group);
		//bool		deleteDiagnosisItem(DiagnosisItem &item);
		//bool		deleteRace(Race &race);
		//bool		deleteDevice(Device &device);
		//bool		deleteUserAccount(UserAccount &user);

		// Get List
		bool		fetchPatientList(vector <Patient> *list, wstring query = L"SELECT * FROM PatientTbl");
		bool		fetchPatientList(vector <Patient> *list, FetchPropertyPatient &prop);
		bool		fetchExamList(vector <Exam> *list, FetchPropertyExam &prop);
		bool		fetchExamList(vector <Exam> *list, wstring query = L"SELECT * FROM ExamTbl");
		bool		fetchExamIdList(vector <int> *list, FetchPropertyExam &prop);
		bool		fetchScanPatternList(vector <ScanPattern> *list);
		bool		fetchScanResultList(vector <ScanResult> *list, FetchPropertyScanResult &prop);
		bool		fetchFundusResultList(vector <FundusResult> *list, FetchPropertyFundusResult &prop);
		bool		fetchPhysicianList(vector <Physician> *list);
		bool		fetchOperatorList(vector <Operator> *list);
		bool		fetchDiagnosisGroupList(vector <DiagnosisGroup> *list);
		bool		fetchDiagnosisItemList(vector <DiagnosisItem> *list, long groupIndex);
		bool		fetchRaceList(vector <Race> *list);
		bool		fetchDeviceList(vector <Device> *list);
		bool		fetchUserAccountList(vector <UserAccount> *list);

		bool		fetchStitchableFundusResultList(wstring patient_ids, wstring measureDate, int eyeSide, vector<FundusStitchResult> *list);

		int			getPatientListSize();
		// Get One Item
		bool		getLastExam(Exam *exam, long patientIndex);
	
		// Get Current Index
		bool		getLastSerialIndex(string tableName, long *ret);
//		bool		getCurrentPatientIndex(long *index);
//		bool		getCurrentExamIndex(long *index);

		bool		isScanResultExistInCondition(FetchPropertyScanResult &prop);
		bool		isFundusResultInCondition(FetchPropertyFundusResult &prop);

		bool		correctExamColumns();
		bool		correctPatientColumns();
	};
}