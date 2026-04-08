#pragma once
#include "ReportCtrlDef.h"

using namespace ReportCtrl;

class REPORTCTRL_DLL_API DrawThicknessMap
{
	static const int kMaxThicknessILMRPE = 500;
	static const int kMaxThicknessILMIPL = 250;
	static const int kMaxThicknessILMGCL = 200;
	static const int kMinThicknessEPIEND = 300;
	static const int kMaxThicknessEPIEND = 800;
	static const int kMinThicknessEPIBOW = 20 * 10;
	static const int kMaxThicknessEPIBOW = 80 * 10;


public:
	DrawThicknessMap();
	virtual ~DrawThicknessMap();
	virtual CBitmap* getBitmap(void) { return &m_bmpThick; }

	void initialize(void);
	void setThicknessLayer(OcularLayerType upper, OcularLayerType lower);
	void setThickMapData(std::vector <float> data, int width, int height);
	void setDiscPoint(std::vector <Gdiplus::PointF> points);
	void setCupPoint(std::vector <Gdiplus::PointF> points);
	void setTransparency(float transparency);

	void drawThicknessMap(CDC *pDC, CRect rect);
	void drawColorTable(CDC *pDC, CRect rect, Gdiplus::Color clrText);
	bool saveToPng(const std::wstring& filePath) const;

protected:
	void makeColorTable(void);
	void makeColorTable(DWORD *table, unsigned long minIndex, unsigned long maxIndex, LPCTSTR lutFileName);

	void drawColorTableILMRPE(CDC *pDC, CRect rect, Gdiplus::Color clrText);
	void drawColorTableILMIPL(CDC *pDC, CRect rect, Gdiplus::Color clrText);
	void drawColorTableILMGCL(CDC *pDC, CRect rect, Gdiplus::Color clrText);
	void drawColorTableEPIEND(CDC *pDC, CRect rect, Gdiplus::Color clrText);
	void drawColorTableEPIBOW(CDC *pDC, CRect rect, Gdiplus::Color clrText);

	void drawColorTableFor(DWORD* table, int tableSize, CDC *pDC, Gdiplus::Rect rect);

private:
	CBitmap m_bmpThick;
	std::vector <Gdiplus::PointF> m_pointDisc;
	std::vector <Gdiplus::PointF> m_pointCup;
	Gdiplus::Point m_pointCenter;

	float m_transparency;
	OcularLayerType m_layerUpper;
	OcularLayerType m_layerLower;
	
	DWORD	m_colorTableILMRPE[kMaxThicknessILMRPE];
	DWORD	m_colorTableILMIPL[kMaxThicknessILMIPL];
	DWORD	m_colorTableILMGCL[kMaxThicknessILMGCL];
	DWORD	m_colorTableEPIEND[kMaxThicknessEPIEND];
	DWORD	m_colorTableEPIBOW[kMaxThicknessEPIBOW];

	std::vector<DWORD> m_colorPixels;
	int m_mapWidth = 0;
	int m_mapHeight = 0;

	Gdiplus::StringFormat m_sfLeftTop;
	Gdiplus::StringFormat m_sfLeftCenter;
	Gdiplus::StringFormat m_sfLeftBottom;

	int getEncoderClsid(const WCHAR* format, CLSID* pClsid) const;
};

