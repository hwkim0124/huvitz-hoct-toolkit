// ViewScanSignal.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ViewScanSignal.h"

#include "CppUtil2.h"
#include "SigProc2.h"
#include "SigChain2.h"

using namespace std;
using namespace CppUtil;
using namespace SigProc;
using namespace SigChain;


struct ViewScanSignal::ViewScanSignalImpl
{
	int width;
	int height;

	CaptureMode mode;
	int captStart;
	int captStep;
	int captCount;
	int dataMulti;

	bool isChartPeak;
	bool isChartSNR;
	bool isChartFWHM;

	int chartPeakRange[2];
	int chartDataRange[2];
	int chartPeakOffset[2];

	float plotRatioX;
	float plotRatioY;
	float peakTopValue;
	CRect plotRect;

	Magnitude magnitude;


	IntensityDataCallback cbIntensityData;

	ViewScanSignalImpl() : isChartPeak(true), isChartSNR(false), isChartFWHM(false), 
		mode(CaptureMode::REFER), dataMulti(1),
		chartPeakRange{ 0, 10240 }, chartDataRange{ 0, 1025 }, chartPeakOffset{ 50, 50 }
	{
	}
};


// ViewScanSignal

IMPLEMENT_DYNAMIC(ViewScanSignal, CStatic)

ViewScanSignal::ViewScanSignal()
	: d_ptr(make_unique<ViewScanSignalImpl>())
{
	getImpl().cbIntensityData = std::bind(&ViewScanSignal::callbackIntensityData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

}


ViewScanSignal::~ViewScanSignal()
{
}


void ViewScanSignal::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, getWidth(), getHeight());
	CBitmap* oldBmp = pMemDC->SelectObject(&bitmap);

	drawIntensityGraph(pMemDC);
	drawIntensityDataLine(pMemDC);

	BitBlt(hDC, 0, 0, getWidth(), getHeight(), hMemDC, 0, 0, SRCCOPY);
	pMemDC->SelectObject(oldBmp);

	bitmap.DeleteObject();
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	return;
}


void ViewScanSignal::drawIntensityGraph(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	CRect rect(0, 0, width, height);
	pDC->FillSolidRect(&rect, RGB(255, 255, 255));

	CPen pen, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	oldPen = pDC->SelectObject(&pen);

	CRect rcPlot(60, 10, getWidth() - 20, getHeight() - 40);
	pDC->MoveTo(rcPlot.left, rcPlot.bottom);
	pDC->LineTo(rcPlot.left, rcPlot.top);
	pDC->MoveTo(rcPlot.left, rcPlot.bottom);
	pDC->LineTo(rcPlot.right, rcPlot.bottom);

	CFont font, *oldFont;
	font.CreateFontW(14, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SetBkMode(TRANSPARENT);

	CString text;
	text.Format(_T("%d"), getImpl().chartPeakRange[0]);
	rect.SetRect(0, rcPlot.bottom - 30, rcPlot.left-5, rcPlot.bottom-10);
	pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);

	text.Format(_T("%d"), getImpl().chartPeakRange[1]);
	rect.SetRect(0, rcPlot.top, rcPlot.left-5, rcPlot.top + 20);
	pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);

	text.Format(_T("%d"), getImpl().chartDataRange[0]);
	rect.SetRect(rcPlot.left+5, rcPlot.bottom+5, rcPlot.left + 50, rcPlot.bottom + 20);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	text.Format(_T("%d"), getImpl().chartDataRange[1]);
	rect.SetRect(rcPlot.right - 50, rcPlot.bottom+5, rcPlot.right-5, rcPlot.bottom + 20);
	pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);

	getImpl().plotRect = rcPlot;
	getImpl().plotRatioX = (float)(getImpl().chartDataRange[1] - getImpl().chartDataRange[0]) / rcPlot.Width();
	getImpl().plotRatioY = (float)(getImpl().chartPeakRange[1] - getImpl().chartPeakRange[0]) / rcPlot.Height();

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewScanSignal::drawIntensityDataLine(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	int orgX = getImpl().plotRect.left;
	int orgY = getImpl().plotRect.bottom;
	int topY = getImpl().plotRect.top;
	int wPlot = getImpl().plotRect.Width();
	int hPlot = getImpl().plotRect.Height();

	int index = getImpl().magnitude.getAxialDataIndex();
	float* data = getImpl().magnitude.getAxialData(index);
	if (data == nullptr) {
		return;
	}

	CPen pen, pen2, pen3, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 32, 32));
	pen2.CreatePen(PS_SOLID, 2, RGB(32, 255, 32));
	pen3.CreatePen(PS_DOT, 1, RGB(64, 192, 255));
	oldPen = pDC->SelectObject(&pen);

	CFont font, *oldFont;
	font.CreateFontW(16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SetBkMode(TRANSPARENT);

	CRect rect;
	CString text;

	int idxAtPos = 0;
	int xinc, yinc, xpos, ypos;
	bool found = false;

	Magnitude& magnObj = getImpl().magnitude;
	int startIdx = getImpl().chartDataRange[0];
	int valueMin = getImpl().chartPeakRange[0];

	for (xinc = 0; xinc < wPlot; xinc++) {
		idxAtPos = (int)(xinc * getImpl().plotRatioX) + startIdx;
		yinc = (int)((data[idxAtPos] - valueMin) / getImpl().plotRatioY);

		xpos = orgX + xinc;
		ypos = orgY - yinc;
		ypos = (ypos > orgY ? orgY : ypos);
		ypos = (ypos < topY ? topY : ypos);

		if (xinc == 0) {
			pDC->MoveTo(xpos, ypos);
		}
		else {
			pDC->LineTo(xpos, ypos);
		}

		if (idxAtPos >= magnObj.peakTopIndex() && !found) {
			int xnew, ynew;

			/*
			xnew = ((wPlot - xinc) < 200 ? (orgX + wPlot - 200) : xpos);
			ynew = ((orgY - ypos) < 100 ? (orgY - 100) : ypos);
			ynew = ((ynew < topY) ? topY + 5 : ynew);
			*/

			xnew = orgX + wPlot - 300;
			ynew = orgY - 200;

			getImpl().peakTopValue = magnObj.peakTopValue();
			text.Format(_T("Peak %d at %d"), (int)magnObj.peakTopValue(), magnObj.peakTopIndex());
			rect.SetRect(xnew + 20, ynew, xnew + 260, ynew + 20);
			pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

			double fwhm = magnObj.peakFWHM();
			double resol = ChainSetup::getResampler()->getPixelResolutionInTissue(fwhm);
			resol = resol / getImpl().dataMulti;

			text.Format(_T("FWHM %.2f (%d ~ %d), %.4lf"), magnObj.peakFWHM(),
				magnObj.peakHalfIndex(0), magnObj.peakHalfIndex(1), resol);
			rect.SetRect(xnew + 20, ynew + 20, xnew + 260, ynew + 40);
			pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

			fwhm = magnObj.meanPeakFWHM();
			resol = ChainSetup::getResampler()->getPixelResolutionInTissue(fwhm);
			resol = resol / getImpl().dataMulti;

			text.Format(_T("FWHM %.2f avg %.4lf"), magnObj.meanPeakFWHM(), resol);
			rect.SetRect(xnew + 20, ynew + 40, xnew + 260, ynew + 60);
			pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

			text.Format(_T("SNR %.2f avg %.2f max %.2f"), magnObj.peakSNR(), magnObj.meanPeakSNR(), magnObj.maxPeakSNR());
			rect.SetRect(xnew + 20, ynew + 60, xnew + 260, ynew + 80);
			pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

			text.Format(_T("Sig. %.2f noise: %.2f"), magnObj.signalLevel(), magnObj.noiseLevel());
			rect.SetRect(xnew + 20, ynew + 80, xnew + 260, ynew + 100);
			pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
			found = true;

			pDC->SelectObject(&pen2);
			pDC->Ellipse(xpos - 5, ypos - 5, xpos + 5, ypos + 5);
			int x1 = (int)((magnObj.peakHalfIndex(0) - startIdx) / getImpl().plotRatioX);
			int x2 = (int)((magnObj.peakHalfIndex(1) - startIdx) / getImpl().plotRatioX);
			int yh = (int)((magnObj.peakTopValue()*0.5 - valueMin) / getImpl().plotRatioY);

			x1 = (x1 < 0 ? 0 : x1) + orgX;
			x2 = (x2 < 0 ? 0 : x2) + orgX;
			yh = orgY - (yh < 0 ? 0 : yh) ;
			yh = (yh < topY ? topY : yh);

			pDC->MoveTo(x1, yh);
			pDC->LineTo(x2, yh);

			pDC->MoveTo(xpos, ypos);
			pDC->SelectObject(&pen);
		}
	}

	pDC->SelectObject(&pen3);
	yinc = (int)((magnObj.noiseLevel()- valueMin) / getImpl().plotRatioY);
	ypos = orgY - yinc;
	ypos = (ypos >= orgY ? (orgY - 1) : ypos);
	ypos = (ypos < topY ? topY : ypos);
	pDC->MoveTo(orgX + 1, ypos);
	pDC->LineTo(orgX + wPlot, ypos);

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	pen2.DeleteObject();
	pen3.DeleteObject();
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewScanSignal::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


int ViewScanSignal::getWidth(void) const
{
	return getImpl().width;
}


int ViewScanSignal::getHeight(void) const
{
	return getImpl().height;
}


void ViewScanSignal::setCaptureMode(CaptureMode mode)
{
	getImpl().mode = mode;
	return;
}


void ViewScanSignal::setCaptureParams(int start, int istep, int count)
{
	getImpl().captStart = start;
	getImpl().captStep = istep;
	getImpl().captCount = count;
	return;
}


void ViewScanSignal::setChartPeakRange(int vmax, int vmin)
{
	getImpl().chartPeakRange[0] = vmin;
	getImpl().chartPeakRange[1] = vmax;
	return;
}


void ViewScanSignal::setChartPeakOffset(int offset1, int offset2)
{
	getImpl().chartPeakOffset[0] = offset1;
	getImpl().chartPeakOffset[1] = offset2;
	getImpl().magnitude.setAxialPeakRangeOffset(offset1, offset2);
	return;
}


void ViewScanSignal::setChartDataRange(int start, int close)
{
	getImpl().chartDataRange[0] = start;
	getImpl().chartDataRange[1] = close;
	return;
}


void ViewScanSignal::setDataMultiple(int multi)
{
	getImpl().dataMulti = multi;
	return;
}


void ViewScanSignal::setAverageSize(int size)
{
	getImpl().magnitude.setAxialDataAverageSize(size);
	return;
}


void ViewScanSignal::setFixedNoiseLevel(float level)
{
	getImpl().magnitude.setFixedNoiseValue(level);
	return;
}


void ViewScanSignal::setChartPeak(bool flag)
{
	getImpl().isChartPeak = flag;
	return;
}


void ViewScanSignal::setChartFWHM(bool flag)
{
	getImpl().isChartFWHM = flag;
	return;
}


void ViewScanSignal::setChartSNR(bool flag)
{
	getImpl().isChartSNR = flag;
	return;
}


void ViewScanSignal::clearChartHistory(void)
{
	getImpl().magnitude.clearAxialDataHistory();
	return;
}

float ViewScanSignal::getPeakTopValue(void)
{
	return getImpl().peakTopValue;
}

int ViewScanSignal::getPeakIndexDiff(int value)
{
	Magnitude& magnObj = getImpl().magnitude;

	return abs(magnObj.peakTopIndex() - value);
}

ViewScanSignal::ViewScanSignalImpl & ViewScanSignal::getImpl(void) const
{
	return *d_ptr;
}


void ViewScanSignal::callbackIntensityData(float * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	// Find peak of axial intensity data. 
	getImpl().magnitude.setIntensityData(data, width, height, 1, 0, getImpl().dataMulti);
	getImpl().magnitude.updateDataProfile();

	updateWindow();

	// LogD() << "Instensity, w: " << width << ", h: " << height;
	return;
}


IntensityDataCallback * ViewScanSignal::getIntensityCallback(void) const
{
	return &getImpl().cbIntensityData;
}


BEGIN_MESSAGE_MAP(ViewScanSignal, CStatic)
END_MESSAGE_MAP()



// ViewScanSignal message handlers


void ViewScanSignal::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	// TODO:  Add your code to draw the specified item
	updateWindow();
	return;
}
