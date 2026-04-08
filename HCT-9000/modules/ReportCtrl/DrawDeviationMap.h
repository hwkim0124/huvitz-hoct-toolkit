#pragma once
#include "ReportCtrlDef.h"

using namespace ReportCtrl;

class REPORTCTRL_DLL_API DrawDeviationMap
{
	static constexpr int kMaxPercentile = 100;

public:
	typedef enum _DevMapType {
		DEV_MAP_NORMATIVE,
		DEV_MAP_DEVIATION
	} DevMapType;

public:
	DrawDeviationMap();
	virtual ~DrawDeviationMap();

	void initialize(void);
	void drawDeviationMap(HDC hDC, CRect rect, PatternName patternName,
		std::pair<OcularLayerType, OcularLayerType> layers, float wResizeRatio = 1.0f);
	void drawColorTableDevMap(CDC *pDC, CRect rect, Gdiplus::Color clrText,
		PatternName patternName, std::pair<OcularLayerType, OcularLayerType> layers);
	void drawDeviationMapLabel(CDC *pDC, Gdiplus::RectF rect, Gdiplus::Font *font,
		Gdiplus::Color color = Gdiplus::Color::Black, bool left = true);
	void setDeviationMapType(DevMapType type);

protected:
	void setDeviationMapData(std::vector <float> data, std::pair<int, int> mapSize,
		std::pair<int, int> dataSize, std::pair<int, int> center, float mapRangeRatio);
	void clearDeviationMapData();
	COLORREF getPercentileColor_Dev(int percent);
	COLORREF getPercentileColor_Nor(PatternName patternName,
		std::pair<OcularLayerType, OcularLayerType> layers, int percent);
	COLORREF getPercentileColorMacular(int percent);
	COLORREF getPercentileColorDisc(int percent);
	void makeColorTable(DWORD *table, unsigned long minIndex, unsigned long maxIndex,
		LPCTSTR lutFileName);
	void drawColorTableFor(DWORD* table, int tableSize, CDC *pDC, Gdiplus::Rect rect);
	void drawColorTableDevMap_Nor(CDC *pDC, CRect rect, Gdiplus::Color clrText, PatternName patternName,
		std::pair<OcularLayerType, OcularLayerType> layers);
	void drawColorTableDevMap_Nor_Macular(CDC *pDC, CRect rect, Gdiplus::Color clrText);
	void drawColorTableDevMap_Nor_Disc(CDC *pDC, CRect rect, Gdiplus::Color clrText);
	void drawColorTableDevMap_Dev(CDC *pDC, CRect rect, Gdiplus::Color clrText);

private:
	std::vector <float> m_data;
	std::pair<int, int> m_mapSize;
	std::pair<int, int> m_dataSize;
	std::pair<int, int> m_center;
	float m_mapRangeRatio;
		// 그리는 영역의 범위와 map 범위의 비율
		// ex) 그리는 영역이 9mm 인데, map 은 6mm 만 그린다면, 값은 6/9;
	
	DevMapType m_mapType;

	DWORD m_colorTableDevMap[kMaxPercentile];

	Gdiplus::StringFormat m_sfLeftTop;
	Gdiplus::StringFormat m_sfLeftCenter;
	Gdiplus::StringFormat m_sfLeftBottom;
};

