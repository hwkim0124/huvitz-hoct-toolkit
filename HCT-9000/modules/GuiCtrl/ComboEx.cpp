// ComboEx.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "GuiCtrl.h"
#include "ComboEx.h"


// CComboEx

IMPLEMENT_DYNAMIC(CComboEx, CComboBox)

CComboEx::CComboEx()
{
	m_crEditText = ::GetSysColor(COLOR_WINDOWTEXT);
	m_crEditBK = ::GetSysColor(COLOR_WINDOW);
	m_crListText = ::GetSysColor(COLOR_WINDOWTEXT);
	m_crListBK = ::GetSysColor(COLOR_WINDOW);
	m_crListHighLightText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_crListHighLightBK = ::GetSysColor(COLOR_HIGHLIGHT);

	m_nFormat = DT_CENTER | DT_SINGLELINE | DT_VCENTER;
	m_nMarginLeft = 0;
}

CComboEx::~CComboEx()
{
}

void CComboEx::SetDrawText(int nFormat, int nMarginLeft)
{
	m_nFormat = nFormat;
	m_nMarginLeft = nMarginLeft;

	return;
}

void CComboEx::SetEditColor(COLORREF crText, COLORREF crBK)
{
	m_crEditText = crText;
	m_crEditBK = crBK;
	return;
}

void CComboEx::SetListColor(COLORREF crText, COLORREF crBK)
{
	m_crListText = crText;
	m_crListBK = crBK;
	return;
}

void CComboEx::SetListHighLightColor(COLORREF crText, COLORREF crBK)
{
	m_crListHighLightText = crText;
	m_crListHighLightBK = crBK;
	return;
}



BEGIN_MESSAGE_MAP(CComboEx, CComboBox)
END_MESSAGE_MAP()



// CComboEx 메시지 처리기입니다.

void CComboEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT(lpDrawItemStruct->CtlType == ODT_COMBOBOX);

	if (lpDrawItemStruct->itemID == -1) return;
	
	CDC dc;
	CString strData;
	CRect rcItem;

	dc.Attach(lpDrawItemStruct->hDC);
	GetLBText(lpDrawItemStruct->itemID, strData);
	rcItem = lpDrawItemStruct->rcItem;

	COLORREF crOldTextColor = dc.GetTextColor();
	COLORREF crOldBkColor = dc.GetBkColor();

	if (lpDrawItemStruct->itemState & ODS_COMBOBOXEDIT)
	{
		dc.SetTextColor(m_crEditText);
		dc.SetBkColor(m_crEditBK);
		dc.FillSolidRect(&rcItem, m_crEditBK);
	}
	else if ((lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		dc.SetTextColor(m_crListHighLightText);
		dc.SetBkColor(m_crListHighLightBK);
		dc.FillSolidRect(&rcItem, m_crListHighLightBK);
	}
	else
	{
		dc.SetTextColor(m_crListText);
		dc.SetBkColor(m_crListBK);
		dc.FillSolidRect(&rcItem, m_crListBK);
	}

	rcItem.OffsetRect(m_nMarginLeft, 0);
	dc.DrawText(strData.GetBuffer(1), strData.GetLength(), &rcItem, m_nFormat);

	dc.SetTextColor(crOldTextColor);
	dc.SetBkColor(crOldBkColor);

	dc.Detach();
}


int CComboEx::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	int iComp = 0;
	ASSERT(lpCompareItemStruct->CtlType == ODT_COMBOBOX);
	LPCTSTR lpszText1 = (LPCTSTR)lpCompareItemStruct->itemData1;
	ASSERT(lpszText1 != NULL);
	LPCTSTR lpszText2 = (LPCTSTR)lpCompareItemStruct->itemData2;
	ASSERT(lpszText2 != NULL);

	if (NULL != lpszText1 && NULL != lpszText2)
	{
		iComp = _tcscmp(lpszText2, lpszText1);
	}

	return iComp;
}


void CComboEx::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	ASSERT(lpMeasureItemStruct->CtlType == ODT_COMBOBOX);
}