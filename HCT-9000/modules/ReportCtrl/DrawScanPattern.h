#pragma once

class DrawScanPattern
{
public:
	DrawScanPattern();
	virtual ~DrawScanPattern();

//	virtual void setScanPattern(OctScanPattern pattern);
//	virtual OctScanPattern getScanPattern(void);
	virtual void setScanLine(int index, int scanIndex);
//	virtual int getScanLine(void);

protected:
	virtual void drawScanPattern(CDC *pDC, CRect rect, OctScanPattern pattern, float pixelPerMM);
	virtual void drawScanLine(CDC *pDC, CRect rect, int index, int scanIndex);
//	virtual void drawScanLine(CDC *pDC, CRect rect, OctScanPattern pattern, float pixelPerMM);
//	virtual void setWndPixelPer1MM(float) = 0;
//	virtual float getWndPixelPer1MM(void) = 0;

private:
	void drawPatternLine(CDC *pDC, CPoint ptCenter, OctScanPattern pattern, float pixelPerMM);
	void drawPatternCross(CDC *pDC, CPoint ptCenter, OctScanPattern pattern, float pixelPerMM);
	void drawPatternRadial(CDC *pDC, CPoint ptCenter, OctScanPattern pattern, float pixelPerMM);
	void drawPatternRaster(CDC *pDC, CPoint ptCenter, OctScanPattern pattern, float pixelPerMM);
	void drawPatternCircle(CDC *pDC, CPoint ptCenter, OctScanPattern pattern, float pixelPerMM);
	void drawPatternRect(CDC *pDC, CPoint ptCenter, OctScanPattern pattern, float pixelPerMM);

	void drawCircle(CDC *pDC, CRect rect, bool bArrow);
	void drawLine(CDC *pDC, int x1, int y1, int x2, int y2, bool bArrow);

private:
//	OctScanPattern m_pattern;
	int m_scanIndex[2];
};

