// ImageEdit.cpp : implementation file
//

#include "stdafx.h"
#include "GuiCtrl.h"
#include "ImageEdit.h"


// ImageEdit

IMPLEMENT_DYNAMIC(ImageEdit, CEdit)

ImageEdit::ImageEdit()
{
	
}

ImageEdit::~ImageEdit()
{
}

void ImageEdit::loadImage(UINT id, LPCTSTR pType)
{
	m_imageBK.Load(id, pType);

	return;
}


BEGIN_MESSAGE_MAP(ImageEdit, CEdit)
//	ON_WM_PAINT()
END_MESSAGE_MAP()



// ImageEdit message handlers


void ImageEdit::OnPaint()
{
	CEdit::OnPaint();

	CClientDC dc(this);

	Gdiplus::Graphics G(dc.m_hDC);
	G.DrawImage(m_imageBK.m_pBitmap, 0, 0);

	return;
}
