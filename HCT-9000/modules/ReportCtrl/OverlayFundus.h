#pragma once
#include "ReportCtrlDef.h"
#include "DrawScanPattern.h"
#include "DrawThicknessMap.h"
#include "DrawRadiusMap.h"
#include "DrawEtdrsChart.h"
#include "DrawRnflChart.h"
#include "DrawDeviationMap.h"
#include "DrawGccChart.h"
#include "CppUtil2.h"

// OverlayFundus
using namespace CppUtil;
using namespace ReportCtrl;

class REPORTCTRL_DLL_API OverlayFundus : public CWnd,
	public DrawThicknessMap, public DrawRadiusMap, public DrawEtdrsChart, public DrawRnflChart,
	public DrawGccChart, public DrawDeviationMap

{
	DECLARE_DYNAMIC(OverlayFundus)
	typedef struct _ScanLine
	{
		int direction;		// -1 : no, 0 : horz, 1 : vert
		Gdiplus::Point pt1;
		Gdiplus::Point pt2;
		bool hasHandle;
		Gdiplus::Point ptH;	// handle center;		
	} ScanLine;

	const int kHandleSize = 24;
	const int kCenterCrossSize = 8;

public:
	typedef enum _target_t {
		CONFIRM = 0,
		ANALYSIS,
		MEASURE
	} target_t;

public:
	OverlayFundus();
	virtual ~OverlayFundus();
	void	initialize(EyeSide side);
	void	setCompareType(EyeSide side);
	void	showEnfaceImage(bool show);
	void	showDeviationImage(bool show);
	void	showThicknessMap(bool show);
	void	showThicknessChart(bool show);
	void	showRadiusMap(bool show);
	void	showScanPattern(bool show);
	void	showScanLine(bool show);
	void	showScanPatternCenterCross(bool show);
	void	showAngioScanIndex(bool show, bool isHorizontal, int scanLines, int scanIndex);

	void	setWndPixelPer1MM(float pixel);
	void	setScanPattern(OctScanPattern pattern, target_t target);
	void	setScanPatternAngio(OctScanPattern pattern);

	void	setScanIndex(int index, int idxLine);
	void	setScanIndexHD(int index, int idxLine);

	void	setChartType(ThickChartType type);
	void	setChartCenter(float cx, float cy);
	void	setChartCenterForMacularDisc(float discCenterX, float discCenterY, float macularCenterX, float macularCenterY);
	
	void	clearEnfaceImage(void);
	void	setEnfaceImage(CppUtil::CvImage image);

	void	clearDeviationImage(void);
	void	setDeviationImage(CppUtil::CvImage image, std::pair<int, int> mapSize,
				std::pair<int, int> center, std::pair<OcularLayerType, OcularLayerType> layers,
				float mapRangeRatio);

	bool	isChartArea(CPoint point);

	int		getCloseScanIndex(CPoint point);
	int		getCloseScanLine(CPoint point, int idxDisp, function<float(ScanLine, CPoint)> fnGetDist = nullptr);
	float	getDistance(ScanLine line, CPoint point);
	float	getDistanceHandle(ScanLine, CPoint point);

	void	drawOverlayForReport(CDC *pDC, CRect rtClient, float pixelPerMM);
	void	drawEnfaceImage(CDC *pDC, CRect rect_in);
	void	drawThicknessChart(CDC *pDC, CRect rect, float pixelPerMM);
	void	drawScanPointForReport(CDC *pDC, Gdiplus::Color color, CRect rect);

	CRect	getScanPatternRect(void);
	CRect	getScanDeviationMapRect(OctScanPattern pattern);
	CPoint	getScanPatternCenter(void);
	CPoint	getChartCenter(void);

	void	setScanPoint(POINT pt);
	POINT	getScanPoint();

private:
	void	drawScanPattern(CDC *pDC, CRect rect, OctScanPattern pattern);
	void	drawScanPatternForReport(CDC *pDC, CRect rtScan, CRect rtClient,
				OctScanPattern pattern, float pixelPerMM);
	void	drawScanLine(CDC *pDC, int index);
	void	drawScanLine(CDC *pDC, CRect rect, float pixelPerMM, int index);
	void	drawPatternLines(CDC *pDC);
	void	drawPatternLinesForReport(CDC *pDC, CRect rect, float pixelPerMM);
	void	drawPatternCircle(CDC *pDC, CRect rect);
	void	drawPatternRect(CDC *pDC, CRect rect);
	void	drawCircle(CDC *pDC, CRect rect);
	void	drawArrowCircle(CDC *pDC, int x1, int y1, int x2, int y2, Gdiplus::Color color);
	void	drawLine(CDC *pDC, ScanLine line);
	void	drawLine(CDC *pDC, int x1, int y1, int x2, int y2);
	void	drawArrowLine(CDC *pDC, ScanLine line, Gdiplus::Color color);
	void	drawArrowLine(CDC *pDC, int x1, int y1, int x2, int y2, Gdiplus::Color color);
	void	drawScanPoint(CDC *pDC, POINT pt, Gdiplus::Color color);
	void	drawAngioScanIndex(CDC *pDC, CRect rect, OctScanPattern pattern, int scanIdx);

private:
	bool	m_showEnfaceImage;
	bool	m_showDeviationMap;
	bool	m_showThicknessMap;
	bool	m_showThicknessChart;
	bool	m_showRadiusMap;
	bool	m_showScanPattern;
	bool	m_showScanLine;
	bool	m_showScanPatternCenterCross;

	bool m_showAngioScanIndex;
	bool m_isHorizontal;
	int m_scanLines;
	int m_scanIdx;
	CvImage m_imageEnface;
	float	m_pixelPerMM;

	vector <ScanLine> m_scanLine;
	vector <ScanLine> m_scanLineHD;			// 3d pattern, H/V
	ScanLine m_scanLineSelected[2];

	OctScanPattern m_scanPattern;
	ThickChartType m_chartType;
	float	m_chartCenterX;
	float	m_chartCenterY;
	
	float	m_chartCenterDiscX;
	float	m_chartCenterDiscY;
	float	m_chartCenterMacularX;
	float	m_chartCenterMacularY;

	POINT	m_scanPoint;

	std::pair<OcularLayerType, OcularLayerType> m_layers;
	target_t m_target;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

