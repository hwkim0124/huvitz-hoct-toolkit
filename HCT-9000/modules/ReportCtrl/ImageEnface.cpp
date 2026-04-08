#include "stdafx.h"
#include "ImageEnface.h"


ImageEnface::ImageEnface()
{
	setFlipVertical(true);
}

ImageEnface::~ImageEnface()
{
}

void ImageEnface::getDstArea(int *xDst, int *yDst, int *wDst, int *hDst)
{
	*xDst = 0;
	*yDst = 0;
	*wDst = getWndWidth();
	*hDst = getWndHeight();
}