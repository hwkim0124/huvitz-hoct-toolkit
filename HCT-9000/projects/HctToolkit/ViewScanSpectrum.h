#pragma once

#include "OctSystem2.h"

#include <memory>
#include <string>

// ViewScanSpectrum

class ViewScanSpectrum : public CStatic
{
	DECLARE_DYNAMIC(ViewScanSpectrum)

public:
	ViewScanSpectrum();
	virtual ~ViewScanSpectrum();

	static const int DATA_WIDTH = LINE_CAMERA_CCD_PIXELS;
	static const int DATA_HEIGHT = SPECTRO_CALIB_BSCAN_WIDTH;
	static const int BUFFER_SIZE = (DATA_WIDTH * DATA_HEIGHT);

public:
	void updateWindow(void);
	void drawSpectrumGraph(CDC* pDC);
	void drawSpectrumDataLine(CDC* pDC);
	void drawResampleDataLine(CDC* pDC);
	void drawGraphText(CDC* pDC);

	void setDisplaySize(int width, int height);
	void setAverageSize(int size);
	int getAverageSize(void);

	int getWidth(void) const;
	int getHeight(void) const;

	unsigned short* getSpectrumLine(void);
	unsigned short* getSpectrumData(void);
	float* getResampleLine(void);
	float* getResampleData(void);

	int getDataWidth(void);
	int getDataHeight(void);

	void initSpectrumLine(unsigned short* data, int index);
	void initResampleLine(float* data, int index);

	bool saveSpectrumData(const std::wstring& path);
	bool loadSpectrumData(const std::wstring& path);
	bool saveResampleData(const std::wstring& path);
	bool loadResampleData(const std::wstring& path);

	SpectrumDataCallback* getSpectrumCallback(void) const;
	ResampleDataCallback* getResampleCallback(void) const;

protected:
	struct ViewScanSpectrumImpl;
	std::unique_ptr<ViewScanSpectrumImpl> d_ptr;
	ViewScanSpectrumImpl& getImpl(void) const;

	void callbackSpectrumData(unsigned short* data, unsigned int width, unsigned int height);
	void callbackResampleData(float* data, unsigned int width, unsigned int height);

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};


