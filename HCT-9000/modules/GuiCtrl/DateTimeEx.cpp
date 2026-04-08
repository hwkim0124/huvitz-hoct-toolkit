// DateTimeEx.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "GuiCtrl.h"
#include "DateTimeEx.h"


// DateTimeEx

IMPLEMENT_DYNAMIC(DateTimeEx, CDateTimeCtrl)

DateTimeEx::DateTimeEx()
{

}

DateTimeEx::~DateTimeEx()
{
}


BEGIN_MESSAGE_MAP(DateTimeEx, CDateTimeCtrl)
	ON_NOTIFY_REFLECT(DTN_DROPDOWN, &DateTimeEx::OnDtnDropdown)
END_MESSAGE_MAP()



// DateTimeEx 메시지 처리기입니다.


void DateTimeEx::OnDtnDropdown(NMHDR *pNMHDR, LRESULT *pResult)
{
	HWND hWndDateTime = (HWND)pNMHDR->hwndFrom;
	HWND hWndMonthCal = DateTime_GetMonthCal(hWndDateTime);
	HWND hWndDropDown = ::GetParent(hWndMonthCal);

	DWORD dwWidth;
	WINDOWINFO wi;


	if (hWndMonthCal && hWndDropDown)
	{
		RECT rcIdeal;

		//  Remove the window theme from the month calendar control
		SetWindowTheme(hWndMonthCal, L"", L"");

		//MonthCal_SetCALID(hWndMonthCal, CAL_GREGORIAN_US);


		//  Get the ideal size of the month calendar control
		ZeroMemory(&rcIdeal, sizeof(rcIdeal));
		MonthCal_GetMinReqRect(hWndMonthCal, &rcIdeal);
		dwWidth = MonthCal_GetMaxTodayWidth(hWndMonthCal);
		if (dwWidth > (DWORD)rcIdeal.right)
		{
			rcIdeal.right = dwWidth;
		}

		//  Add some padding
		InflateRect(&rcIdeal, 11, 11);

		//  Determine the new size of the drop down window such
		//  that the client area of the window is large enough 
		//  to display the month calendar control
		ZeroMemory(&wi, sizeof(wi));
		wi.cbSize = sizeof(WINDOWINFO);
		::GetWindowInfo(hWndDropDown, &wi);
		AdjustWindowRectEx(&rcIdeal, wi.dwStyle, FALSE, wi.dwExStyle);

		//  Update the size of the drop down window
		::SetWindowPos(hWndDropDown,
			NULL,
			0,
			0,
			rcIdeal.right - rcIdeal.left,
			rcIdeal.bottom - rcIdeal.top,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	}


	*pResult = 0;
}
