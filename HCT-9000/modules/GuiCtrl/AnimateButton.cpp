// AnimateButton.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "GuiCtrl.h"
#include "AnimateButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// AnimateButton

AnimateButton::AnimateButton()
{
	m_bPlay = false;
	m_nIndex = 0;
}

AnimateButton::~AnimateButton()
{
}


BEGIN_MESSAGE_MAP(AnimateButton, CButton)
//{{AFX_MSG_MAP(AnimateButton)
	ON_WM_TIMER()
	ON_WM_DRAWITEM()
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()



// AnimateButton 메시지 처리기입니다.

void AnimateButton::loadStdImage(UINT id, LPCTSTR type)
{
	BOOL ret;
	ret = m_stdImage.Load(id, type);

	return;
}

void AnimateButton::loadAniImage(UINT *id, UINT count, LPCTSTR type)
{
	m_aniImages.clear();
	m_aniImages.resize(count);

	BOOL ret;
	for (UINT i = 0; i < count; i++)
	{
		ret = m_aniImages[i].Load(id[i], type);
	}
	
	return;
}

void AnimateButton::play(void)
{
	m_bPlay = true;
	m_nIndex = 0;
	SetTimer(100, 50, NULL);
}

void AnimateButton::stop(void)
{
	m_bPlay = false;
	Invalidate();
	KillTimer(100);
}

void AnimateButton::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 100)
	{
		m_nIndex += 1;

		if (m_nIndex >= 25)
		{
			m_nIndex = 0;
		}

		Invalidate();
	}

	CWnd::OnTimer(nIDEvent);
}


void AnimateButton::PreSubclassWindow()
{
	// Set control to owner draw
	ModifyStyle(0, BS_OWNERDRAW, SWP_FRAMECHANGED);

	CWnd::PreSubclassWindow();
}


BOOL AnimateButton::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void AnimateButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
//	CRect rect = lpDIS->rcItem;
//	Gdiplus::Image *pImage;
//	CBitmap bmp, *pOldBitmap;
//	CDC memDC;

	Gdiplus::Graphics G(pDC->m_hDC);

	if (m_bPlay)
	{
		if (m_aniImages[m_nIndex])
		{
			G.DrawImage(m_aniImages[m_nIndex], 0, 0);
		//	G.DrawImage(m_stdImage, 0, 0);
		}
	}
	else
	{
		if (m_stdImage)
		{
			G.DrawImage(m_stdImage, 0, 0);
		}

	}

	return;
}


void AnimateButton::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CButton::OnDrawItem(nIDCtl, lpDrawItemStruct);
}
