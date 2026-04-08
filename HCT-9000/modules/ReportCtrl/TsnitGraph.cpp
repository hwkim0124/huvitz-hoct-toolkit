// TsnitGraph.cpp : implementation file
//

#include "stdafx.h"
#include "ReportCtrl.h"
#include "TsnitGraph.h"


// TsnitGraph

IMPLEMENT_DYNAMIC(TsnitGraph, CWnd)

TsnitGraph::TsnitGraph()
{
	m_isTsnit = true;
	m_rangeMin = 0;
	m_rangeMax = 250;
}

TsnitGraph::~TsnitGraph()
{
}


BEGIN_MESSAGE_MAP(TsnitGraph, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()



// TsnitGraph message handlers




int TsnitGraph::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}


void TsnitGraph::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	CRect rectGraph;
	rectGraph.SetRect(10, rect.top + 10, 330, rect.Height() - 20);
	drawThicknessGraph(&dc, rectGraph, false);

	drawColorTable(&dc, Gdiplus::RectF(340, 30, 40, 170), Gdiplus::Color::White, 14);

	return;
}


