#pragma once

#include "OctSystem2.h"

#include <memory>

// ViewColorCamera

class ViewColorCamera : public CStatic
{
	DECLARE_DYNAMIC(ViewColorCamera)

public:
	ViewColorCamera();
	virtual ~ViewColorCamera();

	static const int IMAGE_WIDTH = 960;
	static const int IMAGE_HEIGHT = 750;

public:
	void updateWindow(void);
	void drawCircleGuide(CDC * pDC);
	void drawPixelCoords(CDC* pDC);
	void drawAlignGuide(CDC* pDC);
	void drawPixelsPitch(CDC* pDC);
	void drawImagePosition(CDC* pDC);

	void setViewSize(int width, int height);
	int getWidth(void) const;
	int getHeight(void) const;
	int getImageWidth(void) const;
	int getImageHeight(void) const;
	int getFrameWidth(void) const;
	int getFrameHeight(void) const;
	unsigned char* getFrameData(void) const;
	CppUtil::CvImage getOriginalImage(void) const;

	bool processWhiteBalance(float* param1, float* param2);
	void setWhiteBalanceParameters(float param1, float param2);
	bool useAutoWhiteBalance(bool isset = false, bool flag = false);
	bool useRadialColorCorrection(bool isset = false, bool flag = false);
	bool useRemoveReflectionLight(bool isset = false, bool flag = false, int lightRadius = 0, int shadowRadius = 0, int peripheryRadius = 0);
	void setOverBrightPixelThreshold(int threshold);
	bool useMaskROI(bool isset = false, bool flag = false, int x = 0, int y = 0, int radius = 0);
	bool useLiveMode(bool isset, bool flag = false);
	void removeReflectionLight(int centerRadius, int shadowRadius, int peripheryRadius);
	int calculateCenterMean(int lightRadius);
	int calculateOverBrightPixel(int lightRadius, int threshold);

	void setPixelCoords(std::vector<std::pair<unsigned int, unsigned int>> coords);
	void clearPixelCoords(void);
	void showAlignGuide(bool flag);
	void showCircleGuide(bool flag);

	bool loadFrameData(CString filename);
	bool saveFrameData(CString& filename);
	bool saveImage(CString& filename);
	bool saveCalibrationImage(CString& filename);

	void updateFrameImage(void);

	ColorCameraFrameCallback* getFrameCallbackFunction(void);
	ColorCameraImageCallback* getImageCallbackFunction(void);

protected:
	struct ViewColorCameraImpl;
	std::unique_ptr<ViewColorCameraImpl> d_ptr;
	ViewColorCameraImpl& getImpl(void) const;

	void callbackCameraImage(unsigned char* data, unsigned int width, unsigned int height, unsigned int frameCount);
	void callbackCameraFrame(unsigned char* data, unsigned int width, unsigned int height, unsigned int frameCount, unsigned int flipMode);

	void processCameraData(unsigned char* data, unsigned int width, unsigned int height, unsigned int flipMode);


protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

protected:

};


