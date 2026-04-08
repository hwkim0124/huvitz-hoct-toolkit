#pragma once


class REPORTCTRL_DLL_API DrawRnflChart
{
public:
	DrawRnflChart();
	~DrawRnflChart();
	void initialize(EyeSide side);
	void setCompareType(EyeSide side);
	void showChartColor(bool show);
	bool isShowingRnflChartColor();

	void clearData(void);
	void setRnflQuadThickness(int index, float thickness);
	void setRnflQuadPercentile(int index, float percentile);
	void setRnflClockThickness(int index, float thickness);
	void setRnflClockPercentile(int index, float percentile);

	void drawRnflChartQuad(CDC *pDC, CPoint ptCenter, int radius, Gdiplus::Font &font, Gdiplus::Color Line, Gdiplus::Color value, Gdiplus::Color tsnit);
	void drawRnflChartClock(CDC *pDC, CPoint ptCenter, int radius, Gdiplus::Font &font, Gdiplus::Color Line, Gdiplus::Color value);
	void drawRnflColorTable(CDC *pDC, CRect rect, Gdiplus::Color value, int fontSize);

protected:
	Gdiplus::Color getPercentileColor(float percent);

private:
	EyeSide m_eyeSide;
	CString m_direction[4];
	bool m_showChartColor;

	float m_thicknessQuad[4];
	float m_percentileQuad[4];
	float m_thicknessClock[12];
	float m_percentileClock[12];
};

