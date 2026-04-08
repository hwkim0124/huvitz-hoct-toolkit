#pragma once

class REPORTCTRL_DLL_API DrawGccChart
{
public:
	DrawGccChart();
	~DrawGccChart();
	void initialize(EyeSide side);
	void setCompareType(EyeSide side);
	void showChartColor(bool show);
	bool isShowingGccChartColor();

	void clearData(void);
	void setGccChartThickness(int index, float thick);
	float getGccChartThickness(int index);
	void setGccChartThicknessSI(int index, float thick);
	float getGccChartThicknessSI(int index);
	void setGccChartPercentile(int index, float percent);
	float getGccChartPercentile(int index);
	void setGccChartPercentileSI(int index, float percent);
	float getGccChartPercentileSI(int index);

	void drawGccChart(CDC *pDC, CPoint ptCenter, float radius, Gdiplus::Font &font, Gdiplus::Color line, Gdiplus::Color value, Gdiplus::Color tsnit);
	void drawGccChartSI(CDC *pDC, CPoint ptCenter, float radius, Gdiplus::Font &font, Gdiplus::Color line, Gdiplus::Color value, Gdiplus::Color tsnit);
	void drawGccColorTable(CDC *pDC, CRect rect, Gdiplus::Color colorText, int fontSize);

protected:
	Gdiplus::Color getPercentileColor(float percent);

private:
	EyeSide m_eyeSide;
	CString m_direction[4];
	bool	m_showChartColor;

	float m_thickness[6];
	float m_percentile[6];
	float m_thicknessSI[2]; /*0: superior, 1: inferior*/
	float m_percentileSI[2]; /*0: superior, 1: inferior*/
};

