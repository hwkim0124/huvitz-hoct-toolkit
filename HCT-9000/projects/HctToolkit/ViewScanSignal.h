#pragma once

#include "OctSystem2.h"

#include <memory>
#include <string>

// ViewScanSignal

class ViewScanSignal : public CStatic
{
	DECLARE_DYNAMIC(ViewScanSignal)

public:
	ViewScanSignal();
	virtual ~ViewScanSignal();

	enum class CaptureMode {
		REFER = 0,
		FOCUS, 
		POLAR
	};

public:
	void updateWindow(void);
	void drawIntensityGraph(CDC* pDC);
	void drawIntensityDataLine(CDC* pDC);

	void setDisplaySize(int width, int height);
	int getWidth(void) const;
	int getHeight(void) const;

	void setCaptureMode(CaptureMode mode);
	void setCaptureParams(int start, int istep, int count);
	void setChartPeakRange(int vmax, int vmin);
	void setChartPeakOffset(int offset1, int offset2);
	void setChartDataRange(int start, int close);
	void setDataMultiple(int multi);
	void setAverageSize(int size);
	void setFixedNoiseLevel(float level);

	void setChartPeak(bool flag);
	void setChartFWHM(bool flag);
	void setChartSNR(bool flag);

	void clearChartHistory(void);
	float getPeakTopValue(void);
	int getPeakIndexDiff(int value);

	IntensityDataCallback* getIntensityCallback(void) const;

protected:
	struct ViewScanSignalImpl;
	std::unique_ptr<ViewScanSignalImpl> d_ptr;
	ViewScanSignalImpl& getImpl(void) const;

	void callbackIntensityData(float* data, unsigned int width, unsigned int height);

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};


