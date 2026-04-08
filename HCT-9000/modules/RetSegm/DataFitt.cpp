#include "stdafx.h"
#include "DataFitt.h"
#include "ImageMat.h"
#include "Boundary.h"
#include "CppUtil2.h"
#include "SegmLayer.h"

using namespace RetSegm;
using namespace std;

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;


DataFitt::DataFitt()
{
}



bool RetSegm::DataFitt::interpolateBoundaryByLinearFitting(const std::vector<int>& input, std::vector<int>& output, bool sideFitt)
{
	const int fittSize = (int)(COARSE_LINEAR_FITT_DATA_SIZE * input.size());

	float slope = 0.0f;
	int count = 0;
	int size = (int)input.size();
	output = input;

	int x1 = -1, x2 = -1;
	int y1 = -1, y2 = -1;

	if (input[0] < 0) {
		for (int i = 0; i < size; i++) {
			if (input[i] >= 0) {
				if (!sideFitt) {
					for (int j = 0; j < i; j++) {
						output[j] = input[i];
					}
				}
				else {
					int xs1 = 0, xs2 = 0;
					int ys1 = 0, ys2 = 0;
					int cnt1 = 0, cnt2 = 0;
					int j;

					for (j = i; j < size; j++) {
						if (input[j] >= 0) {
							xs1 += j;
							ys1 += input[j];
							if (++cnt1 >= fittSize) {
								x1 = xs1 / cnt1;
								y1 = ys1 / cnt1;
								break;
							}
						}
					}

					for (j = j + 1; j < size; j++) {
						if (input[j] >= 0) {
							xs2 += j;
							ys2 += input[j];
							if (++cnt2 >= fittSize) {
								x2 = xs2 / cnt2;
								y2 = ys2 / cnt2;
								break;
							}
						}
					}

					if (x1 >= 0 && x2 >= 0 && x2 > x1) {
						slope = (float)(y2 - y1) / (float)(x2 - x1);
						for (int k = (x1 - 1), dist = -1; k >= 0; k--, dist--) {
							output[k] = (int)(y1 + dist * slope);
							output[k] = max(0, output[k]);
						}
					}
				}
				break;
			}
		}
	}

	if (input[size - 1] < 0) {
		for (int i = size - 1; i >= 0; i--) {
			if (input[i] >= 0) {
				if (!sideFitt) {
					for (int j = i + 1; j < size; j++) {
						output[j] = input[i];
					}
				}
				else {
					int xs1 = 0, xs2 = 0;
					int ys1 = 0, ys2 = 0;
					int cnt1 = 0, cnt2 = 0;
					int j;

					for (j = i - 1; j >= 0; j--) {
						if (input[j] >= 0) {
							xs2 += j;
							ys2 += input[j];
							if (++cnt2 >= fittSize) {
								x2 = xs2 / cnt2;
								y2 = ys2 / cnt2;
								break;
							}
						}
					}

					for (j = j - 1; j >= 0; j--) {
						if (input[j] >= 0) {
							xs1 += j;
							ys1 += input[j];
							if (++cnt1 >= fittSize) {
								x1 = xs1 / cnt1;
								y1 = ys1 / cnt1;
								break;
							}
						}
					}

					if (x1 >= 0 && x2 >= 0 && x2 > x1) {
						slope = (float)(y2 - y1) / (float)(x2 - x1);
						for (int k = (x2 + 1), dist = 1; k < size; k++, dist++) {
							output[k] = (int)(y2 + dist * slope);
							output[k] = max(0, output[k]);
						}
					}
				}
				break;
			}
		}
	}

	if (output[0] < 0 || output[size - 1] < 0) {
		return false;
	}

	for (int i = 0; i < size; i++)
	{
		if (output[i] >= 0) {
			x1 = i;
			y1 = output[i];
		}
		else
		{
			for (int j = i + 1; j < size; j++) {
				if (output[j] >= 0) {
					x2 = j;
					y2 = output[j];

					slope = (float)(y2 - y1) / (float)(x2 - x1);
					for (int k = (x1 + 1), dist = 1; k < x2; k++, dist++) {
						output[k] = (int)(y1 + dist * slope);
					}
					i = x2;
					break;
				}
			}
		}
	}
	return true;
}


bool RetSegm::DataFitt::smoothBoundaryLine(std::vector<int>& input, std::vector<int>& output, float filtSize)
{
	Mat msrc(1, (int)input.size(), CV_32SC1, input.data());
	Mat mdst(1, (int)output.size(), CV_32SC1, output.data());

	Size fsize = Size((int)(input.size() * filtSize), 1);
	blur(msrc, mdst, fsize);
	return true;
}



bool RetSegm::DataFitt::buildIdealBoundaryOfIOS(SegmLayer& layerIOS, SegmLayer& layerOut, SegmLayer& result)
{
	int size = layerIOS.getSize();
	auto outer = layerOut.getPoints();
	auto xs = layerIOS.getXs();
	auto ys = layerIOS.getYs();

	auto ideal = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	auto dataX = vector<int>(size);
	auto dataY = vector<int>(size);

	for (int i = 0; i < size; i++) {
		dataX[i] = xs[i];
		dataY[i] = ys[i];

		if (outer[i].valid == false) {
			//	dataY[i] = -1;
		}
	}

	int diff;

	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY)) {
		for (int i = 0; i < size; i++) {
			diff = fittY[i] - dataY[i];
			if (diff > 10.0) {
				dataY[i] = -1;
			}
		}
	}

	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY)) {
		for (int i = 0; i < size; i++) {
			diff = fittY[i] - dataY[i];
			if (diff > 5.0) {
				dataY[i] = -1;
			}
		}
	}


	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY)) {
		for (int i = 0; i < size; i++) {
			ideal[i] = (int)fittY[i];
		}
	}

	result.initialize(ideal, layerIOS.getRegionWidth(), layerIOS.getRegionHeight());
	return true;
}



bool RetSegm::DataFitt::buildIdealBoundrayOfIOS(const std::vector<int>& input, std::vector<int>& output)
{
	int size = (int)input.size();
	auto ideal = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	auto dataX = vector<int>(size);
	auto dataY = vector<int>(size);

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		dataY[i] = input[i];
	}

	int diff;

	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY)) {
		for (int i = 0; i < size; i++) {
			diff = dataY[i] - fittY[i];
			if (diff > 25.0) {
				dataY[i] = -1;
			}
		}
	}

	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY)) {
		for (int i = 0; i < size; i++) {
			diff = dataY[i] - fittY[i];
			if (diff > 15.0) {
				dataY[i] = -1;
			}
		}
	}

	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY)) {
		for (int i = 0; i < size; i++) {
			ideal[i] = (int)fittY[i];
		}
	}

	output = ideal;
	// output = dataY;
	return true;
}





// Deprecated from here
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RetSegm::DataFitt::createIdealBoundaryOfIOS(Boundary & boundIOS, Boundary& boundOut, Boundary & result)
{
	int size = boundIOS.getSize();
	auto outer = boundOut.getPoints();
	auto xs = boundIOS.getPointXs();
	auto ys = boundIOS.getPointYs();

	auto ideal = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	auto dataX = vector<int>(size);
	auto dataY = vector<int>(size);

	for (int i = 0; i < size; i++) {
		dataX[i] = xs[i];
		dataY[i] = ys[i];

		if (outer[i].isValid() == false) {
		//	dataY[i] = -1;
		}
	}

	/*
	int halfWind = 5;
	int diff;
	int open_pos, close_pos;
	bool opened = false;
	for (int i = halfWind; i < (size-halfWind); i++) {
		diff = dataY[i - halfWind] - dataY[i + halfWind];
		if (diff > halfWind) {
			opened = true;
			open_pos = i - halfWind;
		}
		diff = dataY[i + halfWind] - dataY[i - halfWind];
		if (diff > halfWind) {
			if (opened) {
				opened = false;
				close_pos = i + halfWind;
				DebugOut() << "speckle : " << open_pos << ", " << close_pos << " => " << (close_pos - open_pos);
			}
		}
	}
	*/

	int diff;

	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY)) {
		for (int i = 0; i < size; i++) {
			diff = fittY[i] - dataY[i];
			if (diff > 10.0) {
				dataY[i] = -1;
			}
		}
	}

	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY)) {
		for (int i = 0; i < size; i++) {
			diff = fittY[i] - dataY[i];
			if (diff > 5.0) {
				dataY[i] = -1;
			}
		}
	}


	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY)) {
		for (int i = 0; i < size; i++) {
			ideal[i] = (int)fittY[i];
		}
	}
	result.createPoints(ideal, boundIOS.getRegionWidth(), boundIOS.getRegionHeight());
	return true;

	/*
	CvMemStorage* hullStorage = cvCreateMemStorage(0);
	CvSeq* ptseq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), hullStorage);
	CvSeq* hull;

	for (int i = 0; i < size; i++) {
		if (outer[i].isValid()) {
			Point pt(xs[i], ys[i]);
			cvSeqPush(ptseq, &pt);
		}
	}

	hull = cvConvexHull2(ptseq, NULL, CV_CLOCKWISE);

	auto ideal = vector<int>(size, -1);
	for (int i = 0; i < hull->total; i++) {
		Point pt = **CV_GET_SEQ_ELEM(Point*, hull, i);
		DebugOut() << pt.x << ", " << pt.y;
		ideal[pt.x] = pt.y;
	}
	result.createPoints(ideal);
	return true;

	auto dataX = vector<double>(size, -1);
	auto dataY = vector<double>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = (double)i;
		if (ideal[i] > 0) {
			
			dataY[i] = (double)ideal[i];
		}
	}

	auto fittY = vector<double>(size, -1.0);

	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY)) {
		for (int i = 0; i < size; i++) {
			ideal[i] = (int)fittY[i];
		}
	}
	result.createPoints(ideal);

	return true;
	*/

	/*
	auto dataX = vector<double>(size);
	auto dataY = vector<double>(size);

	auto input = vector<Point>();

	for (int i = 0; i < size; i++) {
		input.push_back(Point(xs[i], ys[i]));
	}
	auto output = vector<Point>(input.size());




	//Mat_<Point> inpMat(1, input.size());
	//Mat_<Point> outMat(1, input.size());

	Mat inpMat(1, input.size(), CV_32SC2);
	Mat outMat(1, input.size(), CV_32SC2);

	DebugOut() << sizeof(int) << ", " << CV_32SC2 << ", " << sizeof(Point);
	for (int i = 0; i < outMat.cols; i++) {
		inpMat.at<Point>(0, i).y = 0;
		outMat.at<Point>(0, i).y = 1;
		DebugOut() << i << ": " << inpMat.at<Point>(0, i).y << " <> " << outMat.at<Point>(0, i).y;
	}




	CvMemStorage* hullStorage = cvCreateMemStorage(0);
	CvSeq* ptseq = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), hullStorage);
	CvSeq* hull;

	for (int i = 0; i < outMat.cols; i++) {
		Point pt(input[i].x, input[i].y);
		cvSeqPush(ptseq, &pt);
	}

	hull = cvConvexHull2(ptseq, NULL, true);
	for (int i = 0; i < hull->total; i++) {
		Point pt = **CV_GET_SEQ_ELEM(Point*, hull, i);
		DebugOut() << pt.x << ", " << pt.y;
	}

	memcpy(inpMat.data, input.data(), input.size()*sizeof(Point));
	for (int i = 0; i < outMat.cols; i++) {
		DebugOut() << i << ": " << inpMat.at<Point>(0, i).y << " <> " << outMat.at<Point>(0, i).y;
	}
	cvConvexHull2(&inpMat, output.data());

	for (int i = 0; i < outMat.cols; i++) {
		DebugOut() << i << ": " << inpMat.at<Point>(0, i).y << " <> " << outMat.at<Point>(0, i).y;
	}

	return true;
	for (int i = 0; i < size; i++) {
		dataX[i] = (double)xs[i];
		dataY[i] = (double)ys[i];
		if (outer[i].isValid() == false) {
			dataY[i] = -1.0;
		}
	}

	auto fittY = vector<double>(size, -1.0);
	auto ideal = vector<int>(size, -1);

	double diff;

	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY)) {
		for (int i = 0; i < size; i++) {
			diff = fittY[i] - dataY[i];
			if (diff > 10.0) {
				// dataY[i] = -1.0;
			}
		}
	}

	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY)) {
		for (int i = 0; i < size; i++) {
			ideal[i] = (int)fittY[i];
		}
	}
	result.createPoints(ideal);
	return true;
	*/
}
