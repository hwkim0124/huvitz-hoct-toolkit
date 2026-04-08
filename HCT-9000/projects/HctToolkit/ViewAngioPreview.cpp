// ViewAngioPreview.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ViewAngioPreview.h"

#include "CppUtil2.h"
#include "OctAngio2.h"

#include <numeric>
#include <algorithm>

using namespace std;
using namespace CppUtil;
using namespace OctAngio;

// ViewAngioPreview
struct ViewAngioPreview::ViewAngioPreviewImpl
{
	CvImage image;
	int width;
	int height;
	int srcWidth;
	int srcHeight;
	float clipLimit;
	int lineIdxSel;
	int pointIdxSel;
	int bscanIdxSel = -1;

	float decorrAverage;
	float decorrStddev;
	float decorrMaxVal;
	float intensAverage;
	float intensStddev;
	float intensMaxVal;

	bool useEnhance;
	bool useSelects;
	bool useOverlayText;
	bool useDecorrHorzLines = false;
	bool useDecorrVertLines = false;

	int numDecorrAvgLines = 5;
	int numDecorrTopLines = 5;
	float lineDecorrThresh = 0.15f;

	vector<float> decorrHorzLines;
	vector<float> decorrVertLines;
	vector<int> sortedHorzLines;
	vector<int> sortedVertLines;

	ViewAngioPreviewImpl() : useEnhance(false), useSelects(false), useOverlayText(false) {

	}
};


IMPLEMENT_DYNAMIC(ViewAngioPreview, CStatic)

ViewAngioPreview::ViewAngioPreview()
	: d_ptr(make_unique<ViewAngioPreviewImpl>())
{

}

ViewAngioPreview::~ViewAngioPreview()
{
}


void ViewAngioPreview::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	HBITMAP hbmp = getImpl().image.createDIBitmap(hDC);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	// Copy the bits from the memory DC into the current DC.
	drawSelections(pMemDC);
	drawDecorrLines(pMemDC);
	drawOverlayText(pMemDC);
	drawBscanLineSelected(pMemDC);

	BitBlt(hDC, 0, 0, getImpl().width, getImpl().height, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	DeleteObject(hbmp);
}



void ViewAngioPreview::drawDecorrLines(CDC * pDC)
{
	float thresh = getImpl().lineDecorrThresh;
	int lineAvgSize = getImpl().numDecorrAvgLines;
	int lineTopSize = getImpl().numDecorrTopLines;

	CFont font, *oldFont;
	font.CreateFontW(16, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(32, 64, 255));
	auto bkMode = pDC->SetBkMode(TRANSPARENT);

	int w = getWidth();
	int h = getHeight();
	int sw = getImpl().srcWidth;
	int sh = getImpl().srcHeight;

	CString text;
	CRect rect;

	if (getImpl().useDecorrHorzLines) {
		auto clips = vector<bool>(h, false);
		auto itext = vector<int>();
		auto idata = vector<float>();

		for (const auto idx : getImpl().sortedHorzLines) {
			int k1 = max(idx - lineAvgSize / 2, 0);
			int k2 = min(idx + lineAvgSize / 2, sh - 1);
			int sz = (k2 - k1 + 1);
			float dc = accumulate(getImpl().decorrHorzLines.begin() + k1, getImpl().decorrHorzLines.begin() + k2, 0.0f) / sz;

			if (dc < thresh) {
				continue;
			}

			CPen pen, *oldPen;
			pen.CreatePen(PS_DOT, 1, RGB(max((int)(128 + 255 * dc), 255), 0, 0));
			oldPen = pDC->SelectObject(&pen);

			int y = (int)(((float)idx / (float)getImpl().srcHeight) * getImpl().height);
			pDC->MoveTo(0, y);
			pDC->LineTo(w - 1, y);

			if (none_of(clips.begin() + k1, clips.begin() + k2, [](bool e) { return e; })) {
				itext.push_back(y);
				idata.push_back(dc);
			}
			std::fill(clips.begin() + k1, clips.begin() + k2, true);
			pDC->SelectObject(oldPen);
			pen.DeleteObject();
		}

		int count = 0;
		for (const auto y : itext) {
			text.Format(_T("%.2f"), idata[count]);
			rect = CRect(w - 40 - count * 30, y - 10, w - 1 - count * 30, y + 10);
			pDC->SetBkMode(OPAQUE);
			pDC->DrawText(text, &rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
			pDC->SetBkMode(TRANSPARENT);

			if (++count >= lineTopSize) {
				break;
			}
		}
	}

	if (getImpl().useDecorrVertLines) {
		auto clips = vector<bool>(w, false);
		auto itext = vector<int>();
		auto idata = vector<float>();

		for (const auto idx : getImpl().sortedVertLines) {
			int k1 = max(idx - lineAvgSize / 2, 0);
			int k2 = min(idx + lineAvgSize / 2, sw - 1);
			int sz = (k2 - k1 + 1);
			float dc = accumulate(getImpl().decorrVertLines.begin() + k1, getImpl().decorrVertLines.begin() + k2, 0.0f) / sz;

			if (dc < thresh) {
				continue;
			}

			CPen pen, *oldPen;
			pen.CreatePen(PS_DOT, 1, RGB(max((int)(128 + 255 * dc), 255), 0, 0));
			oldPen = pDC->SelectObject(&pen);

			int x = (int)(((float)idx / (float)getImpl().srcWidth) * getImpl().width);
			pDC->MoveTo(x, 0);
			pDC->LineTo(x, h - 1);

			if (none_of(clips.begin() + k1, clips.begin() + k2, [](bool e) { return e; })) {
				itext.push_back(x);
				idata.push_back(dc);
			}
			std::fill(clips.begin() + k1, clips.begin() + k2, true);

			pDC->SelectObject(oldPen);
			pen.DeleteObject();
		}

		int count = 0;
		for (const auto x : itext) {
			text.Format(_T("%.2f"), idata[count]);
			rect = CRect(x - 20, 10 + count * 20, x + 20, 30 + count * 20);
			pDC->SetBkMode(OPAQUE);
			pDC->DrawText(text, &rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
			pDC->SetBkMode(TRANSPARENT);

			if (++count >= lineTopSize) {
				break;
			}
		}
	}

	pDC->SetBkMode(bkMode);
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewAngioPreview::drawBscanLineSelected(CDC * pDC)
{
	int lineIdx = getImpl().bscanIdxSel;
	if (lineIdx < 0 || lineIdx >= getImpl().decorrHorzLines.size()) {
		return;
	}

	CPen pen, *oldPen;
	pen.CreatePen(PS_DOT, 1, RGB(128, 128, 255));
	oldPen = pDC->SelectObject(&pen);

	int w = getWidth();
	int y = (int)(((float)lineIdx / (float)getImpl().srcHeight) * getImpl().height);
	pDC->MoveTo(0, y);
	pDC->LineTo(w - 1, y);

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	return;
}


void ViewAngioPreview::drawOverlayText(CDC * pDC)
{
	if (!getImpl().useOverlayText) {
		return;
	}

	CFont font, *oldFont;
	font.CreateFontW(18, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(64, 255, 32));
	pDC->SetBkMode(TRANSPARENT);

	CString text;
	CRect rect;

	text.Format(_T("Decorr.: %.2f (avg), %.3f (std), %.2f (max)"), 
				getImpl().decorrAverage, getImpl().decorrStddev, getImpl().decorrMaxVal);
	rect = CRect(25, getHeight() - 50, getWidth(), getHeight() - 30);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	text.Format(_T("Intens.: %.0f (avg), %.1f (std), %.0f (max)"),
		getImpl().intensAverage, getImpl().intensStddev, getImpl().intensMaxVal);
	rect = CRect(25, getHeight() - 30, getWidth(), getHeight() - 10);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewAngioPreview::drawSelections(CDC* pDC)
{
	if (!getImpl().useSelects) {
		return;
	}

	CPen pen, *oldPen;
	pen.CreatePen(PS_DOT, 1, RGB(0, 255, 0));
	oldPen = pDC->SelectObject(&pen);
	auto bkMode = pDC->SetBkMode(TRANSPARENT);

	CFont font, *oldFont;
	font.CreateFontW(20, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);
	auto textClr = pDC->SetTextColor(RGB(0, 255, 0));

	int w = getWidth();
	int h = getHeight();

	int y = (int)(((float)getImpl().lineIdxSel / (float)getImpl().srcHeight) * getImpl().height);
	int x = (int)(((float)getImpl().pointIdxSel / (float)getImpl().srcWidth) * getImpl().width);

	pDC->MoveTo(0, y);
	pDC->LineTo(w - 1, y);
	pDC->MoveTo(x, 0);
	pDC->LineTo(x, h - 1);

	CString text;
	CRect rect;

	text.Format(_T("%d"), getImpl().lineIdxSel);
	rect = CRect(10, y + 5, 50, y + 25);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	text.Format(_T("%d"), getImpl().pointIdxSel);
	rect = CRect(x + 10, 5, x + 50, 25);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(oldFont);
	pDC->SelectObject(oldPen);
	pDC->SetBkMode(bkMode);
	pDC->SetTextColor(textClr);

	font.DeleteObject();
	pen.DeleteObject();
	return;
}


void ViewAngioPreview::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


void ViewAngioPreview::setDecorrelationValues(float avg, float stdev, float maxv)
{
	getImpl().decorrAverage = avg;
	getImpl().decorrStddev = stdev;
	getImpl().decorrMaxVal = maxv;
	getImpl().useOverlayText = true;
	return;
}


void ViewAngioPreview::setIntensityValues(float avg, float stdev, float maxv)
{
	getImpl().intensAverage = avg;
	getImpl().intensStddev = stdev;
	getImpl().intensMaxVal = maxv;
	getImpl().useOverlayText = true;
	return;
}


void ViewAngioPreview::setDecorrOverLines(int avgSize, int topSize, float thresh)
{
	getImpl().numDecorrAvgLines = max(avgSize, 1);
	getImpl().numDecorrTopLines = max(topSize, 1);
	getImpl().lineDecorrThresh = max(thresh, 0.01f);
	return;
}


int ViewAngioPreview::getWidth(void) const
{
	return getImpl().width;
}


int ViewAngioPreview::getHeight(void) const
{
	return getImpl().height;
}


int ViewAngioPreview::getImageWidth(void) const
{
	return getImpl().image.getWidth();
}


int ViewAngioPreview::getImageHeight(void) const
{
	return getImpl().image.getHeight();
}

void ViewAngioPreview::setEnhancement(bool flag, float contrast)
{
	getImpl().useEnhance = flag;
	getImpl().clipLimit = contrast;
	return;
}


void ViewAngioPreview::drawImage(const unsigned char * data, int width, int height)
{
	if (data == nullptr) {
		return;
	}

	getImpl().srcWidth = width;
	getImpl().srcHeight = height;

	CvImage image;
	if (image.fromBitsData(data, width, height)) {
		image.resizeTo(&getImpl().image, getWidth(), getHeight());
		if (getImpl().useEnhance) {
			getImpl().image.equalizeHistogram(getImpl().clipLimit);
		}
		getImpl().image.flipVertical();

		computeDecorrOverLines();
		updateWindow();
	}
	return;
}


void ViewAngioPreview::computeDecorrOverLines(void)
{
	auto& angio = Angiography::getAngiogram();
	auto hdata = angio.getDecorrelationsOnHorzLines(true);
	auto vdata = angio.getDecorrelationsOnVertLines(true);

	if (hdata.size() != angio.imageHeight() || vdata.size() != angio.imageWidth()) {
		return;
	}

	auto vsort = vector<int>(angio.imageWidth());
	auto hsort = vector<int>(angio.imageHeight());

	iota(begin(vsort), end(vsort), 0);
	iota(begin(hsort), end(hsort), 0);

	sort(begin(hsort), end(hsort), [&hdata](const auto& lhs, const auto& rhs) {
		return hdata[lhs] > hdata[rhs];
	});
	sort(begin(vsort), end(vsort), [&vdata](const auto& lhs, const auto& rhs) {
		return vdata[lhs] > vdata[rhs];
	});

	getImpl().decorrHorzLines = hdata; 
	getImpl().decorrVertLines = vdata;
	getImpl().sortedHorzLines = hsort;
	getImpl().sortedVertLines = vsort;
	return;
}



bool ViewAngioPreview::saveImage(CString& dirPath, CString & filename)
{
	CString dirname = (dirPath.IsEmpty() ? _T(".//export") : dirPath);
	CreateDirectory(dirname, NULL);

	CString path = filename;
	if (path.IsEmpty()) {
		CTime time = CTime::GetCurrentTime();
		path = time.Format(_T("ANGIO_%y%m%d_%H%M%S.png"));
	}
	if (!getImpl().image.isEmpty()) {
		path = dirname + _T("//") + path;
		filename = path;
		return getImpl().image.saveFile(wtoa(path), true, 100);
	}
	return false;
}


void ViewAngioPreview::showLineSelected(bool flag, int lineIdx, int pointIdx)
{
	getImpl().useSelects = flag;
	getImpl().lineIdxSel = lineIdx;
	getImpl().pointIdxSel = pointIdx;
	return;
}


void ViewAngioPreview::showDecorrOverHorzLines(bool flag)
{
	getImpl().useDecorrHorzLines = flag;
	updateWindow();
}


void ViewAngioPreview::showDecorrOverVertLines(bool flag)
{
	getImpl().useDecorrVertLines = flag;
	updateWindow();
}

void ViewAngioPreview::selectBscanLine(int lineIdx)
{
	getImpl().bscanIdxSel = lineIdx;
	updateWindow();
	return;
}


ViewAngioPreview::ViewAngioPreviewImpl & ViewAngioPreview::getImpl(void) const
{
	// TODO: insert return statement here
	return *d_ptr;
}


BEGIN_MESSAGE_MAP(ViewAngioPreview, CStatic)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// ViewAngioPreview message handlers

void ViewAngioPreview::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	// TODO:  Add your code to draw the specified item
	updateWindow();
}

void ViewAngioPreview::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (getImpl().useSelects) {
		float yrate = (float)point.y / (float)getImpl().height;
		getImpl().lineIdxSel = (int)(yrate * getImpl().srcHeight);
		float xrate = (float)point.x / (float)getImpl().width;
		getImpl().pointIdxSel = (int)(xrate * getImpl().srcWidth);
		updateWindow();
	}

	CStatic::OnLButtonDown(nFlags, point);
}


