#pragma once

#include "OctSystem2.h"

#include <memory>

// ViewRetinaCamera

class ViewRetinaCamera : public CStatic
{
	DECLARE_DYNAMIC(ViewRetinaCamera)

public:
	ViewRetinaCamera();
	virtual ~ViewRetinaCamera();

public:
	void updateWindow(void);
	void drawCameraInfo(CDC* pDC);
	void drawAlignGuide(CDC* pDC);
	void drawFocusStatus(CDC* pDC);
	void drawSplitPoints(CDC* pDC);
	void drawSplitWindows(CDC* pDC);
	void drawWorkingDots(CDC* pDC);
	void drawOpticDiscRegion(CDC* pDC);
	void drawTrackTarget(CDC* pDC);
	void drawTrackFeature(CDC* pDC);

	void setDisplaySize(int width, int height);
	int getWidth(void) const;
	int getHeight(void) const;
	int getImageWidth(void) const;
	int getImageHeight(void) const;
	void updateImage(unsigned char* data, unsigned int width, unsigned int height);

	void showCameraInfo(bool flag);
	void showAlignGuide(bool flag);
	void showFocusStatus(bool flag);
	void showSplitPoints(bool flag);
	void showSplitWindows(bool flag);
	void showCircleMask(bool flag);
	void showWorkingDots(bool flag);
	void showTrackTarget(bool flag);

	void showTrackFeature(bool flag);

	RetinaCameraImageCallback* getCallbackFunction(void);

	bool saveImage(CString& filename);

protected:
	struct ViewRetinaCameraImpl;
	std::unique_ptr<ViewRetinaCameraImpl> d_ptr;
	ViewRetinaCameraImpl& getImpl(void) const;

	void callbackCameraImage(unsigned char* data, unsigned int width, unsigned int height);
	void countFrameRate(void);

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


