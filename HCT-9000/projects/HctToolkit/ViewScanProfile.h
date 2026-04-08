#pragma once

#include "OctSystem2.h"


#include <memory>
#include <vector>

// ViewScanProfile

class ViewScanProfile : public CStatic
{
	DECLARE_DYNAMIC(ViewScanProfile)

public:
	ViewScanProfile();
	virtual ~ViewScanProfile();

public:
	void updateWindow(void);
	bool saveImage(CString& filename);

	void setDisplaySize(int width, int height);
	void setThresholdPeak(int level);
	void setThresholdSubs(int level);
	void insertProfile(int index);
	void removeProfile(int index);
	void clearProfiles(void);

	unsigned short* getProfile(int index);
	int getThresholdPeak(void) const;
	int getThresholdSubs(void) const;

	void setPeakIndexs(const std::vector<int>& indexs);
	void setPeakValues(const std::vector<int>& values);
	void clearPeaks(void);

	void setCalibrationPoints(const std::vector<int>& points);
	void clearCalibrationPoints(void);
	void setResamplingParameters(double* params);

	int getWidth(void) const;
	int getHeight(void) const;

	void setCenterX(int x);
	void setRangeX(int x1, int x2);
	void setRangeY(int y1, int y2);
	void clearGraphInfo(void);
	void setShowGraphInfo(bool flag);
	void setRecordValues(bool flag);
	void setIsProfileEnds(bool flag);

	SpectrumDataCallback* getCallbackFunction(void);

protected:
	void drawProfileGraph(CDC* pDC);
	void drawProfileEnds(CDC* pDC);
	void drawProfileText(CDC* pDC);
	void drawProfileLines(CDC* pDC);
	void drawProfilePeaks(CDC* pDC);
	void drawCalibrationLines(CDC* pDC);
	
	void analyzeProfile(void);

protected:
	struct ViewScanProfileImpl;
	std::unique_ptr<ViewScanProfileImpl> d_ptr;
	ViewScanProfileImpl& getImpl(void) const;

public:
	void callbackProfileImage(unsigned short* data, unsigned int width, unsigned int height);

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};


