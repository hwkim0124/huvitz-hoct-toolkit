#pragma once

#include "OctSystem2.h"

#include <memory>

#include <afxwin.h>
#include <afxcmn.h>


// ViewCorneaCamera

class ViewCorneaCamera : public CStatic
{
	DECLARE_DYNAMIC(ViewCorneaCamera)

public:
	ViewCorneaCamera();
	virtual ~ViewCorneaCamera();

public:
	void updateWindow(void);
	void drawCameraInfo(CDC* pDC);
	void drawKeratoPoints(CDC* pDC);
	void drawFocusStatus(CDC* pDC);
	void drawStageStatus(CDC* pDC);
	void drawAlignGuide(CDC* pDC);
	void drawSpotWindows(CDC* pDC);

	void setDisplaySize(int width, int height);
	int getWidth(void) const;
	int getHeight(void) const;
	int getImageWidth(void) const;
	int getImageHeight(void) const;

	void showCameraInfo(bool flag);
	void showKeratoPoints(bool flag);
	void showFocusStatus(bool flag);
	void showAlignGuide(bool flag);
	void showSpotWindows(bool flag);
	void showStageStatus(bool flag);

	CorneaCameraImageCallback* getCallbackFunction(void);

	bool saveImage(CString& filename);

protected:
	struct ViewCorneaCameraImpl;
	std::unique_ptr<ViewCorneaCameraImpl> d_ptr;
	ViewCorneaCameraImpl& getImpl(void) const;

	void callbackCameraImage(unsigned char* data, unsigned int width, unsigned int height, bool isCenter, float centX, float centY, bool isFocus, float distZ);
	void countFrameRate(void);

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


