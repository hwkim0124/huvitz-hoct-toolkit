#pragma once
#include "ImageBase.h"
#include "ReportCtrlDef.h"
#include "FundusImageProcess.h"
#include "FundusAnnotation.h"
#include "FundusAiAnalysis.h"

class REPORTCTRL_DLL_API ImageFundus : public ImageBase
{
public:
	ImageFundus();
	virtual ~ImageFundus();

	void setFundusColor(FundusColor color);
	auto getFundusColor()->FundusColor;
	void setAdjustParam(float br, float ct, float ub, float vr, float centralBright, float gammaCorrection);
	bool getAdjustParam(float& out_br, float& out_ct, float& out_ub, float& out_vr, float& out_cb, float& out_gc);
	void setAdjustParamSaved(float br, float ct, float ub, float vr, float centralBright, float gammaCorrection);
	bool getAdjustParamSaved(float& out_br, float& out_ct, float& out_ub, float& out_vr, float& out_cb, float& out_gc);
	void setBrightness(float br);
	float getBrightness(); // [2025.08.01] : hwajunlee
	void setContrast(float ct);
	float getContrast(); // [2025.08.12] : hwajunlee
	void setUB(float ub);
	float getUB();
	void setVR(float vr);
	float getVR();
	void setCentralBright(float func); // [2025.05.23].hwajunlee
	float getCentralBright();
	void setGC(float gc); // [2025.05.27].hwajunlee
	float getGammaCorrection();
	void setEdgeSharpen(bool use, unsigned int kSize);
	void setFundusImage(CppUtil::CvImage& image, int type);
	void setFundusImage(unsigned char *data, unsigned int width, unsigned int height);
	void setFundusImageType(int value);
	void updateImage(void);
	
	void setWndPixelPer1MM(double pixel);
	void setZoomRatio(float ratio);
	void setCenterPos(CPoint point);
	void getCenterPos(CPoint &point);
	void moveCenterPos(int offsetX, int offsetY);

	void drawToDC(CDC *pDC, CRect rtClient, float pixelPerMM);
	void drawToDCForReport(CDC *pDC, CRect rtClient, float pixelPerMM, FundusColor color, CRect rtParentWnd);

	void setEmbossing(int emboss);
	int getEmbossing();

	void setFixationTarget(int fix);
	int getFixationTarget(void);

	void resetAnnotationStatus();
	void clearAnnotations();
	void addAnnotation(int id, int x, int y, std::wstring strContent, int textX, int textY);
	void removeAnnotation(int id);
	int selectAnnotation(int x, int y);
	auto getAnnotationsNum()->const int;
	bool getAnnotationByIndex(int& out_id, std::pair<int, int>& out_pointPos,
		std::pair<int, int>& out_textPos, std::wstring& out_textContent, int index);
	auto getAnnotation(int index)->std::wstring;
	bool isAnnotationEditingMode();
	bool isAnnotationValid(FundusAnnotation& annotation) const;
	bool saveImageTo(std::string strFileName);
	void addFundusAiAnalysisResult(std::wstring name, std::vector<uchar> roiImageData,
		double labelPosXIn, double labelPosYIn);
	void resetFundusAiAnalysisResult();

	void makeCustomImage(int fundusImageType, bool isSaveAll = false);
	bool saveCustomImageTo(CString strFileName, int fundusImageType, bool isSaveAll = false);

	void setFundusInfo(float focusValue, int flashValue);

	void makeDisplayImage(void);

	bool isFileExist(CString filePath);

	void saveFundusVunoImage(std::wstring wstrResultPath, CString strFundusFilename, CString strVunoFilename);
	void SaveDCAsFile(CDC* pDC, CRect rect, const CString& filePath);
	void SaveBitmapToFile(CBitmap* pBitmap, const CString& filePath);
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

protected:
	bool makeDisplayImage(cv::Mat &dstImage, cv::Mat &srcImage, FundusColor color);

	void moveFundusImage(void);

	void drawImage(CDC *pDC, CRect rect, CppUtil::CvImage image);

private:
	FundusColor m_colorMode;

	CppUtil::CvImage m_originImage;
	CppUtil::CvImage m_originCopy;
	CppUtil::CvImage m_imageCustom1;
	CppUtil::CvImage m_imageCustom2;
	CppUtil::CvImage m_imageCustom3;

	// modify 2025.05.19 - hwajunlee
	// Fundus image Enhancement option Level 4 
	CppUtil::CvImage m_imageCustom4;

	bool m_useCorrection;
	float m_displayBr;
	float m_displayCt;
	float m_displayUb;
	float m_displayVr;
	float m_displayCb; // [2025.05.23].hwajunlee
	float m_displayGc; // [2025.05.27].hwajunlee
	bool m_useEdgeSharpen;
	int m_kernelSize;

	double m_wndPixelPer1MM;
	float m_zoomRatio;
	CPoint m_ptCenter;
	
	float m_savedBr;
	float m_savedCt;
	float m_savedUb;
	float m_savedVr;
	float m_savedCb;  // hwajunlee, Central Bright
	float m_savedGc; // hwajunlee, Gamma Correction

	int m_fundusImageType;
	int m_fixationTarget;

	float m_focusValue;
	int m_flashValue;

	FundusImageProcess::EmbossType m_emboss;
	std::vector<FundusAnnotation> m_annotationList;
	std::vector<FundusAiAnalysis> m_fundusAiAnalysisList;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

