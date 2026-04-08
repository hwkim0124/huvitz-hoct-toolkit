// EditBoxComment.cpp : implementation file
//

#include "stdafx.h"
#include "GuiCtrl.h"
#include "EditBoxComment.h"


// EditBoxComment

IMPLEMENT_DYNAMIC(EditBoxComment, CEdit)

EditBoxComment::EditBoxComment()
{
}

EditBoxComment::~EditBoxComment()
{
}

BEGIN_MESSAGE_MAP(EditBoxComment, CEdit)
	ON_WM_NCPAINT()
END_MESSAGE_MAP()



// EditBoxComment message handlers

void EditBoxComment::OnNcPaint()
{
	CDC* pDC = GetWindowDC();

	CRect rect;
	GetWindowRect(&rect);
	rect.OffsetRect(-rect.left, -rect.top);
	CRect rcBound(rect.left + 1, rect.top + 1, rect.right - 1, rect.bottom - 1);

	// Draw a single line around the outside
	if (GetFocus() == this) {
		pDC->FrameRect(&rcBound, &CBrush(kColorEditBorderP));
	}
	else {
		pDC->FrameRect(&rcBound, &CBrush(kColorEditBorderN));
	}

	ReleaseDC(pDC);
}

