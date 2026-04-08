#pragma once

#include "OctSystem2.h"
#include <memory>

// ScanImageView

class ScanImageView : public CStatic
{
	DECLARE_DYNAMIC(ScanImageView)

public:
	ScanImageView();
	virtual ~ScanImageView();

public:
	void updateWindow(void);
	void drawOverlayText(CDC* pDC);

	void setDisplaySize(int width, int height);
	int getWidth(void) const;
	int getHeight(void) const;

	PreviewImageCallback* getCallbackFunction(void);


protected:
	struct ScanImageViewImpl;
	std::unique_ptr<ScanImageViewImpl> d_ptr;
	ScanImageViewImpl& getImpl(void) const;

	void callbackPreviewImage(unsigned char* data, unsigned int width, unsigned int height, float quality);


protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};


