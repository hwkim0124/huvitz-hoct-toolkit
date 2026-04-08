#pragma once

#include "OctSystem2.h"
#include <memory>

using namespace OctSystem;


// ViewAngioPreview

class ViewAngioPreview : public CStatic
{
	DECLARE_DYNAMIC(ViewAngioPreview)

public:
	ViewAngioPreview();
	virtual ~ViewAngioPreview();

public:
	void updateWindow(void);
	void drawOverlayText(CDC* pDC);
	void drawSelections(CDC* pDC);
	void drawDecorrLines(CDC* pDC);
	void drawBscanLineSelected(CDC* pDC);

	void setDisplaySize(int width, int height);
	void setDecorrelationValues(float avg, float stdev, float maxv);
	void setIntensityValues(float avg, float stdev, float maxv);
	void setDecorrOverLines(int avgSize, int topSize, float thresh);

	int getWidth(void) const;
	int getHeight(void) const;
	int getImageWidth(void) const;
	int getImageHeight(void) const;
	void setEnhancement(bool flag, float contrast);

	void drawImage(const unsigned char* data, int width, int height);
	bool saveImage(CString& dirPath, CString& filename);
	void showLineSelected(bool flag, int lineIdx, int pointIdx);
	void showDecorrOverHorzLines(bool flag);
	void showDecorrOverVertLines(bool flag);
	void selectBscanLine(int lineIdx);

protected:
	struct ViewAngioPreviewImpl;
	std::unique_ptr<ViewAngioPreviewImpl> d_ptr;
	ViewAngioPreviewImpl& getImpl(void) const;

	void computeDecorrOverLines(void);

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


