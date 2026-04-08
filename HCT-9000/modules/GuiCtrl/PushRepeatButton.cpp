// PushRepeatButton.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "GuiCtrl.h"
#include "PushRepeatButton.h"


// PushRepeatButton

IMPLEMENT_DYNAMIC(PushRepeatButton, PushButton)

PushRepeatButton::PushRepeatButton()
{
	m_callback = nullptr;
	m_pUserData = nullptr;
}

PushRepeatButton::~PushRepeatButton()
{
}

BEGIN_MESSAGE_MAP(PushRepeatButton, PushButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// PushRepeatButton 메시지 처리기입니다.

void PushRepeatButton::setPressCallback(PressCallbackFunc callback, int controlId, void *pUserData)
{
	m_callback = callback;
	m_pUserData = pUserData;
	m_controlId = controlId;
}


void PushRepeatButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_callback)
	{
		m_callback(m_pUserData, PUSHBUTTON_DOWN, m_controlId);
	}
}

void PushRepeatButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_callback)
	{
		m_callback(m_pUserData, PUSHBUTTON_UP, m_controlId);
	}
}