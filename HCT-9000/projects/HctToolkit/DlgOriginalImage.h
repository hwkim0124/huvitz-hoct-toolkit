#pragma once

#include "ViewColorCamera.h"

#include <memory>
#include "afxwin.h"
#include "afxcmn.h"

#include "OctSystem2.h"

// DlgOriginalImage 대화 상자입니다.

class DlgOriginalImage : public CDialogEx
{
	DECLARE_DYNAMIC(DlgOriginalImage)

public:
	DlgOriginalImage(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~DlgOriginalImage();
	void updateWindow(void);

protected:
	

protected:
	ViewColorCamera * originView;
	HICON m_hIcon;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGORIGINALIMAGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	struct DlgOriginalImageImpl;
	std::unique_ptr<DlgOriginalImageImpl> d_ptr;
	DlgOriginalImageImpl& getImpl(void) const;
	
	DECLARE_MESSAGE_MAP()

public: 
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void setImage(CppUtil::CvImage src);
	CppUtil::CvImage getImage(void);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	const int m_nBasic = 95;

	int ViewWidth_;
	int ViewHeight_;
	int HScrollPos_;
	int VScrollPos_;
	int HPageSize_;
	int VPageSize_;
	int delta;
	int m_nHdelta;
};
