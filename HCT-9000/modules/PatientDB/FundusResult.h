#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API FundusResult
	{
	public:
		FundusResult();
		virtual ~FundusResult();
		void clear(void);

		bool operator==(FundusResult& f) const;

	private:
		long			m_index;
		long			m_examIdx;
		long			m_deviceIdx;
		COleDateTime	m_measureTime;
		int				m_eyeSide;
		int				m_fixaionTarget;
		int				m_flashLevel;
		int				m_shutterSpeed;
		int				m_ISO;
		int				m_panorma;
		int				m_stereo;
		wstring			m_filePath;
		Status			m_status;

	public:
		long			getIndex(void) { return m_index; }
		long			getExamIdx(void) { return m_examIdx; }
		long			getDeviceIdx(void) { return m_deviceIdx; }
		COleDateTime	getMeasureTime(void) { return m_measureTime; }
		int				getEyeSide(void) { return m_eyeSide; }
		int				getFixationTarget(void) { return m_fixaionTarget; }
		int				getFlashLevel(void) { return m_flashLevel; }
		int				getShutterSpeed(void) { return m_shutterSpeed; }
		int				getISO(void) { return m_ISO; }
		int				getPanorma(void) { return m_panorma; }
		int				getStereo(void) { return m_stereo; }
		wstring			getFilePath(void) { return m_filePath; }
		Status			getStatus(void) { return m_status; }

		void			setIndex(long idx);
		void			setExamIdx(long idx);
		void			setDeviceIdx(long idx);
		void			setMeasureTime(COleDateTime time);
		void			setEyeSide(int side);
		void			setFixationTarget(int idx);
		void			setFlashLevel(int level);
		void			setShutterSpeed(int speed);
		void			setISO(int ISO);
		void			setPanorma(int panorma);
		void			setStereo(int stereo);
		void			setFilePath(wstring filePath);
		void			setStatus(Status status);
	};

	class PATIENTDB_DLL_API FundusStitchResult
	{
	public:
		FundusStitchResult();
		virtual ~FundusStitchResult();
		void clear(void);

	private:
		long			m_serialID;
		long			m_patientID;
		long			m_examID;
		long			m_deviceID;
		COleDateTime	m_measureTime;
		int				m_eyeSide;
		wstring         m_examPath;
		wstring         m_resultPath;
		int				m_fixaionTarget;

	public:
		long			getSerialID(void) { return m_serialID; }
		long			getPatientID(void) { return m_patientID; }
		long			getExamID(void) { return m_examID; }
		long			getDeviceID(void) { return m_deviceID; }
		COleDateTime	getMeasureTime(void) { return m_measureTime; }
		int				getEyeSide(void) { return m_eyeSide; }
		wstring			getExamPath(void) { return m_examPath; }
		wstring			getResultPath(void) { return m_resultPath; }
		int				getFixationTarget(void) { return m_fixaionTarget; }

		void			setSerialID(long id) { m_serialID = id;};
		void			setPatientID(long id) { m_patientID = id; };
		void			setExamID(long id) { m_examID = id; };
		void			setDeviceID(long id) { m_deviceID = id; };
		void			setMeasureTime(COleDateTime time) { m_measureTime = time; };
		void			setEyeSide(int side) { m_eyeSide = side; };
		void			setExamPath(wstring examPath) { m_examPath = examPath; };
		void			setResultPath(wstring resultPath) { m_resultPath = resultPath; };
		void			setFixationTarget(int idx) { m_fixaionTarget = idx; };
	};
}
