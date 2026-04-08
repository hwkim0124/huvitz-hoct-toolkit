#pragma once

#include "OctSystem2.h"

#include <memory>

class ViewScrollColorCamera : public CScrollView
{
	DECLARE_DYNCREATE(ViewScrollColorCamera)

public:
	ViewScrollColorCamera();
	virtual ~ViewScrollColorCamera();

public:
	void updateWindow(void);
	void OnInitialUpdate();

	int getViewWidth(void);
	int getViewHeight(void);
	int getImageWidth(void);
	int getImageHeight(void);
	int getFrameWidth(void);

	void setViewWidth(int Width);
	void setViewHeight(int Height);
	void setInitSize(int Width, int Height);

	int getTimeCount(void);
	void setTimeCount(int t);

	void zoomIn(void);
	void zoomOut(void);

	//bool processWhiteBalance(float* param1, float* param2);
	void setWhiteBalanceParameters(float param1, float param2);
	bool useAutoWhiteBalance(bool isset = false, bool flag = false);
	bool useRadialColorCorrection(bool isset = false, bool flag = false);
	bool useMaskROI(bool isset = false, bool flag = false, int x = 0, int y = 0, int radius = 0);
	bool useLiveMode(bool isset, bool flag = false);

	void updateFrameImage(void);

	ColorCameraFrameCallback* getFrameCallbackFunction(void);
	ColorCameraImageCallback* getImageCallbackFunction(void);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void setScroll(void);

	int t;

protected:
	virtual void OnDraw(CDC* pDC);      // 이 뷰를 그리기 위해 재정의되었습니다.								
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	struct ViewScrollColorCameraImpl;
	std::unique_ptr<ViewScrollColorCameraImpl> d_ptr;
	ViewScrollColorCameraImpl& getImpl(void) const;

	void callbackCameraImage(unsigned char* data, unsigned int width, unsigned int height, unsigned int flipMode);
	void callbackCameraFrame(unsigned char* data, unsigned int width, unsigned int height, unsigned int frameCount, unsigned int flipMode);

	void processCameraData(unsigned char* data, unsigned int width, unsigned int height, unsigned int flipMode);

protected:
	DECLARE_MESSAGE_MAP()


};
