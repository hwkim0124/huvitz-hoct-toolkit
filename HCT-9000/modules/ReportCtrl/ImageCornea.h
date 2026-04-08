#pragma once
#include "ImageBase.h"
#include "ReportCtrlDef.h"

class REPORTCTRL_DLL_API ImageCornea : public ImageBase
{
	const CRect kRectZLevel[7] =
	{
		CRect(0, 31, 30, 62),
		CRect(0, 31, 30, 52),
		CRect(0, 31, 30, 42),
		CRect(0, 30, 30, 30),		// center
		CRect(0, 20, 30, 30),
		CRect(0, 10, 30, 30),
		CRect(0,  0, 30, 30),
	};

	const UINT kRadiusSPupil = 66;

public:
	ImageCornea();
	virtual ~ImageCornea();
	void loadResourceImage(void);

	void showFocusGuide(bool show);
	void showLimitGuide(bool show);
	void showSPupilGuide(bool show);
	void showPupilInfo(bool show);
	void showAimingDot(bool show);
	void showTrackingTarget(bool show);
	void showCenterGuide(bool show);

	void setLimit(bool left, bool right, bool upper, bool lower, bool front, bool rear);
	void setFocusInfo(bool isFocus, UINT zLevel);
	void setPupilInfo(bool isFind, bool isSmall, float pupilSize);
	void setCenter(int x, int y);

	float getPupilSize();

protected:
	void drawFocusGuide(CDC *pDC);
	void drawZLevelGuide(CDC* pDC);
	void drawMoterLimitGuide(CDC *pDC);
	void drawSmallPupilGuide(CDC *pDC);
	void drawPupilInfomation(CDC *pDC);
	void drawAimmingDot(CDC *pDC);
	void drawTrackingTarget(CDC *pDC);
	void drawCenterGuide(CDC *pDC);

	void getDstArea(int *xDst, int *yDst, int *wDst, int *hDst);

protected:
	GdiPlusBitmapResource m_focusGuideImage[3];		// focused, not focused, 
	GdiPlusBitmapResource m_zLevelGuideImage[2];	// left, right
	GdiPlusBitmapResource m_smallPupilImage;

	bool m_bShowFocusGuide;
	bool m_isFocus;
	UINT m_zLevel;

	bool m_bShowSPupilGuide;

	bool m_bShowPupilInfo;
	bool m_isFindPupil;
	bool m_isSmallPupil;
	float m_pupilSize;

	bool m_bShowLimitGuide;
	bool m_leftEnd;
	bool m_rightEnd;
	bool m_upperEnd;
	bool m_lowerEnd;
	bool m_frontEnd;
	bool m_rearEnd;

	int m_idxImageLeftEnd;
	int m_idxImageRightEnd;
	int m_idxImageUpperEnd;
	int m_idxImageLowerEnd;
	int m_idxImageFrontEnd;
	int m_idxImageRearEnd;

	bool m_showAimingDot;
	bool m_showTrackingTarget;
	bool m_showCenterGuide;
	POINT m_center;

	CBitmap m_bmpArrowLeft[9];
	CBitmap m_bmpArrowRight[9];
	CBitmap m_bmpArrowUp[9];
	CBitmap m_bmpArrowDown[9];
	CBitmap m_bmpArrowFront[9];
	CBitmap m_bmpArrowBack[9];

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
};

