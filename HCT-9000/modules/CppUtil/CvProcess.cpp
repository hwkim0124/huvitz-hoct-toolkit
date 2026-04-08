#include "stdafx.h"
#include "CvProcess.h"
#include "CvImage.h"


#include "CppUtil2.h"

using namespace CppUtil;
using namespace cv;
using namespace std;


struct CvProcess::CvProcessImpl
{
	cv::Mat matSum;
	cv::Mat matBase;
	cv::Mat matMask;
	int countOfSum;

	CvProcessImpl()
	{
	}
};


CvProcess::CvProcess() :
	d_ptr(make_unique<CvProcessImpl>())
{
}


CvProcess::~CvProcess() = default;
CppUtil::CvProcess::CvProcess(CvProcess && rhs) = default;
CvProcess & CppUtil::CvProcess::operator=(CvProcess && rhs) = default;


CppUtil::CvProcess::CvProcess(const CvProcess & rhs)
	: d_ptr(make_unique<CvProcessImpl>(*rhs.d_ptr))
{
}


CvProcess & CppUtil::CvProcess::operator=(const CvProcess & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool CppUtil::CvProcess::startAveraging(CvImage & image)
{
	if (image.isEmpty()) {
		getImpl().countOfSum = 0;
		return false;
	}

	getImpl().matBase = image.getCvMat().clone();
	getImpl().matMask = Mat(image.getHeight(), image.getWidth(), CV_32SC1, Scalar(1));

	getImpl().matSum = Mat(image.getHeight(), image.getWidth(), CV_32SC1, Scalar(0));
	
	Mat matSrc;
	getImpl().matBase.convertTo(matSrc, CV_32SC1);
	getImpl().matSum += matSrc;

	getImpl().countOfSum = 1;
	return true;
}


bool CppUtil::CvProcess::insertAveraging(CvImage & image, double ccLimit, int numIters, double termEps)
{
	if (image.isEmpty()) {
		return false;
	}

	Mat matWarp = Mat::eye(2, 3, CV_32F);
	const int motionType = MOTION_EUCLIDEAN; // MOTION_AFFINE; // MOTION_TRANSLATION;// MOTION_EUCLIDEAN;

	TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, numIters, termEps);

	try {
		Mat matSrc = image.getCvMat();
		Mat matDst;
		double cc = findTransformECC(getImpl().matBase, matSrc, matWarp, motionType, criteria);

		LogD() << "Transform ecc: " << cc << ", limit: " << ccLimit << ", countOfSum: " << getImpl().countOfSum;

		if (cc > ccLimit) {
			warpAffine(matSrc, matDst, matWarp, matSrc.size(), INTER_LINEAR | WARP_INVERSE_MAP);
			Mat mat32s;
			matDst.convertTo(mat32s, CV_32SC1);
			//cv::add(getImpl().matSum, mat32s, getImpl().matSum);
			getImpl().matSum += mat32s;

			Mat matCnt;
			matCnt = cv::min(mat32s, 1);
			//matCnt.convertTo(mat32s, CV_32SC1);
			//getImpl().matMask += matCnt;
			//cv::add(getImpl().matMask, matCnt, getImpl().matMask);
			getImpl().matMask += matCnt;
			getImpl().countOfSum++;
			return true;
		}
	}
	catch (...) {
	}
	return false;
}


int CppUtil::CvProcess::closeAveraging(CvImage & image)
{
	if (getImpl().countOfSum > 1) {
		getImpl().matSum /= getImpl().matMask;
	}
	getImpl().matSum.convertTo(image.getCvMat(), CV_8UC1);
	return getImpl().countOfSum;
}


bool CppUtil::CvProcess::registerAngioImages(int lineIdx, int imageIdx, CvImage & imgRefer, CvImage & imgRegist, int numIters, double termEps, double ccLimit)
{
	Mat matWarp = Mat::eye(2, 3, CV_32F);
	const int motionType = MOTION_TRANSLATION;// MOTION_EUCLIDEAN;

	TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, numIters, termEps);

	int w = imgRefer.getWidth();
	int h = imgRefer.getHeight();
	Rect roi(0, 0, w, 512);

	Mat matRef = imgRefer.getCvMat();
	Mat matSrc = imgRegist.getCvMat();
	Mat matDst;

	try {
		double cc = findTransformECC(matRef(roi), matSrc(roi), matWarp, motionType, criteria);
		// LogD() << "Angio line: " << lineIdx << ", image: " << imageIdx << ", transform cc: " << cc;

		if (cv::sum(matWarp != matWarp)[0] > 0) {
			LogD() << "Angio line: " << lineIdx << ", image: " << imageIdx << ", transform failed as NaN!";
			return false;
		}

		// Ignore lateral translation.
		matWarp.at<float>(0, 2) = 0.0f;

		if (cc < ccLimit) {
			LogD() << "Angio line: " << lineIdx << ", image: " << imageIdx << ", transform cc: " << cc << " is under the limit: " << ccLimit;
			// return false;
		}

		// LogD() << matWarp.at<float>(1, 0) << ", " << matWarp.at<float>(1, 1) << ", " << matWarp.at<float>(1, 2);
		warpAffine(matSrc, matDst, matWarp, matSrc.size(), INTER_CUBIC | WARP_INVERSE_MAP);
		matDst.setTo(0, matDst < 0);
		imgRegist.getCvMat() = matDst.clone();
		return true;
	}
	catch (const std::exception &ex) {
		LogD() << ex.what() << std::endl;
	}
	LogD() << "Angio line: " << lineIdx << ", image: " << imageIdx << ", registration failed!";
	return false;
}


bool CppUtil::CvProcess::computeTranslation(CvImage& imgRefer, CvImage& imgImage, float& transX, float& transY, float& ecc, int numIters, double termEps, double ccLimit)
{
	Mat matWarp = Mat::eye(2, 3, CV_32F);
	const int motionType = MOTION_TRANSLATION;

	TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, numIters, termEps);

	int w = imgRefer.getWidth();
	int h = imgRefer.getHeight();
	Rect roi(0, 0, w, 512);

	Mat matRef = imgRefer.getCvMat();
	Mat matSrc = imgImage.getCvMat();

	transX = transY = ecc = 0.0f;

	try {
		double cc = findTransformECC(matRef(roi), matSrc(roi), matWarp, motionType, criteria);

		if (cv::sum(matWarp != matWarp)[0] > 0) {
			LogD() << "Transform failed as NaN!";
			return false;
		}

		transX = matWarp.at<float>(0, 2);
		transY = matWarp.at<float>(1, 2);
		ecc = (float)cc;
		return true;
	}
	catch (const std::exception& ex) {
		LogD() << ex.what() << std::endl;
	}
	return false;
}


CvProcess::CvProcessImpl & CppUtil::CvProcess::getImpl(void)
{
	return *d_ptr;
}
