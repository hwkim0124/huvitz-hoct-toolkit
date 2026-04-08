#pragma once

#include "OctSystem2.h"
#include <memory>

// SpectrumView

class SpectrumView : public CStatic
{
	DECLARE_DYNAMIC(SpectrumView)

public:
	SpectrumView();
	virtual ~SpectrumView();


public:
	void updateWindow(void);
	void drawSpectrumGraph(CDC* pDC);
	void drawSpectrumDataLine(CDC* pDC);
	void drawResampleDataLine(CDC* pDC);

	void setDisplaySize(int width, int height);
	void setAverageSize(int size);

	int getWidth(void) const;
	int getHeight(void) const;

	unsigned short* getSpectrumData(void);
	float* getResampleData(void);
	float* getResampleData2(void);

	SpectrumDataCallback* getSpectrumCallback(void) const;
	ResampleDataCallback* getResampleCallback(void) const;

protected:
	struct SpectrumViewImpl;
	std::unique_ptr<SpectrumViewImpl> d_ptr;
	SpectrumViewImpl& getImpl(void) const;

	void callbackSpectrumData(unsigned short* data, unsigned int width, unsigned int height);
	void callbackResampleData(float* data, unsigned int width, unsigned int height);

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};


