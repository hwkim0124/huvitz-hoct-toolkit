#pragma once
#include <memory>
#include "afxwin.h"
#include "afxcmn.h"

#include "ViewScrollColorCamera.h"
#include "OctSystem2.h"

// DlgOriginalLive 대화 상자입니다.

class DlgOriginalLive : public CDialogEx
{
	DECLARE_DYNAMIC(DlgOriginalLive)

public:
	DlgOriginalLive(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~DlgOriginalLive();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGORIGINALLIVE };
#endif

protected:
	void initControls(void);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

protected:
	ViewScrollColorCamera* liveView;
	OctDevice::ColorCamera* colorCamera;


private:
	bool initiated;
	bool isLive;
	int liveWidth;
	int liveHeight;
	float calcRatio(void);

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedZoomout();
	afx_msg void OnBnClickedZoomin();

public:
	void setParameter(bool flagMaskROI, int xMaskROI, int yMaskROI, int radiusMaskROI, bool flagWhiteBalance, float WBParam1, float WBParam2);

	float timeCount;
	float fps;

	CString m_Size;
	CString m_Fps;
};
