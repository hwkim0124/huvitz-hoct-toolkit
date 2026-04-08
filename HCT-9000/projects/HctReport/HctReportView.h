
// HctReportView.h : interface of the CHctReportView class
//

#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <sstream>

using namespace std;
using namespace Gdiplus;

class CHctReportView : public CScrollView
{
protected: // create from serialization only
	CHctReportView();
	DECLARE_DYNCREATE(CHctReportView)

// Attributes
public:
	CHctReportDoc* GetDocument() const;

	std::vector<std::pair<int, int>> _imageRowCols;
	std::vector<std::pair<int, int>> _imageStartXYs;
	std::vector<std::pair<int, int>> _imageEndXYs;

	std::vector<BscanData*> _bscanObjects;
	std::vector<SegmImage*> _imageObjects;
	std::vector<wstring> _imageTitles;
	std::vector<Gdiplus::Bitmap*> _imageBitmaps;

	int _screenWidth;
	int _screenHeight;
	int _indexOfImageSelected;
	int _columnAtPointer;
	int _rowAtPointer;

	std::unique_ptr<Gdiplus::CachedBitmap> _pScreenBitmap;

// Operations
public:
	void createBitmapOfPattern(void);
	void createBitmapOfWideAnterior(void);
	void createBitmapOfAnteriorLens(void);
	void createBitmapOfLensThickness(void);

	void updateLayoutOfImages(void);
	void updateScreenBitmap(void);

	void drawImageObjects(Gdiplus::Graphics& G);
	void drawIntensityGraph(Gdiplus::Graphics& G, int index);
	void drawPatternReport(Gdiplus::Graphics& G);

	bool getGraphStartXY(int index, int& x, int& y);
	bool getIndexOfImageAtPosition(int x, int y);

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CHctReportView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // debug version in HctReportView.cpp
inline CHctReportDoc* CHctReportView::GetDocument() const
   { return reinterpret_cast<CHctReportDoc*>(m_pDocument); }
#endif

