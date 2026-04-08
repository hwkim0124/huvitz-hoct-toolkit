#pragma once
#include "ReportCtrlDef.h"

using namespace ReportCtrl;

class REPORTCTRL_DLL_API DrawRadiusMap
{
	static const int kMinRadius = 475;
	static const int kMaxRadius = 975;
	static const int kTextWidth = 30;
	static const int kTextHeight = 12;

public:
	DrawRadiusMap();
	~DrawRadiusMap();

	void initialize(void);
	void setRadiusMapData(std::vector <float> data, int width, int height);
	void setTransparency(float transparency);

	void drawRadiusMap(CDC *pDC, CRect rect);
	void drawColorTable(CDC *pDC, CRect rect, Gdiplus::Color clrText);

protected:
	void makeColorTable(void);
	void makeColorTable(DWORD *table, unsigned long minIndex, unsigned long maxIndex, COLORREF minColor, COLORREF maxColor);
	
private:
	CBitmap m_bmpRadius;
	float m_transparency;
	OcularLayerType m_layerUpper;
	OcularLayerType m_layerLower;

	DWORD	m_colorTable[kMaxRadius];
};

