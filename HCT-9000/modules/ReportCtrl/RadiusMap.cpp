// RadiusMap.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "ReportCtrl.h"
#include "RadiusMap.h"


// ThicknessMap
using namespace Gdiplus;

IMPLEMENT_DYNAMIC(RadiusMap, CWnd)

RadiusMap::RadiusMap()
{
	setTransparency(1.0);
}

RadiusMap::~RadiusMap()
{
}


BEGIN_MESSAGE_MAP(RadiusMap, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// RadiusMap 메시지 처리기입니다.



void RadiusMap::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	CRect rectMap;
	rectMap.top = rect.top;
	rectMap.bottom = rect.bottom;
	rectMap.left = (rect.Width() - rect.Height()) / 2;
	rectMap.right = (rect.Width() + rect.Height()) / 2;

	CRect rectTable;
	rectTable.top = rect.top;
	rectTable.bottom = rect.bottom;
	rectTable.left = rect.right - 70;
	rectTable.right = rect.right;

	drawRadiusMap(&dc, rectMap);
	drawColorTable(&dc, rectTable, Gdiplus::Color::White);


	return;
}
