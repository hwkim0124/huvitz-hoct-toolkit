#include "stdafx.h"
#include "DrawRadiusMap.h"

using namespace ReportCtrl;
using namespace Gdiplus;

DrawRadiusMap::DrawRadiusMap()
	: m_transparency(1.0f)
{
	makeColorTable();
}


DrawRadiusMap::~DrawRadiusMap()
{
}

void DrawRadiusMap::makeColorTable(void)
{
//	makeColorTable(m_colorTable, 0, kMinRadius, RGB(0, 0, 0), RGB(0, 0, 0));
	makeColorTable(m_colorTable, kMinRadius, kMaxRadius, RGB(255, 0, 0), RGB(0, 255, 0));
}

void DrawRadiusMap::makeColorTable(DWORD *table, unsigned long minIndex, unsigned long maxIndex, COLORREF minColor, COLORREF maxColor)
{
	unsigned long step_count = maxIndex - minIndex;
	BYTE minR, minG, minB;
	BYTE maxR, maxG, maxB;
	BYTE resR, resG, resB;

	maxR = GetRValue(maxColor);
	maxG = GetGValue(maxColor);
	maxB = GetBValue(maxColor);
	minR = GetRValue(minColor);
	minG = GetGValue(minColor);
	minB = GetBValue(minColor);

	for (unsigned long i = 0; i < step_count; i++)
	{
		resR = minR + BYTE((float)(maxR - minR) / (float)(step_count)* i);
		resG = minG + BYTE((float)(maxG - minG) / (float)(step_count)* i);
		resB = minB + BYTE((float)(maxB - minB) / (float)(step_count)* i);

		table[minIndex + i] = (DWORD)(resR << 16) | (DWORD)(resG << 8) | (DWORD)(resB);
	}
}

void DrawRadiusMap::initialize(void)
{
	m_bmpRadius.DeleteObject();

	return;
}

void DrawRadiusMap::setRadiusMapData(std::vector <float> data, int width, int height)
{
	auto temp = std::make_unique<DWORD[]>(width * height);
	unsigned short value;

	for (size_t i = 0; i < data.size(); i++)
	{
		value = (unsigned short)(data[i] * 100);
		if (value < kMinRadius)
		{
			temp[i] = 0x000000;		// black
		}
		else if (value > kMaxRadius)
		{
			temp[i] = 0xFFFFFF;		// white
		}
		else
		{
			temp[i] = m_colorTable[value];
		}
	}

	m_bmpRadius.DeleteObject();
	m_bmpRadius.CreateBitmap(width, height, 1, 32, temp.get());


	return;
}

void DrawRadiusMap::setTransparency(float transparency)
{
	m_transparency = transparency;

	return;
}

void DrawRadiusMap::drawRadiusMap(CDC * pDC, CRect rect)
{
	if (!m_bmpRadius.m_hObject)
	{
		return;
	}

	ColorMatrix colorMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, m_transparency, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	ImageAttributes imageAtt;
	imageAtt.SetColorMatrix(&colorMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);

	Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	Rect rtDraw(rect.left, rect.top, rect.Width(), rect.Height());

	Gdiplus::Bitmap bitmap((HBITMAP)m_bmpRadius, NULL);
	G.DrawImage(&bitmap, rtDraw, 0, 0, bitmap.GetWidth(), bitmap.GetHeight(), UnitPixel, &imageAtt);

	//	CDC memDC;
	//	memDC.CreateCompatibleDC(pDC);
	//	memDC.SelectObject(&m_bmpThick);
	//
	//	BITMAP bmpInfo;
	//	m_bmpThick.GetBitmap(&bmpInfo);
	//
	//	BLENDFUNCTION bf;
	//	bf.AlphaFormat = 0;
	//	bf.BlendFlags = 0;
	//	bf.BlendOp = AC_SRC_OVER;
	//	bf.SourceConstantAlpha = m_constantAlpha;
	//
	//	pDC->AlphaBlend(rect.left, rect.top, rect.Width(), rect.Height(), &memDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, bf);

	//	memDC.DeleteDC();

	return;
}


void DrawRadiusMap::drawColorTable(CDC *pDC, CRect rect, Gdiplus::Color clrText)
{
	// draw color table
	int height = kMaxRadius - kMinRadius;

	CBitmap bitmap;
	bitmap.CreateBitmap(1, height, 1, 32, &m_colorTable[kMinRadius]);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(&bitmap);

	pDC->SetStretchBltMode(HALFTONE);
	pDC->StretchBlt(rect.left, rect.top, 15, rect.Height(), &memDC,
		0, height - 1, 1, -height, SRCCOPY);

	memDC.DeleteDC();
	bitmap.DeleteObject();


	// draw thickness text
	int px = rect.left + 20;
	int py = 0;

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	StringFormat SF;
	SF.SetAlignment(StringAlignmentNear);
	SF.SetLineAlignment(StringAlignmentNear);
	Font font(_T("Calibri"), 14, FontStyleRegular, UnitPixel);

	Gdiplus::SolidBrush brushText(clrText);

	py = rect.top + ((rect.Height() / 5) * 0);
	G.DrawString(_T("9.75"), -1, &font, Gdiplus::PointF(px, py), &brushText);
	py = rect.top + ((rect.Height() / 5) * 1) - kTextHeight / 2;
	G.DrawString(_T("8.75"), -1, &font, Gdiplus::PointF(px, py), &brushText);
	py = rect.top + ((rect.Height() / 5) * 2) - kTextHeight / 2;
	G.DrawString(_T("7.75"), -1, &font, Gdiplus::PointF(px, py), &brushText);
	py = rect.top + ((rect.Height() / 5) * 3) - kTextHeight / 2;
	G.DrawString(_T("6.75"), -1, &font, Gdiplus::PointF(px, py), &brushText);
	py = rect.top + ((rect.Height() / 5) * 4) - kTextHeight / 2;
	G.DrawString(_T("5.75"), -1, &font, Gdiplus::PointF(px, py), &brushText);
	py = rect.top + ((rect.Height() / 5) * 5) - kTextHeight;
	G.DrawString(_T("4.75"), -1, &font, Gdiplus::PointF(px, py), &brushText);

	return;
}