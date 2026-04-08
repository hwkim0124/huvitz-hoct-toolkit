#pragma once
struct FundusAnnotationImpl;
class FundusAnnotation
{
	const float kDispTextMarginX = 4.0f;
	const float kDispTextMarginY = 1.0f;

public:
	FundusAnnotation(int id, std::pair<int, int> pointPos, std::pair<int, int> textPos,
		std::wstring textContent);
	FundusAnnotation(const FundusAnnotation& refObj);
	~FundusAnnotation();

	FundusAnnotation& operator =(const FundusAnnotation& refObj);

	auto id()->int;
	auto pointPos()->std::pair<int, int>;
	auto textPos()->std::pair<int, int>;
	auto textContent()->std::wstring;
	void setId(int id);
	void setContent(std::wstring value);
	bool selectAnnotation(std::pair<int, int> pt, CRect rcWnd, CRect rcImage);
	void unSelectAnnotation();
	void drawTo(CDC* pDC, CRect rcWnd, CRect rcImage, CRect rcRoi);

protected:
	std::shared_ptr<FundusAnnotationImpl> m_pImpl;

protected:
	CFont m_font;
};

