#include "stdafx.h"
#include "DrawDeviationMap.h"
#include "atlimage.h"

using namespace ReportCtrl;
using namespace Gdiplus;

DrawDeviationMap::DrawDeviationMap()
	: m_mapType(DEV_MAP_NORMATIVE)
{
	m_sfLeftTop.SetAlignment(StringAlignmentNear);
	m_sfLeftTop.SetLineAlignment(StringAlignmentNear);
	m_sfLeftCenter.SetAlignment(StringAlignmentNear);
	m_sfLeftCenter.SetLineAlignment(StringAlignmentCenter);
	m_sfLeftBottom.SetAlignment(StringAlignmentNear);
	m_sfLeftBottom.SetLineAlignment(StringAlignmentFar);

	makeColorTable(m_colorTableDevMap, 0, kMaxPercentile, L"LUT\\colorscale_deviation_map.bmp");
}

DrawDeviationMap::~DrawDeviationMap()
{
}

void DrawDeviationMap::initialize(void)
{
}

void DrawDeviationMap::setDeviationMapData(std::vector<float> data,
	std::pair<int, int> mapSize, std::pair<int, int> dataSize,
	std::pair<int, int> center, float mapRangeRatio)
{
	clearDeviationMapData();

	m_mapSize = mapSize;
	m_dataSize = dataSize;
	m_center = center;
	m_data.clear();
	m_data = data;
	m_mapRangeRatio = mapRangeRatio;
}

void DrawDeviationMap::setDeviationMapType(DevMapType type)
{
	m_mapType = type;
}

void DrawDeviationMap::clearDeviationMapData()
{
	m_mapSize = make_pair(0, 0);
	m_dataSize = make_pair(0, 0);
	m_center = make_pair(0, 0);
	m_data.clear();
}

void DrawDeviationMap::drawDeviationMap(HDC hDC, CRect rect, PatternName patternName,
	std::pair<OcularLayerType, OcularLayerType> layers, float wResizeRatio)
{
	if (m_data.size() == 0 || m_mapSize.first <= 0 || m_mapSize.second <= 0) {
		return;
	}

	float zoomRateCpX = (float)rect.Width() / (float)m_mapSize.first;
	float zoomRateCpY = (float)rect.Height() / (float)m_mapSize.second;
	float zoomRateMap = (float)rect.Height() / (float)m_mapSize.second;
	float zoomRateDataSize = zoomRateMap * m_mapRangeRatio;

	int cx = static_cast<int>((float)m_center.first * zoomRateCpX + (float)rect.Width() / 2.0f);
	int cy = static_cast<int>((float)m_center.second * zoomRateCpY + (float)rect.Height() / 2.0f);
	int mapSizeX = static_cast<int>((float)m_mapSize.first * zoomRateDataSize); // map 이 그려지는 영역 실제 크기 w
	int mapSizeY = static_cast<int>((float)m_mapSize.second * zoomRateDataSize); // map 이 그려지는 영역 실제 크기 h
	int dataSizeX = static_cast<int>((float)m_dataSize.first * zoomRateDataSize); // data 가 그려지는 영역 실제 크기 w
	int dataSizeY = static_cast<int>((float)m_dataSize.second * zoomRateDataSize); // data 가 그려지는 영역 실제 크기 h

	// make map image
	Gdiplus::Bitmap mapBmp(dataSizeX, dataSizeY, PixelFormat32bppARGB);
	for (int y = 0; y < dataSizeY; y++) {
		for (int x = 0; x < dataSizeX; x++) {
			// get data index
			int x_in_data = static_cast<int>((float)x / zoomRateDataSize);
			int y_in_data = static_cast<int>((float)y / zoomRateDataSize);
			int dataIdx = y_in_data * m_dataSize.first + x_in_data;
			// get data
			int perValue = (int)m_data[dataIdx];
			// set pixel with this color
			// - normative map
			if (m_mapType == DEV_MAP_NORMATIVE) {
				if (perValue > 5 && perValue <= 95) {
					mapBmp.SetPixel(x, y, Color(0, 0, 0, 0));
				}
				else {
					auto color = getPercentileColor_Nor(patternName, layers, perValue);
					auto r = GetRValue(color);
					auto g = GetGValue(color);
					auto b = GetBValue(color);
					mapBmp.SetPixel(x, y, Color(70, r, g, b));
				}
			}
			// - deviation map
			else {
				auto color = getPercentileColor_Dev(perValue);
				auto r = GetRValue(color);
				auto g = GetGValue(color);
				auto b = GetBValue(color);
				mapBmp.SetPixel(x, y, Color(70, r, g, b));
			}
		}
	}

	Gdiplus::Graphics G(hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	int mapSizeResized = static_cast<int>((float)mapSizeX * wResizeRatio);

	int sx = max(cx - mapSizeResized / 2, 0);
	int sy = max(cy - mapSizeY / 2, 0);
	int ex = min(cx + mapSizeResized / 2, rect.Width());
	int ey = min(cy + mapSizeY / 2, rect.Height());

	Rect rtDraw(rect.left + sx, rect.top + sy, ex - sx, ey - sy);

	G.DrawImage(&mapBmp, rtDraw);
}

void DrawDeviationMap::drawColorTableDevMap(CDC *pDC, CRect rect, Gdiplus::Color clrText,
	PatternName patternName, std::pair<OcularLayerType, OcularLayerType> layers)
{
	if (m_mapType == DEV_MAP_NORMATIVE) {
		drawColorTableDevMap_Nor(pDC, rect, clrText, patternName, layers);
	}
	else if (m_mapType == DEV_MAP_DEVIATION) {
		drawColorTableDevMap_Dev(pDC, rect, clrText);
	}
	else {
		// do nothing
	}
}

void DrawDeviationMap::drawDeviationMapLabel(CDC *pDC, Gdiplus::RectF rect, Gdiplus::Font *font,
	Gdiplus::Color color, bool left)
{
	CString text;

	if (m_mapType == DEV_MAP_NORMATIVE) {
		text = _T("Normative Map");
	}
	else if (m_mapType == DEV_MAP_DEVIATION) {
		text = _T("Deviation Map");
	}
	else {
		text = _T("Normative Map");
	}

	if (rect.IsEmptyArea()) {
		return;
	}

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetTextRenderingHint(Gdiplus::TextRenderingHintSystemDefault);

	StringFormat SF;
	SF.SetAlignment(left ? Gdiplus::StringAlignmentNear : Gdiplus::StringAlignmentFar);
	SF.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	Gdiplus::SolidBrush brush(color);
	G.DrawString(text, -1, font, rect, &SF, &brush);
}

COLORREF DrawDeviationMap::getPercentileColor_Dev(int percent)
{
	int index = max(percent - 1, 0); // percent 의 범위는 1 ~ 100

	BYTE b = GetRValue(m_colorTableDevMap[index]);
	BYTE g = GetGValue(m_colorTableDevMap[index]);
	BYTE r = GetBValue(m_colorTableDevMap[index]);

	return RGB(r, g, b);
}

COLORREF DrawDeviationMap::getPercentileColor_Nor(PatternName patternName,
	std::pair<OcularLayerType, OcularLayerType> layers, int percent)
{
	if (patternName == PatternName::Macular3D) {
		return getPercentileColorMacular(percent);
	}
	else if (patternName == PatternName::Disc3D) {
		return getPercentileColorDisc(percent);
	}
	else if (patternName == PatternName::MacularDisc) {
		if (layers.first == OcularLayerType::NFL) {
			return getPercentileColorDisc(percent);
		}
		else {
			return getPercentileColorMacular(percent);
		}
	}
	else {
		return RGB(255, 255, 255);
	}
}

COLORREF DrawDeviationMap::getPercentileColorMacular(int percent)
{
	if (percent > 99) {
		return RGB(247, 205, 205);
	}
	else if (percent <= 99 && percent > 95) {
		return RGB(248, 247, 207);
	}
	else if (percent <= 95 && percent > 5) {
		return RGB(84, 253, 94);
	}
	else if (percent <= 5 && percent > 1) {
		return RGB(255, 252, 90);
	}
	else if (percent <= 1) {
		return RGB(253, 69, 73);
	}
	else {
		return RGB(128, 128, 128);
	}
}

COLORREF DrawDeviationMap::getPercentileColorDisc(int percent)
{
	if (percent > 95) {
		return RGB(253, 255, 254);
	}
	else if (percent <= 95 && percent > 5) {
		return RGB(140, 199, 98);
	}
	else if (percent <= 5 && percent > 1) {
		return RGB(249, 239, 104);
	}
	else if (percent <= 1) {
		return RGB(242, 106, 112);
	}
	else {
		return RGB(0, 0, 0);
	}
}

void DrawDeviationMap::makeColorTable(DWORD *table, unsigned long minIndex, unsigned long maxIndex,
	LPCTSTR lutFileName)
{
	// load LUT
	CImage lutImage;
	if (FAILED(lutImage.Load(lutFileName))) {
		return;
	}

	// make table
	const unsigned long kColorCount = static_cast<unsigned long>(lutImage.GetWidth());
	const unsigned long kStepCount = maxIndex - minIndex;
	for (unsigned long i = 0; i < kStepCount; i++) {
		int x = (int)((float)i * ((float)kColorCount / (float)kStepCount));
		COLORREF step_color = lutImage.GetPixel(x, 0);

		BYTE r = GetRValue(step_color);
		BYTE g = GetGValue(step_color);
		BYTE b = GetBValue(step_color);

		table[minIndex + i] = (DWORD)(r << 16) | (DWORD)(g << 8) | (DWORD)(b);
	}
}

void DrawDeviationMap::drawColorTableFor(DWORD* table, int tableSize, CDC *pDC, Gdiplus::Rect rect)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	Gdiplus::Bitmap bmpTable(rect.Width, tableSize);
	for (int i = 0; i < tableSize; i++) {
		int index = tableSize - 1 - i;

		// bgr format
		BYTE b = GetRValue(table[index]);
		BYTE g = GetGValue(table[index]);
		BYTE r = GetBValue(table[index]);

		for (int x = 0; x < rect.Width; x++) {
			Gdiplus::Color color(r, g, b);
			bmpTable.SetPixel(x, i, color);
		}
	}

	G.DrawImage(&bmpTable, rect);
}

void DrawDeviationMap::drawColorTableDevMap_Nor(CDC *pDC, CRect rect_in, Gdiplus::Color clrText,
	PatternName patternName, std::pair<OcularLayerType, OcularLayerType> layers)
{
	if (patternName == PatternName::Macular3D) {
		drawColorTableDevMap_Nor_Macular(pDC, rect_in, clrText);
	}
	else if (patternName == PatternName::Disc3D) {
		drawColorTableDevMap_Nor_Disc(pDC, rect_in, clrText);
	}
	else if (patternName == PatternName::MacularDisc) {
		if (get<1>(layers) == OcularLayerType::NFL) {
			drawColorTableDevMap_Nor_Disc(pDC, rect_in, clrText);
		}
		else {
			drawColorTableDevMap_Nor_Macular(pDC, rect_in, clrText);
		}
	}
	else {
		// do nothing
	}
}

void DrawDeviationMap::drawColorTableDevMap_Nor_Macular(CDC *pDC, CRect rect, Gdiplus::Color clrText)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	Gdiplus::SolidBrush brushRect(Gdiplus::Color::White);
	Gdiplus::Pen pen(Gdiplus::Color::Black);
	Gdiplus::Font font(_T("Calibri"), 14, FontStyleRegular, UnitPixel);

	INT x, y, w, h;
	x = rect.left;
	y = rect.top;
	w = 15;
	h = (rect.Height() / 5);

	Gdiplus::Color brushColor;

	brushColor.SetFromCOLORREF(getPercentileColorMacular(100));
	brushRect.SetColor(brushColor);
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);

	y += h;
	brushColor.SetFromCOLORREF(getPercentileColorMacular(99));
	brushRect.SetColor(brushColor);
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);

	y += h;
	brushColor.SetFromCOLORREF(getPercentileColorMacular(95));
	brushRect.SetColor(brushColor);
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);

	y += h;
	brushColor.SetFromCOLORREF(getPercentileColorMacular(5));
	brushRect.SetColor(brushColor);
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);

	y += h;
	brushColor.SetFromCOLORREF(getPercentileColorMacular(1));
	brushRect.SetColor(brushColor);
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);

	// draw thickness text
	auto px = static_cast<Gdiplus::REAL>(rect.left + 20);
	auto py = 0.0f;

	Gdiplus::SolidBrush brushText(clrText);

	py = (float)rect.top;
	G.DrawString(_T("100%"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftTop, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 5.0f) * 1.0f);
	G.DrawString(_T("99%"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 5.0f) * 2.0f);
	G.DrawString(_T("95%"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 5.0f) * 3.0f);
	G.DrawString(_T("5%"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 5.0f) * 4.0f);
	G.DrawString(_T("1%"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 5.0f) * 5.0f);
	G.DrawString(_T("0%"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftBottom, &brushText);
}

void DrawDeviationMap::drawColorTableDevMap_Nor_Disc(CDC *pDC, CRect rect, Gdiplus::Color clrText)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	Gdiplus::SolidBrush brushRect(Gdiplus::Color::White);
	Gdiplus::Pen pen(Gdiplus::Color::Black);
	Gdiplus::Font font(_T("Calibri"), 14, FontStyleRegular, UnitPixel);

	INT x, y, w, h;
	x = rect.left;
	y = rect.top;
	w = 15;
	h = (rect.Height() / 4);

	Gdiplus::Color brushColor;

	brushColor.SetFromCOLORREF(getPercentileColorDisc(100));
	brushRect.SetColor(brushColor);
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);

	y += h;
	brushColor.SetFromCOLORREF(getPercentileColorDisc(95));
	brushRect.SetColor(brushColor);
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);

	y += h;
	brushColor.SetFromCOLORREF(getPercentileColorDisc(5));
	brushRect.SetColor(brushColor);
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);

	y += h;
	brushColor.SetFromCOLORREF(getPercentileColorDisc(1));
	brushRect.SetColor(brushColor);
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);

	// draw thickness text
	auto px = static_cast<Gdiplus::REAL>(rect.left + 20);
	auto py = 0.0f;

	Gdiplus::SolidBrush brushText(clrText);

	py = (float)rect.top;
	G.DrawString(_T("100%"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftTop, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 4.0f) * 1.0f);
	G.DrawString(_T("95%"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 4.0f) * 2.0f);
	G.DrawString(_T("5%"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 4.0f) * 3.0f);
	G.DrawString(_T("1%"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 4.0f) * 4.0f);
	G.DrawString(_T("0%"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
}

void DrawDeviationMap::drawColorTableDevMap_Dev(CDC *pDC, CRect rect, Gdiplus::Color clrText)
{
	// draw color table
	drawColorTableFor(m_colorTableDevMap, kMaxPercentile, pDC,
		Rect(rect.left, rect.top, 15, rect.Height()));

	// draw thickness text
	auto px = static_cast<Gdiplus::REAL>(rect.left + 20);
	auto py = 0.0f;

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	Font font(_T("Calibri"), 14, FontStyleRegular, UnitPixel);

	Gdiplus::SolidBrush brushText(clrText);

	py = (float)rect.top;
	G.DrawString(_T("+50"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftTop, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 4.0f) * 1.0f);
	G.DrawString(_T("+25"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 4.0f) * 2.0f);
	G.DrawString(_T("0"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 4.0f) * 3.0f);
	G.DrawString(_T("-25"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = (float)rect.top + (((float)rect.Height() / 4.0f) * 4.0f);
	G.DrawString(_T("-50"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
}
