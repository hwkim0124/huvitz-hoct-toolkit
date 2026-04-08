// FlashBar.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "FlashBar.h"


// FlashBar

IMPLEMENT_DYNAMIC(FlashBar, CWnd)

FlashBar::FlashBar()
{
	m_maxValue = 10;
	m_value = 0;
}

FlashBar::~FlashBar()
{
}


BEGIN_MESSAGE_MAP(FlashBar, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// FlashBar 메시지 처리기입니다.
void FlashBar::LoadImages(UINT id1, UINT id2, LPCTSTR type)
{
	m_image[0].Load(id1, type);
	m_image[1].Load(id2, type);
	return;
}

void FlashBar::SetMaxValue(UINT value)
{
	m_maxValue = value;
	return;
}

void FlashBar::SetValue(UINT value)
{
	if (value > m_maxValue)
		return;

	m_value = value;
	Invalidate(TRUE);

	return;
}

void FlashBar::OnPaint()
{
	CPaintDC dc(this); 
	Gdiplus::Graphics G(dc.m_hDC);

	for (UINT i = 0; i < m_maxValue; i++)
	{
		G.DrawImage(m_image[i < m_value], 21 * i, 0);
	}

	return;
}
