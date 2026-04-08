#pragma once


class REPORTCTRL_DLL_API DrawEtdrsChart
{
public:
	DrawEtdrsChart();
	~DrawEtdrsChart();
	void initialize(EyeSide side);
	void showChartColor(bool show);
	bool isShowingEtdrsChartColor();

	void clearData(void);
	
	void setEtdrsChartThickCenter(float thick);
	void setEtdrsChartThickInner(int index, float thick);
	void setEtdrsChartThickOuter(int index, float thick);

	void setEtdrsChartPercentCenter(float percent);
	void setEtdrsChartPercentInner(int index, float percent);
	void setEtdrsChartPercentOuter(int index, float percent);

	float getEtdrsChartThickCenter(void);
	float getEtdrsChartThickInner(int index);
	float getEtdrsChartThickOuter(int index);

	void drawEtdrsChart(CDC *pDC, CPoint ptCenter, float radius, Gdiplus::Font &font,	Gdiplus::Color line, Gdiplus::Color value, Gdiplus::Color tsnit);
	void drawEtdrsColorTable(CDC *pDC, CRect rect, Gdiplus::Color colorText, int fontSize);

	void setCompareType(EyeSide side);
protected:
	Gdiplus::Color getPercentileColor(float percent);

protected:
	EyeSide m_eyeSide;
	CString m_direction[4];
	bool	m_showChartColor;

	float m_thickCenter;
	float m_thickInner[4];
	float m_thickOuter[4];
	float m_percentCenter;
	float m_percentInner[4];
	float m_percentOuter[4];
};

