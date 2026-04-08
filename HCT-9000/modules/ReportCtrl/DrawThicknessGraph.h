#pragma once
#include "ReportCtrlDef.h"

#define MAX_SERIES	2

class REPORTCTRL_DLL_API DrawThicknessGraph
{
public:
	DrawThicknessGraph();
	~DrawThicknessGraph();

	void initialize(void);
	void setTsnitChart(bool tsnit);

	void showCaption(bool show);
	void setCaptionFont(CFont *font);
	void setCaptionColor(COLORREF color);

	void showAxisValue(bool show);
	void setAxisValueFont(CFont *font);
	void setAxisValueColor(COLORREF color);

	void setBkGroundColor(COLORREF color);
	void setAxisLineColor(COLORREF color);
	void setGridLineColor(COLORREF color);
	void setSeriesColor(COLORREF color);
	COLORREF getSeriesColor();

	void setThicknessLayer(OcularLayerType upper, OcularLayerType lower);
	void setThicknessData(UINT series, std::vector <float> data);
	void setNormaltiveData(std::vector <float> normal, std::vector <float> border, std::vector <float> outside);

	void drawThicknessGraph(CDC *pDC, CRect rect, bool report = false);
	void drawThicknessGraphOnlySeries(CDC *pDC, CRect rect, Gdiplus::Color color, bool report = false);
	void drawThicknessGraph(CDC *pDC, CRect rect, COLORREF axisLine, COLORREF gridLine, COLORREF value);
	void drawColorTable(CDC *pDC, Gdiplus::RectF rect_in, Gdiplus::Color colorValue , int fontSize);

private:
	void drawSeries(CDC *pDC, CRect rect, std::vector <float> data, Gdiplus::Color color);
	void drawNormative(CDC *pDC, CRect rect);

	Gdiplus::Color getPercentileColor(float percent);

protected:
	bool m_isTsnit;

	bool m_showCaption;
	CFont m_fontCaption;
	COLORREF m_colorCaption;

	bool m_showAxisValue;
	CFont m_fontAxisValue;
	COLORREF m_colorAxisValue;

	COLORREF m_colorBkGround;
	COLORREF m_colorAxisLine;
	COLORREF m_colorGridLine;
	COLORREF m_colorSeries;

	int m_rangeMin;
	int m_rangeMax;

	std::vector <float> m_data[MAX_SERIES];
	std::vector <float> m_dataNormal;
	std::vector <float> m_dataBorder;
	std::vector <float> m_dataOutside;
};

