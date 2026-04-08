#pragma once
#include "ImageBase.h"
#include "ReportCtrlDef.h"
#include "DrawThicknessMap.h"

class REPORTCTRL_DLL_API ImageRetina : public ImageBase
{
	const int kFixationCenterRow = 64;
	const int kFixationCenterCol = 60;
	const double kFixationSpaceSize = (12.0f / 68.4f);
//	const double kImgPixelPer1MM = (442.0f / 12.0f);
//	const double kImgPixelSize = (1.0f / kImgPixelPer1MM);
	

public:
	ImageRetina();
	virtual ~ImageRetina();

	void	setImage(unsigned char *data, unsigned int width, unsigned int height);
	void	saveImage(CString filePath);
	virtual CppUtil::CvImage getImage(void);

	void	showWorkingDotLine(bool show);
	void	showSplitFocusGuide(bool show);
	void	showMasking(bool show, COLORREF color = 0);
	void	showFixationTarget(bool show);
	void	showFixationPosition(bool show);
	void	showTrackingTarget(bool show, int overlaps, bool isHorizontal, int scanLines);

	void	setWndPixelPer1MM(double pixel);
	double	getWndPixelPer1MM(void);
	void	getDstArea(int *xDst, int *yDst, int *wDst, int *hDst);
	void	setSplitGuideCenter(int x, int y);

	bool	setFixationTarget(CPoint point);
	bool	setFixationTarget(int row, int col);
	void	getFixationTarget(CPoint *point);
	void	getFixationTarget(int *row, int *col);

	void	drawToDC(CDC *pDC, CRect rect, float pixelPerMM);
	void	drawToDCForReport(CDC *pDC, CRect rect, float pixelPerMM);

protected:
//	void	drawImage(CDC *pDC, CRect rect, CvImage image);
	void	drawWorkingDotLine(CDC *pDC);
	void	drawSplitFocusGuide(CDC *pDC);
	void	drawMasking(CDC* pDC);
	void	drawMasking(CDC* pDC, CRect rtClient, CRect rtMask);
	void	drawFixationTarget(CDC *pDC);
	void	drawTrackingTarget(CDC *pDC);
	void	drawTrackingTargetRetina(CDC *pDC);
	void	drawTrackingTargetDisc(CDC *pDC);
	

private:
	bool	m_showWorkingDotLine;
	bool	m_showSplitFocusGuide;
	bool	m_showMasking;
	bool	m_showFixationTarget;
	bool	m_showTrackingTarget;

	double	m_wndPixelPer1MM;
	int		m_splitGuideCenterX;
	int		m_splitGuideCenterY;

	DWORD	m_clrMask;
	int		m_fixationRow;
	int		m_fixationCol;
	bool	m_showFixationPosition;

	bool	m_isHorizontal;
	int		m_overlaps;
	int		m_scanLines;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
};

