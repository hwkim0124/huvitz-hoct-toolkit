#include "stdafx.h"
#include "DrawScanPattern.h"

#define M_PI   3.1415926536

DrawScanPattern::DrawScanPattern()
	: m_nScanLine(-1)
{
}

DrawScanPattern::~DrawScanPattern()
{
}

//void DrawScanPattern::setScanPattern(OctScanPattern pattern)
//{
//	m_pattern = pattern;
//	return;
//}
//
//OctScanPattern DrawScanPattern::getScanPattern(void)
//{
//	return m_pattern;
//}

void DrawScanPattern::setScanLine(int index, int scanIndex)
{
	m_scanIndex[index] = scanIndex;
	return;
}


void DrawScanPattern::drawScanPattern(CDC *pDC, CRect rect, OctScanPattern pattern, float pixelPerMM)
{
	CPoint ptCenter = rect.CenterPoint();

	switch (pattern.getPatternName())
	{
	case OctGlobal::PatternName::MacularLine:
	case OctGlobal::PatternName::DiscLine:
	case OctGlobal::PatternName::AnteriorLine:
		drawPatternLine(pDC, ptCenter, pattern, pixelPerMM);
		break;
	case OctGlobal::PatternName::MacularCross:
		drawPatternCross(pDC, ptCenter, pattern, pixelPerMM);
		break;
	case OctGlobal::PatternName::MacularRadial:
	case OctGlobal::PatternName::DiscRadial:
	case OctGlobal::PatternName::AnteriorRadial:
		drawPatternRadial(pDC, ptCenter, pattern, pixelPerMM);
		break;
	case OctGlobal::PatternName::MacularRaster:
	case OctGlobal::PatternName::DiscRaster:
		drawPatternRaster(pDC, ptCenter, pattern, pixelPerMM);
		break;
	case OctGlobal::PatternName::Macular3D:
	case OctGlobal::PatternName::Disc3D:
	case OctGlobal::PatternName::Anterior3D:
		drawPatternRect(pDC, ptCenter, pattern, pixelPerMM);
		break;
	case OctGlobal::PatternName::DiscCircle:
		drawPatternCircle(pDC, ptCenter, pattern, pixelPerMM);
		break;
	default:
		break;
	}

	return;
}

void DrawScanPattern::drawPatternLine(CDC *pDC, CPoint ptCenter, OctScanPattern pattern, float pixelPerMM)
{
	if (pattern.getDirection() == 0)
	{
		int startX = ptCenter.x + LONG(pattern.getStartX() * pixelPerMM);
		int closeX = ptCenter.x + LONG(pattern.getCloseX() * pixelPerMM);
		int centerY = ptCenter.y + LONG(pattern.centerY() * pixelPerMM);
		drawLine(pDC, startX, centerY, closeX, centerY, false);
	}
	else
	{
		int startY = ptCenter.y + LONG(pattern.getStartY() * pixelPerMM);
		int closeY = ptCenter.y + LONG(pattern.getCloseY() * pixelPerMM);
		int centerX = ptCenter.x + LONG(pattern.centerX() * pixelPerMM);
		drawLine(pDC, centerX, startY, centerX, closeY, false);
	}

	return;
}

void DrawScanPattern::drawPatternCross(CDC *pDC, CPoint ptCenter, OctScanPattern pattern, float pixelPerMM)
{
	int startX = ptCenter.x + LONG(pattern.getStartX() * pixelPerMM);
	int startY = ptCenter.y + LONG(pattern.getStartY() * pixelPerMM);
	int closeX = ptCenter.x + LONG(pattern.getCloseX() * pixelPerMM);
	int closeY = ptCenter.y + LONG(pattern.getCloseY() * pixelPerMM);
	int centerX = ptCenter.x + LONG(pattern.centerX() * pixelPerMM);
	int centerY = ptCenter.y + LONG(pattern.centerY() * pixelPerMM);
	int spaceX = int(pattern.getScanLineSpace() * pixelPerMM);
	int spaceY = int(pattern.getScanLineSpace() * pixelPerMM);

	for (int i = -2; i <= 2; i++)
	{
		drawLine(pDC, startX, centerY + (i * spaceY), closeX, centerY + (i * spaceY), false);
		drawLine(pDC, centerX + (i * spaceX), startY, centerX + (i * spaceX), closeY, false);
	}

	return;
}

void DrawScanPattern::drawPatternRect(CDC *pDC, CPoint ptCenter, OctScanPattern pattern, float pixelPerMM)
{
	int startX = ptCenter.x + LONG(pattern.getStartX() * pixelPerMM);
	int startY = ptCenter.y + LONG(pattern.getStartY() * pixelPerMM);
	int rangeX = LONG(pattern.getScanRangeX() * pixelPerMM);
	int rangeY = LONG(pattern.getScanRangeY() * pixelPerMM);

	Gdiplus::Graphics G(pDC->m_hDC);
	Gdiplus::Pen pen(Gdiplus::Color(255, 87, 79), 2.0f);
	G.DrawRectangle(&pen, startX, startY, rangeX, rangeY);

	return;
}

void DrawScanPattern::drawPatternRadial(CDC *pDC, CPoint ptCenter, OctScanPattern pattern, float pixelPerMM)
{
	int centerX = ptCenter.x + LONG(pattern.centerX() * pixelPerMM);
	int centerY = ptCenter.y + LONG(pattern.centerY() * pixelPerMM);
	int radiusX = LONG(pattern.getScanRangeX() * pixelPerMM / 2);
	int radiusY = LONG(pattern.getScanRangeY() * pixelPerMM / 2);

	for (int i = 0; i < 12; i++)
	{
		int x1 = centerX - int(cos((i * 15) * M_PI / 180.0) * radiusX);
		int y1 = centerY - int(sin((i * 15) * M_PI / 180.0) * radiusY);
		int x2 = centerX + int(cos((i * 15) * M_PI / 180.0) * radiusX);
		int y2 = centerY + int(sin((i * 15) * M_PI / 180.0) * radiusY);
		drawLine(pDC, x1, y1, x2, y2, false);
	}

	return;
}

void DrawScanPattern::drawPatternRaster(CDC *pDC, CPoint ptCenter, OctScanPattern pattern, float pixelPerMM)
{
	int startX = ptCenter.x + LONG(pattern.getStartX() * pixelPerMM);
	int startY = ptCenter.y + LONG(pattern.getStartY() * pixelPerMM);
	int closeX = ptCenter.x + LONG(pattern.getCloseX() * pixelPerMM);
	int closeY = ptCenter.y + LONG(pattern.getCloseY() * pixelPerMM);
	int lines = pattern.getNumberOfScanLines();

	if (pattern.getDirection() == 0)
	{
		float spaceY = pattern.getScanRangeY() * pixelPerMM / float(lines - 1);

		for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
		{
			int posY = startY + int(i * spaceY);
			drawLine(pDC, startX, posY, closeX, posY, false);
		}
	}
	else
	{
		float spaceX = pattern.getScanRangeX() * pixelPerMM / float(lines - 1);

		for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
		{
			int posX = startX + int(i * spaceX);
			drawLine(pDC, posX, startY, posX, closeY, false);
		}
	}

	return;
}

void DrawScanPattern::drawPatternCircle(CDC *pDC, CPoint ptCenter, OctScanPattern pattern, float pixelPerMM)
{
	CRect rect;
	rect.left = ptCenter.x + LONG(pattern.getStartX() * pixelPerMM);
	rect.top = ptCenter.y + LONG(pattern.getStartY() * pixelPerMM);
	rect.right = ptCenter.x + LONG(pattern.getCloseX() * pixelPerMM);
	rect.bottom = ptCenter.y + LONG(pattern.getCloseY() * pixelPerMM);

	drawCircle(pDC, rect, false);

	return;
}


void DrawScanPattern::drawScanLine(CDC *pDC, CRect rect, OctScanPattern pattern, float pixelPerMM)
{
	if (m_nScanLine < 0)
	{
		return;
	}

	CPoint ptCenter = rect.CenterPoint();
	int startX = ptCenter.x + LONG(pattern.getStartX() * pixelPerMM);
	int startY = ptCenter.y + LONG(pattern.getStartY() * pixelPerMM);
	int closeX = ptCenter.x + LONG(pattern.getCloseX() * pixelPerMM);
	int closeY = ptCenter.y + LONG(pattern.getCloseY() * pixelPerMM);
	int centerX = ptCenter.x + LONG(pattern.centerX() * pixelPerMM);
	int centerY = ptCenter.y + LONG(pattern.centerY() * pixelPerMM);

	switch (pattern.getPatternName())
	{
	case OctGlobal::PatternName::MacularLine:
	case OctGlobal::PatternName::DiscLine:
	case OctGlobal::PatternName::AnteriorLine:
	{
		if (pattern.getDirection() == 0)
		{
			drawLine(pDC, startX, centerY, closeX, centerY, true);
		}
		else
		{
			drawLine(pDC, centerX, startY, centerX, closeY, true);
		}
	}
	break;

	case OctGlobal::PatternName::MacularCross:
	{
//		int spaceX = int(pattern.getScanLineSpace() * pixelPerMM);
//		int spaceY = int(pattern.getScanLineSpace() * pixelPerMM);
		int spaceX = int(0.25 * pixelPerMM);
		int spaceY = int(0.25 * pixelPerMM);

		if (m_nScanLine < 5)
		{
			int posY = centerY + ((m_nScanLine - 2) * spaceY);
			drawLine(pDC, startX, posY, closeX, posY, true);
		}
		else
		{
			int posX = centerX + ((m_nScanLine - 7) * spaceX);
			drawLine(pDC, posX, startY, posX, closeY, true);
		}
	}
	break;

	case OctGlobal::PatternName::MacularRadial:
	case OctGlobal::PatternName::DiscRadial:
	case OctGlobal::PatternName::AnteriorRadial:
	{
		int radiusX = LONG(pattern.getScanRangeX() * pixelPerMM / 2);
		int radiusY = LONG(pattern.getScanRangeY() * pixelPerMM / 2);
		int x1 = centerX - int(cos(m_nScanLine * 15 * M_PI / 180.0) * radiusX);
		int y1 = centerY - int(sin(m_nScanLine * 15 * M_PI / 180.0) * radiusY);
		int x2 = centerX + int(cos(m_nScanLine * 15 * M_PI / 180.0) * radiusX);
		int y2 = centerY + int(sin(m_nScanLine * 15 * M_PI / 180.0) * radiusY);
		drawLine(pDC, x1, y1, x2, y2, true);
	}
	break;

	case OctGlobal::PatternName::MacularRaster:
	case OctGlobal::PatternName::DiscRaster:
	case OctGlobal::PatternName::Macular3D:
	case OctGlobal::PatternName::Disc3D:
	case OctGlobal::PatternName::Anterior3D:
	{
		int lines = pattern.getNumberOfScanLines();

		if (pattern.getDirection() == 0)
		{
			float spaceY = pattern.getScanRangeY() * pixelPerMM / float(lines - 1);
			int posY = startY + int(m_nScanLine * spaceY);
			drawLine(pDC, startX, posY, closeX, posY, true);
		}
		else
		{
			float spaceX = pattern.getScanRangeX() * pixelPerMM / float(lines - 1);
			int posX = startX + int(m_nScanLine * spaceX);
			drawLine(pDC, posX, startY, posX, closeY, true);
		}
	}
	break;

	case OctGlobal::PatternName::DiscCircle:
	{
		drawCircle(pDC, CRect(startX, startY, closeX, closeY), true);
	}
	break;
	}

	return;
}

void DrawScanPattern::drawCircle(CDC *pDC, CRect rect, bool bArrow)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	if (bArrow)
	{
		Gdiplus::Pen pen(Gdiplus::Color(255, 246, 0), 2);
		G.DrawEllipse(&pen, rect.left, rect.top, rect.Width(), rect.Height());

		Gdiplus::GraphicsPath path;
		Gdiplus::Point points[4] = { Gdiplus::Point(3,-3), Gdiplus::Point(0, 0), Gdiplus::Point(-3,-3), Gdiplus::Point(3,-3) };

		path.AddPolygon(points, 4);
		pen.SetCustomEndCap(&Gdiplus::CustomLineCap(&path, nullptr));

		G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		G.DrawLine(&pen, rect.CenterPoint().x, rect.top, rect.CenterPoint().x + 1, rect.top);
	}
	else
	{
		Gdiplus::Pen pen(Gdiplus::Color(255, 87, 79), 2.0f);
		G.DrawEllipse(&pen, rect.left, rect.top, rect.Width(), rect.Height());
	}

	return;
}


void DrawScanPattern::drawLine(CDC *pDC, int x1, int y1, int x2, int y2, bool bArrow)
{
	if (!bArrow)
	{
		CPen pen;
		pen.CreatePen(PS_SOLID, 2, RGB(255, 87, 79));
		pDC->SelectObject(&pen);
		pDC->MoveTo(x1, y1);
		pDC->LineTo(x2, y2);
	}
	else
	{
		CPen penLine, penArrow;
		penLine.CreatePen(PS_SOLID, 2, RGB(255, 246, 0));
		penArrow.CreatePen(PS_SOLID, 1, RGB(255, 246, 0));
		CBrush brhArrow;
		brhArrow.CreateSolidBrush(RGB(255, 246, 0));

		pDC->SelectObject(&penLine);
		pDC->MoveTo(x1, y1);
		pDC->LineTo(x2, y2);

		float rx = ((x1 - x2) * 10) / sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
		float ry = ((y1 - y2) * 10) / sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));

		CPoint ptArrow[3];
		ptArrow[0].x = x2 + rx - (ry / 2.0f);
		ptArrow[0].y = y2 + ry + (rx / 2.0f);
		ptArrow[1].x = x2 + rx + (ry / 2.0f);
		ptArrow[1].y = y2 + ry - (rx / 2.0f);
		ptArrow[2].x = x2;
		ptArrow[2].y = y2;

		pDC->SelectObject(&penArrow);
		pDC->SelectObject(&brhArrow);
		pDC->SetPolyFillMode(ALTERNATE);
		pDC->Polygon(ptArrow, 3);
	}

	return;

//	Gdiplus::Graphics G(pDC->m_hDC);
//	Gdiplus::Pen pen(Gdiplus::Color(255, 87, 79), 2.0f);
//	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
//
//	if (bArrow)
//	{
//		Gdiplus::GraphicsPath path;
//		Gdiplus::Point points[4] = { Gdiplus::Point(3,-3), Gdiplus::Point(0, 0), Gdiplus::Point(-3,-3), Gdiplus::Point(3,-3) };
//
//		pen.SetColor(Gdiplus::Color(255, 246, 0));
//		path.AddPolygon(points, 4);
//		pen.SetCustomEndCap(&Gdiplus::CustomLineCap(&path, nullptr));
//	}
//
//	G.DrawLine(&pen, x1, y1, x2, y2);
//
//	return;
}