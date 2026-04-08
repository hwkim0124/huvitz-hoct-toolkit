#include "stdafx.h"
#include "DrawCorneaEtdrsChart.h"

using namespace Gdiplus;

DrawCorneaEtdrsChart::DrawCorneaEtdrsChart()
{
	m_direction[0] = _T("S");
	m_direction[1] = _T("N");
	m_direction[2] = _T("I");
	m_direction[3] = _T("T");
}

DrawCorneaEtdrsChart::~DrawCorneaEtdrsChart()
{
}

void DrawCorneaEtdrsChart::initialize(EyeSide side)
{
	m_eyeSide = side;

	return;
}

void DrawCorneaEtdrsChart::clearData(void)
{
	m_corneaEtdrsThicknessCenter = 0.0f;

	for (int side = 0; side < CORNEA_THICK_SIDE_NUM; side++) {
		for (int dir = 0; dir < CORNEA_THICK_DIR_NUM; dir++) {
			m_corneaEtdrsThickness[side][dir] = 0.0f;
		}
	}

	return;
}

void DrawCorneaEtdrsChart::setCorneaEtdrsThicknessCenter(float value)
{
	m_corneaEtdrsThicknessCenter = value;
}

void DrawCorneaEtdrsChart::setCorneaEtdrsThicknessInner(CorneaThickDir dir, float value)
{
	m_corneaEtdrsThickness[CORNEA_THICK_INNER][dir] = value;
}

void DrawCorneaEtdrsChart::setCorneaEtdrsThicknessOuter(CorneaThickDir dir, float value)
{
	m_corneaEtdrsThickness[CORNEA_THICK_OUTER][dir] = value;
}

void DrawCorneaEtdrsChart::drawCorneaEtdrsChart(CDC *pDC, CRect rect)
{
	int centerX = rect.left + rect.Width() / 2;
	int centerY = rect.top + rect.Height() / 2;
	int radius = rect.Width() / 2;

	int radiusCenter, radiusInner, radiusOuter;
	radiusCenter = int((float)radius * 2.0f / 6.0f);
	radiusInner = int((float)radius * 5.0f / 6.0f);
	radiusOuter = radius;

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::StringFormat SF;
	SF.SetAlignment(StringAlignmentCenter);
	SF.SetLineAlignment(StringAlignmentCenter);

	Gdiplus::RectF rcOuter(centerX - radiusOuter, centerY - radiusOuter, radiusOuter * 2, radiusOuter * 2);
	Gdiplus::RectF rcInner(centerX - radiusInner, centerY - radiusInner, radiusInner * 2, radiusInner * 2);
	Gdiplus::RectF rcCenter(centerX - radiusCenter, centerY - radiusCenter, radiusCenter * 2, radiusCenter * 2);

	Gdiplus::SolidBrush brushValue(Color::Black);
	Gdiplus::SolidBrush brushTsnit(Color::Black);
	Gdiplus::Pen pen(Color::Black, 2.0f);

	// 경계선 : 22.5도 부터
	for (int dir = 0; dir < CORNEA_THICK_DIR_NUM; dir++)
	{
		float angle = (m_eyeSide == EyeSide::OS) ? (22.5f + ((float)dir * 45.0f)) : (22.5 - ((float)dir * 45.0f));
		float radian = M_PI * angle / 180;

		int x1 = centerX + int(radiusCenter * cos(radian));
		int y1 = centerY - int(radiusCenter * sin(radian));
		int x2 = centerX + int(radiusOuter * cos(radian));
		int y2 = centerY - int(radiusOuter * sin(radian));
		G.DrawLine(&pen, x1, y1, x2, y2);
	}

	G.DrawEllipse(&pen, rcCenter);
	G.DrawEllipse(&pen, rcInner);
	G.DrawEllipse(&pen, rcOuter);


	// 두께값
	CString text = _T("");
	Gdiplus::PointF ptText;

	ptText.X = centerX;
	ptText.Y = centerY;
	text.Format(_T("%d"), (int)m_corneaEtdrsThicknessCenter);

	Gdiplus::Font font(_T("Calibri"), 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	G.DrawString(text, -1, &font, ptText, &SF, &brushValue);

	for (int dir = 0; dir < CORNEA_THICK_DIR_NUM; dir++)
	{
		float angle = (m_eyeSide == EyeSide::OS) ? (45 + (dir * 45)) : (45 - (dir * 45));
		float radian = M_PI * angle / 180;
		float radius1 = (radiusInner + radiusCenter) / 2;
		float radius2 = (radiusOuter + radiusInner) / 2;

		// inner text
		ptText.X = centerX + int(radius1 * cos(radian));
		ptText.Y = centerY - int(radius1 * sin(radian));
		text.Format(_T("%d"), (int)m_corneaEtdrsThickness[CORNEA_THICK_INNER][dir]);
		G.DrawString(text, -1, &font, ptText, &SF, &brushValue);

		// outer
		ptText.X = centerX + int(radius2 * cos(radian));
		ptText.Y = centerY - int(radius2 * sin(radian));
		text.Format(_T("%d"), (int)m_corneaEtdrsThickness[CORNEA_THICK_OUTER][dir]);
		G.DrawString(text, -1, &font, ptText, &SF, &brushValue);
	}

	// TSNI : 90도부터
	for (int i = 0; i < 4; i++)
	{
		float angle = (m_eyeSide == EyeSide::OS) ? (90 + (i * 90)) : (90 - (i * 90));
		float radian = M_PI * angle / 180;
		float radius = radiusOuter + 10;

		ptText.X = centerX + int(radius * cos(radian));
		ptText.Y = centerY - int(radius * sin(radian));
		G.DrawString(m_direction[i], -1, &font, ptText, &SF, &brushTsnit);
	}

	return;
}
