#pragma once

#include "OctSystem2.h"

#include <memory>

// CorneaCameraView

class CorneaCameraView : public CStatic
{
	DECLARE_DYNAMIC(CorneaCameraView)

public:
	CorneaCameraView();
	virtual ~CorneaCameraView();

public:
	void updateWindow(void);
	void drawCameraInfo(CDC* pDC);
	void drawKeratoPoints(CDC* pDC);
	void drawFocusStatus(CDC* pDC);
	void drawAlignGuide(CDC* pDC);
	void drawSpotWindows(CDC* pDC);

	void setDisplaySize(int width, int height);
	int getWidth(void) const;
	int getHeight(void) const;

	void showCameraInfo(bool flag);
	void showKeratoPoints(bool flag);
	void showFocusStatus(bool flag);
	void showAlignGuide(bool flag);
	void showSpotWindows(bool flag);

	CorneaCameraImageCallback* getCallbackFunction(void);

	void saveImage(CString path = _T(""));

protected:
	struct CorneaCameraViewImpl;
	std::unique_ptr<CorneaCameraViewImpl> d_ptr;
	CorneaCameraViewImpl& getImpl(void) const;

	void callbackCameraImage(unsigned char* data, unsigned int width, unsigned int height);
	void countFrameRate(void);

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};


