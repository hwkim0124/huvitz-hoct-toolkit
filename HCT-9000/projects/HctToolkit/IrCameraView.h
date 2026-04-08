#pragma once

#include "OctSystem2.h"
#include <memory>

// IrCameraView


class IrCameraView : public CStatic
{
	DECLARE_DYNAMIC(IrCameraView)

public:
	IrCameraView();
	virtual ~IrCameraView();


public:
	void updateWindow(void);
	void setOverlay(bool flag);
	void setDisplaySize(int width, int height);
	int getWidth(void) const;
	int getHeight(void) const;
	
	IrCameraFrameCallback* getCallbackFunction(void);


protected:
	struct IrCameraViewImpl;
	std::unique_ptr<IrCameraViewImpl> d_ptr;
	IrCameraViewImpl& getImpl(void) const;

	void callbackIrCamera(unsigned char* data, unsigned int width, unsigned int height);
	void countFrameRate(void);
	void drawOverlay(Gdiplus::Graphics& G);
	void drawOverlay(CDC *pDC);

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};


