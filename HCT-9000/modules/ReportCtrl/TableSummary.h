#pragma once
#include "DrawRnflChart.h"


// TableSummary

using namespace std;

class REPORTCTRL_DLL_API TableSummary : public CWnd
{
	DECLARE_DYNAMIC(TableSummary)

	const int kMaxCols = 6;

public:
	TableSummary();
	virtual ~TableSummary();
	void initialize(void);
	void clearData(void);
	vector <pair<CString, CString>> getData(void);

	void setEyeSide(OctGlobal::EyeSide side);
	void insertData(CString name, CString data);

	void drawTable(CDC *pDC, CRect rect);
	void drawTableForReport(CDC *pDC, CRect rect, OctGlobal::EyeSide eyeSide = OctGlobal::EyeSide::Unknown);

	void addNormRange(int index, pair<float, float> data);
	void clearNormRange(void);
	auto getNormRangeList()->const vector<tuple<int, float, float>>;

private:
	void _drawTable(CDC *pDC, CRect rect);
	void _drawTableForReport(CDC *pDC, CRect rect, OctGlobal::EyeSide eyeSide = OctGlobal::EyeSide::Unknown);
	void _drawTableWithNormRange(CDC *pDC, CRect rect);
	void _drawTableWithNormRangeForReport(CDC *pDC, CRect rect, OctGlobal::EyeSide eyeSide = OctGlobal::EyeSide::Unknown);

private:
	OctGlobal::EyeSide m_eyeSide;
	vector <pair<CString, CString>> m_data;
	vector <tuple<int, float, float>> m_normRange;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
};


