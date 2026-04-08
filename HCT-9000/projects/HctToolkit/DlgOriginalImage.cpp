// DlgOriginalImage.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgOriginalImage.h"
#include "afxdialogex.h"

#include "OctSystem2.h"
#include "CppUtil2.h"
#include "OctResult2.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace OctResult;
using namespace CppUtil;
using namespace std;


// DlgOriginalImage 대화 상자입니다.
struct DlgOriginalImage::DlgOriginalImageImpl
{
	CvImage image;

	DlgOriginalImageImpl()
	{

	}
};

IMPLEMENT_DYNAMIC(DlgOriginalImage, CDialogEx)

DlgOriginalImage::DlgOriginalImage(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGORIGINALIMAGE, pParent), d_ptr(make_unique<DlgOriginalImageImpl>())
{

}

DlgOriginalImage::~DlgOriginalImage()
{
}


void DlgOriginalImage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DlgOriginalImage, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


DlgOriginalImage::DlgOriginalImageImpl & DlgOriginalImage::getImpl(void) const
{
	return *d_ptr;
}

void DlgOriginalImage::setImage(CvImage src)
{
	getImpl().image = src;
	return;
}

CvImage DlgOriginalImage::getImage(void)
{
	return getImpl().image;
}

// DlgOriginalImage 메시지 처리기입니다.

BOOL DlgOriginalImage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
			  // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgOriginalImage::OnClose()
{
	ShowWindow(SW_HIDE);
	CDialogEx::OnClose();
}

void DlgOriginalImage::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow) {
	}
	return;
}

void  DlgOriginalImage::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int delta;

	switch (nSBCode)
	{
	case SB_LINEUP:
		delta = -m_nBasic;
		break;
	case SB_PAGEUP:
		delta = -VPageSize_;
		break;
	case SB_THUMBTRACK:
		delta = static_cast<int>(nPos) - VScrollPos_;
		break;
	case SB_PAGEDOWN:
		delta = VPageSize_;
		break;
	case SB_LINEDOWN:
		delta = m_nBasic;
		break;
	default:
		return;
	}

	int scrollpos = VScrollPos_ + delta;
	int maxpos = ViewHeight_ - VPageSize_;
	if (scrollpos < 0)
		delta = -VScrollPos_;
	else
		if (scrollpos > maxpos)
			delta = maxpos - VScrollPos_;

	if (delta != 0)
	{
		VScrollPos_ += delta;
		SetScrollPos(SB_VERT, VScrollPos_, TRUE);
		ScrollWindow(0, -delta);
	}
	CRect rect;
	GetClientRect(&rect);
	SendMessage(WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM(rect.right - rect.left, rect.top - rect.bottom));
}

void DlgOriginalImage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_nHdelta = 0;

	switch (nSBCode)
	{
	case SB_LINELEFT:
		m_nHdelta = -m_nBasic;
		break;
	case SB_PAGELEFT:
		m_nHdelta = -HPageSize_;
		break;
	case SB_THUMBTRACK:
		m_nHdelta = static_cast<int>(nPos) - HScrollPos_;
		break;
	case SB_PAGERIGHT:
		m_nHdelta = HPageSize_;
		break;
	case SB_LINERIGHT:
		m_nHdelta = m_nBasic;
		break;
	default:
		return;
	}

	int scrollpos = HScrollPos_ + m_nHdelta;
	int maxpos = ViewWidth_ - HPageSize_;

	if (scrollpos < 0)
		m_nHdelta = -HScrollPos_;
	else
		if (scrollpos > maxpos)
			m_nHdelta = maxpos - HScrollPos_;

	if (m_nHdelta != 0)
	{
		HScrollPos_ += m_nHdelta;
		SetScrollPos(SB_HORZ, HScrollPos_, TRUE);
		ScrollWindow(-m_nHdelta, 0);
	}

	CRect rect;
	GetClientRect(&rect);
	SendMessage(WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM(rect.right-rect.left, rect.top - rect.bottom));
}

void DlgOriginalImage::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	ViewWidth_ = getImage().getCols();
	ViewHeight_ = getImage().getRows();

	int HScrollMax = 0;
	HPageSize_ = 0;

	if (cx < ViewWidth_)
	{
		HScrollMax = ViewWidth_ - 1;
		HPageSize_ = cx;
		HScrollPos_ = min(HScrollPos_, ViewWidth_ - HPageSize_ - 1);
	}

	SCROLLINFO si;
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nMin = 0;
	si.nMax = HScrollMax;
	si.nPos = HScrollPos_;
	si.nPage = HPageSize_;
	SetScrollInfo(SB_HORZ, &si, TRUE);

	int VScrollMax = 0;
	VPageSize_ = 0;

	if (cy < ViewHeight_)
	{
		VScrollMax =  ViewHeight_ - 1;
		VPageSize_ = cy;
		VScrollPos_ = min(VScrollPos_, ViewHeight_ - VPageSize_ - 1);
	}

	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = 0;
	si.nMax = VScrollMax;
	si.nPos = VScrollPos_;
	si.nPage = VPageSize_;
	SetScrollInfo(SB_VERT, &si, TRUE);

	updateWindow();
}

BOOL DlgOriginalImage::OnEraseBkgnd(CDC* pDC)
{
	//return TRUE;
	return FALSE;
}

void DlgOriginalImage::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);

	HBITMAP hbmp = getImage().createDIBitmap(hDC);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);
	SetStretchBltMode(hDC, HALFTONE);

	CRect rect;
	GetClientRect(&rect);

	BitBlt(hDC, rect.left, rect.top, getImage().getCols(), getImage().getRows(), hMemDC, HScrollPos_, VScrollPos_, SRCCOPY);

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	DeleteObject(hbmp);
}

