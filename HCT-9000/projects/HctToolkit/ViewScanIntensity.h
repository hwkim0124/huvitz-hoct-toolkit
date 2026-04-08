#pragma once

#include "OctSystem2.h"

#include <memory>
#include <string>

// ViewScanIntensity

class ViewScanIntensity : public CStatic
{
	DECLARE_DYNAMIC(ViewScanIntensity)

public:
	ViewScanIntensity();
	virtual ~ViewScanIntensity();

	static const int FRAME_WIDTH = SPECTRO_CALIB_FRAME_DATA_WIDTH;
	static const int FRAME_HEIGHT = SPECTRO_CALIB_FRAME_DATA_HEIGHT;
	static const int FRAME_SIZE = FRAME_WIDTH * FRAME_HEIGHT;

	const float SNR_RATIO_MAX = 40.0f;
	const int IMAGE_ROW_OFFSET = 24;
	const int HISTOGRAM_BINS = 512;
	const int HISTOGRAM_UNIT = 20;
	const float SIGNAL_RATIO = 0.005f;
	const float NOISE_RATIO = 0.45f;

	enum class DisplayMode {
		INTENSITY,
		HISTOGRAM,
		SNR_RATIO
	};


public:
	void updateWindow(void);
	void drawIntensityGraph(CDC* pDC);
	void drawIntensityDataLine(CDC* pDC);
	void drawCompensatedDataLine(CDC* pDC);
	void drawBackgroundDataLine(CDC* pDC);
	void drawGraphText(CDC* pDC);

	void drawSnrRatioGraph(CDC* pDC);
	void drawSnrRatioLine(CDC* pDC);
	void drawSnrRatioText(CDC* pDC);

	void setDisplaySize(int width, int height);
	void setDisplayMode(DisplayMode mode);
	int getWidth(void) const;
	int getHeight(void) const;
	void setCurrentLine(int index);

	float* getIntensityLine(int index = (FRAME_WIDTH /2));
	float* getIntensityData(void);
	float* getCompensatedLine(int index = (FRAME_WIDTH /2));
	float* getCompensatedData(void);
	float* getBackgroundLine(void);

	void setIntensityLine(float* data, int index = (FRAME_WIDTH / 2));
	void setCompensatedLine(float* data, int index = (FRAME_WIDTH / 2));
	void setBackgroundLine(float* data = nullptr);

	bool evaluateIntensityLine(CString& text);

	bool saveIntensityData(const std::wstring& path);
	bool loadIntensityData(const std::wstring& path);
	bool saveCompensatedData(const std::wstring& path);
	bool loadCompensatedData(const std::wstring& path);
	bool saveBackgroundData(const std::wstring& path);
	bool loadBackgroundData(const std::wstring& path);

	IntensityDataCallback* getIntensityCallback(void) const;

protected:
	struct ViewScanIntensityImpl;
	std::unique_ptr<ViewScanIntensityImpl> d_ptr;
	ViewScanIntensityImpl& getImpl(void) const;

	void callbackIntensityData(float* data, unsigned int width, unsigned int height);

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};


