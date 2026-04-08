#pragma once

#include "OctSystem2.h"

#include <memory>
#include <algorithm>
#include <numeric>

// ViewScanPreview

class ViewScanPreview : public CStatic
{
	DECLARE_DYNAMIC(ViewScanPreview)

public:
	ViewScanPreview();
	virtual ~ViewScanPreview();

public:
	void updateWindow(void);
	void drawOverlayText(CDC* pDC);
	void drawPatternQualities(CDC* pDC);
	void drawAlignGuide(CDC* pDC);
	void drawEnfaceGuide(CDC* pDC);
	void drawPixelsPitch(CDC* pDC);
	void drawDecorrelations(CDC* pDC);
	void drawSlabLayers(CDC* pDC);
	void drawCorneaCenter(CDC* pDC);
	void drawAutoMeasureResult(CDC* pDC);

	void setDisplaySize(int width, int height);
	void setDecorrelationValues(float avg=0.0f, float stdev=0.0f, float maxv=0.0f);
	void setIntensityValues(float avg = 0.0f, float stdev = 0.0f, float maxv = 0.0f);
	void setSlabLayers(std::vector<int> upper = std::vector<int>(), std::vector<int> lower = std::vector<int>());

	int getWidth(void) const;
	int getHeight(void) const;
	int getImageWidth(void) const;
	int getImageHeight(void) const;
	int getSourceWidth(void) const;
	int getSourceHeight(void) const;
	double getRefractiveIndex(void) const;

	void showOverlayInfo(bool flag);
	void showAlignGuide(bool flag, float range = 6.0f);
	void overlayDecorrelations(bool flag);
	void showPatternQualities(bool flag);
	void showAutoMeasureResult(bool flag);

	void setAcquisitionTime(float acqTime);
	void setTriggerTimeStep(float timeStep);
	void setQualityIndex(float quality);
	void setDisplayStretched(bool flag);
	void setDecorrelationData(float* data);
	void setRefractiveIndex(double refractiveIndex);
	void setPatternQualities(int index, std::vector<float> stat);
	void clearResultData(bool cont = false);
	void setAxialLengths(std::vector<float> vect);
	void setLensThickness(std::vector<float> vect);
	void setAnteriorChamberDepths(std::vector<float> vect);
	void setCorneaCenterThickness(std::vector<float> vect);

	void drawScanImage(const OctScanImage* image, bool vflip = false);
	bool saveImage(CString& filename);
	bool saveImageOriginal(CString& filename);

	PreviewImageCallback2* getCallbackFunction(void);
	void callbackPreviewImage(const unsigned char* data, unsigned int width, 
		unsigned int height, float quality = 0.0f, float sigRatio = 0.0f,
		unsigned int refPoint = 0, unsigned int idxOfImage = 0);

protected:
	struct ViewScanPreviewImpl;
	std::unique_ptr<ViewScanPreviewImpl> d_ptr;
	ViewScanPreviewImpl& getImpl(void) const;


protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};


