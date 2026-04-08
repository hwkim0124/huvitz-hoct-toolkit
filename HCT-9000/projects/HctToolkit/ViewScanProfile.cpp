// ViewScanProfile.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ViewScanProfile.h"

#include <vector>
#include <array>
#include <set>

#include "OctSystem2.h"
#include "CppUtil2.h"
#include "SigProc2.h"

using namespace std;
using namespace CppUtil;
using namespace SigProc;
using namespace OctSystem;


struct ViewScanProfile::ViewScanProfileImpl
{
	CvImage image;
	int width;
	int height;

	int threshPeak;
	int threshSubs;

	int rangeX[2] = { 0, LINE_CAMERA_CCD_PIXELS-1 };
	int rangeY[2] = { 0, LINE_CAMERA_PIXEL_VALUE_MAX-1 };
	int centerX = LINE_CAMERA_CCD_PIXELS / 2;

	int peakIdx;
	float peakVal, peakMax, peakMin, peakAvg, peakSum;
	float fwhmVal, fwhmMax, fwhmX1, fwhmX2;
	float fwhmValW, fwhmMaxW;
	float pixelSum, pixelMax;
	bool showText;
	bool recordVals;
	int recordCnt;
	bool isProfileEnds;
	float headValue[2];
	float tailValue[2];

	Resampler resampler;

	set<int> indexSet;
	vector<array<unsigned short, LINE_CAMERA_CCD_PIXELS>> profiles;
	vector<int> peakIndexs;
	vector<int> peakValues;
	vector<int> calibPoints;

	SpectrumDataCallback callback;

	ViewScanProfileImpl() : threshPeak(0), threshSubs(0), peakIdx(0), peakVal(0.0f), peakMax(0.0f), peakMin(99999.0f),
		peakAvg(0.0f), peakSum(0.0f),
		fwhmVal(0.0f), fwhmMax(0.0f), fwhmX1(0.0f), fwhmX2(0.0f),
		fwhmValW(0.0f), fwhmMaxW(0.0f),
		pixelSum(0.0f), pixelMax(0.0f),
		showText(true), recordVals(true), recordCnt(0),
		isProfileEnds(false)
	{

	}
};

// ViewScanProfile

IMPLEMENT_DYNAMIC(ViewScanProfile, CStatic)

ViewScanProfile::ViewScanProfile()
	: d_ptr(make_unique<ViewScanProfileImpl>())
{
	getImpl().callback = std::bind(&ViewScanProfile::callbackProfileImage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}


ViewScanProfile::~ViewScanProfile()
{
}


void ViewScanProfile::updateWindow(void)
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
	drawProfileText(pMemDC);
	drawProfileLines(pMemDC);
	drawProfileEnds(pMemDC);
	drawProfilePeaks(pMemDC);
	drawCalibrationLines(pMemDC);


	BitBlt(hDC, 0, 0, getWidth(), getHeight(), hMemDC, 0, 0, SRCCOPY);
	pMemDC->SelectObject(oldBmp);

	bitmap.DeleteObject();
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	return;
}

bool ViewScanProfile::saveImage(CString& filename)
{
	CString dirname = _T(".//export");
	CreateDirectory(dirname, NULL);

	CString path = filename;
	if (path.IsEmpty()) {
		CTime time = CTime::GetCurrentTime();
		path = time.Format(_T("PROFILE_%y%m%d_%H%M%S.png"));
	}

	CRect rect;
	GetClientRect(&rect);
	int width = rect.Width();
	int height = rect.Height();
	if (width <= 0 || height <= 0) {
		return false;
	}

	CImage image;
	if (FAILED(image.Create(width, height, 32))) {
		return false;
	}

	CClientDC dc(this);
	HDC hMemDC = image.GetDC();
	BitBlt(hMemDC, 0, 0, width, height, dc.GetSafeHdc(), 0, 0, SRCCOPY);
	image.ReleaseDC();

	path = dirname + _T("//") + path;
	filename = path;
	HRESULT hr = image.Save(path);

	image.Destroy();
	return (hr == S_OK);
}


void ViewScanProfile::drawProfileGraph(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();
	float ratioX = (float)(d_ptr->rangeX[1] - d_ptr->rangeX[0] + 1) / width;
	float ratioY = (float)(d_ptr->rangeY[1] - d_ptr->rangeY[0] + 1) / height;

	if (ratioX <= 0 || ratioY <= 0) {
		return;
	}

	CRect rect(0, 0, width, height);
	pDC->FillSolidRect(&rect, RGB(0, 0, 0));

	CPen pen, pen2, pen3, *old;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 32, 32));
	pen2.CreatePen(PS_SOLID, 1, RGB(32, 255, 32));
	pen3.CreatePen(PS_SOLID, 1, RGB(32, 32, 255));
	old = pDC->SelectObject(&pen);

	int y_val, y_pos;
	y_val = getImpl().threshPeak - d_ptr->rangeY[0];
	y_pos = (int)(height - y_val / ratioY);
	pDC->MoveTo(0, y_pos);
	pDC->LineTo(width - 1, y_pos);

	y_val = getImpl().threshSubs - d_ptr->rangeY[0];
	y_pos = (int)(height - y_val / ratioY);
	pDC->MoveTo(0, y_pos);
	pDC->LineTo(width - 1, y_pos);

	int x_val, x_pos;
	x_val = LINE_CAMERA_CCD_PIXELS / 2 - d_ptr->rangeX[0];
	x_pos = (int)(x_val / ratioX);

	pDC->SelectObject(&pen2);
	pDC->MoveTo(x_pos, 0);
	pDC->LineTo(x_pos, height - 1);

	// x_val = LINE_CAMERA_CCD_PIXELS/2 - d_ptr->rangeX[0];
	x_val = d_ptr->centerX - d_ptr->rangeX[0];
	x_pos = (int)(x_val / ratioX);

	pDC->SelectObject(&pen3);
	pDC->MoveTo(x_pos, 0);
	pDC->LineTo(x_pos, height - 1);

	pDC->SelectObject(old);
	pen.DeleteObject();
	pen2.DeleteObject();
	return;
}

void ViewScanProfile::drawProfileEnds(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();
	int sizeX = d_ptr->rangeX[1] - d_ptr->rangeX[0] + 1;
	int sizeY = d_ptr->rangeY[1] - d_ptr->rangeY[0] + 1;


	if (getImpl().isProfileEnds == false) {
		return;
	}
	if (sizeX <= 0 || sizeY <= 0) {
		return;
	}

	float ratioX = (float)width / (float)sizeX;
	float ratioY = (float)sizeY / (float)height;

	if (ratioX <= 0 || ratioY <= 0) {
		return;
	}

	CPen pen, *old;
	pen.CreatePen(PS_DOT, 1, RGB(32, 32, 160));
	old = pDC->SelectObject(&pen);

	if (ratioX >= 8) {
		for (int i = 1; i < sizeX; i++) {
			auto x = (int)(i * ratioX);
			pDC->MoveTo(x, 0);
			pDC->LineTo(x, height - 1);
		}
	}

	CFont font;
	font.CreateFontW(18, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);
	pDC->SetTextColor(RGB(0, 128, 255));
	pDC->SetBkMode(TRANSPARENT);

	CString str;
	CRect rect;
	
	if (d_ptr->rangeX[1] < 1024) {
		auto pos1 = 0; // d_ptr->rangeX[0];
		auto pos2 = 1; // d_ptr->rangeX[1];
		auto val1 = d_ptr->headValue[0];
		auto val2 = d_ptr->headValue[1];
		auto diff = val2 - val1;
		str.Format(_T("Head Diff: %.0f (%d: %.0f, %d: %.0f)"), diff, (int)pos1, val1, (int)pos2, val2);
	}
	else {
		auto pos1 = 2046; // d_ptr->rangeX[0];
		auto pos2 = 2047; // d_ptr->rangeX[1];
		auto val1 = d_ptr->tailValue[0];
		auto val2 = d_ptr->tailValue[1];
		auto diff = val2 - val1;
		str.Format(_T("Tail Diff: %.0f (%d: %.0f, %d: %.0f)"), diff, (int)pos1, val1, (int)pos2, val2);
	}

	rect = CRect(20, 20, 300, 40);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(old);
	pen.DeleteObject();
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewScanProfile::drawProfileText(CDC * pDC)
{
	if (!d_ptr->showText) {
		return;
	}

	CFont font;
	font.CreateFontW(18, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);
	pDC->SetTextColor(RGB(0, 128, 255));
	pDC->SetBkMode(TRANSPARENT);

	CString str;
	CRect rect;

	str.Format(_T("Peak value: %.0f, max: %.0f, min: %.0f, avg: %.0f"), d_ptr->peakVal, d_ptr->peakMax, d_ptr->peakMin, d_ptr->peakAvg);
	rect = CRect(20, 80, 400, 100);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	str.Format(_T("Peak index: %d"), d_ptr->peakIdx);
	rect = CRect(20, 100, 400, 120);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	str.Format(_T("FWHM value: %.2f (%.2f), max: %.2f (%.2f)"), d_ptr->fwhmVal, d_ptr->fwhmValW, d_ptr->fwhmMax, d_ptr->fwhmMaxW);
	rect = CRect(20, 120, 400, 140);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	str.Format(_T("FWHM range: %.2f, %.2f"), d_ptr->fwhmX1, d_ptr->fwhmX2);
	rect = CRect(20, 140, 400, 160);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	str.Format(_T("Pixel sum: %.0f, max: %.0f"), d_ptr->pixelSum, d_ptr->pixelMax);
	rect = CRect(20, 160, 400, 180);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewScanProfile::drawProfileLines(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();
	float ratioX = (float)(d_ptr->rangeX[1] - d_ptr->rangeX[0] + 1) / width;
	float ratioY = (float)(d_ptr->rangeY[1] - d_ptr->rangeY[0] + 1) / height;

	if (ratioX <= 0 || ratioY <= 0) {
		return;
	}

	int d1_idx, d2_idx;
	int y1_val, y1_pos;
	int y2_val, y2_pos;
	size_t size = getImpl().profiles.size();
	for (int k = 0; k < size; k++) {
		CPen pen, *old;
		pen.CreatePen(PS_SOLID, 1, RGB((k / 10) * 25, 255 - (k < 5 ? k : 0) * 25, (k / 5) * 25));
		old = pDC->SelectObject(&pen);

		unsigned short* buff = getImpl().profiles[k].data();
		d1_idx = d_ptr->rangeX[0];

		y1_val = buff[d1_idx] - d_ptr->rangeY[0];
		y1_pos = (int)(height - y1_val / ratioY);
		pDC->MoveTo(0, y1_pos);

		int sizeX = width;
		for (int j = 1; j < sizeX; j++) {
			d2_idx = d1_idx + (int)(j * ratioX);
			y2_val = buff[d2_idx] - d_ptr->rangeY[0];
			y2_pos = (int)(height - y2_val / ratioY);
			pDC->LineTo(j, y2_pos);
		}

		pDC->SelectObject(old);
		pen.DeleteObject();
	}

	return;
}


void ViewScanProfile::drawProfilePeaks(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();
	float ratioX = (float)(d_ptr->rangeX[1] - d_ptr->rangeX[0] + 1) / width;
	float ratioY = (float)(d_ptr->rangeY[1] - d_ptr->rangeY[0] + 1) / height;

	if (ratioX <= 0 || ratioY <= 0) {
		return;
	}

	CPen pen, *old;
	pen.CreatePen(PS_SOLID, 1, RGB(192, 192, 255));
	old = pDC->SelectObject(&pen);

	CFont font;
	font.CreateFontW(18, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);
	pDC->SetTextColor(RGB(255, 128, 128));
	pDC->SetBkMode(TRANSPARENT);

	size_t size = getImpl().peakIndexs.size();
	for (int k = 0; k < size; k++) {
		int valX = getImpl().peakIndexs[k] - d_ptr->rangeX[0];
		int valY = getImpl().peakValues[k] - d_ptr->rangeY[0];
		int posX = (int)(valX / ratioX);
		int posY = (int)(height - valY / ratioY);
		pDC->MoveTo(posX - 5, posY - 5);
		pDC->LineTo(posX + 5, posY + 5);
		pDC->MoveTo(posX + 5, posY - 5);
		pDC->LineTo(posX - 5, posY + 5);

		posY = (posY <= 25 ? 25 : posY);
		CRect rect = CRect(posX - 50, posY - 25, posX + 50, posY - 3);
		float index = (float)getImpl().peakIndexs[k];
		float data = (float)getImpl().resampler.getWavelengthResampledAtPixelPosition(index);
		CString str;
		str.Format(_T("%.2f"), data);
		pDC->DrawTextW(str, &rect, DT_CENTER | DT_SINGLELINE);
	}

	pDC->SelectObject(old);
	pen.DeleteObject();
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewScanProfile::drawCalibrationLines(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();
	float ratioX = (float)(d_ptr->rangeX[1] - d_ptr->rangeX[0] + 1) / width;
	float ratioY = (float)(d_ptr->rangeY[1] - d_ptr->rangeY[0] + 1) / height;

	if (ratioX <= 0 || ratioY <= 0) {
		return;
	}

	CPen pen, *old;
	pen.CreatePen(PS_DOT, 1, RGB(255, 64, 64));
	old = pDC->SelectObject(&pen);

	size_t size = getImpl().calibPoints.size();
	for (int k = 0; k < size; k++) {
		int valX = getImpl().calibPoints[k] - d_ptr->rangeX[0];
		int posX = (int)(valX / ratioX);
		pDC->MoveTo(posX, 0);
		pDC->LineTo(posX, height - 1);
	}

	pDC->SelectObject(old);
	pen.DeleteObject();
	return;
}


void ViewScanProfile::analyzeProfile(void)
{
	if (getImpl().profiles.size() <= 0) {
		return;
	}

	unsigned short* buff = getImpl().profiles[0].data();
	int kernel = 1; // 5;

	int size = LINE_CAMERA_CCD_PIXELS;
	float data[LINE_CAMERA_CCD_PIXELS] = { 0.0f };

	float ksum;
	for (int i = 0; i < size; i++) {
		ksum = 0.0f;
		for (int j = -kernel / 2; j <= +kernel / 2; j++) {
			int k = min(max(i + j, 0), size-1);
			ksum += buff[k];
		}
		data[i] = ksum / kernel;
	}

	int pidx = 0;
	float pval = 0;
	for (int i = 0; i < size; i++) {
		if (pval < data[i]) {
			pval = data[i];
			pidx = i;
		}
	}

	float half = pval / 2.0f;
	float fwhm_x1 = 0.0f;
	float fwhm_x2 = size - 1.0f;

	for (int i = pidx; i >= 0; i--) {
		if (data[i] < half) {
			fwhm_x1 = i + (half - data[i]) / (data[i + 1] - data[i]);
			break;
		}
	}

	for (int i = pidx; i < size; i++) {
		if (data[i] < half) {
			fwhm_x2 = i - (half - data[i]) / (data[i - 1] - data[i]);
			break;
		}
	}

	float tot_sum = 0.0f;
	for (int i = 0; i < size; i++) {
		tot_sum += buff[i];
	}

	d_ptr->peakIdx = pidx;
	d_ptr->peakVal = pval;
	d_ptr->fwhmVal = fwhm_x2 - fwhm_x1;
	d_ptr->fwhmX1 = fwhm_x1;
	d_ptr->fwhmX2 = fwhm_x2;
	d_ptr->pixelSum = tot_sum;

	double w1 = getImpl().resampler.getWavelengthResampledAtPixelPosition(fwhm_x1);
	double w2 = getImpl().resampler.getWavelengthResampledAtPixelPosition(fwhm_x2);
	d_ptr->fwhmValW = (float)(w2 - w1);

	if (d_ptr->recordVals) {
		d_ptr->peakMax = max(d_ptr->peakMax, d_ptr->peakVal);
		d_ptr->peakMin = min(d_ptr->peakMin, d_ptr->peakVal);
		d_ptr->fwhmMax = max(d_ptr->fwhmMax, d_ptr->fwhmVal);
		d_ptr->fwhmMaxW = max(d_ptr->fwhmMaxW, d_ptr->fwhmValW);
		d_ptr->pixelMax = max(d_ptr->pixelMax, d_ptr->pixelSum);
	}

	d_ptr->peakSum += d_ptr->peakVal;
	if (++d_ptr->recordCnt % 100 == 0) {
		d_ptr->peakAvg = d_ptr->peakSum / 100.0f;
		d_ptr->peakSum = 0.0f;
	}

	d_ptr->headValue[0] = data[0];
	d_ptr->headValue[1] = data[1];
	d_ptr->tailValue[0] = data[LINE_CAMERA_CCD_PIXELS-2];
	d_ptr->tailValue[1] = data[LINE_CAMERA_CCD_PIXELS-1];
	return;
}


void ViewScanProfile::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


void ViewScanProfile::setThresholdPeak(int level)
{
	getImpl().threshPeak = level;
	return;
}


void ViewScanProfile::setThresholdSubs(int level)
{
	getImpl().threshSubs = level;
	return;
}


void ViewScanProfile::insertProfile(int index)
{
	getImpl().indexSet.insert(index);
	return;
}


void ViewScanProfile::removeProfile(int index)
{
	getImpl().indexSet.erase(index);
	return;
}


void ViewScanProfile::clearProfiles(void)
{
	getImpl().indexSet.clear();
	return;
}


unsigned short * ViewScanProfile::getProfile(int index)
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


int ViewScanProfile::getThresholdPeak(void) const
{
	return getImpl().threshPeak;
}


int ViewScanProfile::getThresholdSubs(void) const
{
	return getImpl().threshSubs;
}


void ViewScanProfile::clearPeaks(void)
{
	getImpl().peakIndexs.clear();
	getImpl().peakValues.clear();
	return;
}


void ViewScanProfile::setCalibrationPoints(const std::vector<int>& points)
{
	getImpl().calibPoints.clear();
	getImpl().calibPoints.assign(points.cbegin(), points.cend());
	return;
}


void ViewScanProfile::clearCalibrationPoints(void)
{
	getImpl().calibPoints.clear();
	return;
}


void ViewScanProfile::setResamplingParameters(double * params)
{
	getImpl().resampler.setWavelengthFunctionCoefficients(params);
	LogD() << "Resample: " << getImpl().resampler.getWavelengthFunctionCoefficient(0);
	LogD() << "Resample: " << getImpl().resampler.getWavelengthFunctionCoefficient(1);
	LogD() << "Resample: " << getImpl().resampler.getWavelengthFunctionCoefficient(2);
	LogD() << "Resample: " << getImpl().resampler.getWavelengthFunctionCoefficient(3);
	return;
}


void ViewScanProfile::setPeakIndexs(const std::vector<int>& indexs)
{
	getImpl().peakIndexs.clear();
	getImpl().peakIndexs.assign(indexs.cbegin(), indexs.cend());
	return;
}


void ViewScanProfile::setPeakValues(const std::vector<int>& values)
{
	getImpl().peakValues.clear();
	getImpl().peakValues.assign(values.cbegin(), values.cend());
	return;
}


int ViewScanProfile::getWidth(void) const
{
	return getImpl().width;
}


int ViewScanProfile::getHeight(void) const
{
	return getImpl().height;
}


void ViewScanProfile::setCenterX(int x)
{
	d_ptr->centerX = min(max(x, 0), LINE_CAMERA_CCD_PIXELS-1);
	return;
}


void ViewScanProfile::setRangeX(int x1, int x2)
{
	if (x1 < x2) {
		d_ptr->rangeX[0] = min(max(x1, 0), LINE_CAMERA_CCD_PIXELS-1);
		d_ptr->rangeX[1] = min(max(x2, 0), LINE_CAMERA_CCD_PIXELS-1);
	}
	return;
}


void ViewScanProfile::setRangeY(int y1, int y2)
{
	if (y1 < y2) {
		d_ptr->rangeY[0] = min(max(y1, 0), (int)LINE_CAMERA_PIXEL_VALUE_MAX-1);
		d_ptr->rangeY[1] = min(max(y2, 0), (int)LINE_CAMERA_PIXEL_VALUE_MAX-1);
	}
	return;
}


void ViewScanProfile::clearGraphInfo(void)
{
	d_ptr->peakIdx = 0;
	d_ptr->peakVal = 0.0f;
	d_ptr->fwhmVal = 0.0f;
	d_ptr->fwhmX1 = 0.0f;
	d_ptr->fwhmX2 = 0.0f;
	d_ptr->pixelSum = 0.0f;

	d_ptr->peakMax = 0.0f;
	d_ptr->peakMin = 99999.0f;
	d_ptr->peakAvg = 0.0f;
	d_ptr->peakSum = 0.0f;
	d_ptr->fwhmMax = 0.0f;
	d_ptr->pixelMax = 0.0f;

	d_ptr->recordCnt = 0;
	d_ptr->headValue[0] = 0;
	d_ptr->headValue[1] = 0;
	d_ptr->tailValue[0] = 0;
	d_ptr->tailValue[1] = 0;
	return;
}

void ViewScanProfile::setRecordValues(bool flag)
{
	if (flag) {
		clearGraphInfo();
	}

	d_ptr->recordVals = flag;
	return;
}

void ViewScanProfile::setIsProfileEnds(bool flag)
{
	d_ptr->isProfileEnds = flag;
}


void ViewScanProfile::setShowGraphInfo(bool flag)
{
	d_ptr->showText = flag;
	return;
}


SpectrumDataCallback * ViewScanProfile::getCallbackFunction(void)
{
	return &getImpl().callback;
}


ViewScanProfile::ViewScanProfileImpl & ViewScanProfile::getImpl(void) const
{
	return *d_ptr;
}


void ViewScanProfile::callbackProfileImage(unsigned short * data, unsigned int width, unsigned int height)
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

	analyzeProfile();
	updateWindow();

	// DebugOut2() << "Profile image, width: " << width << ", height: " << height;
	return;
}


BEGIN_MESSAGE_MAP(ViewScanProfile, CStatic)
END_MESSAGE_MAP()



// ViewScanProfile message handlers




void ViewScanProfile::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	// TODO:  Add your code to draw the specified item
	updateWindow();
}
