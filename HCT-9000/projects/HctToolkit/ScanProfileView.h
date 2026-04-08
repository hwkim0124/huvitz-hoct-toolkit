#pragma once

#include "OctSystem2.h"

#include <memory>
#include <vector>

// ScanProfileView

class ScanProfileView : public CStatic
{
	DECLARE_DYNAMIC(ScanProfileView)

public:
	ScanProfileView();
	virtual ~ScanProfileView();

public:
	void updateWindow(void);

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

	int getWidth(void) const;
	int getHeight(void) const;

	SpectrumDataCallback* getCallbackFunction(void);

protected:
	void drawProfileGraph(CDC* pDC);
	void drawProfileLines(CDC* pDC);
	void drawProfilePeaks(CDC* pDC);

protected:
	struct ScanProfileViewImpl;
	std::unique_ptr<ScanProfileViewImpl> d_ptr;
	ScanProfileViewImpl& getImpl(void) const;

	void callbackProfileImage(unsigned short* data, unsigned int width, unsigned int height);

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};


