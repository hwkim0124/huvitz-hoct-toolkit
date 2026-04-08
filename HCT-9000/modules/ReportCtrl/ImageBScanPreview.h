#pragma once
#include "ImageBase.h"
#include "ReportCtrlDef.h"


// ImageBScanPreview

class REPORTCTRL_DLL_API ImageBScanPreview : public ImageBase
{
	DECLARE_DYNAMIC(ImageBScanPreview)

public:
	ImageBScanPreview();
	virtual ~ImageBScanPreview();

	void	setImage(unsigned char *data, unsigned int width, unsigned int height,
				int sigCenter, std::tuple<bool, int, int> scanInfo);
	void	setScanPattern(OctScanPattern pattern);
	void	setScanQuality(float quality);
	void	setSubGuidePos(int value);
	void	setAxialLength(float value);
	float	getAxialLength();
	void	showTopoGuideLine(bool value);
	void	showAxialLength(bool value);
	void	setAxialLengthMsg(CString szMsg);
	void	setAxialLengthNumMsg(CString szMsg);
private:
	void	getDstArea(int *xDst, int *yDst, int *wDst, int *hDst);
	void	drawGuidLine(CDC *pDC);
	void	drawSSI(CDC* pDC);
	void	drawPatternInfo(CDC* pDC);
	void	drawScanInfo(CDC* pDC);
	void	drawAxialLength(CDC *pDC);
private:
	OctScanPattern	m_pattern;
	float	m_quality;
	CFont	m_fontNotoSans15;
	CFont	m_fontNotoSans16;
	std::tuple<bool, int, int> m_scanInfo;
	CString m_szAxialLengthMsg;
	CString m_szAxialLengthNumMsg;
	bool	m_showAxialLength;
	int		m_sigCenter;
	float	m_axialLength;
	bool	m_showTopoGuideLine;
	int		m_subGuidePos{-1};

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};


