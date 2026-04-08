// TableSummary.cpp : implementation file
//

#include "stdafx.h"
#include "ReportCtrl.h"
#include "TableSummary.h"
#include "ReportCtrlDef.h"


using namespace Gdiplus;

// TableSummary

IMPLEMENT_DYNAMIC(TableSummary, CWnd)

TableSummary::TableSummary()
{
	clearData();
}

TableSummary::~TableSummary()
{
}

void TableSummary::setEyeSide(OctGlobal::EyeSide side)
{
	m_eyeSide = side;

	return;
}

void TableSummary::initialize(void)
{
	clearData();
}

void TableSummary::clearData(void)
{
	m_data.clear();
	clearNormRange();
	return;
}

vector <pair<CString, CString>> TableSummary::getData(void)
{
	return m_data;
}

void TableSummary::insertData(CString name, CString data)
{
	pair <CString, CString> item;
	item.first = name;
	item.second = data;
	m_data.push_back(item);

	return;
}

BEGIN_MESSAGE_MAP(TableSummary, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// TableSummary message handlers


int TableSummary::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}


void TableSummary::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	drawTable(&dc, rect);

	return;
}

void TableSummary::drawTable(CDC *pDC, CRect rect)
{
	if (m_normRange.size() > 0) {
		_drawTableWithNormRange(pDC, rect);
	}
	else {
		_drawTable(pDC, rect);
	}
}

void TableSummary::drawTableForReport(CDC *pDC, CRect rect, OctGlobal::EyeSide eyeSide)
{
	if (m_normRange.size() > 0) {
		_drawTableWithNormRangeForReport(pDC, rect);
	}
	else {
		_drawTableForReport(pDC, rect);
	}
}

void TableSummary::addNormRange(int index, pair<float, float> data)
{
	m_normRange.push_back(make_tuple(index, get<0>(data), get<1>(data)));
}

void TableSummary::clearNormRange()
{
	m_normRange.clear();
}

auto TableSummary::getNormRangeList()->const vector<tuple<int, float, float>>
{
	return m_normRange;
}

void TableSummary::_drawTable(CDC *pDC, CRect rect)
{
	Font fontHeader(L"Noto Sans CJK KR Regular", 14, FontStyleRegular, UnitPixel);
	Font fontContent(L"Noto Sans CJK KR Regular", 12, FontStyleRegular, UnitPixel);

	int px = rect.left;
	int py = rect.top;
	int	rowWidth = rect.Width() / 2;
	int colHeight = rect.Height() / (kMaxCols + 1); // rect.Height() / (m_data.size() + 1);

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::StringFormat sfCenter;
	sfCenter.SetAlignment(Gdiplus::StringAlignmentCenter);
	sfCenter.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	Gdiplus::StringFormat sfLeft;
	sfLeft.SetAlignment(Gdiplus::StringAlignmentNear);
	sfLeft.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	// header
	SolidBrush brushHeadBk(Color(58, 61, 70));
	SolidBrush brushHeadText(Color(255, 255, 255));

	RectF rectHeader(rect.left, rect.top, rect.Width(), colHeight);
	G.FillRectangle(&brushHeadBk, rectHeader);
	G.DrawString(_T("Summary Parameter"), -1, &fontHeader, rectHeader, &sfCenter, &brushHeadText);
	for (size_t i = 0; i < m_data.size(); i++)
	{
		int px = rect.left;
		int py = rectHeader.GetBottom() + (i * colHeight);

		// name
		CString sName;
		sName.Format(_T("  %s"), m_data[i].first);

		RectF rectName(px, py, rowWidth, colHeight);
		SolidBrush brushNameBk(Color(230, 230, 230));
		SolidBrush brushNameText(Color(0, 0, 0));

		G.FillRectangle(&brushNameBk, rectName);
		G.DrawString(sName, -1, &fontContent, rectName, &sfLeft, &brushNameText);

		// value
		CString sValue;
		sValue.Format(_T("%s"), m_data[i].second);

		RectF rectValue(px + rowWidth, py, rowWidth, colHeight);
		SolidBrush brushValueBk(Color(246, 246, 246));
		SolidBrush brushValueText(Color(0, 0, 0));

		G.FillRectangle(&brushValueBk, rectValue);
		G.DrawString(sValue, -1, &fontContent, rectValue, &sfCenter, &brushValueText);
	}

	return;
}

void TableSummary::_drawTableForReport(CDC *pDC, CRect rect, OctGlobal::EyeSide eyeSide)
{
	if (m_data.size() == 0) {
		return;
	}

	Font fontHeader(L"Noto Sans CJK KR Regular", 18, FontStyleRegular, UnitPixel);
	Font fontContent(L"Noto Sans CJK KR Regular", 16, FontStyleRegular, UnitPixel);


	int px = rect.left;
	int py = rect.top;
	int	rowWidth = rect.Width() / 2;
	int colHeight = rect.Height() / (kMaxCols + 1);	//rect.Height() / (m_data.size() + 1);

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::StringFormat sfCenter;
	sfCenter.SetAlignment(Gdiplus::StringAlignmentCenter);
	sfCenter.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	Gdiplus::StringFormat sfLeft;
	sfLeft.SetAlignment(Gdiplus::StringAlignmentNear);
	sfLeft.SetLineAlignment(Gdiplus::StringAlignmentCenter);


	// header
	SolidBrush brushHeadBk(Color(58, 61, 70));
	SolidBrush brushHeadText(Color(255, 255, 255));

	RectF rectHeader(rect.left, rect.top, rect.Width(), colHeight);
	G.FillRectangle(&brushHeadBk, rectHeader);

	if (eyeSide == OctGlobal::EyeSide::OD) {
		G.DrawString(_T("OD Summary Parameter"), -1, &fontHeader, rectHeader, &sfCenter,
			&brushHeadText);
	}
	else if (eyeSide == OctGlobal::EyeSide::OS) {
		G.DrawString(_T("OS Summary Parameter"), -1, &fontHeader, rectHeader, &sfCenter,
			&brushHeadText);
	}
	else {
		G.DrawString(_T("Summary Parameter"), -1, &fontHeader, rectHeader, &sfCenter,
			&brushHeadText);
	}

	for (size_t i = 0; i < m_data.size(); i++)
	{
		int px = rect.left;
		int py = rectHeader.GetBottom() + (i * colHeight);

		// name
		CString sName;
		sName.Format(_T("  %s"), m_data[i].first);

		RectF rectName(px, py, rowWidth, colHeight);
		SolidBrush brushNameBk(Color(230, 230, 230));
		SolidBrush brushNameText(Color(0, 0, 0));

		G.FillRectangle(&brushNameBk, rectName);
		G.DrawString(sName, -1, &fontContent, rectName, &sfCenter, &brushNameText);

		// value
		CString sValue;
		sValue.Format(_T("%s"), m_data[i].second);

		RectF rectValue(px + rowWidth, py, rowWidth, colHeight);
		SolidBrush brushValueBk(Color(246, 246, 246));
		SolidBrush brushValueText(Color(0, 0, 0));

		G.FillRectangle(&brushValueBk, rectValue);
		G.DrawString(sValue, -1, &fontContent, rectValue, &sfCenter, &brushValueText);
	}

	return;
}

void TableSummary::_drawTableWithNormRange(CDC *pDC, CRect rect)
{
	Font fontHeader(L"Noto Sans CJK KR Regular", 14, FontStyleRegular, UnitPixel);
	Font fontContent(L"Noto Sans CJK KR Regular", 12, FontStyleRegular, UnitPixel);

	int px = rect.left;
	int py = rect.top;
	int	rowWidthLabel = static_cast<int>((float)rect.Width() * 0.4f);
	int	rowWidthValue = static_cast<int>((float)rect.Width() * 0.3f);
	int	rowWidthNormRange = static_cast<int>((float)rect.Width() * 0.3f);
	int colHeight = rect.Height() / (kMaxCols + 1); // rect.Height() / (m_data.size() + 1);

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::StringFormat sfCenter;
	sfCenter.SetAlignment(Gdiplus::StringAlignmentCenter);
	sfCenter.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	Gdiplus::StringFormat sfLeft;
	sfLeft.SetAlignment(Gdiplus::StringAlignmentNear);
	sfLeft.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	// header
	SolidBrush brushHeadBk(Color(58, 61, 70));
	SolidBrush brushHeadText(Color(255, 255, 255));

	RectF rectHeaderSum(rect.left, rect.top, rect.Width() * 0.7f, colHeight);
	G.FillRectangle(&brushHeadBk, rectHeaderSum);
	G.DrawString(_T("Summary"), -1, &fontHeader, rectHeaderSum, &sfCenter, &brushHeadText);

	RectF rectHeaderNormRange(rectHeaderSum.GetRight(), rect.top, rect.Width() * 0.3f, colHeight);
	G.FillRectangle(&brushHeadBk, rectHeaderNormRange);
	G.DrawString(_T("Normal"), -1, &fontHeader, rectHeaderNormRange, &sfCenter, &brushHeadText);

	for (size_t i = 0; i < m_data.size(); i++)
	{
		int px = rect.left;
		int py = rectHeaderSum.GetBottom() + (i * colHeight);

		// name
		CString sName;
		sName.Format(_T("  %s"), m_data[i].first);

		RectF rectName(px, py, rowWidthLabel, colHeight);
		SolidBrush brushNameBk(Color(230, 230, 230));
		SolidBrush brushNameText(Color(0, 0, 0));

		G.FillRectangle(&brushNameBk, rectName);
		G.DrawString(sName, -1, &fontContent, rectName, &sfLeft, &brushNameText);

		// value
		CString sValue;
		sValue.Format(_T("%s"), m_data[i].second);

		RectF rectValue(px + rowWidthLabel, py, rowWidthValue, colHeight);
		SolidBrush brushValueBk(Color(246, 246, 246));

		// Check outside normal range
		bool isAbnormal = false;
		if (m_data[i].second == _T("---")) {
			isAbnormal = true;
		}
		else {
			auto rangeItr = std::find_if(m_normRange.begin(), m_normRange.end(),
				[i](auto item) { return get<0>(item) == i; }
			);
			if (rangeItr != m_normRange.end()) {
				auto normRangeItem = *rangeItr;
				float minRange = get<1>(normRangeItem);
				float maxRange = get<2>(normRangeItem);

				if (minRange != ReportCtrl::kSummaryValueInvalid &&
					maxRange != ReportCtrl::kSummaryValueInvalid) {
					float currentValue = _ttof(m_data[i].second);
					if (currentValue < minRange || currentValue > maxRange) {
						isAbnormal = true;
					}
				}
			}
		}

		SolidBrush brushValueText(isAbnormal ? Color(255, 0, 0) : Color(0, 0, 0));

		G.FillRectangle(&brushValueBk, rectValue);
		G.DrawString(sValue, -1, &fontContent, rectValue, &sfCenter, &brushValueText);

		SolidBrush brushValueNormRange(Color(0, 0, 0));
		RectF rectNormRange(rectValue.GetRight(), py, rowWidthNormRange, colHeight);
		G.FillRectangle(&brushValueBk, rectNormRange);

		// norm
		auto itr = std::find_if(m_normRange.begin(), m_normRange.end(),
			[i](auto item) { return get<0>(item) == i; }
		);
		if (itr != m_normRange.end()) {
			auto normRangeItem = *itr;

			CString szNormRange;
			if (get<1>(normRangeItem) != ReportCtrl::kSummaryValueInvalid &&
				get<2>(normRangeItem) != ReportCtrl::kSummaryValueInvalid) {
				szNormRange.Format(_T("%.2f~%.2f"), get<1>(normRangeItem), get<2>(normRangeItem));
			}
			else {
				szNormRange = _T("");
			}

			G.DrawString(szNormRange, -1, &fontContent, rectNormRange, &sfCenter, &brushValueNormRange);
		}
	}

	return;
}

void TableSummary::_drawTableWithNormRangeForReport(CDC *pDC, CRect rect, OctGlobal::EyeSide eyeSide)
{
	if (m_data.size() == 0) {
		return;
	}

	Font fontHeader(L"Noto Sans CJK KR Regular", 18, FontStyleRegular, UnitPixel);
	Font fontContent(L"Noto Sans CJK KR Regular", 16, FontStyleRegular, UnitPixel);

	int px = rect.left;
	int py = rect.top;
	int	rowWidthLabel = static_cast<int>((float)rect.Width() * 0.4f);
	int	rowWidthValue = static_cast<int>((float)rect.Width() * 0.3f);
	int	rowWidthNormRange = static_cast<int>((float)rect.Width() * 0.3f);
	int colHeight = rect.Height() / (kMaxCols + 1);	//rect.Height() / (m_data.size() + 1);

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::StringFormat sfCenter;
	sfCenter.SetAlignment(Gdiplus::StringAlignmentCenter);
	sfCenter.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	Gdiplus::StringFormat sfLeft;
	sfLeft.SetAlignment(Gdiplus::StringAlignmentNear);
	sfLeft.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	// header
	SolidBrush brushHeadBk(Color(58, 61, 70));
	SolidBrush brushHeadText(Color(255, 255, 255));

	RectF rectHeaderSum(rect.left, rect.top, rect.Width() * 0.7f, colHeight);
	G.FillRectangle(&brushHeadBk, rectHeaderSum);

	if (eyeSide == OctGlobal::EyeSide::OD) {
		G.DrawString(_T("OD Summary"), -1, &fontHeader, rectHeaderSum, &sfCenter,
			&brushHeadText);
	}
	else if (eyeSide == OctGlobal::EyeSide::OS) {
		G.DrawString(_T("OS Summary"), -1, &fontHeader, rectHeaderSum, &sfCenter,
			&brushHeadText);
	}
	else {
		G.DrawString(_T("Summary"), -1, &fontHeader, rectHeaderSum, &sfCenter,
			&brushHeadText);
	}

	// norm
	RectF rectHeaderNormRange(rectHeaderSum.GetRight(), rect.top, rect.Width() * 0.3f, colHeight);
	G.FillRectangle(&brushHeadBk, rectHeaderNormRange);
	G.DrawString(_T("Normal"), -1, &fontHeader, rectHeaderNormRange, &sfCenter, &brushHeadText);

	for (size_t i = 0; i < m_data.size(); i++)
	{
		int px = rect.left;
		int py = rectHeaderSum.GetBottom() + (i * colHeight);

		// name
		CString sName;
		sName.Format(_T("  %s"), m_data[i].first);

		RectF rectName(px, py, rowWidthLabel, colHeight);
		SolidBrush brushNameBk(Color(230, 230, 230));
		SolidBrush brushNameText(Color(0, 0, 0));

		G.FillRectangle(&brushNameBk, rectName);
		G.DrawString(sName, -1, &fontContent, rectName, &sfCenter, &brushNameText);

		// value
		CString sValue;
		sValue.Format(_T("%s"), m_data[i].second);

		RectF rectValue(px + rowWidthLabel, py, rowWidthValue, colHeight);
		SolidBrush brushValueBk(Color(246, 246, 246));
		
		// Check outside normal range
		bool isAbnormal = false;

		if (m_data[i].second == _T("---")) {
			isAbnormal = true;
		}
		else {
			auto rangeItr = std::find_if(m_normRange.begin(), m_normRange.end(),
				[i](auto item) { return get<0>(item) == i; }
			);
			if (rangeItr != m_normRange.end()) {
				auto normRangeItem = *rangeItr;
				float minRange = get<1>(normRangeItem);
				float maxRange = get<2>(normRangeItem);

				if (minRange != ReportCtrl::kSummaryValueInvalid &&
					maxRange != ReportCtrl::kSummaryValueInvalid) {
					float currentValue = _ttof(m_data[i].second);
					if (currentValue < minRange || currentValue > maxRange) {
						isAbnormal = true;
					}
				}
			}
		}

		SolidBrush brushValueText(isAbnormal ? Color(255, 0, 0) : Color(0, 0, 0));

		G.FillRectangle(&brushValueBk, rectValue);
		G.DrawString(sValue, -1, &fontContent, rectValue, &sfCenter, &brushValueText);

		SolidBrush brushValueTextNormRange(Color(0, 0, 0));
		RectF rectNormRange(rectValue.GetRight(), py, rowWidthNormRange, colHeight);
		G.FillRectangle(&brushValueBk, rectNormRange);

		// norm
		auto itr = std::find_if(m_normRange.begin(), m_normRange.end(),
			[i](auto item) { return get<0>(item) == i; }
		);
		if (itr != m_normRange.end()) {
			auto normRangeItem = *itr;

			CString szNormRange;
			if (get<1>(normRangeItem) != ReportCtrl::kSummaryValueInvalid &&
				get<2>(normRangeItem) != ReportCtrl::kSummaryValueInvalid) {
				szNormRange.Format(_T("%.2f~%.2f"), get<1>(normRangeItem), get<2>(normRangeItem));
			}
			else {
				szNormRange = _T("");
			}

			G.DrawString(szNormRange, -1, &fontContent, rectNormRange, &sfCenter, &brushValueTextNormRange);
		}
	}

	return;
}
