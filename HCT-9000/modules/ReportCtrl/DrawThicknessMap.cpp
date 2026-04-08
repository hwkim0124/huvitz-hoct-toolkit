#include "stdafx.h"
#include "DrawThicknessMap.h"
#include "atlimage.h"


using namespace ReportCtrl;
using namespace Gdiplus;

DrawThicknessMap::DrawThicknessMap()
	: m_transparency(1.0f)
	, m_layerUpper(OcularLayerType::NONE)
	, m_layerLower(OcularLayerType::NONE)
	, m_mapWidth(0)
	, m_mapHeight(0)
{
	makeColorTable();

	m_sfLeftTop.SetAlignment(StringAlignmentNear);
	m_sfLeftTop.SetLineAlignment(StringAlignmentNear);
	m_sfLeftCenter.SetAlignment(StringAlignmentNear);
	m_sfLeftCenter.SetLineAlignment(StringAlignmentCenter);
	m_sfLeftBottom.SetAlignment(StringAlignmentNear);
	m_sfLeftBottom.SetLineAlignment(StringAlignmentFar);
}


DrawThicknessMap::~DrawThicknessMap()
{
}


void DrawThicknessMap::makeColorTable(void)
{
	// ILM-RPE : 0 ~ 500
	makeColorTable(m_colorTableILMRPE, 0, 500, L"LUT\\colorscale_thickness_ilm.bmp");

	// ILM-IPL : 0 ~ 250
	makeColorTable(m_colorTableILMIPL, 0, 250, L"LUT\\colorscale_thickness.bmp");

	// ONH : 0 ~ 350
	//	makeColorTable(m_colorTableONH, 0, 350, L"LUT\\colorscale_bscan_rgb.bmp");

	// ILM-GCL : 0 ~ 200
	makeColorTable(m_colorTableILMGCL, 0, 200, L"LUT\\colorscale_thickness.bmp");

	// EPI-END : 300 ~ 800
	makeColorTable(m_colorTableEPIEND, 300, 800, L"LUT\\colorscale_thickness.bmp");

	// EPI : 20 ~ 80
	makeColorTable(m_colorTableEPIBOW, 200, 800, L"LUT\\colorscale_thickness.bmp");

	return;
}

void DrawThicknessMap::makeColorTable(DWORD *table, unsigned long minIndex, unsigned long maxIndex, LPCTSTR lutFileName)
{
	// load LUT
	CImage lutImage;
	if (FAILED(lutImage.Load(lutFileName))) {
		return;
	}

	// make table
	const int kColorCount = lutImage.GetWidth();
	const int kStepCount = maxIndex - minIndex;
	for (unsigned long i = 0; i < kStepCount; i++) {
		int x = (int)((float)i * ((float)kColorCount / (float)kStepCount));
		COLORREF step_color = lutImage.GetPixel(x, 0);

		BYTE r = GetRValue(step_color);
		BYTE g = GetGValue(step_color);
		BYTE b = GetBValue(step_color);

		table[minIndex + i] = (DWORD)(r << 16) | (DWORD)(g << 8) | (DWORD)(b);
	}
}

void DrawThicknessMap::initialize(void)
{
	m_bmpThick.DeleteObject();

	return;
}

void DrawThicknessMap::setThicknessLayer(OcularLayerType upper, OcularLayerType lower)
{
	m_layerUpper = upper;
	m_layerLower = lower;

	return;
}

void DrawThicknessMap::setThickMapData(std::vector <float> data, int width, int height)
{
	m_colorPixels.clear();
	m_mapWidth = 0;
	m_mapHeight = 0;

	auto temp = std::make_unique<DWORD[]>(width * height);
	unsigned short index;
	unsigned short maxThickness;
	DWORD* colorTable;

	if (m_layerUpper == OcularLayerType::ILM && m_layerLower == OcularLayerType::NFL) {
		maxThickness = kMaxThicknessILMGCL;
		colorTable = m_colorTableILMGCL;
	}
	else if (m_layerUpper == OcularLayerType::ILM && m_layerLower == OcularLayerType::IPL) {
		maxThickness = kMaxThicknessILMIPL;
		colorTable = m_colorTableILMIPL;
	}
	else if (m_layerUpper == OcularLayerType::ILM && m_layerLower == OcularLayerType::RPE) {
		maxThickness = kMaxThicknessILMRPE;
		colorTable = m_colorTableILMRPE;
	}
	else if (m_layerUpper == OcularLayerType::EPI && m_layerLower == OcularLayerType::END) {
		maxThickness = kMaxThicknessEPIEND;
		colorTable = m_colorTableEPIEND;
	}
	else if (m_layerUpper == OcularLayerType::EPI && m_layerLower == OcularLayerType::BOW) {
		maxThickness = kMaxThicknessEPIBOW;
		colorTable = m_colorTableEPIBOW;
	}
	else {
		return;
	}

	for (size_t i = 0; i < data.size(); i++)
	{
		if (data[i] < 0)
		{
			//	temp[i] = 0x7f7f7f;
		}
		else
		{
			if (m_layerUpper == OcularLayerType::EPI && m_layerLower == OcularLayerType::BOW)
			{
				index = (unsigned short)(data[i] * 10);
			}
			else
			{
				index = (unsigned short)data[i];
			}

			if (index >= maxThickness)
			{
				temp[i] = 0xFFFFFF;		// white
			}
			else
			{
				temp[i] = colorTable[index];
			}
		}
	}

	m_colorPixels.assign(temp.get(), temp.get() + (width * height));
	m_mapWidth = width;
	m_mapHeight = height;

	m_bmpThick.DeleteObject();
	m_bmpThick.CreateBitmap(width, height, 1, 32, m_colorPixels.data());

	// get center position
	float lowest = maxThickness;
	m_pointCenter = { -1, -1 };
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			auto value = data.at(y * width + x);
			if (value > 0.0f && value < lowest) {
				lowest = value;
				m_pointCenter = { x, y };
			}
		}
	}

	return;
}

void DrawThicknessMap::setDiscPoint(std::vector <Gdiplus::PointF> points)
{
	m_pointDisc = points;

	return;
}

void DrawThicknessMap::setCupPoint(std::vector <Gdiplus::PointF> points)
{
	m_pointCup = points;

	return;
}


void DrawThicknessMap::setTransparency(float transparency)
{
	m_transparency = transparency;

	return;
}

void DrawThicknessMap::drawThicknessMap(CDC * pDC, CRect rect)
{
	if (!m_bmpThick.m_hObject)
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

	Gdiplus::Bitmap bitmap((HBITMAP)m_bmpThick, NULL);
	G.DrawImage(&bitmap, rtDraw, 0, 0, bitmap.GetWidth(), bitmap.GetHeight(), UnitPixel, &imageAtt);

	// disc
	vector <Gdiplus::PointF> pointDisc;

	for (size_t i = 0; i < m_pointDisc.size(); i++)
	{
		Gdiplus::PointF point;
		point.X = rect.left + m_pointDisc[i].X / (float)bitmap.GetWidth() * rect.Width();
		point.Y = rect.top + m_pointDisc[i].Y / (float)bitmap.GetHeight() * rect.Height();
		pointDisc.push_back(point);
	}

	Gdiplus::SolidBrush brush1(Color(128, 128, 128));
	G.FillPolygon(&brush1, pointDisc.data(), (INT)pointDisc.size());

	// cup
	vector <Gdiplus::PointF> pointCup;

	for (size_t i = 0; i < m_pointCup.size(); i++)
	{
		Gdiplus::PointF point;
		point.X = rect.left + m_pointCup[i].X / (float)bitmap.GetWidth() * rect.Width();
		point.Y = rect.top + m_pointCup[i].Y / (float)bitmap.GetHeight() * rect.Height();
		pointCup.push_back(point);
	}

	Gdiplus::SolidBrush brush2(Color(192, 192, 192));
	G.FillPolygon(&brush2, pointCup.data(), (INT)pointCup.size());

	// center
	// - only for anterior
	if (m_layerUpper == OcularLayerType::EPI && m_pointCenter.X > 0 && m_pointCenter.Y > 0) {
		int x = rect.left + m_pointCenter.X / (float)bitmap.GetWidth() * rect.Width();
		int y = rect.top + m_pointCenter.Y / (float)bitmap.GetHeight() * rect.Height();

		Gdiplus::Point pt1 = { x - 7, y };
		Gdiplus::Point pt2 = { x + 7, y };
		Gdiplus::Point pt3 = { x, y - 7 };
		Gdiplus::Point pt4 = { x, y + 7 };

		//Gdiplus::Pen pen(Gdiplus::Color(0x80, 0, 0x80), 2.0f);
		Gdiplus::Pen pen(Gdiplus::Color(0, 0, 0), 2.0f);
		G.DrawLine(&pen, pt1, pt2);
		G.DrawLine(&pen, pt3, pt4);
	}

	return;
}

void DrawThicknessMap::drawColorTable(CDC *pDC, CRect rect, Gdiplus::Color clrText)
{
	if (m_layerUpper == OcularLayerType::ILM && m_layerLower == OcularLayerType::NFL)
	{
		drawColorTableILMGCL(pDC, rect, clrText);
	}
	else if (m_layerUpper == OcularLayerType::ILM && m_layerLower == OcularLayerType::IPL)
	{
		drawColorTableILMIPL(pDC, rect, clrText);
	}
	else if (m_layerUpper == OcularLayerType::ILM && m_layerLower == OcularLayerType::RPE)
	{
		drawColorTableILMRPE(pDC, rect, clrText);
	}
	else if (m_layerUpper == OcularLayerType::EPI && m_layerLower == OcularLayerType::END)
	{
		drawColorTableEPIEND(pDC, rect, clrText);
	}
	else if (m_layerUpper == OcularLayerType::EPI && m_layerLower == OcularLayerType::BOW)
	{
		drawColorTableEPIBOW(pDC, rect, clrText);
	}
	else
	{
		return;
	}

	return;
}


void DrawThicknessMap::drawColorTableILMRPE(CDC *pDC, CRect rect, Gdiplus::Color clrText)
{
	// draw color table
	drawColorTableFor(m_colorTableILMRPE, kMaxThicknessILMRPE, pDC,
		Rect(rect.left, rect.top, 15, rect.Height()));

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
	G.DrawString(_T("500"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftTop, &brushText);
	py = rect.top + ((rect.Height() / 5) * 1);
	G.DrawString(_T("400"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 2);
	G.DrawString(_T("300"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 3);
	G.DrawString(_T("200"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 4);
	G.DrawString(_T("100"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 5);
	G.DrawString(_T("0レm"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftBottom, &brushText);


	return;
}


void DrawThicknessMap::drawColorTableILMIPL(CDC *pDC, CRect rect, Gdiplus::Color clrText)
{
	// draw color table
	drawColorTableFor(m_colorTableILMIPL, kMaxThicknessILMIPL, pDC,
		Rect(rect.left, rect.top, 15, rect.Height()));

	// draw thickness text
	int px = rect.left + 20;
	int py = 0;

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	Gdiplus::SolidBrush brushText(clrText);

	Font font(_T("Calibri"), 14, FontStyleRegular, UnitPixel);

	py = rect.top + ((rect.Height() / 5) * 0);
	G.DrawString(_T("250"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftTop, &brushText);
	py = rect.top + ((rect.Height() / 5) * 1);
	G.DrawString(_T("200"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 2);
	G.DrawString(_T("150"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 3);
	G.DrawString(_T("100"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 4);
	G.DrawString(_T("50"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 5);
	G.DrawString(_T("0レm"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftBottom, &brushText);

	return;
}

void DrawThicknessMap::drawColorTableILMGCL(CDC *pDC, CRect rect, Gdiplus::Color clrText)
{
	// draw color table
	drawColorTableFor(m_colorTableILMGCL, kMaxThicknessILMGCL, pDC,
		Rect(rect.left, rect.top, 15, rect.Height()));

	// draw thickness text
	int px = rect.left + 20;
	int py = 0;

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	Font font(_T("Calibri"), 14, FontStyleRegular, UnitPixel);

	Gdiplus::SolidBrush brushText(clrText);

	py = rect.top + ((rect.Height() / 5) * 0);
	G.DrawString(_T("200"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftTop, &brushText);
	py = rect.top + ((rect.Height() / 5) * 1);
	G.DrawString(_T("160"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 2);
	G.DrawString(_T("120"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 3);
	G.DrawString(_T("80"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 4);
	G.DrawString(_T("40"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 5);
	G.DrawString(_T("0レm"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftBottom, &brushText);

	return;
}

void DrawThicknessMap::drawColorTableEPIEND(CDC *pDC, CRect rect, Gdiplus::Color clrText)
{
	// draw color table
	int height = kMaxThicknessEPIEND - kMinThicknessEPIEND;

	CBitmap bitmap;
	bitmap.CreateBitmap(1, height, 1, 32, &m_colorTableEPIEND[kMinThicknessEPIEND]);

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
	G.DrawString(_T("800"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftTop, &brushText);
	py = rect.top + ((rect.Height() / 5) * 1);
	G.DrawString(_T("700"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 2);
	G.DrawString(_T("600"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 3);
	G.DrawString(_T("500"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 4);
	G.DrawString(_T("400"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 5) * 5);
	G.DrawString(_T("300レm"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftBottom, &brushText);


	return;
}

void DrawThicknessMap::drawColorTableEPIBOW(CDC *pDC, CRect rect, Gdiplus::Color clrText)
{
	// draw color table
	int height = kMaxThicknessEPIBOW - kMinThicknessEPIBOW;

	CBitmap bitmap;
	bitmap.CreateBitmap(1, height, 1, 32, &m_colorTableEPIBOW[kMinThicknessEPIBOW]);

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

	Font font(_T("Calibri"), 14, FontStyleRegular, UnitPixel);

	Gdiplus::SolidBrush brushText(clrText);

	py = rect.top + ((rect.Height() / 6) * 0);
	G.DrawString(_T("80"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftTop, &brushText);
	py = rect.top + ((rect.Height() / 6) * 1);
	G.DrawString(_T("70"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 6) * 2);
	G.DrawString(_T("60"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 6) * 3);
	G.DrawString(_T("50"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 6) * 4);
	G.DrawString(_T("40"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 6) * 5);
	G.DrawString(_T("30"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftCenter, &brushText);
	py = rect.top + ((rect.Height() / 6) * 6);
	G.DrawString(_T("20レm"), -1, &font, Gdiplus::PointF(px, py), &m_sfLeftBottom, &brushText);

	return;
}

void DrawThicknessMap::drawColorTableFor(DWORD* table, int tableSize, CDC *pDC, Gdiplus::Rect rect)
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

bool DrawThicknessMap::saveToPng(const std::wstring& filePath) const
{
	if (m_colorPixels.empty() || m_mapWidth <= 0 || m_mapHeight <= 0) {
		return false;
	}

	Bitmap bitmap(m_mapWidth, m_mapHeight, PixelFormat32bppARGB);
	for (int y = 0; y < m_mapHeight; ++y) {
		for (int x = 0; x < m_mapWidth; ++x) {
			DWORD pixel = m_colorPixels[y * m_mapWidth + x];
			BYTE r = (pixel >> 16) & 0xFF;
			BYTE g = (pixel >> 8) & 0xFF;
			BYTE b = pixel & 0xFF;
			bitmap.SetPixel(x, y, Color(255, r, g, b));
		}
	}

	CLSID pngClsid;
	if (getEncoderClsid(L"image/png", &pngClsid) < 0) {
		return false;
	}

	return bitmap.Save(filePath.c_str(), &pngClsid, nullptr) == Ok;
}

int DrawThicknessMap::getEncoderClsid(const WCHAR* format, CLSID* pClsid) const
{
	UINT num = 0;
	UINT size = 0;
	GetImageEncodersSize(&num, &size);
	if (size == 0) {
		return -1;
	}

	std::unique_ptr<BYTE[]> buffer(new BYTE[size]);
	if (buffer == nullptr) {
		return -1;
	}

	ImageCodecInfo* pImageCodecInfo = reinterpret_cast<ImageCodecInfo*>(buffer.get());
	if (pImageCodecInfo == nullptr) {
		return -1;
	}

	GetImageEncoders(num, size, pImageCodecInfo);
	for (UINT j = 0; j < num; ++j) {
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
			*pClsid = pImageCodecInfo[j].Clsid;
			return static_cast<int>(j);
		}
	}

	return -1;
}