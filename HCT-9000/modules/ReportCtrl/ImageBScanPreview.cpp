// ImageBScanPreview.cpp : implementation file
//

#include "stdafx.h"
#include "ReportCtrl.h"
#include "ImageBScanPreview.h"


// ImageBScanPreview

IMPLEMENT_DYNAMIC(ImageBScanPreview, CWnd)

ImageBScanPreview::ImageBScanPreview()
	: m_axialLength(-1.0f), m_showAxialLength(false), m_sigCenter(-1), m_showTopoGuideLine(false)
{
	m_fontNotoSans15.CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Noto Sans CJK KR Regular"));
	m_fontNotoSans16.CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Noto Sans CJK KR Regular"));
}

ImageBScanPreview::~ImageBScanPreview()
{
	m_fontNotoSans15.DeleteObject();
	m_fontNotoSans16.DeleteObject();
}

void ImageBScanPreview::setImage(unsigned char *data, unsigned int width, unsigned int height,
	int sigCenter, std::tuple<bool, int, int> scanInfo)
{
	if (data == nullptr) {
		return;
	}

	//unsigned int resize_py = height - 512 - 96;
	//unsigned int resize_height = 512;
	//unsigned char *resize_data = data + (resize_py * width);

	ImageBase::setImage(data, width, height);

	//if (sigCenter > 0) {
	//	int sigCenterInResized = sigCenter - 96;
	//	m_sigCenter = (int)((float)getWndHeight() / (float)resize_height * (float)sigCenterInResized);
	//}
	//else {
	//	m_sigCenter = -1;
	//}
	m_sigCenter = sigCenter;
	m_scanInfo = scanInfo;

	return;
}

void ImageBScanPreview::setScanPattern(OctScanPattern pattern)
{
	m_pattern = pattern;

	return;
}

void ImageBScanPreview::setScanQuality(float quality)
{
	m_quality = quality;

	return;
}

void ImageBScanPreview::setSubGuidePos(int value)
{
	m_subGuidePos = value;
}

void ImageBScanPreview::showTopoGuideLine(bool value)
{
	m_showTopoGuideLine = value;
}

void ImageBScanPreview::setAxialLength(float value)
{
	m_axialLength = value;
	m_szAxialLengthMsg = CString("");
}

float ImageBScanPreview::getAxialLength()
{
	return m_axialLength;
}

void ImageBScanPreview::showAxialLength(bool value)
{
	m_showAxialLength = value;
}

void ImageBScanPreview::setAxialLengthMsg(CString szMsg)
{
	m_axialLength = -1.0f;
	m_szAxialLengthMsg = szMsg;
}

void ImageBScanPreview::setAxialLengthNumMsg(CString szMsg)
{
	m_szAxialLengthNumMsg = szMsg;
}

void ImageBScanPreview::getDstArea(int *xDst, int *yDst, int *wDst, int *hDst)
{
	*xDst = 0;
	*yDst = 0;
	*wDst = getWndWidth();
	*hDst = getWndHeight();
}


BEGIN_MESSAGE_MAP(ImageBScanPreview, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()

// ImageBScanPreview message handlers


void ImageBScanPreview::OnPaint()
{
	CPaintDC dc(this);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, getWndWidth(), getWndHeight());

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	memDC.SelectObject(&bitmap);

	if (!m_image.isEmpty())
	{
		drawImage(&memDC);
		drawGuidLine(&memDC);
		drawSSI(&memDC);
		drawPatternInfo(&memDC);
		drawScanInfo(&memDC);
		drawAxialLength(&memDC);
	}

	dc.BitBlt(0, 0, getWndWidth(), getWndHeight(), &memDC, 0, 0, SRCCOPY);

	bitmap.DeleteObject();
	memDC.DeleteDC();
}

void ImageBScanPreview::drawGuidLine(CDC *pDC)
{
	CRect rect;
	GetClientRect(&rect);

	if (m_showTopoGuideLine) {
		CPen penHorz(PS_SOLID, 1, RGB(255, 0, 0));
		CPen penVert(PS_SOLID, 1, RGB(0, 204, 192));

		pDC->SelectObject(&penVert);
		pDC->MoveTo(rect.Width() / 2, 0);
		pDC->LineTo(rect.Width() / 2, rect.Height());

		pDC->SelectObject(&penHorz);
		pDC->MoveTo(0, rect.Height() / 5);
		pDC->LineTo(rect.Width(), rect.Height() / 5);
		/*pDC->MoveTo(0, rect.Height() / 5 * 4);
		pDC->LineTo(rect.Width(), rect.Height() / 5 * 4);*/

		penHorz.DeleteObject();
		penVert.DeleteObject();
	}
	else {
		CPen penHorz(PS_SOLID, 1, RGB(0, 204, 192));
		CPen penVert(PS_SOLID, 1, RGB(221, 255, 190));

		pDC->SelectObject(&penVert);
		pDC->MoveTo(rect.Width() / 2, 0);
		pDC->LineTo(rect.Width() / 2, rect.Height());

		pDC->SelectObject(&penHorz);
		pDC->MoveTo(0, rect.Height() / 4);
		pDC->LineTo(rect.Width(), rect.Height() / 4);
		pDC->MoveTo(0, rect.Height() / 4 * 3);
		pDC->LineTo(rect.Width(), rect.Height() / 4 * 3);

		penHorz.DeleteObject();
		penVert.DeleteObject();
	}
	
	// sub guide line
	if (m_subGuidePos >= 0) {
		CPen penSubGuide(PS_SOLID, 1, RGB(255, 0, 0));
		pDC->SelectObject(&penSubGuide);
		pDC->MoveTo(m_subGuidePos, 0);
		pDC->LineTo(m_subGuidePos, rect.Height());
	}


	return;
}

void ImageBScanPreview::drawSSI(CDC* pDC)
{
	int ssi;
	ssi = (int)m_quality;

	CString text;
	text.Format(_T("SSI (%d/10)"), ssi);

	CRect rect;
	rect.left = 13;
	rect.right = 93;
	rect.top = getWndHeight() - 46;
	rect.bottom = getWndHeight() - 26 + 1;
	pDC->SetTextColor(RGB(255, 255, 255));
	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&m_fontNotoSans16);
	pDC->DrawText(text, rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);


	for (int i = 0; i < 10; i++)
	{
		CRect rect;
		rect.left = 13 + (i * 21);
		rect.right = 32 + (i * 21);
		rect.top = getWndHeight() - 20;
		rect.bottom = getWndHeight() - 12;

		if (i >= ssi) {
			pDC->FillRect(&rect, &CBrush(RGB(65, 65, 65)));
		}
		else if (i < 3) {
			pDC->FillRect(&rect, &CBrush(RGB(217, 48, 45)));
		}
		else if (i < 6) {
			pDC->FillRect(&rect, &CBrush(RGB(220, 220, 89)));
		}
		else {
			pDC->FillRect(&rect, &CBrush(RGB(106, 177, 71)));
		}
	}

	return;
}

void ImageBScanPreview::drawPatternInfo(CDC* pDC)
{
	PatternName name = m_pattern.getPatternName();
	int points = m_pattern.getNumberOfScanPoints();
	int lines = m_pattern.getNumberOfScanLines();
	CString sName = CString(m_pattern.getPatternNameString());
	int overlap = m_pattern.getScanOverlaps();
	float rangeX = m_pattern.getScanRangeX();
	float rangeY = m_pattern.getScanRangeY();
	CString text;

	switch (name)
	{
	case OctGlobal::PatternName::MacularLine:
	case OctGlobal::PatternName::AnteriorLine:
	case OctGlobal::PatternName::AnteriorFull:
	{
		text.Format(_T("A%d, %s x%d, %.1fmm"), points, sName, overlap, rangeX);
	}
	break;
	case OctGlobal::PatternName::MacularCross:
	case OctGlobal::PatternName::MacularRadial:
	case OctGlobal::PatternName::DiscRadial:
	case OctGlobal::PatternName::AnteriorRadial:
	case OctGlobal::PatternName::MacularRaster:
	case OctGlobal::PatternName::Macular3D:
	case OctGlobal::PatternName::DiscRaster:
	case OctGlobal::PatternName::Disc3D:
	case OctGlobal::PatternName::Anterior3D:
	case OctGlobal::PatternName::MacularDisc:
	case OctGlobal::PatternName::Topography:
	{
		text.Format(_T("A%d, B%d, %s x%d, %.1fx%.1fmm"), points, lines, sName, overlap, rangeX, rangeY);
	}
	break;
	case OctGlobal::PatternName::DiscCircle:
	{
		text.Format(_T("A%d, %s x%d, %.1fx%.1fmm"), points, sName, overlap, rangeX, rangeY);
	}
	break;
	default:
		break;
	}


	CRect rect;
	rect.left = 246;
	rect.right = getWndWidth() - 13;//546;
	rect.top = getWndHeight() - 25;
	rect.bottom = getWndHeight() - 10 + 1;
	pDC->SetTextColor(RGB(186, 186, 186));
	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&m_fontNotoSans15);
	pDC->DrawText(text, rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	return;
}

void ImageBScanPreview::drawScanInfo(CDC* pDC)
{
	if (!get<0>(m_scanInfo)) {
		return;
	}

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	Gdiplus::StringFormat sfLeftTop;
	sfLeftTop.SetAlignment(Gdiplus::StringAlignmentFar);
	sfLeftTop.SetLineAlignment(Gdiplus::StringAlignmentNear);
	Gdiplus::Font fontText(_T("Noto Sans CJK KR Regular"), 20, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	Gdiplus::SolidBrush brushText(Gdiplus::Color(0, 255, 255));

	//
	CRect rtClient;
	GetClientRect(&rtClient);

	CString text;

	text.Format(_T("%d / %d"), get<1>(m_scanInfo), get<2>(m_scanInfo));

	G.DrawString(text, -1, &fontText, Gdiplus::PointF(rtClient.Width() - 20, 15), &sfLeftTop, &brushText);
}

void ImageBScanPreview::drawAxialLength(CDC *pDC)
{
	if (!m_showAxialLength) {
		return;
	}

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	// axial length value
	Gdiplus::StringFormat sfLeftTop;

	sfLeftTop.SetAlignment(Gdiplus::StringAlignmentNear);
	sfLeftTop.SetLineAlignment(Gdiplus::StringAlignmentNear);
	Gdiplus::Font fontText(_T("Noto Sans CJK KR Regular"), 20, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	Gdiplus::SolidBrush brushText(Gdiplus::Color(0, 255, 255));

	if (!m_szAxialLengthNumMsg.IsEmpty()) {
		G.DrawString(m_szAxialLengthNumMsg, -1, &fontText, Gdiplus::PointF(8, 40), &sfLeftTop, &brushText);
	}
	if (!m_szAxialLengthMsg.IsEmpty()) {
		G.DrawString(m_szAxialLengthMsg, -1, &fontText, Gdiplus::PointF(8, 15), &sfLeftTop, &brushText);
	}
	else {
		CString text;
		if (m_axialLength < 0.0f) {
			text.Format(_T("Axial Length : Unknown"));
		}
		else if (m_axialLength < 5.0f) {
			text.Format(_T("Axial Length : Error"));
		}
		else {
			text.Format(_T("Axial Length : %.1f"), m_axialLength);
		}
		G.DrawString(text, -1, &fontText, Gdiplus::PointF(8, 15), &sfLeftTop, &brushText);
	}

	// guide line
	CRect rect;
	GetClientRect(&rect);
	Gdiplus::REAL dashGuideLine[] = { 5, 5 };
	// - screen center
	Gdiplus::Pen penGuideLine(Gdiplus::Color(255, 0, 0), 2);
	penGuideLine.SetDashPattern(dashGuideLine, 2);
	G.DrawLine(&penGuideLine, 0, rect.Height() / 2, rect.Width(), rect.Height() / 2);
	// - target center
	//   : 표시하지 않기로 함.
	/*
	if (m_sigCenter > 0) {
		Gdiplus::Pen penTarget(Gdiplus::Color(255, 255, 0), 2);
		penTarget.SetDashPattern(dashGuideLine, 2);
		G.DrawLine(&penTarget, 0, m_sigCenter, rect.Width(), m_sigCenter);
	}
	*/
}
