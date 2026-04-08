#pragma once

#include "OctSystem2.h"

#include <memory>

// RetinaCameraView

class RetinaCameraView : public CStatic
{
	DECLARE_DYNAMIC(RetinaCameraView)

public:
	RetinaCameraView();
	virtual ~RetinaCameraView();

public:
	void updateWindow(void);
	void drawCameraInfo(CDC* pDC);
	void drawAlignGuide(CDC* pDC);

	void setDisplaySize(int width, int height);
	int getWidth(void) const;
	int getHeight(void) const;

	void showCameraInfo(bool flag);
	void showAlignGuide(bool flag);

	IrCameraFrameCallback* getCallbackFunction(void);

	void saveImage(CString path = _T(""));

protected:
	struct RetinaCameraViewImpl;
	std::unique_ptr<RetinaCameraViewImpl> d_ptr;
	RetinaCameraViewImpl& getImpl(void) const;

	void callbackIrCameraFrame(unsigned char* data, unsigned int width, unsigned int height);
	void countFrameRate(void);

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};


