#pragma once

#include "RetSegmDef.h"

#include <string>
#include <memory>
#include <vector>

namespace cv {
	class Mat;
}

namespace Gdiplus {
	class Bitmap;
}

namespace RetSegm
{
	class RETSEGM_DLL_API ImageMat
	{
	public:
		ImageMat();
		virtual ~ImageMat();

	private:
		// cv::Mat_<uchar> m_imageMat;
		std::unique_ptr<cv::Mat> m_pCvMat;
		std::vector<int> m_meanCols;
		std::vector<int> m_maxValCols;
		std::vector<int> m_maxIdxCols;

		float m_mean;
		float m_stddev;
		float m_sampleRatioWidth;
		float m_sampleRatioHeight;

	public:
		bool fromFile(std::wstring path);
		bool fromBitmap(int width, int height, const unsigned char* pBits, int paddingSize);
		const unsigned char* getBitsData(void) const;
		int getWidth(void) const;
		int getHeight(void) const;
		int getRows(void) const;
		int getCols(void) const;
		int getWidthStep(void) const;
		int getByteSize(void) const;

		void convertToRGB(ImageMat* pRGBMat) const;
		Gdiplus::Bitmap* createBitmap(void) const;
		cv::Mat& getCvMat(void);
		const cv::Mat& getCvMatConst(void) const;

		void setMeanStddev(float mean, float stddev);
		inline float getMean(void) {
			return m_mean;
		} 
		inline float getStddev(void) {
			return m_stddev;
		}
		inline float getSampleRatioWidth(void) const {
			return m_sampleRatioWidth;
		}
		inline float getSampleRatioHeight(void) const {
			return m_sampleRatioHeight;
		}
		inline void setSampleRatio(float widthRatio, float heightRatio) {
			m_sampleRatioWidth = widthRatio;
			m_sampleRatioHeight = heightRatio;
		}

		void setMeanOfColumns(const cv::Mat& mean);
		void setMaxValOfColumns(const cv::Mat& vals);
		void setMaxIdxOfColumns(const cv::Mat& idxs);


	private:
		// Even though friend declarations violates ImageMat's encapsulation, 
		// theses classes implement algorithms processing ImageMat's internal data as input, 
		// and its output could be got back and stored within ImageMat for the further uses
		// without through a little cumbersome interfaces. 

		// This is restricted to the classes only for the purpose of data processing to ImageMat. 
		friend class Preprocess;
		friend class Coarse;
	};
}
