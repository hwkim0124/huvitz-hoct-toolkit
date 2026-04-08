#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API ScanResult
	{
	public:
		ScanResult();
		virtual ~ScanResult();
		void clear(void);

		bool operator==(ScanResult& s) const;

	private:
		long			m_index;
		long			m_examIdx;
		long			m_patternIdx;
		long			m_deviceIdx;
		COleDateTime	m_measureTime;
		int				m_eyeSide;
		int				m_fixationTarget;
		int				m_SSI;
		int				m_SQI;
		int				m_AScans;
		int				m_BScans;
		double			m_scanWidth;
		double			m_scanHeight;
		Direction		m_scanDirection;
		double			m_xOffset;
		double			m_yOffset;
		int				m_baseLine;
		wstring			m_filePath;
		Status			m_status;

	public:
		long			getIndex(void) { return m_index; }
		long			getExamIdx(void) { return m_examIdx; }
		long			getPatternIdx(void) { return m_patternIdx; }
		long			getDeviceIdx(void) { return m_deviceIdx; }
		COleDateTime	getMeasureTime(void) { return m_measureTime; }
		int				getEyeSide(void) { return m_eyeSide; }
		int				getFixationTarget(void) { return m_fixationTarget; }
		int				getSSI(void) { return m_SSI; }
		int				getSQI(void) { return m_SQI; }
		int				getAScans(void) { return m_AScans; }
		int				getBScans(void) { return m_BScans; }
		double			getScanWidth(void) { return m_scanWidth; }
		double			getScanHeight(void) { return m_scanHeight; }
		Direction		getScanDirection(void) { return m_scanDirection; }
		double			getXOffset(void) { return m_xOffset; }
		double			getYOffset(void) { return m_yOffset; }
		int				getBaseLine(void) { return m_baseLine; }
		wstring			getFilePath(void) { return m_filePath; }
		Status			getStatus(void) { return m_status; }

		void			setIndex(long idx);
		void			setExamIdx(long idx);
		void			setPatternIdx(long idx);
		void			setDeviceIdx(long idx);
		void			setMeasureTime(COleDateTime time);
		void			setEyeSide(int side);
		void			setFixationTarget(int idx);
		void			setSSI(int value);
		void			setSQI(int value);
		void			setAScans(int scans);
		void			setBScans(int scans);
		void			setScanWidth(double width);
		void			setScanHeight(double height);
		void			setScanDirection(Direction direction);
		void			setXOffset(double value);
		void			setYOffset(double value);
		void			setBaseLine(int baseline);
		void			setFilePath(wstring filePath);
		void			setStatus(Status status);
	};
}