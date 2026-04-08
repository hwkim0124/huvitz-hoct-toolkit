#pragma once

#include "OctSystem2.h"
#include <memory>

class ViewTopoPreview : public CStatic
{
	DECLARE_DYNAMIC(ViewTopoPreview)
public:
	ViewTopoPreview();
	virtual ~ViewTopoPreview();

protected:
	struct ViewTopoPreviewImpl;
	std::unique_ptr<ViewTopoPreviewImpl> d_ptr;
	ViewTopoPreviewImpl& getImpl(void) const;
	std::vector<int> m_layer;

public:
	void setDisplaySize(int width, int height);
	void setDisplayStretched(bool flag);
	void clearResultData(bool cont = false);
	void setAcquisitionTime(float acqTime);
	void setQualityIndex(float quality);
	void setPatternQualities(int index, std::vector<float> stat);
	void updateWindow(void);
	void drawOverlayText(CDC * pDC);
	void drawPatternQualities(CDC * pDC);
	void drawAlignGuide(CDC * pDC);
	void drawEnfaceGuide(CDC * pDC);
	void drawPixelsPitch(CDC * pDC);
	void drawSlabLayers(CDC * pDC);
	void drawScanImage(const OctScanImage * image, bool vflip = false);
	void drawSegmentLayer(CDC * pDC);
	void setLayerPoints(std::vector<int>& points);
	void clearLayerPoints(void);
	void setAverageCurvature(float value);
	void clearAverageCurvature(void);
	void clearHorizontalCurvature(void);
	void clearVerticalCurvature(void);
	void setMajorCurvature(float value);
	void setHorizontalCurvature(float value);
	void setVerticalCurvature(float value);
	float getHorizontalCurvature(void);
	float getVerticalCurvature(void);
	void clearMajorCurvature(void);
	void setMinorCurvature(float value);
	void clearMinorCurvature(void);
	void setMajorAxis(float value);
	void clearMajorAxis(void);
	void setMinorAxis(float value);
	void clearMinorAxis(void);
	void setImage(unsigned char * data, unsigned int width, unsigned int height, int sigCenter);
	int getWidth(void) const;
	int getHeight(void) const;
	int getImageWidth(void) const;
	int getImageHeight(void) const;
	int getSourceWidth(void) const;
	int getSourceHeight(void) const;
	void setSubGuidePos(int value);
	void setRangeGuideline(bool stat);
	void setCurveGuideline(bool stat);
};

