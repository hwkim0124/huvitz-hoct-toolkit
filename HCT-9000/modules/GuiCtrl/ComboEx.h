#pragma once
#include "GuiCtrlDef.h"

// CComboEx

class GUICTRL_DLL_API CComboEx : public CComboBox
{
	DECLARE_DYNAMIC(CComboEx)

public:
	CComboEx();
	virtual ~CComboEx();

	void SetDrawText(int nFormat = DT_CENTER | DT_SINGLELINE | DT_VCENTER, int nMarginLeft = 0);
	void SetEditColor(COLORREF crText, COLORREF crBK);
	void SetListColor(COLORREF crText, COLORREF crBK);
	void SetListHighLightColor(COLORREF crText, COLORREF crBK);


private:
	int m_nFormat;
	int m_nMarginLeft;
	COLORREF m_crEditText;
	COLORREF m_crEditBK;
	COLORREF m_crListText;
	COLORREF m_crListBK;
	COLORREF m_crListHighLightText;
	COLORREF m_crListHighLightBK;


protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
};


