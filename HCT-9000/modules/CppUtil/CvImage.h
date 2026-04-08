#pragma once

#include "CppUtilDef.h"

#include <string>
#include <memory>
#include <vector>

namespace cv {
	class Mat;
}

namespace Gdiplus {
	class Bitmap;
}


namespace CppUtil
{
	struct FundusProcessParams
	{
		bool useFundusFILR;
		bool useRemoveLight;
		int removeLight1;
		int removeLight2;
		int removeLight3;
		bool measureWhiteBalance;
		int whiteBalanceB;
		int whiteBalanceG;
		int whiteBalanceR;
		bool measureCentralBrightness;
		bool measureDomainTransform;
		float fundusbrightness;
		float fundusbrightnessgamma;
	};

	class CPPUTIL_DLL_API CvImage
	{
	public:
		CvImage();
		virtual ~CvImage();

		CvImage(CvImage&& rhs);
		CvImage& operator=(CvImage&& rhs);
		CvImage(const CvImage& rhs);
		CvImage& operator=(const CvImage& rhs);

		enum class FlipMode { NONE = 0, VERTICAL, HORIZONTAL, BOTH };
		enum class MaskType { NONE = 0, CIRCLE };

	private:
		struct CvImageImpl;
		std::unique_ptr<CvImageImpl> d_ptr;

	public:
		bool fromFile(const std::wstring path, bool grayscale = true);
		bool fromBitsData(const unsigned char* pBits, int width, int height, int padding = 0);
		bool fromBitsDataNoCopy(const unsigned char* pBits, int width, int height, int padding = 0);
		bool fromFloat32(const unsigned char* pData, int width, int height, int padding = 0);
		bool fromRaw16(const unsigned char* pData, int width, int height, int padding = 0);
		bool fromRGB24(const unsigned char* pData, int width, int height, int padding = 0);
		bool fromBayer16s(const unsigned char* pData, int width, int height, FlipMode flip = FlipMode::NONE, bool useFundusFILR = false);

		unsigned char* getBitsData(void) const;
		bool saveFile(const std::string path, bool vflipt = false, unsigned int imgQuality = 95);
		bool saveFileAsBinary(const std::string path);
		bool saveFileAsRaw(const std::string path);
		bool saveFileAsYml(const std::string path, std::string name = "OpenCvMatrix");
		bool saveFileAsCV16UC1(const std::string path);
		bool loadFileAsYml(const std::string path, std::string name = "OpenCvMatrix");

		void flipVertical(void);
		void flipHorizontal(void);
		void rotate90(bool cw = true);

		unsigned char getAtInByte(int x, int y);
		float getAtInFloat(int x, int y);
		unsigned char getRedAt(int x, int y);
		unsigned char getBlueAt(int x, int y);
		unsigned char getGreenAt(int x, int y);

		std::vector<unsigned char> copyDataInBytes(void) const;
		std::vector<float> copyDataInFloats(void) const;
		void convertToFloat(void);
		void convertToUchar(void);

		int getWidth(void) const;
		int getHeight(void) const;
		int getRows(void) const;
		int getCols(void) const;
		int getWidthStep(void) const;
		int getByteSize(void) const;
		bool isEmpty(void) const;
		void clear(void);

		void correctGamma(double gamma);

		void applyBilateralFilter(int distance, double sigmaColor, double sigmaSpace);
		void applyGaborFilter(int ksize1, int ksize2, int kstep, int orients, double divider, double sigma, double lambda = 10.0, double gamma = 1.0, double psi = 0);
		void applyGaussianBlur(double sigma, int ksize = 0, int ksize2 = 0);
		void applyWeighted(CvImage* image, double alpha, double beta, double gamma = 0.0);
		void applyMedianBlur(int ksize);
		void applyGuidedFilter(int r = 1, double eps = 0.1, int depth = -1);

		void applyBinarized(int thresh);
		void applyMorphologyTransform(int ksize);
		void applyGradientConvolution(int kernRows, int kernCols, bool ascent = true);
		void gradients(int ksize, CvImage* dest = nullptr);

		void applyRemoveReflectionLight(int lightRadius, int shadowRadius, int peripheryRadius);
		void applyPreprocessingFAZ(float threshold);
		std::vector<double> applyDewarping(std::vector<int> layerEPI, float resX, float resY, double reflactiveIndex);

		int calculateCenterMean(int lightRadius);
		int calculateOverBrightPixel(int lightRadius, int threshold);

		void equalizeHistogram(double clipLimit = 16.0, int addScalar = 8, bool nocopy = false);
		void normalizeImage(double min, double max);
		bool processGrayWorld(float rateRadius, float* corrR, float* corrB);
		void balanceColorByGrayWorld(float corrR, float corrB);

		void processRadialColorCorrection(std::vector<int>& xs, std::vector<int>& ys);
		void processRadialColorCorrection(std::vector<int>& xs, std::vector<int>& ys,
			std::pair<int, int>& roi0, std::pair<int, int>& roi1, bool useFundusFILR = false);

		void denoise();
		void blur(int xsize, int ysize, double xsigma = 0.0, double ysigma = 0.0);

		bool convertToRGB(CvImage* pRGB) const;
		bool convertRGB16ToRGB8(double alpha, double beta) const;

		//new Fundus Image Process
		bool domainTransform(float sigma_s = 60.0f, double sigma_r = 0.4f);
		cv::Mat ImageFusion(const cv::Mat& original, const cv::Mat& background, double a);
		bool saveFile_tiff(const std::string path);
		bool AdjustWhiteBalance(int b, int g, int r);
		bool ApplyRadialBrightnessMask(float brightness, float gamma);
		cv::Mat GammaCorrection(const cv::Mat& src, double gamma);
		void ApplyGammaCorrectEnhanceContrast(CvImage* pRGB);
		bool deepCopy(CppUtil::CvImage image);
		void RemoveCalibration(float focusValue, int flashLevel);
		double computePercentile(const cv::Mat& channel, double percentile);
		void convert16UC3To8UC3();
		bool processImageLv4Optimized(const FundusProcessParams& params, float focusValue, int fixation);
		//

		cv::Mat EnhanceContrast(const cv::Mat& src, float clipLimit, int tileGridSize);

		void applyMaskROI(std::pair<int, int> center, int radius, bool resize);
		void drawCircleMask(int x1, int y1, int x2, int y2);
		void drawCircleMask(int cx, int cy, int rad, bool invert = false);

		Gdiplus::Bitmap* createBitmap(void) const;
		Gdiplus::Bitmap* getBitmap(bool redraw = false);

		bool resize(int width, int height);
		bool resizeTo(CvImage* image, int width, int height) const;
		bool resizeFastTo(CvImage* image, int width, int height) const;
		bool crop(int x0, int y0, int w, int h);
		void copyTo(CvImage* image) const;
		void copyTo(CvImage* image, int x, int y, int w, int h) const;

		HBITMAP createDIBitmap(HDC hDC, bool vflip = false) const;

		cv::Mat& getCvMat(void) const;
		cv::Mat* getCvMatPtr(void) const;
		const cv::Mat& getCvMatConst(void) const;

		void getMeanStddev(float* mean, float* stddev);
		void getMinMaxValue(double* minval, double* maxval);
		void getMeanStddevInCircle(float* mean, float* stddev, int radius);
		bool reduceMeanStddev(int dimen, std::vector<float>& mean, std::vector<float>& stddev);
		int getPercentile(int pval, int step, int radius);

		void applyRemoveReflectionLight(int lightRadius, int shadowRadius, int peripheryRadius, float focusValue);
		void applyBiasFieldCorrection(bool vertical);
		void applyVesselEnhancement(double sigma, double alpha, double beta);
	};
}
