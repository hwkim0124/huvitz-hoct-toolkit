// ThicknessGraph.cpp : implementation file
//

#include "stdafx.h"
#include "ReportCtrl.h"
#include "ThicknessGraph.h"


// ThicknessGraph

IMPLEMENT_DYNAMIC(ThicknessGraph, CWnd)

ThicknessGraph::ThicknessGraph()
{
	m_isTsnit = false;
}

ThicknessGraph::~ThicknessGraph()
{
}


BEGIN_MESSAGE_MAP(ThicknessGraph, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()



// ThicknessGraph message handlers




int ThicknessGraph::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


void ThicknessGraph::OnPaint()
{
	CPaintDC dc(this);

	CRect rectClient;
	GetClientRect(&rectClient);

	drawThicknessGraph(&dc, rectClient);

	return;
}