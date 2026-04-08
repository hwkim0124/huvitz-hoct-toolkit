#pragma once
#pragma once
#include "GuiCtrlDef.h"
#include "GdiPlusBitmapResource.h"
#include "PushButton.h"

// PushButton

#define PUSHBUTTON_DOWN -1
#define PUSHBUTTON_UP   1

typedef void(*PressCallbackFunc)(void *pUserData, int press, int controlId);

class GUICTRL_DLL_API PushRepeatButton : public PushButton
{
	DECLARE_DYNAMIC(PushRepeatButton)

public:
	PushRepeatButton();
	virtual ~PushRepeatButton();
	void	setPressCallback(PressCallbackFunc callback, int controlId, void *pUserData);

private:
	PressCallbackFunc m_callback;
	void* m_pUserData;
	int m_controlId;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


