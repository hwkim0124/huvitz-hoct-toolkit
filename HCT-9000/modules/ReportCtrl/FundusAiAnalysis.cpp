#include "stdafx.h"
#include "FundusAiAnalysis.h"

using namespace Gdiplus;

std::vector<std::tuple<std::wstring, std::wstring, Gdiplus::Color>> FundusAiAnalysis::m_colorList = {
	std::make_tuple(L"Hemorrhage", L"Hemorrhage", Gdiplus::Color(236, 100, 46)),
	std::make_tuple(L"Hard Exudate", L"HardExudate", Gdiplus::Color(255, 255, 0)),
	std::make_tuple(L"Cotton Wool Patch", L"CWP", Gdiplus::Color(135, 135, 135)),
	std::make_tuple(L"Drusen & Drusenoid Deposits", L"Drusen", Gdiplus::Color(172, 125, 18)),
	std::make_tuple(L"Macular Hole", L"MacularHole", Gdiplus::Color(0, 0, 0)),
	std::make_tuple(L"Vascular Abnormality", L"VascularAbnormality", Gdiplus::Color(153, 0, 0)),
	std::make_tuple(L"Membrane", L"Membrane", Gdiplus::Color(0, 255, 0)),
	std::make_tuple(L"Chorioretinal Atrophy/Scar", L"ChroioretinalAtrophy", Gdiplus::Color(255, 255, 153)),
	std::make_tuple(L"Myelinated Nerve Fiber", L"MyelinatedNerveFiber", Gdiplus::Color(239, 239, 239)),
	std::make_tuple(L"RNFL Defect", L"RNFLDefect", Gdiplus::Color(45, 45, 45)),
	std::make_tuple(L"Glaucomatous Disc Change", L"GlaucomatousDiscChange", Gdiplus::Color(255, 51, 153)),
	std::make_tuple(L"Non-glaucomatous Disc Change", L"NonGlaucomatousDiscChange", Gdiplus::Color(153, 0, 153)),
};

FundusAiAnalysis::FundusAiAnalysis(std::wstring name, std::vector<uchar> roiImageData,
	double labelPosX, double labelPosY)
{
	m_name = name;
	m_roiImageData = roiImageData;
	m_labelPosX = labelPosX;
	m_labelPosY = labelPosY;
}

FundusAiAnalysis::~FundusAiAnalysis()
{
}

void FundusAiAnalysis::drawTo(CDC* pDC, CRect rcWnd, CRect rcImage, CRect rcRoi)
{
	if (pDC == nullptr) {
		return;
	}

	Gdiplus::Graphics G(pDC->GetSafeHdc());

	// roi image
	// 1. create image by data
	auto destRect = Gdiplus::Rect(rcRoi.left, rcRoi.top, rcRoi.Width(), rcRoi.Height());
	const size_t kBufferSize = m_roiImageData.size() * sizeof(unsigned char);
	// - make memory stream
	HGLOBAL	hMem = ::GlobalAlloc(GMEM_MOVEABLE, kBufferSize);
	if (!hMem) {
		AfxThrowMemoryException();
		return;
	}

	LPVOID pImageBuffer = ::GlobalLock(hMem);
	CopyMemory(pImageBuffer, &m_roiImageData[0], kBufferSize);
	::GlobalUnlock(hMem);

	CComPtr<IStream> spStream;
	if (::CreateStreamOnHGlobal(hMem, FALSE, &spStream) != S_OK) {
		return ;
	}
	// - make image from the memory stream
	auto pImage = Image::FromStream(spStream);

	// - draw
	G.DrawImage(pImage, destRect);

	// - release resources
	spStream.Release();

	// label
	auto fWidthRatio = (float)rcWnd.Width() / (float)rcImage.Width();
	auto fHeightRatio = (float)rcWnd.Height() / (float)rcImage.Height();
	auto textX = (float)rcWnd.left + (float)m_labelPosX * fWidthRatio;
	auto textY = (float)rcWnd.top + (float)m_labelPosY * fHeightRatio;
	CString szName(m_name.c_str());

	Gdiplus::Font font(_T("Noto Sans CJK KR Medium"), 14);
	Gdiplus::Color labelColor = Gdiplus::Color::Aqua; // default color

	auto item = std::find_if(m_colorList.begin(), m_colorList.end(),
		[this](auto x) { return std::get<0>(x) == m_name || std::get<1>(x) == m_name; });
	if (item != m_colorList.end()) {
		labelColor = std::get<2>(*item);
	}
	
	Gdiplus::RectF rcLabel;
	G.MeasureString(szName, (int)m_name.length(), &font, Gdiplus::PointF(textX, textY), &rcLabel);

	// label position top-left
	textY -= rcLabel.Height;

	// over to right
	float overedToRight = rcLabel.GetRight() - destRect.GetRight();
	if (overedToRight > 0) {
		textX -= overedToRight;
	}

	// over to top
	if (textY < 0.0) {
		textY = 0.0;
	}

	Gdiplus::SolidBrush brush(labelColor);
	G.DrawString(szName, (int)m_name.length(), &font, Gdiplus::PointF(textX, textY), &brush);
}

double FundusAiAnalysis::getLabelPosX()
{
	return m_labelPosX;
}

double FundusAiAnalysis::getLabelPosY()
{
	return m_labelPosY;
}

void FundusAiAnalysis::setLabelPosX(double value)
{
	m_labelPosX = value;
}

void FundusAiAnalysis::setLabelPosY(double value)
{
	m_labelPosY = value;
}
