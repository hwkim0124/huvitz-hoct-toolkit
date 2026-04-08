// WndImage.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "ReportCtrl.h"
#include "ImageBase.h"


// ImageBase

IMPLEMENT_DYNAMIC(ImageBase, CWnd)

ImageBase::ImageBase() 
	: m_flipVert(false)
{
	m_image.clear();

	::InitializeCriticalSection(&m_cs);
}

ImageBase::~ImageBase()
{
	::DeleteCriticalSection(&m_cs);
}

void ImageBase::setFlipVertical(bool flip)
{
	m_flipVert = flip;

	return;
}

void ImageBase::clearImage(void)
{
	m_image.clear();
	clearMeasurePoints();

	return;
}

void ImageBase::setImage(unsigned char *data, unsigned int width, unsigned int height)
{
	if (data == nullptr)
	{
		return;
	}

	CvImage image = CvImage();
	if (image.fromBitsData(data, width, height)) 
	{
		setImage(image);
	}

	return;
}

void ImageBase::setImage(CvImage image)
{
	::EnterCriticalSection(&m_cs);
	m_image = image;
	::LeaveCriticalSection(&m_cs);

	Invalidate(FALSE);

	return;
}

bool ImageBase::hasImage(void)
{
	return (m_image.isEmpty() == false);
}

CvImage ImageBase::getImage(void)
{
	return m_image;
}

void ImageBase::saveImage(CString filepath)
{
	if (!m_image.isEmpty())
	{
		CT2CA pszConvertedAnsiString(filepath);
		std::string s(pszConvertedAnsiString);

		m_image.saveFile(s);
	}

	return;
}

void ImageBase::setDimension(float width, float height)
{
	m_actualLengthX = width;
	m_actualLengthY = height;

	return;
}

void ImageBase::getDimension(float &width, float &height)
{
	width = m_actualLengthX;
	height = m_actualLengthY;

	return;
}

unsigned int ImageBase::getWndWidth(void)
{
	CRect rect;
	GetClientRect(&rect);

	return rect.Width();
}

unsigned int ImageBase::getWndHeight(void)
{
	CRect rect;
	GetClientRect(&rect);

	return rect.Height();
}

unsigned int ImageBase::getImgWidth(void)
{
	return m_image.getWidth();
}

unsigned int ImageBase::getImgHeight(void)
{
	return m_image.getHeight();
}

void ImageBase::getSrcArea(int *xSrc, int *ySrc, int *wSrc, int *hSrc)
{
	*xSrc = 0;
	*ySrc = 0;
	*wSrc = getImgWidth();
	*hSrc = getImgHeight();

	return;
}

void ImageBase::getDstArea(int *xDst, int *yDst, int *wDst, int *hDst)
{
	*xDst = 0;
	*yDst = 0;
	*wDst = getWndWidth();
	*hDst = getWndHeight();

	return;
}


BEGIN_MESSAGE_MAP(ImageBase, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// ImageBase 메시지 처리기입니다.


void ImageBase::OnPaint()
{
	CPaintDC dc(this);

	drawImage(&dc);

	return;
}

void ImageBase::drawImage(CDC *pDC)
{
	::EnterCriticalSection(&m_cs);
	drawImage(pDC, m_image);
	::LeaveCriticalSection(&m_cs);

	return;
}

void ImageBase::drawImage(CDC *pDC, CRect rect)
{
	::EnterCriticalSection(&m_cs);
	drawImage(pDC, m_image, rect);
	::LeaveCriticalSection(&m_cs);

	return;
}

void ImageBase::drawImage(CDC *pDC, CvImage image)
{
	if (image.isEmpty())
	{
		return;
	}

	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);

	HBITMAP hbmp = image.createDIBitmap(hDC, m_flipVert);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	int xSrc, ySrc, wSrc, hSrc;
	int xDst, yDst, wDst, hDst;
	getSrcArea(&xSrc, &ySrc, &wSrc, &hSrc);
	getDstArea(&xDst, &yDst, &wDst, &hDst);

	SetStretchBltMode(hDC, HALFTONE);
	StretchBlt(hDC, xDst, yDst, wDst, hDst, hMemDC, xSrc, ySrc, wSrc, hSrc, SRCCOPY);

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	DeleteObject(hbmp);


	return;
}

void ImageBase::drawImage(CDC *pDC, CvImage image, CRect rect)
{
	if (image.isEmpty())
	{
		return;
	}

	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);

	HBITMAP hbmp = image.createDIBitmap(hDC, m_flipVert);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	SetStretchBltMode(hDC, HALFTONE);
	StretchBlt(hDC, rect.left, rect.top, rect.Width(), rect.Height(), hMemDC, 
		0, 0, getImgWidth(), getImgHeight(), SRCCOPY);

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	DeleteObject(hbmp);

	return;
}

void ImageBase::maskIRFundusImage(cv::Mat& mat, int radisus)
{
	auto nChanels = mat.channels();

	auto data = (unsigned char *)mat.data;

	int cx = mat.cols / 2;
	int cy = mat.rows / 2;
	for (int y = 0; y < mat.rows; y++) {
		for (int x = 0; x < mat.cols; x++) {
			if ((cx - x) * (cx - x) + (cy - y) * (cy - y) > radisus * radisus) {
				int dataIdx = nChanels * (mat.cols * y + x);
				if (nChanels == 1) {
					if (data[dataIdx] < 50) {
						data[dataIdx] = 255;
					}
				}
				else if (nChanels == 3) {
					if (data[dataIdx] < 50 && data[dataIdx + 1] < 50 && data[dataIdx + 2] < 50) {
						data[dataIdx] = 255;
						data[dataIdx + 1] = 255;
						data[dataIdx + 2] = 255;
					}
				}
			}
		}
	}
}