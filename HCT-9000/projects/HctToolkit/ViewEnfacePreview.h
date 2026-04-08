#pragma once

#include "OctSystem2.h"
#include <memory>

#include <afxwin.h>
#include <afxcmn.h>

// ViewEnfacePreview

class ViewEnfacePreview : public CStatic
{
	DECLARE_DYNAMIC(ViewEnfacePreview)

public:
	ViewEnfacePreview();
	virtual ~ViewEnfacePreview();

public:
	void updateWindow(void);
	void drawPixelsPitch(CDC* pDC);

	void setEqualization(bool flag, float limit = 16.0f, float scalar = 0.0f);
	void setDisplaySize(int width, int height);
	void setScanRange(float x, float y);
	void setMeasureRange(float x, float y);
	void showGridLines(bool flag);
	void showTopoGridLines(bool flag);

	int getWidth(void) const;
	int getHeight(void) const;
	int getImageWidth(void) const;
	int getImageHeight(void) const;
	int getSourceWidth(void) const;
	int getSourceHeight(void) const;
	unsigned char* getImageBitData(void) const;
	bool IsEmptyImage(void) const;

	EnfaceImageCallback* getCallbackFunction(void);
	void callbackEnfaceImage(const unsigned char* data, unsigned int width, unsigned int height);

	bool saveImage(CString& filename);
	

protected:
	struct ViewEnfacePreviewImpl;
	std::unique_ptr<ViewEnfacePreviewImpl> d_ptr;
	ViewEnfacePreviewImpl& getImpl(void) const;

	void drawGridLines(CDC* pDC);
	void drawTopoGridLines(CDC * pDC);

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

};


