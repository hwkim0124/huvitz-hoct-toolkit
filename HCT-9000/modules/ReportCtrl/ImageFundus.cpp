#include "stdafx.h"
#include "ImageFundus.h"

#include <gdiplus.h>

ImageFundus::ImageFundus()
	: m_colorMode(FundusColor::color)
	, m_useCorrection(false), m_zoomRatio(1.0f)
	, m_emboss(FundusImageProcess::EmbossType::EMBOSS_TYPE_NONE)
{
	m_unitLength = _T("㎜");
	m_unitArea = _T("㎜²");
//	m_pixelSizeX = (double)12 / (double)2700;
//	m_pixelSizeY = (double)12 / (double)2700;

	bool test = false;

	if (test)
	{
		float tester = 1.78f;
		if (GlobalSettings::useFundusFILR_Enable()) {
			m_pixelSizeX = 0.00463f * tester;		//pixel pitch 기준값들 +-7% 범위.
			m_pixelSizeY = 0.00463f * tester;
		}
		else {
			m_pixelSizeX = 0.00369f * tester;
			m_pixelSizeY = 0.00369f * tester;
		}
	}
	else {
		if (GlobalSettings::useFundusFILR_Enable()) {
			m_pixelSizeX = 0.00463f;		//pixel pitch 기준값들 +-7% 범위.
			m_pixelSizeY = 0.00463f;
		}
		else {
			m_pixelSizeX = 0.00369f;
			m_pixelSizeY = 0.00369f;
		}
	}

	m_savedBr = -1.0f;
	m_savedCt = -1.0f;
	m_savedUb = -1.0f;
	m_savedVr = -1.0f;

	// hwajunlee
	m_savedCb = 1.0f;
	m_savedGc = 1.0f;

	m_fixationTarget = 0;
}

ImageFundus::~ImageFundus()
{
}

void ImageFundus::setFundusColor(FundusColor color)
{
	m_colorMode = color;

	return;
}

auto ImageFundus::getFundusColor()->FundusColor
{
	return m_colorMode;
}

void ImageFundus::setFixationTarget(int fix)
{
	m_fixationTarget = fix;

	return;
}

int ImageFundus::getFixationTarget(void)
{
	return m_fixationTarget;
}

void ImageFundus::setAdjustParam(float bright, float contrast, float ub, float vr, float centralBright, float gammaCorrection)
{
	m_displayBr = bright;
	m_displayCt = contrast;
	m_displayUb = ub;
	m_displayVr = vr;
	m_displayCb = centralBright;
	m_displayGc = gammaCorrection;
	return;
}

bool ImageFundus::getAdjustParam(float& out_br, float& out_ct, float& out_ub, float& out_vr, float& out_cb, float& out_gc)
{
	out_br = m_displayBr;
	out_ct = m_displayCt;
	out_ub = m_displayUb;
	out_vr = m_displayVr;
	out_cb = m_displayCb;
	out_gc = m_displayGc;

	return true;
}

void ImageFundus::setAdjustParamSaved(float bright, float contrast, float ub, float vr, float centralBright, float gammaCorrection)
{
	m_savedBr = bright;
	m_savedCt = contrast;
	m_savedUb = ub;
	m_savedVr = vr;
	m_savedCb = centralBright;
	m_savedGc = gammaCorrection;

	return;
}

bool ImageFundus::getAdjustParamSaved(float& out_br, float& out_ct, float& out_ub, float& out_vr, float& out_cb, float& out_gc)
{
	out_br = m_savedBr;
	out_ct = m_savedCt;
	out_ub = m_savedUb;
	out_vr = m_savedVr;
	out_cb = m_savedCb;
	out_gc = m_savedGc;

	return true;
}

void ImageFundus::setBrightness(float value)
{
	m_displayBr = value;
	return;
}

// [2025.08.01] : hwajunlee
float ImageFundus::getBrightness()
{
	return m_displayBr;
}

void ImageFundus::setContrast(float value)
{
	m_displayCt = value;
	return;
}

float ImageFundus::getContrast()
{
	return m_displayCt;
}

void ImageFundus::setUB(float value)
{
	m_displayUb = value;
	return;
}

float ImageFundus::getUB()
{
	return m_displayUb;
}

void ImageFundus::setVR(float value)
{
	m_displayVr = value;
	return;
}

float ImageFundus::getVR()
{
	return m_displayVr;
}

// [2025.05.23].hwajunlee
void ImageFundus::setCentralBright(float value)
{
	m_displayCb = value;
	return;
}

float ImageFundus::getCentralBright()
{
	return m_displayCb;
}

// [2025.05.27].hwajunlee
void ImageFundus::setGC(float value)
{
	m_displayGc = value;
	return;
}

float ImageFundus::getGammaCorrection()
{
	return m_displayGc;
}

void ImageFundus::setEdgeSharpen(bool use, unsigned int kSize)
{
	m_useEdgeSharpen = use;
	m_kernelSize = kSize;

	return;
}

void ImageFundus::setFundusImage(CppUtil::CvImage& image, int type)
{
	switch (type)
	{
	case 1:
		m_imageCustom1.deepCopy(image);
		break;
	case 2:
		m_imageCustom2.deepCopy(image);
		break;
	case 3:
		m_imageCustom3.deepCopy(image);
		break;
	case 4:
		m_imageCustom4.deepCopy(image);
		break;
	}
}

void ImageFundus::setFundusImage(unsigned char *data, unsigned int width, unsigned int height)
{
	if (data == nullptr)
	{
		return;
	}

	m_originImage.fromRGB24(data, width, height);

	return;
}

void ImageFundus::setFundusImageType(int value)
{
	m_fundusImageType = value;
	return;
}

void ImageFundus::setFundusInfo(float focusValue, int flashValue)
{
	m_focusValue = focusValue;
	m_flashValue = flashValue;
	return;
}

void ImageFundus::updateImage(void)
{
	// 이미지 데이터나 color, parameter 바뀔때만 호출한다.
	makeDisplayImage();
	Invalidate();

	return;
}

void ImageFundus::resetAnnotationStatus()
{
	std::vector<int> vecInvalidIds;
	for (auto item : m_annotationList) {
		if (!isAnnotationValid(item)) {
			vecInvalidIds.push_back(item.id());
		}
		item.unSelectAnnotation();
	}

	for (auto item : vecInvalidIds) {
		removeAnnotation(item);
	}
}

void ImageFundus::clearAnnotations()
{
	m_annotationList.clear();
}

void ImageFundus::addAnnotation(int id, int x, int y, std::wstring strContent, int textX, int textY)
{
	int newId = static_cast<int>(m_annotationList.size());

	if (id >= 0) {
		auto selectedItem = std::find_if(m_annotationList.begin(), m_annotationList.end(),
			[id](auto item) { return item.id() == id; });
		if (selectedItem != m_annotationList.end()) {
			(*selectedItem).setContent(strContent);
		}
		else {
			m_annotationList.push_back(
				FundusAnnotation(strContent.length() > 0 ? newId : id, make_pair(x, y),
					make_pair(textX, textY), strContent)
			);
		}
	}
	else {
		m_annotationList.push_back(
			FundusAnnotation(newId, make_pair(x, y), make_pair(textX, textY), strContent)
		);
	}
}

void ImageFundus::removeAnnotation(int id)
{
	std::remove_if(m_annotationList.begin(), m_annotationList.end(),
		[id](auto item) { return item.id() == id; });
	m_annotationList.pop_back();

	int newId = 0;
	for (auto item : m_annotationList) {
		item.setId(newId);
		newId++;
	}
}

int ImageFundus::selectAnnotation(int x, int y)
{
	CRect rcWnd;
	GetClientRect(&rcWnd);

	CRect rcImage(0, 0, m_image.getWidth(), m_image.getHeight());

	// 역순으로 검색한다.
	const int nCnt = (int)m_annotationList.size();
	for (int i = nCnt - 1; i >= 0; i--) {
		auto item = m_annotationList.at(i);
		if (item.selectAnnotation(make_pair(x, y), rcWnd, rcImage)) {
			return item.id();
		}
	}

	return -1;
}

auto ImageFundus::getAnnotationsNum()->const int
{
	return (int)m_annotationList.size();
}

bool ImageFundus::getAnnotationByIndex(int& out_id, std::pair<int, int>& out_pointPos,
	std::pair<int, int>& out_textPos, std::wstring& out_textContent, int index)
{
	if (index < 0 || index >= (int)m_annotationList.size()) {
		return false;
	}

	auto item = m_annotationList.at(index);
	out_id = item.id();
	out_pointPos = item.pointPos();
	out_textPos = item.textPos();
	out_textContent = item.textContent();

	return true;
}

auto ImageFundus::getAnnotation(int index)->std::wstring
{
	if (index < 0 || index >= (int)m_annotationList.size()) {
		return L"";
	}

	return m_annotationList.at(index).textContent();
}

bool ImageFundus::isAnnotationEditingMode()
{
	return m_drawingTool == DrawingTool::annotation;
}

bool ImageFundus::isAnnotationValid(FundusAnnotation& annotation) const
{
	if (annotation.textContent().length() <= 0) {
		return false;
	}
	else {
		return true;
	}
}

bool ImageFundus::saveImageTo(std::string strFileName)
{
	return m_originImage.saveFile(strFileName);
}

void ImageFundus::addFundusAiAnalysisResult(std::wstring name, std::vector<uchar> roiImageData,
	double labelPosXIn, double labelPosYIn)
{
	const double kMinGapHeight = 90.0;

	// avoid label overlaps
	auto labelPosX = labelPosXIn;
	auto labelPosY = labelPosYIn;

	bool bOverlaped = false;
	do {
		bOverlaped = false;
		for (auto item : m_fundusAiAnalysisList) {
			if (abs(item.getLabelPosY() - labelPosY) < kMinGapHeight) {
				labelPosY = item.getLabelPosY() - kMinGapHeight;
				bOverlaped = true;
			}
		}
	} while (bOverlaped);

	m_fundusAiAnalysisList.push_back(FundusAiAnalysis(name, roiImageData, labelPosX, labelPosY));
}

void ImageFundus::resetFundusAiAnalysisResult()
{
	m_fundusAiAnalysisList.clear();
}

void ImageFundus::makeCustomImage(int fundusImageType, bool isSaveAll)
{
	if (m_originImage.isEmpty())
	{
		return;
	}

	if (isSaveAll) {
		fundusImageType = 5;
	}
	
	FundusImageProcess enhancer;

	switch (fundusImageType)
	{
	case 0:
		break;

	case 5: 
		// ALL
		m_originCopy.copyTo(&m_imageCustom1);

		// lv1		
		enhancer.correct(m_imageCustom1.getCvMat(), m_displayBr, m_displayCt, m_displayUb, m_displayVr, m_displayCb, m_displayGc, 1, m_fixationTarget, m_flashValue, m_focusValue);
		if (m_useEdgeSharpen)
		{
			enhancer.edgeEnhance(m_imageCustom1.getCvMat(), m_kernelSize);
		}
		enhancer.translateColor(m_imageCustom1.getCvMat(), m_colorMode, m_emboss);
		enhancer.masking(m_imageCustom1.getCvMat(), m_imageCustom1.getCvMat().rows / 2);

		m_originCopy.copyTo(&m_imageCustom2);

		// lv2
		enhancer.correct(m_imageCustom2.getCvMat(), m_displayBr, m_displayCt, m_displayUb, m_displayVr, m_displayCb, m_displayGc, 2, m_fixationTarget, m_flashValue, m_focusValue);
		if (m_useEdgeSharpen)
		{
			enhancer.edgeEnhance(m_imageCustom2.getCvMat(), m_kernelSize);
		}
		enhancer.translateColor(m_imageCustom2.getCvMat(), m_colorMode, m_emboss);
		enhancer.masking(m_imageCustom2.getCvMat(), m_imageCustom2.getCvMat().rows / 2);

		m_originCopy.copyTo(&m_imageCustom3);

		// lv3
		enhancer.correct(m_imageCustom3.getCvMat(), m_displayBr, m_displayCt, m_displayUb, m_displayVr, m_displayCb, m_displayGc, 3, m_fixationTarget, m_flashValue, m_focusValue);
		if (m_useEdgeSharpen)
		{
			enhancer.edgeEnhance(m_imageCustom3.getCvMat(), m_kernelSize);
		}
		enhancer.translateColor(m_imageCustom3.getCvMat(), m_colorMode, m_emboss);
		enhancer.masking(m_imageCustom3.getCvMat(), m_imageCustom3.getCvMat().rows / 2);


		// modify 2025.05.19 - hwajunlee
		// Fundus image Enhancement option Level 4 
		// White balance -> Central Brightness -> Domain Transform
		//if (GlobalSettings::useFundusFILR_Enable()) {
		//	m_originCopy.copyTo(&m_imageCustom4);
		//	enhancer.correct(m_imageCustom4.getCvMat(), m_displayBr, m_displayCt, m_displayUb, m_displayVr, m_displayCb, m_displayGc, 4, m_fixationTarget, m_flashValue, m_focusValue);
		//	if (m_useEdgeSharpen)
		//	{
		//		enhancer.edgeEnhance(m_imageCustom4.getCvMat(), m_kernelSize);
		//	}
		//	enhancer.translateColor(m_imageCustom4.getCvMat(), m_colorMode, m_emboss);
		//	enhancer.masking(m_imageCustom4.getCvMat(), m_imageCustom4.getCvMat().rows / 2);
		//}
		break;
	}

	return;
}

bool ImageFundus::saveCustomImageTo(CString strFileName, int fundusImageType, bool isSaveAll)
{
	if (isSaveAll) {
		fundusImageType = 5;  // modify 2025.05.19 - hwajunlee
	}

	CString s1 = strFileName + _T("\\fundus_Lv1.jpg");
	CString s2 = strFileName + _T("\\fundus_Lv2.jpg");
	CString s3 = strFileName + _T("\\fundus_Lv3.jpg");

	// modify 2025.05.19 - hwajunlee
	// Fundus image Enhancement option Level 4 
	// White balance -> Central Brightness -> Domain Transform
	//CString s4 = strFileName + _T("\\fundus_Lv4.jpg");

	CT2CA pszConvertedAnsiString1(s1);
	std::string string1(pszConvertedAnsiString1);

	CT2CA pszConvertedAnsiString2(s2);
	std::string string2(pszConvertedAnsiString2);

	CT2CA pszConvertedAnsiString3(s3);
	std::string string3(pszConvertedAnsiString3);

	// modify 2025.05.19 - hwajunlee
	// Fundus image Enhancement option Level 4 
	// White balance -> Central Brightness -> Domain Transform
	//CT2CA pszConvertedAnsiString4(s4);
	//std::string string4(pszConvertedAnsiString4);

	bool isSaved = false;

	switch (fundusImageType)
	{
	case 0:
		break;
	case 1:
		if (!m_image.isEmpty()) {
			isSaved = m_image.saveFile(string1);
		}
		break;
	case 2:
		if (!m_image.isEmpty()) {
			isSaved = m_image.saveFile(string2);
		}
		break;
	case 3:
		if (!m_image.isEmpty()) {
			isSaved = m_image.saveFile(string3);
		}
		break;
	// modify 2025.05.19 - hwajunlee
	// Fundus image Enhancement option Level 4 
	// White balance -> Central Brightness -> Domain Transform
	case 4:
		//if (GlobalSettings::useFundusFILR_Enable()) {
		//	if (!m_image.isEmpty()) {
		//		isSaved = m_image.saveFile(string4);
		//	}
		//}
		isSaved = true;
		break;
	case 5:
		if (!GlobalSettings::useFundusFILR_Enable()) {
			if (!m_imageCustom1.isEmpty() &&
				!m_imageCustom2.isEmpty() &&
				!m_imageCustom3.isEmpty())
			{
				isSaved = m_imageCustom1.saveFile(string1) &&
					m_imageCustom2.saveFile(string2) &&
					m_imageCustom3.saveFile(string3);
			}
		}
		else {
			if (!m_imageCustom1.isEmpty() &&
				!m_imageCustom2.isEmpty() &&
				!m_imageCustom3.isEmpty()

				// modify 2025.05.19 - hwajunlee
				// Fundus image Enhancement option Level 4 
				// White balance -> Central Brightness -> Domain Transform
				//&& !m_imageCustom4.isEmpty()
				)
			{
				isSaved = m_imageCustom1.saveFile(string1) &&
					m_imageCustom2.saveFile(string2) &&
					m_imageCustom3.saveFile(string3)

					// modify 2025.05.19 - hwajunlee
					// Fundus image Enhancement option Level 4 
					// White balance -> Central Brightness -> Domain Transform
					//&& m_imageCustom4.saveFile(string4)
					;
			}
		}
		break;
	default:
		break;
	}
	return isSaved;
}

void ImageFundus::makeDisplayImage(void)
{
	if (m_fundusImageType == 4 && !m_imageCustom4.isEmpty())
	{
		if (m_imageCustom4.isEmpty())
		{
			return;
		}
		makeDisplayImage(m_image.getCvMat(), m_imageCustom4.getCvMat(), m_colorMode);
	}
	else
	{
		if (m_originImage.isEmpty())
		{
			return;
		}
		makeDisplayImage(m_image.getCvMat(), m_originImage.getCvMat(), m_colorMode);
	}

	return;
}

bool ImageFundus::makeDisplayImage(cv::Mat &dstImage, cv::Mat &srcImage, FundusColor color)
{
	srcImage.copyTo(dstImage);
	srcImage.copyTo(m_originCopy.getCvMat());
	
	FundusImageProcess enhancer;
	enhancer.correct(dstImage, m_displayBr, m_displayCt, m_displayUb, m_displayVr,m_displayCb, m_displayGc, m_fundusImageType, m_fixationTarget, m_flashValue, m_focusValue);
	if (m_useEdgeSharpen)
	{
		enhancer.edgeEnhance(dstImage, m_kernelSize);
	}
	enhancer.translateColor(dstImage, color, m_emboss);
	enhancer.masking(dstImage, dstImage.rows / 2);

	return true;
}

bool ImageFundus::isFileExist(CString filePath)
{
	CFile file;
	if (file.Open(CString(filePath), CFile::modeRead)) {
		file.Close();
		return true;
	}
	else {
		return false;
	}
}

void ImageFundus::saveFundusVunoImage(std::wstring wstrResultPath, CString strFundusFilename, CString strVunoFilename)
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	dc.FillSolidRect(rect, RGB(0, 0, 0));

	float pixelPerMM = (float)m_wndPixelPer1MM * m_zoomRatio;

	CRect rtImage;
	rtImage.left = rect.CenterPoint().x - int(m_actualLengthX * pixelPerMM / 2);
	rtImage.right = rect.CenterPoint().x + int(m_actualLengthX * pixelPerMM / 2);
	rtImage.top = rect.CenterPoint().y - int(m_actualLengthY * pixelPerMM / 2);
	rtImage.bottom = rect.CenterPoint().y + int(m_actualLengthY * pixelPerMM / 2);

	CppUtil::CvImage image;

	CString strVunoResultPath(wstrResultPath.c_str());
	CString fundusPath = strVunoResultPath + L"/" + strFundusFilename;
	CString vunoPath = strVunoResultPath + L"/" + strVunoFilename;

	if (isFileExist(fundusPath)) {
		std::wstring path(fundusPath);
		image.fromFile(path, false);

		drawImage(&dc, rtImage, image);
		drawMeasure(&dc, rtImage, rect);

		for (auto item : m_fundusAiAnalysisList) {
			item.drawTo(&dc, rtImage, CRect(0, 0, image.getWidth(), image.getHeight()), rect);
		}

		SaveDCAsFile(&dc, rect, strVunoResultPath + L"/" + strVunoFilename);
	}
}

void ImageFundus::SaveDCAsFile(CDC* pDC, CRect rect, const CString& filePath)
{
	// 1. Memory DC generation
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	// 2. Create bitmap(size of area to save)
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());

	// 3. Attach a bitmap to a memory DC
	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

	// 4. Copy the contents of the original DC to the memory DC (screen capture)
	// BitBlt(targetX, targetY, width, height, sourceDC, sourceX, sourceY, operation)
	memDC.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, rect.left, rect.top, SRCCOPY);

	// 5. Freeing a bitmap from a memory DC
	memDC.SelectObject(pOldBitmap);

	// 6. Calling GDI+ initialization and storage functions (next steps)
	SaveBitmapToFile(&bitmap, filePath);
}

int ImageFundus::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0;          // Number of registered image encoders
	UINT size = 0;         // Size of the encoder information array(in bytes)
	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0) return -1;  // No encoder

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL) return -1; // Memory allocation failure

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		// Check if it matches the requested MIME Type (e.g. "image/jpeg")
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  
		}
	}
	free(pImageCodecInfo);
	return -1; 
}

void ImageFundus::SaveBitmapToFile(CBitmap* pBitmap, const CString& filePath)
{
	HBITMAP hBitmap = (HBITMAP)pBitmap->GetSafeHandle();
	if (hBitmap == NULL) return;

	Gdiplus::Bitmap* pGdiBitmap = Gdiplus::Bitmap::FromHBITMAP(hBitmap, NULL);
	if (pGdiBitmap == NULL) return;

	// Convert file path to WCHAR (GDI+ uses Unicode WCHAR)
	// Use MFC's ATL/MFC conversion macros.
	USES_CONVERSION;
	LPCWSTR wszFilePath = T2CW(filePath);

	CLSID encoderClsid;

	// Find the encoder for the file format you want to save (e.g. JPEG)
	if (GetEncoderClsid(L"image/jpeg", &encoderClsid) >= 0)
	{
		pGdiBitmap->Save(wszFilePath, &encoderClsid);
	}

	delete pGdiBitmap;
}

void ImageFundus::setWndPixelPer1MM(double pixel)
{
	m_wndPixelPer1MM = pixel;

	return;
}

void ImageFundus::setZoomRatio(float ratio)
{
	m_zoomRatio = ratio;

	moveFundusImage();
	return;
}

void ImageFundus::setCenterPos(CPoint point)
{
	m_ptCenter = point;

	moveFundusImage();
	return;
}

void ImageFundus::getCenterPos(CPoint &point)
{
	point = m_ptCenter;

	return;
}

void ImageFundus::moveCenterPos(int offsetX, int offsetY)
{
	m_ptCenter.Offset(offsetX, offsetY);

	moveFundusImage();
}

void ImageFundus::moveFundusImage(void)
{
	int sx = int(m_actualLengthX * m_wndPixelPer1MM * m_zoomRatio);
	int sy = int(m_actualLengthY * m_wndPixelPer1MM * m_zoomRatio);
	int px = int(m_ptCenter.x - (sx / 2));
	int py = int(m_ptCenter.y - (sy / 2));

	MoveWindow(px, py, sx, sy);

	return;
}

BEGIN_MESSAGE_MAP(ImageFundus, ImageBase)
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()


void ImageFundus::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	drawToDC(&dc, rect, (float)m_wndPixelPer1MM * m_zoomRatio);

	return;
}


void ImageFundus::drawToDC(CDC *pDC, CRect rtClient, float pixelPerMM)
{
	pDC->FillSolidRect(rtClient, RGB(0, 0, 0));

	CRect rtImage;
	rtImage.left = rtClient.CenterPoint().x - int(m_actualLengthX * pixelPerMM / 2);
	rtImage.right = rtClient.CenterPoint().x + int(m_actualLengthX * pixelPerMM / 2);
	rtImage.top = rtClient.CenterPoint().y - int(m_actualLengthY * pixelPerMM / 2);
	rtImage.bottom = rtClient.CenterPoint().y + int(m_actualLengthY * pixelPerMM / 2);

	drawImage(pDC, rtImage, m_image);
	drawMeasure(pDC, rtImage, rtClient);

	if (m_showMeasure) {
		for (auto annotation : m_annotationList) {
			annotation.drawTo(pDC, rtImage, CRect(0, 0, m_image.getWidth(), m_image.getHeight()),
				rtClient);
		}

		for (auto item : m_fundusAiAnalysisList) {
 			item.drawTo(pDC, rtImage, CRect(0, 0, m_image.getWidth(), m_image.getHeight()),
				rtClient);
		}
	}
	return;
}

void ImageFundus::drawToDCForReport(CDC *pDC, CRect rtClient, float pixelPerMM, FundusColor color, CRect rtParentWnd)
{
	pDC->FillSolidRect(rtClient, RGB(255, 255, 255));

	CvImage image = CvImage();

	if (m_fundusImageType == 4 && !m_imageCustom4.isEmpty())
	{
		makeDisplayImage(image.getCvMat(), m_imageCustom4.getCvMat(), color);
	}
	else
	{
		makeDisplayImage(image.getCvMat(), m_originImage.getCvMat(), color);
	}

	// masking
	ImageBase::maskIRFundusImage(image.getCvMat(), image.getWidth() / 2);

	CRect rtImage;
	rtImage.left = rtClient.CenterPoint().x - int(m_actualLengthX * pixelPerMM / 2);
	rtImage.right = rtClient.CenterPoint().x + int(m_actualLengthX * pixelPerMM / 2);
	rtImage.top = rtClient.CenterPoint().y - int(m_actualLengthY * pixelPerMM / 2);
	rtImage.bottom = rtClient.CenterPoint().y + int(m_actualLengthY * pixelPerMM / 2);

	//
	CvImage imageToDraw;
	image.copyTo(&imageToDraw);
	if (rtImage.Width() > rtClient.Width() || rtImage.Height() > rtClient.Height()) {
		// get offset
		float imageSizeInParentWndW = (float)m_windowSizeX / m_zoomRatio;
		float imageSizeInParentWndH = (float)m_windowSizeY / m_zoomRatio;
		float distXFromCenterInParentWnd = (float)m_ptCenter.x - (float)rtParentWnd.Width() / 2.0f;
		float distYFromCenterInParentWnd = (float)m_ptCenter.y - (float)rtParentWnd.Height() / 2.0f;
		float distXFromCenterInImage = distXFromCenterInParentWnd * ((float)rtImage.Width() / imageSizeInParentWndW);
		float distYFromCenterInImage = distYFromCenterInParentWnd * ((float)rtImage.Height() / imageSizeInParentWndH);
		float cpXInImage = (float)rtImage.Width() / 2.0f - distXFromCenterInImage / 2.0f;
		float cpYInImage = (float)rtImage.Height() / 2.0f - distYFromCenterInImage / 2.0f;

		// resize image to display
		imageToDraw.resize(rtImage.Width(), rtImage.Height());

		// 보여 줄 이미지 영역 결정
		int cropW = static_cast<int>((float)rtImage.Width() / m_zoomRatio);
		int cropH = static_cast<int>((float)rtImage.Height() / m_zoomRatio);
		int cropX = static_cast<int>(cpXInImage - (float)cropW / 2.0f);
		int cropY = static_cast<int>(cpYInImage - (float)cropH / 2.0f);

		// 영역 설정 예외처리
		if (cropX < 0) {
			cropX = 0;
		}
		else if (cropX + cropW > rtImage.Width()) {
			cropX = rtImage.Width() - cropW;
		}
		if (cropY < 0) {
			cropY = 0;
		}
		else if (cropY + cropH > rtImage.Height()) {
			cropY = rtImage.Height() - cropH;
		}

		// 4의 배수로 맞춤
		cropX = cropX - cropX % 4;
		cropY = cropY - cropY % 4;
		cropW = cropW - cropW % 4;
		cropH = cropH - cropH % 4;
		
		// crop
		imageToDraw.crop(cropX, cropY, cropW, cropH);

		// draw image
		drawImage(pDC, rtClient, imageToDraw);

		// draw measure
		float dispImageSizeRatioW = (float)rtClient.Width() / (float)cropW;
		float dispImageSizeRatioH = (float)rtClient.Height() / (float)cropH;
		CRect rtMeasure(
			rtImage.left,
			rtImage.top,
			rtImage.left + static_cast<int>((float)rtImage.Width() * dispImageSizeRatioW),
			rtImage.top + static_cast<int>((float)rtImage.Height() * dispImageSizeRatioH));
		int offsetX = rtImage.Width() / 2 - static_cast<int>((float)(cropX + cropW / 2) * dispImageSizeRatioW);
		int offsetY = rtImage.Height() / 2 - static_cast<int>((float)(cropY + cropH / 2) * dispImageSizeRatioW);
		rtMeasure.OffsetRect(POINT{ offsetX, offsetY });

		drawMeasure(pDC, rtMeasure, rtClient);

		if (m_showMeasure) {
			for (auto annotation : m_annotationList) {
				annotation.drawTo(pDC, rtMeasure, CRect(0, 0, m_image.getWidth(), m_image.getHeight()),
					rtClient);
			}

			for (auto item : m_fundusAiAnalysisList) {
				item.drawTo(pDC, rtMeasure, CRect(0, 0, m_image.getWidth(), m_image.getHeight()),
					rtClient);
			}
		}
	}
	else {
		drawImage(pDC, rtImage, imageToDraw);
		drawMeasure(pDC, rtImage, rtClient);

		if (m_showMeasure) {
			for (auto annotation : m_annotationList) {
				annotation.drawTo(pDC, rtImage, CRect(0, 0, m_image.getWidth(), m_image.getHeight()),
					rtClient);
			}

			for (auto item : m_fundusAiAnalysisList) {
				item.drawTo(pDC, rtClient, CRect(0, 0, m_image.getWidth(), m_image.getHeight()),
					rtImage);
			}
		}
	}

	return;
}

void ImageFundus::setEmbossing(int emboss)
{
	switch (emboss) {
	case 0:
		m_emboss = FundusImageProcess::EmbossType::EMBOSS_TYPE_NONE;
		break;
	case 1:
		m_emboss = FundusImageProcess::EmbossType::EMBOSS_TYPE_1;
		break;
	case 2:
		m_emboss = FundusImageProcess::EmbossType::EMBOSS_TYPE_2;
		break;
	default:
		m_emboss = FundusImageProcess::EmbossType::EMBOSS_TYPE_NONE;
		break;
	}
}

int ImageFundus::getEmbossing()
{
	switch (m_emboss) {
	case FundusImageProcess::EmbossType::EMBOSS_TYPE_NONE:
		return 0;
	case FundusImageProcess::EmbossType::EMBOSS_TYPE_1:
		return 1;
	case FundusImageProcess::EmbossType::EMBOSS_TYPE_2:
		return 2;
	default:
		return 0;
	}
}

void ImageFundus::drawImage(CDC *pDC, CRect rect, CvImage image)
{
	if (image.isEmpty())
	{
		return;
	}

	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);

	HBITMAP hbmp = image.createDIBitmap(hDC, true);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	SetStretchBltMode(hDC, HALFTONE);
	SetBrushOrgEx(hDC, 0, 0, NULL);
	//StretchBlt(hDC, rect.left, rect.top, rect.Width(), rect.Height(), 
	//	hMemDC, 0, 0, getImgWidth(), getImgHeight(), SRCCOPY);

	StretchBlt(hDC, rect.left, rect.top, rect.Width(), rect.Height(),
		hMemDC, 0, 0, image.getWidth(), image.getHeight(), SRCCOPY);

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	DeleteObject(hbmp);

	return;
}

BOOL ImageFundus::PreTranslateMessage(MSG* pMsg)
{
	CRect rect;
	GetWindowRect(&rect);

	CPoint point;
	point.x = pMsg->pt.x - rect.left;
	point.y = pMsg->pt.y - rect.top;

	if (m_drawingTool == DrawingTool::length ||	m_drawingTool == DrawingTool::angle || m_drawingTool == DrawingTool::areaPolygon)
	{
		if (pMsg->message == WM_LBUTTONDOWN)
		{
			insertMeasurePoint(point);
			Invalidate();
		}

		return TRUE;
	}
	else if (m_drawingTool == DrawingTool::areaCupDisc)
	{
		static bool dragStart = false;
		static bool isDiscPoint = false;
		static int pointIndex = -1;

		if (pMsg->message == WM_LBUTTONDOWN)
		{
			if (hasCupDiscPoints() == false)
			{
				createCupDiscPoints(point);
				Invalidate();
			}
			else if (getCupDiscPoint(point, isDiscPoint, pointIndex))
			{
				dragStart = true;
			}
		}
		else if (pMsg->message == WM_LBUTTONUP)
		{
			dragStart = false;
		}
		else if (pMsg->message == WM_MOUSEMOVE)
		{
			if (dragStart)
			{
				moveCupDiscPoint(point, isDiscPoint, pointIndex);
				Invalidate();
			}
		}

		return TRUE;
	}

	return ImageBase::PreTranslateMessage(pMsg);
}

void ImageFundus::OnSize(UINT nType, int cx, int cy)
{
	ImageBase::OnSize(nType, cx, cy);

	m_windowSizeX = cx;
	m_windowSizeY = cy;

	return;
}
