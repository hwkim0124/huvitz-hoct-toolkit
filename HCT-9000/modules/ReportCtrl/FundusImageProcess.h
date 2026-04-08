#pragma once
#include "ReportCtrlDef.h"

class FundusImageProcess
{
public:
	typedef enum _EmbossType {
		EMBOSS_TYPE_NONE = 0,
		EMBOSS_TYPE_1,
		EMBOSS_TYPE_2,
	} EmbossType;

public:
	FundusImageProcess();
	virtual ~FundusImageProcess();

	void edgeEnhance(cv::Mat& mat, unsigned int ksize = 19);
	void correct(cv::Mat & mat, float mtgt, float dtgt, float ub, float vr, float cb, float gm, int fundusImageType, int fixation, int flashValue, float focusValue);
	void translateColor(cv::Mat& mat, ReportCtrl::FundusColor color, EmbossType emboss);
	void masking(cv::Mat &mat, unsigned int radius);

	std::pair<float, float> getMedian(unsigned char *buf, int width);
	void applyEmbossing(cv::Mat& mat, EmbossType type);

	void ApplyRadialBrightnessMask(cv::Mat & mat, float brightness, float gamma);


	bool correctGamma(cv::Mat & mat, double gamma);
	cv::Mat enhance_contrast(cv::Mat & image, float clip, int tile);
	cv::Mat cicle_mask(cv::Mat & mat, int rad_out, int rad_inn = 0);
	void shading_correction(cv::Mat & mat);
	void postImageProcess(cv::Mat & mat, int m_fundusImageType);
	void applyRemoveReflectionLight(cv::Mat & mat, int lightRadius, int shadowRadius, int peripheryRadius, int flashValue, float focusValue);
	void applyRemoveReflectionLight2(cv::Mat & mat, int lightRadius, int shadowRadius, int peripheryRadius, int flashValue, float focusValue);
	bool isAvailableValue(int flashValue, float focusValue);

	cv::Mat loadROIData(const std::string& filename, int index);
	void postImageProcess2(cv::Mat& image, int level);
	void postImageProcess2_SonyCamera(cv::Mat& image, int level, bool oldSet);
	
private:
	
	void getYUV(float *y, float *u, float *v, float r, float g, float b);
	void getRGB(float *r, float *g, float *b, float y, float u, float v);
	std::pair<float, float> getMeanStdDev(unsigned char *buf, int width, int xoff, int yoff, int side);

};
