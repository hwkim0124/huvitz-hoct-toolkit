#include "stdafx.h"
#include "FundusAnnotation.h"


struct FundusAnnotationImpl {
	int id;
	std::pair<int, int> pointPos;
	std::pair<int, int> textPos;
	std::wstring textContent;
	CRect labelRect = { 0, 0, 0, 0};
	bool bSelected = false;
};

FundusAnnotation::FundusAnnotation(int id, std::pair<int, int> pointPos, std::pair<int, int> textPos,
	std::wstring textContent)
	: m_pImpl(make_shared<FundusAnnotationImpl>())
{
	m_font.CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Noto Sans CJK KR Medium"));

	m_pImpl->id = id;
	m_pImpl->pointPos = pointPos;
	m_pImpl->textPos = textPos;
	m_pImpl->textContent = textContent;
}

FundusAnnotation::FundusAnnotation(const FundusAnnotation& refObj)
{
	m_font.CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Noto Sans CJK KR Medium"));

	m_pImpl = refObj.m_pImpl;
}

FundusAnnotation::~FundusAnnotation()
{
	m_font.DeleteObject();
}

FundusAnnotation& FundusAnnotation::operator =(const FundusAnnotation& refObj)
{
	m_pImpl = refObj.m_pImpl;

	return *this;
}

auto FundusAnnotation::id()->int
{
	return m_pImpl->id;
}

auto FundusAnnotation::pointPos()->std::pair<int, int>
{
	return m_pImpl->pointPos;
}

auto FundusAnnotation::textPos()->std::pair<int, int>
{
	return m_pImpl->textPos;
}

auto FundusAnnotation::textContent()->std::wstring
{
	return m_pImpl->textContent;
}

void FundusAnnotation::setId(int id)
{
	m_pImpl->id = id;
}

void FundusAnnotation::setContent(std::wstring value)
{
	m_pImpl->textContent = value;
}

bool FundusAnnotation::selectAnnotation(std::pair<int, int> pt, CRect rcWnd, CRect rcImage)
{
	auto fWidthRatio = (float)rcWnd.Width() / (float)rcImage.Width();
	auto fHeightRatio = (float)rcWnd.Height() / (float)rcImage.Height();
	int ptX = rcWnd.left + int((float)get<0>(pt) * fWidthRatio);
	int ptY = rcWnd.top + int((float)get<1>(pt) * fHeightRatio);

	if (m_pImpl->labelRect.PtInRect({ ptX, ptY })) {
		m_pImpl->bSelected = true;
	}
	else {
		m_pImpl->bSelected = false;
	}

	return m_pImpl->bSelected;
}

void FundusAnnotation::unSelectAnnotation()
{
	m_pImpl->bSelected = false;
}

void FundusAnnotation::drawTo(CDC* pDC, CRect rcWnd, CRect rcImage, CRect rcRoi)
{
	if (pDC == nullptr) {
		return;
	}

	// get data
	auto fWidthRatio = (float)rcWnd.Width() / (float)rcImage.Width();
	auto fHeightRatio = (float)rcWnd.Height() / (float)rcImage.Height();
	auto ptX = (float)rcWnd.left + (float)get<0>(m_pImpl->pointPos) * fWidthRatio;
	auto ptY = (float)rcWnd.top + (float)get<1>(m_pImpl->pointPos) * fHeightRatio;
	if (!rcRoi.PtInRect({ (LONG)ptX, (LONG)ptY })) {
		return;
	}

	CString szContent(m_pImpl->textContent.c_str());
	float textX = ptX - 3 + kDispTextMarginX + 20;
	float textY = ptY - 6 - kDispTextMarginY - 30;
	float kPointR = 6.0f;

	// prepare to draw
	Gdiplus::Graphics G(pDC->GetSafeHdc());
	const Gdiplus::Color kColorBG(125, 255, 255, 255);
	const Gdiplus::SolidBrush kBrushBG(kColorBG);
	const Gdiplus::SolidBrush kBrushPen(Gdiplus::Color(255, 0, 0));
	const Gdiplus::SolidBrush kBrushPenSelected(Gdiplus::Color(0, 0, 255));
	const Gdiplus::Pen kPen(Gdiplus::Color(90, 255, 0, 0));
	const Gdiplus::Pen kPenSelected(Gdiplus::Color(90, 0, 0, 255));
	const Gdiplus::Pen kPenPoint(Gdiplus::Color(255, 0, 0), 2.0);
	const Gdiplus::Pen kPenPointSelected(Gdiplus::Color(0, 0, 255), 2.0);

	Gdiplus::Font font(_T("Noto Sans CJK KR Medium"), 14);
	Gdiplus::RectF rcText;
	G.MeasureString(szContent, szContent.GetLength(), &font, Gdiplus::PointF(0, 0), &rcText);
	textY -= rcText.Height;
	rcText.Offset(textX, textY);

	Gdiplus::RectF rcBG(rcText.X - kDispTextMarginX, rcText.Y - kDispTextMarginY,
		rcText.Width + kDispTextMarginX * 2, rcText.Height + kDispTextMarginY * 2);

	// draw
	if (szContent.GetLength()) {
		G.FillRectangle(&kBrushBG, rcBG);
		G.FillEllipse(&kBrushBG, ptX - kPointR / 2.0f, ptY - kPointR / 2.0f, kPointR, kPointR);
		Gdiplus::PointF contackPtPt(ptX + 1, ptY - 1);
		Gdiplus::PointF contackPtText(rcText.GetLeft() - kDispTextMarginX,
			rcText.GetBottom() + kDispTextMarginY);
		if (m_pImpl->bSelected) {
			G.DrawRectangle(&kPenSelected, rcBG);
			G.DrawString(szContent, szContent.GetLength(), &font, Gdiplus::PointF(textX, textY),
				&kBrushPenSelected);
			G.DrawLine(&kPenSelected, contackPtPt, contackPtText);
			G.DrawEllipse(&kPenPointSelected, ptX - kPointR / 2.0f, ptY - kPointR / 2.0f, kPointR,
				kPointR);
		}
		else {
			G.DrawRectangle(&kPen, rcBG);
			G.DrawString(szContent, szContent.GetLength(), &font, Gdiplus::PointF(textX, textY),
				&kBrushPen);
			G.DrawLine(&kPen, contackPtPt, contackPtText);
			G.DrawEllipse(&kPenPoint, ptX - kPointR / 2.0f, ptY - kPointR / 2.0f, kPointR, kPointR);
		}
	}
	else {
		G.FillEllipse(&kBrushBG, ptX - kPointR / 2.0f, ptY - kPointR / 2.0f, kPointR, kPointR);
		G.DrawEllipse(&kPenPointSelected, ptX - kPointR / 2.0f, ptY - kPointR / 2.0f, kPointR,
			kPointR);
	}

	// store label rect
	m_pImpl->labelRect.SetRect((int)rcText.X, (int)rcText.Y, (int)(rcText.X + rcText.Width),
		(int)(rcText.Y + rcText.Height));
}
