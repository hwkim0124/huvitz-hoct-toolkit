#pragma once
#include "afxwin.h"
#include "ReportCtrlDef.h"
#include "CppUtil2.h"

//OverlayBScan
using namespace CppUtil;
using namespace ReportCtrl;

class REPORTCTRL_DLL_API OverlayBScan : public CWnd
{
	DECLARE_DYNAMIC(OverlayBScan)
	typedef struct _BScanLine
	{
		int direction;		// -1 : no, 0 : horz, 1 : vert
		Gdiplus::Point pt1;
		Gdiplus::Point pt2;
		bool hasHandle;
		Gdiplus::Point ptH;	// handle center;
		Gdiplus::Point ptH2;	// handle center;
	} BScanLine;

	const int kHandleSize = 24;

public:
	/*typedef enum _target_t {
		BIOCONFIRM = 0,
		BIOANALYSIS,
		BIOMEASURE
	} target_t;*/

public:
	OverlayBScan();
	virtual ~OverlayBScan();
	void	initialize(EyeSide side);
	void	showScanPattern(bool show);
	void	showScanLine(bool show);

	void	setWndPixelPer1MM(float pixel);
	//void	setScanPattern(OctScanPattern pattern, target_t target);
	void	setScanLine(CRect rect);
	
	void	setBScanIndex(int index, int idxLine);
	int		getCloseScanIndex(CPoint point);
	int		getCloseScanLine(CPoint point, int idxDisp, function<float(BScanLine, CPoint)> fnGetDist = nullptr);
	float	getDistance(BScanLine line, CPoint point);
	float	getDistanceHandle(BScanLine, CPoint point);
	float	getDistanceHandle2(BScanLine, CPoint point);
	void	drawOverlayForReport(CDC *pDC, CRect rtClient, float pixelPerMM);
	void	drawScanPointForReport(CDC *pDC, Gdiplus::Color color, CRect rect);

	CRect	getScanPatternRect(void);

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
	void	drawLine(CDC *pDC, BScanLine line);
	void	drawLine(CDC *pDC, int x1, int y1, int x2, int y2);
	void	drawArrowLine(CDC *pDC, BScanLine line, Gdiplus::Color color);
	void	drawArrowLine(CDC *pDC, int x1, int y1, int x2, int y2, Gdiplus::Color color);
	void	drawScanPoint(CDC *pDC, POINT pt, Gdiplus::Color color);

private:
	bool	m_showScanPattern;
	bool	m_showScanLine;

	float	m_pixelPerMM;

	vector <BScanLine> m_scanLine;
	BScanLine m_scanLineSelected[2];

	OctScanPattern m_scanPattern;
	POINT	m_scanPoint;

	std::pair<OcularLayerType, OcularLayerType> m_layers;
	//target_t m_target;
	int count;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	//	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

