#pragma once
#include "resource.h"		// main symbols
#include "DrawMeasureTool.h"
#include "DrawScanPattern.h"
#include "CppUtil2.h"

// WndImage
using namespace CppUtil;
using namespace ReportCtrl;

class ImageBase : public CWnd, public DrawMeasureTool
{
	DECLARE_DYNAMIC(ImageBase)
public:
	ImageBase();
	virtual ~ImageBase();

	virtual void setFlipVertical(bool flip);

	virtual void clearImage(void);

	virtual void setImage(unsigned char *data, unsigned int width, unsigned int height);
	virtual void setImage(CppUtil::CvImage image);
	virtual bool hasImage(void);
	virtual CppUtil::CvImage getImage(void);
	virtual void saveImage(CString filepath);

	virtual void setDimension(float width, float height);
	virtual void getDimension(float &widht, float &height);

	virtual void drawToDC(CDC *pDC, CRect rtClient, float pixelPerMM) {};
//	virtual void drawToDC(CDC *pDC, CRect rect) {};

	virtual void drawImage(CDC *pDC, CRect rect);
	virtual void drawImage(CDC *pDC, CppUtil::CvImage image, CRect rect);

protected:
	unsigned int getImgWidth(void);
	unsigned int getImgHeight(void);
	unsigned int getWndWidth(void);
	unsigned int getWndHeight(void);
	virtual void getSrcArea(int *xSrc, int *ySrc, int *wSrc, int *hSrc);
	virtual void getDstArea(int *xDst, int *yDst, int *wDst, int *hDst);

	virtual void drawImage(CDC *pDC);
	virtual void drawImage(CDC *pDC, CppUtil::CvImage image);

	static void maskIRFundusImage(cv::Mat& mat, int radisus);

protected:
	CRITICAL_SECTION m_cs;

	CRect		m_rtDisplay;
	bool		m_flipVert;
	CppUtil::CvImage		m_image;

	float		m_actualLengthX;
	float		m_actualLengthY;


protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
};


