// OctImport.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "OctImport.h"
#include "OctImportData.h"
#include "odbcinst.h"
#include "OctHttpSetup.h"
#include "OctHttpFile.h"
#include "OctHttpFundus.h"
#include "OctHttpPatient.h"
#include "EyeScapeVd.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace octimport;
using namespace octhttp;

COctImport::COctImport()
{
}

COctImport::~COctImport()
{
}

COctImport::COctImport(std::wstring path) :isDelete(0), isUpdate(0)
{
	COctImport();
	mImportPath = path.c_str();
}

void COctImport::SetDelete(int del)
{
	isDelete = del;
}

void COctImport::SetUpdate(int upd)
{
	isUpdate = upd;
}

BOOL IsExist(CString path)
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

void ConnectToHost() {
	// TODO: 설정 읽어서 하기
	wstring host = L"127.0.0.1";
	OctHttpSetup::setIpAddress(host);
	int port = 8080;
	OctHttpSetup::setPort(port);
}

bool COctImport::Import() {

#if 1
	CString mdbPath = mImportPath + _T("\\esc.mdb");

	wchar_t attribute[1024];
	wchar_t *pAttributes = attribute;

	memset(attribute, 0, sizeof(attribute));
	lstrcpy(pAttributes, L"DSN=Test");
	pAttributes += 9;

	lstrcpy(pAttributes, L"DBQ=");
	pAttributes += 4;

	lstrcpy(pAttributes, mdbPath);
	pAttributes += mdbPath.GetLength() + 1;


	if (IsExist(mdbPath)) {

		BOOL ok = SQLConfigDataSource(
			NULL,
			ODBC_ADD_DSN,
			_T("Microsoft Access Driver (*.mdb)"),
			attribute);
		// 널문자를 아래와 같이 추가해서 넘겨주면 인식을 못한다.
		// wchar_t 로 선언해서 하니까 됨...
		//_T("DSN=Test")_T("\0")_T("DBQ=%s\0") + new_mdbPath + _T("\0\0"));
		//_T("DSN=Test\0DBQ=E:\\huvitz\\nidek_export\\backup2\\esc.mdb\0\0"));

		
		CEyeScapeVd esc;
		//esc.SetConnectInfo(connInfo(dirPath));
		esc.SetConnectInfo(_T("DSN=Test;DBQ=") + mdbPath + _T(";"));
		if (esc.Open()) {
			while (!esc.IsEOF()) {
				std::wcout << "MDB Test ID: " << esc.m_id.GetString() << std::endl;
				esc.MoveNext();
			}
			esc.Close();
		}

		return TRUE;
	}

#endif
	CString dirPath = mImportPath;
	CFileFind finder;
	
	CString strWildcard(dirPath);
	strWildcard += _T("\\*.*");

	BOOL bWorking = finder.FindFile(strWildcard);

	if (!bWorking) {
		return FALSE;
	}

	ConnectToHost();
	RecurseDirectory(dirPath);

	return 0;


}

void COctImport::RecurseDirectory(CString root) {
	CFileFind finder;

	CString strWildcard(root);
	strWildcard += _T("\\*.*");

	BOOL bWorking = finder.FindFile(strWildcard);

	while (bWorking) {
		bWorking = finder.FindNextFileW();

		if (finder.IsDots()) {
			continue;
		}

		CString str = finder.GetFilePath();

		if (finder.IsDirectory()) {
			RecurseDirectory(str);
		}
		else {
			CString fileName = finder.GetFileName();
			CString configFileName = _T("PatientData.CSV");
			//TODO: 환자 파일 설정으로 넣기
			//mHoctImportConfig->mPatientFileName.GetWindowTextW(configFileName);

			if ((fileName.MakeLower()).Compare(configFileName.MakeLower()) == 0) {
				// 디렉토리 이름
				//CString dir = str.Left(str.GetLength() - fileName.GetLength());

				CString patientID = ImportPatient(str);
				if (patientID.GetLength() > 0) {
					CString dir = str.Left(str.GetLength() - fileName.GetLength());
					ImportScan(patientID, dir);
				}
			}
		}
	}

	finder.Close();
}


time_t convertTime(COleDateTime time)
{
	//	auto span = time - COleDateTime(1970, 1, 1, 0, 0, 0);
	//	auto ret = span.GetTotalSeconds();
	//	return ret;

	int dMinuteZone;
	TIME_ZONE_INFORMATION tzi;
	GetTimeZoneInformation(&tzi);
	dMinuteZone = tzi.Bias;
	return size_t((double)time * 86400) - 2209161600 + 60 * dMinuteZone;
}


void RecursiveDelete(CString szPath)
{
	CFileFind ff;
	CString path = szPath;

	if (path.Right(1) != "\\")
		path += _T("\\");

	path += _T("*.*");

	BOOL res = ff.FindFile(path);

	while (res)
	{
		res = ff.FindNextFileW();
		if (ff.IsDots()) {
			continue;
		}

		if (ff.IsDirectory()) {
			path = ff.GetFilePath();
			RecursiveDelete(path);
			RemoveDirectory(path);
		}
		else
		{
			BOOL ok = DeleteFile(ff.GetFilePath());
		}
	}
}


BOOL COctImport::ImportScan(CString patientID, CString dirPath) {
	CFileFind finder;

	CString msg;
	CString strWildcard(dirPath);
	strWildcard += _T("\\*.*");

	BOOL bWorking = finder.FindFile(strWildcard);

	while (bWorking) {
		bWorking = finder.FindNextFileW();

		if (finder.IsDots()) {
			continue;
		}

		CString str = finder.GetFilePath();

		if (!finder.IsDirectory()) {
			CString fileName = finder.GetFileName();
			CString configFileName = _T("DataFileList.CSV");
			//TODO: 데이터 csv 파일 입력 설정 필요

			if ((fileName.MakeLower()).Compare(configFileName.MakeLower()) == 0) {
				OctImportData csv(str);

				for (int i = 1; i < csv.GetRowCount(); i++) {
					EyeSide es = EyeSide::OD;
					COleDateTime measureDate;
					CString measureTime;
					CString deviceID;
					CString remarks;
					CString filePath = _T("");
					CString fullPath(dirPath);
					CString extension = _T("jpg");

					int flashLevel = 60;
					int fixation = 99;
					int iso = 0;

					for (int j = 0; j < csv.GetColumnCount(i); j++) {
						/*CString log;
						log.Format(_T("%d,%d) %s:%s\n"), j, i, csv.GetValue(j, i), csv.GetValue(j, i + 1));
						OutputDebugString(log);*/
						auto label = csv.GetValue(j, 0).MakeLower();

						if (label.Compare(_T("dfdate")) == 0) {
							measureDate = _wtof(csv.GetValue(j, i));
						}
						else if (label.Compare(_T("dftime")) == 0) {
							measureTime = csv.GetValue(j, i);
						}
						else if (label.Compare(_T("eye")) == 0) {
							es = (EyeSide)(_wtoi(csv.GetValue(j, i)) == 1 ? 2 : 1);
						}
						else if (label.Compare(_T("id_object")) == 0) {
							deviceID = csv.GetValue(j, i);
						}
						else if (label.Compare(_T("remarks")) == 0) {
							remarks = csv.GetValue(j, i);

							auto s = remarks.Find(_T("ISO="));
							auto s_comma = remarks.Find(_T(","), s);
							auto s2 = remarks.Find(_T("FL="));
							auto s2_quote = remarks.Find(_T("\""), s2);

							iso = _wtoi(remarks.Mid(s + 4, s_comma - s - 4));
							flashLevel = _wtoi(remarks.Mid(s2 + 3, s2_quote - s2 - 3));
						}
						else if (label.Compare(_T("filepath")) == 0) {
							filePath = csv.GetValue(j, i);
						}
						else if (label.Compare(_T("filesuf")) == 0) {
							extension = csv.GetValue(j, i);
						}
					}

					if (filePath.GetLength() < 1) {
						continue;
					}

					if (fullPath.Right(1) != _T("\\")) {
						fullPath += _T("\\");
					}
					fullPath += filePath;

					if (!IsExist(fullPath)) {
						continue;
					}

					Fundus postResult(MeasurementType::Color_Fundus, es);

					COleDateTime examTime;

					int hour = _wtoi(measureTime.Left(2));
					int minute = _wtoi(measureTime.Mid(3, 2));
					int second = _wtoi(measureTime.Right(2));
					examTime.SetDateTime(measureDate.GetYear(), measureDate.GetMonth(), measureDate.GetDay(), hour, minute, second);

					auto measTime = convertTime(examTime);

					postResult.setMeasureTime(measTime);
					postResult.setPatientId(patientID.GetString());
					postResult.setDeviceId(deviceID.GetString());
					postResult.setFixation(fixation);
					postResult.setFlashLevel(flashLevel);
					postResult.setShutterSpeed(0);
					postResult.setIso(iso);
					postResult.setPanorama(0);
					postResult.setStereo(0);

					try
					{
						int retryCount = 0;
						int id;
						while (true) {
							id = postResult.post();
							if (id < 0) {
								if (retryCount > 4) {
									break;
								}
								retryCount++;
								Sleep(1000);
							}
							else {
								break;
							}
						}
						if (retryCount > 4) {
							continue;
						}

						octhttp::File postFile(id);
						auto thumbnailPath = fullPath.Left(fullPath.GetLength() - 4) + _T("p") + fullPath.Right(4);
						postFile.post2(fullPath.GetString(), _T("fundus.jpg"));
						//TODO: 썸네일 만들어줘야 하나?
						postFile.post2(fullPath.GetString(), _T("thumbnail.jpg"));

						//auto imagePaths = getFileNamesInDirectory(dirPath + "\\" + directory, _T("*.") + extension);
						//for (auto path : imagePaths) {
						//	postFile.post(path.GetString());
						//}

						postResult.process(id);
					}
					catch (const std::exception& e)
					{
						int len = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, e.what(), -1, NULL, NULL);
						msg = new TCHAR[len];
						MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, (LPWSTR)(LPCWSTR)msg, len);
						continue;
					}

					if (isDelete) {
						CString d = fullPath.Left(fullPath.ReverseFind(_T('\\')));
						RecursiveDelete(d);

						// 고유아이디 폴더
						CString d2 = d;
						d2.Left(d2.ReverseFind(_T('\\')));

						RemoveDirectory(d2);
					}
				}
			}
		}
	}

	finder.Close();
	return TRUE;
}




CString COctImport::ImportPatient(CString csvFilePath) {
	OctImportData csv(csvFilePath);

	CString buffer;
	CString patientID = _T("");
	CString phnFirstName = _T("");
	CString phnMiddleName = _T("");
	CString phnLastName = _T("");
	CString ideFirstName = _T("");
	CString ideMiddleName = _T("");
	CString ideLastName = _T("");
	Patient patient(patientID.GetString());

	for (int i = 1; i < csv.GetRowCount(); i++) {
		for (int j = 0; j < csv.GetColumnCount(i); j++) {

			auto label = csv.GetValue(j, 0).MakeLower();

			if (label.Compare(_T("patientid")) == 0) {
				patientID = csv.GetValue(j, i);
				patient.setPatientId(patientID.GetString());
				patient.setRefract(0.0f, 0.0f);
			}
			else if (label.Compare(_T("birthday")) == 0) {
				COleDateTime birth = _wtof(csv.GetValue(j, i));
				patient.setBirthDate(birth.GetYear(), birth.GetMonth(), birth.GetDay());
			}
			else if (label.Compare(_T("middlename")) == 0) {
				patient.setMiddleName(csv.GetValue(j, i).GetString());
			}
			else if (label.Compare(_T("familyname")) == 0) {
				patient.setLastName(csv.GetValue(j, i).GetString());
			}
			else if (label.Compare(_T("givenname")) == 0) {
				patient.setFirstName(csv.GetValue(j, i).GetString());
			}
			else if (label.Compare(_T("givenname_phn")) == 0) {
				phnFirstName = csv.GetValue(j, i);
			}
			else if (label.Compare(_T("middlename_phn")) == 0) {
				phnMiddleName = csv.GetValue(j, i);
			}
			else if (label.Compare(_T("familyname_phn")) == 0) {
				phnLastName = csv.GetValue(j, i);
			}
			else if (label.Compare(_T("givenname_ide")) == 0) {
				ideFirstName = csv.GetValue(j, i);
			}
			else if (label.Compare(_T("middlename_ide")) == 0) {
				ideMiddleName = csv.GetValue(j, i);
			}
			else if (label.Compare(_T("familyname_ide")) == 0) {
				ideLastName = csv.GetValue(j, i);
			}
			else if (label.Compare(_T("sex")) == 0) {
				// TODO: 확인 필요
				auto gender = _wtoi(csv.GetValue(j, i));
				if (gender == 1) {
					patient.setGender(Gender::Male);
				}
				else {
					patient.setGender(Gender::Female);
				}
			}
			else if (label.Compare(_T("race")) == 0) {
				// TODO: 확인 필요
				auto race = _wtoi(csv.GetValue(j, i));
				if (race == 1) {
					// Other
					patient.setRace(Race::Asian);
				}
				else if (race == 2) {
					// Ameridndian
					patient.setRace(Race::Caucasian);
				}
				else if (race == 3) {
					//Asian
					patient.setRace(Race::Asian);
				}
				else if (race == 4) {
					//African origin
					patient.setRace(Race::Blacks);
				}
				else if (race == 5) {
					//Pac. Isalander
					patient.setRace(Race::Caucasian);
				}
				else if (race == 6) {
					patient.setRace(Race::Caucasian);
				}
				else if (race == 7) {
					patient.setRace(Race::Latino);
				}
				else {
					patient.setRace(Race::Asian);
				}
			}
			else if (label.Compare(_T("description")) == 0) {
				patient.setDescription(csv.GetValue(j, i).GetString());
			}
		}
	}

	if (patientID.GetLength() > 0) {

		CString phnName = phnFirstName + _T(" ") + phnMiddleName + _T(" ") + phnLastName;
		patient.setPhysicianName(phnName.GetString());

		CString ideName = ideFirstName + _T(" ") + ideMiddleName + _T(" ") + ideLastName;
		patient.setOperatorName(ideName.GetString());

		CString msg;
		try
		{
			auto ret = patient.postJson();
			if (ret < 0) {
				// TODO: 설정 필요
				if (isUpdate) {
					ret = patient.update();
				}
				if (ret < 0) {
					return _T("");
				}
			}
		}
		catch (const std::exception& e)
		{
			int len = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, e.what(), -1, NULL, NULL);
			msg = new TCHAR[len];
			MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, (LPWSTR)(LPCWSTR)msg, len);
			if (msg.Compare(_T("Json value is null")) != 0) {
				return _T("");
			}
		}

		return patientID;
	}

	return _T("");
}