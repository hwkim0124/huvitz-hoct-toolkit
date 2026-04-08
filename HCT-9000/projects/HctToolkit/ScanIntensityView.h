#pragma once

#include "OctSystem2.h"

#include <memory>
#include <string>

// ScanIntensityView

class ScanIntensityView : public CStatic
{
	DECLARE_DYNAMIC(ScanIntensityView)

public:
	ScanIntensityView();
	virtual ~ScanIntensityView();

	static const int DATA_WIDTH = SPECTRO_CALIB_BSCAN_WIDTH;
	static const int DATA_HEIGHT = SPECTRO_CALIB_FRAME_DATA_HEIGHT;
	static const int BUFFER_WIDTH = SPECTRO_CALIB_FRAME_DATA_WIDTH ;
	static const int BUFFER_SIZE = SPECTRO_CALIB_FRAME_DATA_SIZE;


public:
	void updateWindow(void);
	void drawIntensityGraph(CDC* pDC);
	void drawIntensityDataLine(CDC* pDC);
	void drawCompensatedDataLine(CDC* pDC);
	void drawBackgroundDataLine(CDC* pDC);
	void drawGraphText(CDC* pDC);

	void setDisplaySize(int width, int height);
	int getWidth(void) const;
	int getHeight(void) const;
	void setCurrentLine(int index);

	float* getIntensityLine(int index = (DATA_WIDTH/2));
	float* getIntensityData(void);
	float* getCompensatedLine(int index = (DATA_WIDTH/2));
	float* getCompensatedData(void);
	float* getBackgroundLine(void);

	void setIntensityLine(float* data, int index = (DATA_WIDTH / 2));
	void setCompensatedLine(float* data, int index = (DATA_WIDTH / 2));
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
	struct ScanIntensityViewImpl;
	std::unique_ptr<ScanIntensityViewImpl> d_ptr;
	ScanIntensityViewImpl& getImpl(void) const;

	void callbackIntensityData(float* data, unsigned int width, unsigned int height);

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};


