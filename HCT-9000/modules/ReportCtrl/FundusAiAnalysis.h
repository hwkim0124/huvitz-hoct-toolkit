#pragma once
class FundusAiAnalysis
{
public:
	FundusAiAnalysis(std::wstring name, std::vector<uchar> roiImageData, double labelPosX, double labelPosY);
	~FundusAiAnalysis();

	void drawTo(CDC* pDC, CRect rcWnd, CRect rcImage, CRect rcRoi);
	
	double getLabelPosX();
	double getLabelPosY();

	void setLabelPosX(double value);
	void setLabelPosY(double value);

private:
	std::wstring m_name;
	std::vector<uchar> m_roiImageData;
	double m_labelPosX;
	double m_labelPosY;
	static std::vector<std::tuple<std::wstring, std::wstring, Gdiplus::Color>> m_colorList;
};

