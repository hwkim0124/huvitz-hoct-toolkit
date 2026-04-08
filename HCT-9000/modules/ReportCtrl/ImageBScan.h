#pragma once
#include "stdafx.h"
#include "ImageBase.h"
#include "DrawMeasureTool.h"
#include "ReportCtrlDef.h"
#include "CppUtil2.h"

class REPORTCTRL_DLL_API ImageBScan : public ImageBase
{
public:
	ImageBScan();
	virtual ~ImageBScan();
	void	showDirection(bool show, Gdiplus::Color color);

	void	saveImage(CString filepath);
	
	void	setEyeSide(EyeSide side);
	void	setScanPattern(OctScanPattern pattern);
	void	setBScanColor(BScanColor color);
	void	setBScanLevel(int min, int max);

	void	setSelectedScanLine(int idxLine);
	int		getSelectedScanLine();
	void	setSelectedScanLineHD(int idxLine);

	void	setOpticNerveDiscRange(int x1, int x2, COLORREF color);
	void	setOpticNerveCupRange(int x1, int x2, COLORREF color);
	void	recalcLUT(void);
	bool	makeDisplayImage(cv::Mat &dstImage, cv::Mat &srcImage, BScanColor color);

	void	clearMeasure(void);
	void	clearSegmentLayer(void);
	void	addSegmentLayer(SegmentLayer layer);

	void	setScanPointX(int value, int width);

	void	drawToDC(CDC *pDC, CRect rect, BScanColor color);
	void	drawToDCRaw(CDC *pDC, CRect rect);

	auto	getLayerPointAt(OcularLayerType layerType, int x)->std::pair<bool, int>;

protected:
	void	drawImage(CDC *pDC, CRect rect, CppUtil::CvImage image);
	void	drawSegmentLayer(CDC *pDC, CRect rect);
	void	drawOpticNerveArea(CDC *pDC, CRect rect);

	void	drawScanDirection(CDC *pDC, Gdiplus::Rect rect, Gdiplus::Color color);
	void	drawScanPointLine(CDC *pDC, CRect rect);

	int		getScanAngle(int idxLine);
	bool	getLayerPoints(vector <Gdiplus::Point>& out_point1, vector <Gdiplus::Point>& out_point2,
				SegmentLayer layer, CRect rect, bool ignoreDiscArea);

public:
	void	setPanelPostion(int panelX, int panelY);

protected:
	OctScanPattern	m_scanPattern;
	EyeSide			m_eyeSide;
	bool			m_showDirection;

	Gdiplus::Color	m_colorScanLine;
	int				m_scanIndex;
	int				m_scanAngle;
	float				m_scanAngleOffset;

	BScanColor	m_bscanColor;
	int			m_rangeMin;
	int			m_rangeMax;

	cv::Mat		m_lutRGB;
	cv::Mat		m_lutPos;
	cv::Mat		m_lutNeg;
	cv::Mat		m_lutUse;		// 사용할맵

	vector <SegmentLayer> m_layers;
	std::pair <int, int> m_rangeDisc;
	std::pair <int, int> m_rangeCup;
	COLORREF m_clrRangeDisc;
	COLORREF m_clrRangeCup;

	int			m_scanPointX;
	int			m_widthForScanPoint; // m_scanPointX 의 기준이 되는 map(?)의 크기

	double m_wndPixelPer1MM;
	float m_zoomRatio;

	int m_panelX;
	int m_panelY;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


static float getScanRangeX(OctScanPattern pattern)
{
	float rangeX = 0.0;

	switch (pattern.getPatternName())
	{
	case PatternName::MacularLine:
	case PatternName::DiscLine:
	case PatternName::AnteriorLine:
	case PatternName::MacularRaster:
	case PatternName::Macular3D:
	case PatternName::MacularCross:
	case PatternName::DiscRaster:
	case PatternName::Disc3D:
	case PatternName::Anterior3D:
	case PatternName::MacularDisc:
	case PatternName::AnteriorFull:
		rangeX = (pattern._direction == 0) ? pattern.getScanRangeX() : pattern.getScanRangeY();
		break;
	case PatternName::MacularRadial:
	case PatternName::DiscRadial:
	case PatternName::AnteriorRadial:
	case PatternName::Topography:
		rangeX = pattern.getScanRangeX();
		break;
	case PatternName::MacularCircle:
	case PatternName::DiscCircle:
		rangeX = pattern.getScanRangeX() * 3.1415f;
		break;
	default:
		break;
	}

	return (rangeX * 1000.0f);
}

static float getScanRangeZ(OctScanPattern pattern, int height)
{
	float rangeZ = 0.0;

	if (pattern.isCorneaScan())
	{
		rangeZ = height * GlobalSettings::getCorneaScanAxialResolution(); //3.064f / CORNEA_REFLECTIVE_INDEX;
	}
	else
	{
		rangeZ = height * GlobalSettings::getRetinaScanAxialResolution();// 3.170f / RETINA_REFLECTIVE_INDEX;
	}

	return rangeZ;
}
