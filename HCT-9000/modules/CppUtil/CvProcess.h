#pragma once

#include "CppUtilDef.h"

#include <string>
#include <memory>


namespace cv {
	class Mat;
}


namespace CppUtil
{
	class CvImage;

	class CPPUTIL_DLL_API CvProcess
	{
	public:
		CvProcess();
		virtual ~CvProcess();

		CvProcess(CvProcess&& rhs);
		CvProcess& operator=(CvProcess&& rhs);
		CvProcess(const CvProcess& rhs);
		CvProcess& operator=(const CvProcess& rhs);

	public:
		bool startAveraging(CvImage& image);
		// bool insertAveraging(CvImage& image, double ccLimit = 0.90, int numIters = 1000, double termEps = 1e-3);
		bool insertAveraging(CvImage& image, double ccLimit = 0.90, int numIters = 100, double termEps = 1e-2);
		int closeAveraging(CvImage& image);

		bool registerAngioImages(int lineIdx, int imageIdx, CvImage& imgRefer, CvImage& imgRegist, int numIters = 500, double termEps = 1e-4, double ccLimit = 0.85);
		bool computeTranslation(CvImage& imgRefer, CvImage& imgImage, float& transX, float& transY, float& ecc, int numIters = 500, double termEps = 1e-4, double ccLimit = 0.85);

	private:
		struct CvProcessImpl;
		std::unique_ptr<CvProcessImpl> d_ptr;
		CvProcess::CvProcessImpl& getImpl(void);
	};
}

