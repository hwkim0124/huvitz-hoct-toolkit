// ScanProfileView.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ScanProfileView.h"

#include <vector>
#include <array>
#include <set>

#include "CppUtil2.h"

using namespace std;
using namespace CppUtil;


struct ScanProfileView::ScanProfileViewImpl
{
	CvImage image;
	int width;
	int height;

	int threshPeak;
	int threshSubs;

	set<int> indexSet;
	vector<array<unsigned short, LINE_CAMERA_CCD_PIXELS>> profiles;
	vector<int> peakIndexs;
	vector<int> peakValues;

	SpectrumDataCallback callback;

	ScanProfileViewImpl() : threshPeak(0), threshSubs(0)
	{

	}
};

// ScanProfileView

IMPLEMENT_DYNAMIC(ScanProfileView, CStatic)

ScanProfileView::ScanProfileView()
	: d_ptr(make_unique<ScanProfileViewImpl>())
{
	getImpl().callback = std::bind(&ScanProfileView::callbackProfileImage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}


ScanProfileView::~ScanProfileView()
{
}


void ScanProfileView::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	// pMemDC->SetMapMode(MM_ANISOTROPIC);
	// pMemDC->SetViewportExt(getWidth(), getHeight());
	// pMemDC->SetWindowExt(LINE_CAMERA_CCD_PIXELS, LINE_CAMERA_PIXEL_VALUE_MAX);


	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, getWidth(), getHeight());
	CBitmap* oldBmp = pMemDC->SelectObject(&bitmap);

	drawProfileGraph(pMemDC);
	drawProfileLines(pMemDC);
	drawProfilePeaks(pMemDC);

	BitBlt(hDC, 0, 0, getWidth(), getHeight(), hMemDC, 0, 0, SRCCOPY);
	pMemDC->SelectObject(oldBmp);

	bitmap.DeleteObject();
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	return;
}


void ScanProfileView::drawProfileGraph(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();
	int ratioX = LINE_CAMERA_CCD_PIXELS / width;
	int ratioY = LINE_CAMERA_PIXEL_VALUE_MAX / height;

	CRect rect(0, 0, width, height);
	pDC->FillSolidRect(&rect, RGB(0, 0, 0));

	CPen pen, *old;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 32, 32));
	old = pDC->SelectObject(&pen);

	pDC->MoveTo(0, height - getImpl().threshPeak / ratioY);
	pDC->LineTo(width, height - getImpl().threshPeak / ratioY);
	pDC->MoveTo(0, height - getImpl().threshSubs / ratioY);
	pDC->LineTo(width, height - getImpl().threshSubs / ratioY);

	pDC->SelectObject(old);
	pen.DeleteObject();
	return;
}


void ScanProfileView::drawProfileLines(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();
	int ratioX = LINE_CAMERA_CCD_PIXELS / width;
	int ratioY = LINE_CAMERA_PIXEL_VALUE_MAX / height;

	size_t size = getImpl().profiles.size();
	for (int k = 0; k < size; k++) {
		CPen pen, *old;
		pen.CreatePen(PS_SOLID, 1, RGB((k / 10) * 25, 255 - (k < 5 ? k : 0) * 25, (k / 5) * 25));
		old = pDC->SelectObject(&pen);

		unsigned short* buff = getImpl().profiles[k].data();
		pDC->MoveTo(0, height - (buff[0] / ratioY));

		int sizeX = LINE_CAMERA_CCD_PIXELS / ratioX;
		for (int j = 1; j < sizeX; j++) {
			pDC->LineTo(j, height - (buff[j * ratioX] / ratioY));
		}

		pDC->SelectObject(old);
		pen.DeleteObject();
	}

	return;
}


void ScanProfileView::drawProfilePeaks(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();
	int ratioX = LINE_CAMERA_CCD_PIXELS / width;
	int ratioY = LINE_CAMERA_PIXEL_VALUE_MAX / height;

	CPen pen, *old;
	pen.CreatePen(PS_SOLID, 1, RGB(32, 32, 255));
	old = pDC->SelectObject(&pen);

	size_t size = getImpl().peakIndexs.size();
	for (int k = 0; k < size; k++) {
		int posX = getImpl().peakIndexs[k] / ratioX;
		int posY = height - getImpl().peakValues[k] / ratioY;
		pDC->MoveTo(posX - 3, posY - 3);
		pDC->LineTo(posX + 3, posY + 3);
		pDC->MoveTo(posX + 3, posY - 3);
		pDC->LineTo(posX - 3, posY + 3);
	}

	pDC->SelectObject(old);
	pen.DeleteObject();
	return;
}


void ScanProfileView::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


void ScanProfileView::setThresholdPeak(int level)
{
	getImpl().threshPeak = level;
	return;
}


void ScanProfileView::setThresholdSubs(int level)
{
	getImpl().threshSubs = level;
	return;
}


void ScanProfileView::insertProfile(int index)
{
	getImpl().indexSet.insert(index);
	return;
}


void ScanProfileView::removeProfile(int index)
{
	getImpl().indexSet.erase(index);
	return;
}


void ScanProfileView::clearProfiles(void)
{
	getImpl().indexSet.clear();
	return;
}


unsigned short * ScanProfileView::getProfile(int index)
{
	int count = 0;
	for (auto item : getImpl().indexSet) {
		if (item == index) {
			return getImpl().profiles[count].data();
		}
		count++;
	}
	return nullptr;
}


int ScanProfileView::getThresholdPeak(void) const
{
	return getImpl().threshPeak;
}


int ScanProfileView::getThresholdSubs(void) const
{
	return getImpl().threshSubs;
}


void ScanProfileView::clearPeaks(void)
{
	getImpl().peakIndexs.clear();
	getImpl().peakValues.clear();
	return;
}


void ScanProfileView::setPeakIndexs(const std::vector<int>& indexs)
{
	getImpl().peakIndexs.clear();
	getImpl().peakIndexs.assign(indexs.cbegin(), indexs.cend());
	return;
}


void ScanProfileView::setPeakValues(const std::vector<int>& values)
{
	getImpl().peakValues.clear();
	getImpl().peakValues.assign(values.cbegin(), values.cend());
	return;
}


int ScanProfileView::getWidth(void) const
{
	return getImpl().width;
}


int ScanProfileView::getHeight(void) const
{
	return getImpl().height;
}


SpectrumDataCallback * ScanProfileView::getCallbackFunction(void)
{
	return &getImpl().callback;
}


ScanProfileView::ScanProfileViewImpl & ScanProfileView::getImpl(void) const
{
	return *d_ptr;
}


void ScanProfileView::callbackProfileImage(unsigned short * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	size_t size = getImpl().indexSet.size();
	getImpl().profiles.resize(size);
	int count = 0;
	for (auto index : getImpl().indexSet) {
		if (index >= 0 && index < (int)height) {
			memcpy((void*)getImpl().profiles[count].data(), (void*)(data + index*width), sizeof(unsigned short)*LINE_CAMERA_CCD_PIXELS);
			count++;
		}
	}

	updateWindow();

	// DebugOut2() << "Profile image, width: " << width << ", height: " << height;
	return;
}


BEGIN_MESSAGE_MAP(ScanProfileView, CStatic)
END_MESSAGE_MAP()



// ScanProfileView message handlers




void ScanProfileView::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{

	// TODO:  Add your code to draw the specified item
}
