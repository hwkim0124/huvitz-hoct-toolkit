#include "stdafx.h"
#include "resource.h"
#include "ImageCornea.h"


ImageCornea::ImageCornea()
	: m_bShowFocusGuide(false), m_bShowLimitGuide(false), m_bShowSPupilGuide(false)
	, m_bShowPupilInfo(false), m_isFindPupil(false), m_isFocus(false), m_isSmallPupil(false)
	, m_showAimingDot(false), m_showTrackingTarget(false), m_showCenterGuide(false)
{
	m_leftEnd = false;
	m_rightEnd = false;
	m_upperEnd = false;
	m_lowerEnd = false;
	m_frontEnd = false;
	m_rearEnd = false;

	m_idxImageLeftEnd = 0;
	m_idxImageRightEnd = 0;
	m_idxImageUpperEnd = 0;
	m_idxImageLowerEnd = 0;
	m_idxImageFrontEnd = 0;
	m_idxImageRearEnd = 0;

	m_pupilSize = 0.0f;

	loadResourceImage();
}


ImageCornea::~ImageCornea()
{

}

void ImageCornea::loadResourceImage(void)
{
	HMODULE hInstDll = GetModuleHandle(TEXT("ReportCtrl"));

	m_focusGuideImage[0].Load(IDB_PNG_FOCUS_N1, _T("PNG"), hInstDll);
	m_focusGuideImage[1].Load(IDB_PNG_FOCUS_N2, _T("PNG"), hInstDll);
	m_focusGuideImage[2].Load(IDB_PNG_FOCUS_F, _T("PNG"), hInstDll);

	m_zLevelGuideImage[0].Load(IDB_PNG_ZLEVEL_L, _T("PNG"), hInstDll);
	m_zLevelGuideImage[1].Load(IDB_PNG_ZLEVEL_R, _T("PNG"), hInstDll);

	m_smallPupilImage.Load(IDB_PNG_SMALL_PUPIL, _T("PNG"), hInstDll);


	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_bmpArrowLeft[0].LoadBitmap(IDB_BMP_ARROW_LEFT1);
	m_bmpArrowLeft[1].LoadBitmap(IDB_BMP_ARROW_LEFT2);
	m_bmpArrowLeft[2].LoadBitmap(IDB_BMP_ARROW_LEFT3);
	m_bmpArrowLeft[3].LoadBitmap(IDB_BMP_ARROW_LEFT4);
	m_bmpArrowLeft[4].LoadBitmap(IDB_BMP_ARROW_LEFT5);
	m_bmpArrowLeft[5].LoadBitmap(IDB_BMP_ARROW_LEFT6);
	m_bmpArrowLeft[6].LoadBitmap(IDB_BMP_ARROW_LEFT7);
	m_bmpArrowLeft[7].LoadBitmap(IDB_BMP_ARROW_LEFT8);
	m_bmpArrowLeft[8].LoadBitmap(IDB_BMP_ARROW_LEFT9);

	m_bmpArrowRight[0].LoadBitmap(IDB_BMP_ARROW_RIGHT1);
	m_bmpArrowRight[1].LoadBitmap(IDB_BMP_ARROW_RIGHT2);
	m_bmpArrowRight[2].LoadBitmap(IDB_BMP_ARROW_RIGHT3);
	m_bmpArrowRight[3].LoadBitmap(IDB_BMP_ARROW_RIGHT4);
	m_bmpArrowRight[4].LoadBitmap(IDB_BMP_ARROW_RIGHT5);
	m_bmpArrowRight[5].LoadBitmap(IDB_BMP_ARROW_RIGHT6);
	m_bmpArrowRight[6].LoadBitmap(IDB_BMP_ARROW_RIGHT7);
	m_bmpArrowRight[7].LoadBitmap(IDB_BMP_ARROW_RIGHT8);
	m_bmpArrowRight[8].LoadBitmap(IDB_BMP_ARROW_RIGHT9);

	m_bmpArrowUp[0].LoadBitmap(IDB_BMP_ARROW_UP1);
	m_bmpArrowUp[1].LoadBitmap(IDB_BMP_ARROW_UP2);
	m_bmpArrowUp[2].LoadBitmap(IDB_BMP_ARROW_UP3);
	m_bmpArrowUp[3].LoadBitmap(IDB_BMP_ARROW_UP4);
	m_bmpArrowUp[4].LoadBitmap(IDB_BMP_ARROW_UP5);
	m_bmpArrowUp[5].LoadBitmap(IDB_BMP_ARROW_UP6);
	m_bmpArrowUp[6].LoadBitmap(IDB_BMP_ARROW_UP7);
	m_bmpArrowUp[7].LoadBitmap(IDB_BMP_ARROW_UP8);
	m_bmpArrowUp[8].LoadBitmap(IDB_BMP_ARROW_UP9);

	m_bmpArrowDown[0].LoadBitmap(IDB_BMP_ARROW_DOWN1);
	m_bmpArrowDown[1].LoadBitmap(IDB_BMP_ARROW_DOWN2);
	m_bmpArrowDown[2].LoadBitmap(IDB_BMP_ARROW_DOWN3);
	m_bmpArrowDown[3].LoadBitmap(IDB_BMP_ARROW_DOWN4);
	m_bmpArrowDown[4].LoadBitmap(IDB_BMP_ARROW_DOWN5);
	m_bmpArrowDown[5].LoadBitmap(IDB_BMP_ARROW_DOWN6);
	m_bmpArrowDown[6].LoadBitmap(IDB_BMP_ARROW_DOWN7);
	m_bmpArrowDown[7].LoadBitmap(IDB_BMP_ARROW_DOWN8);
	m_bmpArrowDown[8].LoadBitmap(IDB_BMP_ARROW_DOWN9);

	m_bmpArrowFront[0].LoadBitmap(IDB_BMP_ARROW_FRONT1);
	m_bmpArrowFront[1].LoadBitmap(IDB_BMP_ARROW_FRONT2);
	m_bmpArrowFront[2].LoadBitmap(IDB_BMP_ARROW_FRONT3);
	m_bmpArrowFront[3].LoadBitmap(IDB_BMP_ARROW_FRONT4);
	m_bmpArrowFront[4].LoadBitmap(IDB_BMP_ARROW_FRONT5);
	m_bmpArrowFront[5].LoadBitmap(IDB_BMP_ARROW_FRONT6);
	m_bmpArrowFront[6].LoadBitmap(IDB_BMP_ARROW_FRONT7);
	m_bmpArrowFront[7].LoadBitmap(IDB_BMP_ARROW_FRONT8);
	m_bmpArrowFront[8].LoadBitmap(IDB_BMP_ARROW_FRONT9);

	m_bmpArrowBack[0].LoadBitmap(IDB_BMP_ARROW_BACK1);
	m_bmpArrowBack[1].LoadBitmap(IDB_BMP_ARROW_BACK2);
	m_bmpArrowBack[2].LoadBitmap(IDB_BMP_ARROW_BACK3);
	m_bmpArrowBack[3].LoadBitmap(IDB_BMP_ARROW_BACK4);
	m_bmpArrowBack[4].LoadBitmap(IDB_BMP_ARROW_BACK5);
	m_bmpArrowBack[5].LoadBitmap(IDB_BMP_ARROW_BACK6);
	m_bmpArrowBack[6].LoadBitmap(IDB_BMP_ARROW_BACK7);
	m_bmpArrowBack[7].LoadBitmap(IDB_BMP_ARROW_BACK8);
	m_bmpArrowBack[8].LoadBitmap(IDB_BMP_ARROW_BACK9);

	return;
}

void ImageCornea::showFocusGuide(bool show)
{
	m_bShowFocusGuide = show;

	return;
}

void ImageCornea::showLimitGuide(bool show)
{
	m_bShowLimitGuide = show;
	return;
}

void ImageCornea::showSPupilGuide(bool show)
{
	m_bShowSPupilGuide = show;
}

void ImageCornea::showPupilInfo(bool show)
{
	m_bShowPupilInfo = show;
	return;
}

void ImageCornea::showAimingDot(bool show)
{
	m_showAimingDot = show;
}

void ImageCornea::showTrackingTarget(bool show)
{
	m_showTrackingTarget = show;
}

void ImageCornea::showCenterGuide(bool show)
{
	m_showCenterGuide = show;
}

void ImageCornea::setLimit(bool left, bool right, bool upper, bool lower, bool front, bool rear)
{
	m_leftEnd  = left;
	m_rightEnd = right;
	m_upperEnd = upper;
	m_lowerEnd = lower;
	m_frontEnd = front;
	m_rearEnd  = rear;

	m_idxImageLeftEnd = left ? (m_idxImageLeftEnd + 1) % 9 : 0;
	m_idxImageRightEnd = right ? (m_idxImageRightEnd + 1) % 9 : 0;
	m_idxImageUpperEnd = upper ? (m_idxImageUpperEnd + 1) % 9 : 0;
	m_idxImageLowerEnd = lower ? (m_idxImageLowerEnd + 1) % 9 : 0;
	m_idxImageFrontEnd = front ? (m_idxImageFrontEnd + 1) % 9 : 0;
	m_idxImageRearEnd = rear ? (m_idxImageRearEnd + 1) % 9 : 0;

	return;
}

void ImageCornea::setFocusInfo(bool isFocus, UINT zLevel)
{
	m_isFocus = isFocus;
	m_zLevel = zLevel;

	return;
}

void ImageCornea::setPupilInfo(bool isFind, bool isSmall, float size)
{
	m_isFindPupil = isFind;
	if (m_isFindPupil) {
		m_isSmallPupil = isSmall;
		m_pupilSize = size;
	}

	return;
}

void ImageCornea::setCenter(int x, int y)
{
	m_center = { x, y };

	return;
}

float ImageCornea::getPupilSize()
{
	return m_pupilSize;
}

void ImageCornea::getDstArea(int *xDst, int *yDst, int *wDst, int *hDst)
{
	*xDst = -4;
	*yDst = 0;
	*wDst = getWndWidth();
	*hDst = getWndHeight();

	return;
}

BEGIN_MESSAGE_MAP(ImageCornea, ImageBase)
	ON_WM_PAINT()
END_MESSAGE_MAP()


void ImageCornea::OnPaint()
{
	CPaintDC dc(this);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, getWndWidth(), getWndHeight());

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	memDC.SelectObject(&bitmap);

	drawImage(&memDC);

	if (m_bShowFocusGuide)
	{
		drawFocusGuide(&memDC);
		drawZLevelGuide(&memDC);
	}
	if (m_bShowLimitGuide)
	{
		drawMoterLimitGuide(&memDC);
	}
	if (m_bShowSPupilGuide)
	{
		drawSmallPupilGuide(&memDC);
	}
	if (m_bShowPupilInfo)
	{
		drawPupilInfomation(&memDC);
	}

	if (m_showAimingDot) {
		drawAimmingDot(&memDC);
	}

	if (m_showCenterGuide) {
		drawCenterGuide(&memDC);
	}

	drawTrackingTarget(&memDC);

	dc.BitBlt(0, 0, getWndWidth(), getWndHeight(), &memDC, 0, 0, SRCCOPY);

	bitmap.DeleteObject();
	memDC.DeleteDC();

	return;
}

void ImageCornea::drawFocusGuide(CDC *pDC)
{
	Gdiplus::Graphics graphics(pDC->m_hDC);
	Gdiplus::Bitmap* pBitmap;

	if (m_isFocus)
	{
		if (m_zLevel == 3)
		{
			pBitmap = m_focusGuideImage[2].m_pBitmap;
		}
		else
		{
			pBitmap = m_focusGuideImage[1].m_pBitmap;
		}
	}
	else
	{
		pBitmap = m_focusGuideImage[0].m_pBitmap;
	}

	int center_x = getWndWidth() / 2;
	int center_y = getWndHeight() / 2;
	int px, py;

	if (pBitmap)
	{
		//px = center_x - (pBitmap->GetWidth() / 2);
		//py = center_y - (pBitmap->GetHeight() / 2);
		//graphics.DrawImage(pBitmap, px, py);

		px = (center_x - 4) - (pBitmap->GetWidth() / 2);
		py = center_y - (pBitmap->GetHeight() / 2);
		graphics.DrawImage(pBitmap, px, py);
	}



	return;
}

void ImageCornea::drawZLevelGuide(CDC* pDC)
{
	if (m_isFocus == false)
	{
		return;
	}

	Gdiplus::Graphics graphics(pDC->m_hDC);

	int xSrc, ySrc, wSrc, hSrc;
	int xDst, yDst;


	xSrc = kRectZLevel[m_zLevel].left;
	wSrc = kRectZLevel[m_zLevel].Width();
	ySrc = kRectZLevel[m_zLevel].top;
	hSrc = kRectZLevel[m_zLevel].Height();

	xDst = 205 + xSrc;
	yDst = 220 + ySrc;
	graphics.DrawImage(m_zLevelGuideImage[0], xDst-4, yDst, xSrc, ySrc, wSrc, hSrc, Gdiplus::Unit::UnitPixel);

	xDst = 424 + xSrc;
	yDst = 220 + ySrc;
	graphics.DrawImage(m_zLevelGuideImage[1], xDst-4, yDst, xSrc, ySrc, wSrc, hSrc, Gdiplus::Unit::UnitPixel);

	return;
}

void ImageCornea::drawMoterLimitGuide(CDC *pDC)
{
	if (m_isFocus && (m_zLevel == 3))
	{
		return;
	}

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	CRect rect;
	GetClientRect(&rect);

	if (m_rightEnd)
	{
		BITMAP bmInfo;
		m_bmpArrowLeft[m_idxImageLeftEnd].GetBitmap(&bmInfo);
		memDC.SelectObject(m_bmpArrowRight[m_idxImageLeftEnd]);

		int x = 80;
		int y = (rect.Height() - bmInfo.bmHeight) / 2;
		int w = bmInfo.bmWidth;
		int h = bmInfo.bmHeight;

		pDC->TransparentBlt(x, y, w, h, &memDC, 0, 0, w, h, RGB(0, 255, 0));
	}

	if (m_leftEnd)
	{
		BITMAP bmInfo;
		m_bmpArrowLeft[m_idxImageRightEnd].GetBitmap(&bmInfo);
		memDC.SelectObject(m_bmpArrowLeft[m_idxImageRightEnd]);

		int x = rect.right - bmInfo.bmWidth - 80;
		int y = (rect.Height() - bmInfo.bmHeight) / 2;
		int w = bmInfo.bmWidth;
		int h = bmInfo.bmHeight;

		pDC->TransparentBlt(x, y, w, h, &memDC, 0, 0, w, h, RGB(0, 255, 0));
	}

	if (m_upperEnd)
	{
		BITMAP bmInfo;
		m_bmpArrowDown[m_idxImageUpperEnd].GetBitmap(&bmInfo);
		memDC.SelectObject(m_bmpArrowDown[m_idxImageUpperEnd]);

		int x = (rect.Width() - bmInfo.bmWidth) / 2;
		int y = 60;
		int w = bmInfo.bmWidth;
		int h = bmInfo.bmHeight;

		pDC->TransparentBlt(x, y, w, h, &memDC, 0, 0, w, h, RGB(0, 255, 0));
	}

	if (m_lowerEnd)
	{
		BITMAP bmInfo;
		m_bmpArrowUp[m_idxImageLowerEnd].GetBitmap(&bmInfo);
		memDC.SelectObject(m_bmpArrowUp[m_idxImageLowerEnd]);

		int x = (rect.Width() - bmInfo.bmWidth) / 2;
		int y = rect.bottom - bmInfo.bmHeight - 60;
		int w = bmInfo.bmWidth;
		int h = bmInfo.bmHeight;

		pDC->TransparentBlt(x, y, w, h, &memDC, 0, 0, w, h, RGB(0, 255, 0));
	}

	if (m_frontEnd)
	{
		BITMAP bmInfo;
		m_bmpArrowBack[m_idxImageFrontEnd].GetBitmap(&bmInfo);
		memDC.SelectObject(m_bmpArrowBack[m_idxImageFrontEnd]);

		int x = (rect.Width() - bmInfo.bmWidth) / 2;
		int y = rect.bottom - bmInfo.bmHeight - 60;
		int w = bmInfo.bmWidth;
		int h = bmInfo.bmHeight;

		pDC->TransparentBlt(x, y, w, h, &memDC, 0, 0, w, h, RGB(0, 255, 0));
	}

	if (m_rearEnd)
	{
		BITMAP bmInfo;
		m_bmpArrowFront[m_idxImageRearEnd].GetBitmap(&bmInfo);
		memDC.SelectObject(m_bmpArrowFront[m_idxImageRearEnd]);

		int x = (rect.Width() - bmInfo.bmWidth) / 2;
		int y = rect.bottom - bmInfo.bmHeight - 60;
		int w = bmInfo.bmWidth;
		int h = bmInfo.bmHeight;

		pDC->TransparentBlt(x, y, w, h, &memDC, 0, 0, w, h, RGB(0, 255, 0));
	}

	return;
}

void ImageCornea::drawSmallPupilGuide(CDC *pDC)
{
	Gdiplus::Graphics G(pDC->m_hDC);

	int center_x = getWndWidth() / 2;
	int center_y = getWndHeight() / 2;
	int px, py;

	Gdiplus::Pen pen(Gdiplus::Color::Purple, 1.0f);
	px = (center_x - 4) - kRadiusSPupil;
	py = center_y - kRadiusSPupil;
	G.DrawEllipse(&pen, px, py, kRadiusSPupil * 2, kRadiusSPupil * 2);

	return;
}

void ImageCornea::drawPupilInfomation(CDC *pDC)
{
	if (!m_isFindPupil)
	{
		return;
	}

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::StringFormat sfLeftTop;
	sfLeftTop.SetAlignment(Gdiplus::StringAlignmentNear);
	sfLeftTop.SetLineAlignment(Gdiplus::StringAlignmentNear);
	Gdiplus::Font fontText(_T("Noto Sans CJK KR Regular"), 20, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	Gdiplus::SolidBrush brushText(Gdiplus::Color(0, 255, 255));

	CString text;
	text.Format(_T("Pupil : %.1f"), m_pupilSize);
	G.DrawString(text, -1, &fontText, Gdiplus::PointF(8, 15), &sfLeftTop, &brushText);

	if (m_isSmallPupil)
	{
		G.DrawImage(m_smallPupilImage, 120, 8);
	}

	return;
}

void ImageCornea::drawAimmingDot(CDC *pDC)
{
	if (m_isFocus && m_zLevel == 3) {
		return;
	}

	if (m_center.x == 0 || m_center.y == 0) {
		return;
	}

	// get info.
	float fImgW = getImgWidth();

	int cx = (float)getWndWidth() / (float)getImgWidth() * (float)m_center.x + getWndWidth() / 2;
	int cy = (float)getWndHeight() / (float)getImgHeight() * (float)m_center.y + getWndHeight() / 2;

	// draw
	Gdiplus::Graphics G(pDC->m_hDC);
	Gdiplus::Pen pen(Gdiplus::Color::Yellow);
	Gdiplus::SolidBrush brush(Gdiplus::Color::Yellow);
	Gdiplus::Rect rect(cx - 5, cy - 5, 10, 10);

	G.FillEllipse(&brush, rect);

	return;
}

void ImageCornea::drawTrackingTarget(CDC *pDC)
{
	if (!m_showTrackingTarget) {
		return;
	}

	if (!OctSystem::RetinaTrack::isTargetRegistered()) {
		return;
	}

	COLORREF clr1 = RGB(255, 32, 32);
	COLORREF clr2 = RGB(32, 255, 32);
	COLORREF oldColor;

	CPen pen1, pen2, *oldPen = nullptr;
	pen1.CreatePen(PS_SOLID, 2, clr1);
	pen2.CreatePen(PS_SOLID, 2, clr2);
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	CBrush *oldBrush;
	oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	CRect rtClient;
	GetClientRect(&rtClient);

	//float ratioX = (float)rtClient.Width() / 640.0f;
	float ratioY = (float)rtClient.Height() / 480.0f;
	float ratioX = ratioY;
	int lineHalfLength = 7;

	int tx = -1;
	int ty = -1;
	int radiusX = (int)(7.0f * ratioX);
	int radiusY = (int)(7.0f * ratioY);
	if (OctSystem::RetinaTrack::getTargetCenter(tx, ty)) {
		tx = (int)(tx * ratioX);
		ty = (int)(ty * ratioY);

		// 십자가
		oldPen = pDC->SelectObject(&pen1);
		pDC->MoveTo(tx - lineHalfLength, ty);
		pDC->LineTo(tx + lineHalfLength, ty);
		pDC->MoveTo(tx, ty - lineHalfLength);
		pDC->LineTo(tx, ty + lineHalfLength);

		// 범위
		//pDC->Ellipse(CRect(tx - radiusX, ty - radiusY, tx + radiusX, ty + radiusY));
	}

	int dx = -1;
	int dy = -1;
	if (OctSystem::RetinaTrack::getDetectedCenter(dx, dy)) {
		dx = (int)(dx * ratioX);
		dy = (int)(dy * ratioY);

		// target center 를 중심으로 했을 때의 상대적인 반대 위치를 표시한다.
		// - 상대 위치를 구한다.
		int rdx = dx - tx;
		int rdy = dy - ty;
		// - x 축을 반전시킨 좌표를 구한다.
		dx = tx - rdx;
		dy = ty + rdy;

		// 십자가
		oldPen = pDC->SelectObject(&pen2);
		pDC->MoveTo(dx - lineHalfLength, dy);
		pDC->LineTo(dx + lineHalfLength, dy);
		pDC->MoveTo(dx, dy - lineHalfLength);
		pDC->LineTo(dx, dy + lineHalfLength);
	}

	if (oldPen != nullptr) {
		pDC->SelectObject(oldPen);
	}
	pDC->SelectObject(oldBrush);
	pen1.DeleteObject();
	pen2.DeleteObject();
}

void ImageCornea::drawCenterGuide(CDC *pDC)
{
	CRect rect;
	GetClientRect(&rect);
	auto cp = rect.CenterPoint();

	Gdiplus::Color lineColor(255, 87, 79);

	// draw
	Gdiplus::Graphics G(pDC->m_hDC);
	Gdiplus::Pen pen(lineColor);
	Gdiplus::SolidBrush brush(lineColor);

	G.DrawLine(&pen, 0, cp.y, rect.right, cp.y);
	G.DrawLine(&pen, cp.x, 0, cp.x, rect.bottom);
}
