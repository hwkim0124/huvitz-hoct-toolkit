#include "stdafx.h"
#include "SegmLayer.h"
#include "CppUtil2.h"

using namespace RetSegm;
using namespace cv;
using namespace std;


struct SegmLayer::SegmLayerImpl
{
	LayerPointList points;
	std::vector<int> deltas;
	int regionWidth;
	int regionHeight;

	SegmLayerImpl() : regionWidth(0), regionHeight(0) {
	}

	SegmLayerImpl(int width, int height) {
		regionWidth = width;
		regionHeight = height;
		points.resize(width);
	}
};


SegmLayer::SegmLayer() :
	d_ptr(make_unique<SegmLayerImpl>())
{
}


SegmLayer::SegmLayer(int width, int height) :
	d_ptr(make_unique<SegmLayerImpl>(width, height))
{
}


RetSegm::SegmLayer::~SegmLayer() = default;
RetSegm::SegmLayer::SegmLayer(SegmLayer && rhs) = default;
SegmLayer & RetSegm::SegmLayer::operator=(SegmLayer && rhs) = default;


RetSegm::SegmLayer::SegmLayer(const SegmLayer & rhs)
	: d_ptr(make_unique<SegmLayerImpl>(*rhs.d_ptr))
{
}


SegmLayer & RetSegm::SegmLayer::operator=(const SegmLayer & rhs)
{
	// SegmLayerImpl struct should be defined before this access code. 
	*d_ptr = *rhs.d_ptr;
	return *this;
}


std::vector<int> RetSegm::SegmLayer::getXs(bool pickValids) const
{
	const LayerPointList& points = getPoints();

	if (pickValids == false) {
		std::vector<int> vect(points.size());
		for (int k = 0; k < vect.size(); k++) {
			vect[k] = d_ptr->points[k].x;
		}
		return vect;
	}
	else {
		std::vector<int> vect;
		for (int k = 0; k < points.size(); k++) {
			if (points[k].valid) {
				vect.push_back(points[k].x);
			}
		}
		return vect;
	}
}


std::vector<int> RetSegm::SegmLayer::getYs(bool pickValids) const
{
	const LayerPointList& points = getPoints();

	if (pickValids == false) {
		std::vector<int> vect(points.size());
		for (int k = 0; k < vect.size(); k++) {
			vect[k] = d_ptr->points[k].y;
		}
		return vect;
	}
	else {
		std::vector<int> vect;
		for (int k = 0; k < points.size(); k++) {
			if (points[k].valid) {
				vect.push_back(points[k].y);
			}
		}
		return vect;
	}
}


std::vector<int> RetSegm::SegmLayer::getYsResized(int width, int height) const
{
	LayerPointList vect = getPointsResized(width, height);
	std::vector<int> ys(vect.size(), -1);
	for (int i = 0; i < vect.size(); i++) {
		ys[i] = vect[i].y;
	}
	return ys;
}


LayerPointList RetSegm::SegmLayer::getPointsResized(int width, int height) const
{
	int horzSize = (width == 0 ? getRegionWidth() : width);
	int vertSize = (height == 0 ? getRegionHeight() : height);

	float horzRatio = (float)getRegionWidth() / (float)horzSize;
	float vertRatio = (float)vertSize / (float)getRegionHeight();

	auto dest = LayerPointList(horzSize);
	LayerPointList& points = getPoints();
	if (points.size() != getRegionWidth()) {
		for (int i = 0; i < horzSize; i++) {
			dest[i].set(i, -1);
		}
	}
	else {
		for (int i = 0; i < horzSize; i++) {
			auto p = points[(int)(i*horzRatio)];
			if (p.valid) {
				dest[i].set(i, (int)(p.y*vertRatio));
			}
			// DebugOut() << dest[i].getX() << ", " << dest[i].getY() << ", " << dest[i].isValid();
		}
	}
	return dest;
}


LayerPointList RetSegm::SegmLayer::getPointsValid(void) const
{
	LayerPointList& points = getPoints();
	LayerPointList vect;
	for (int k = 0; k < points.size(); k++) {
		if (points[k].valid) {
			vect.push_back(points[k]);
		}
	}
	return vect;

}


LayerPointList& RetSegm::SegmLayer::getPoints(void) const
{
	return d_ptr->points;
}


std::vector<int>& RetSegm::SegmLayer::getDeltas(void) const
{
	return d_ptr->deltas;
}


void RetSegm::SegmLayer::setValid(int index, bool flag)
{
	d_ptr->points[index].valid = flag;
	return;
}


bool RetSegm::SegmLayer::isValid(int index)
{
	return d_ptr->points[index].valid;
}


int RetSegm::SegmLayer::getY(int index) const
{
	return d_ptr->points[index].y;
}


void RetSegm::SegmLayer::setY(int index, int y)
{
	d_ptr->points[index].y = y;
	return;
}


void RetSegm::SegmLayer::setPoint(int index, int x, int y)
{
	d_ptr->points[index] = LayerPoint(x, y);
	return;
}


void RetSegm::SegmLayer::setPoint(int index, int x, int y, bool valid)
{
	d_ptr->points[index] = LayerPoint(x, y, valid);
	return;
}


void RetSegm::SegmLayer::setPoint(int index, const LayerPoint & point)
{
	d_ptr->points[index] = point;
	return;
}

void RetSegm::SegmLayer::setPoints(const std::vector<int>& ys)
{
	clear((int)ys.size());
	LayerPointList& points = getPoints();
	for (int x = 0; x < ys.size(); x++) {
		points[x].set(x, ys[x]);
	}
	return;
}

void RetSegm::SegmLayer::initialize(const std::vector<int>& vect, int width, int height)
{
	clear((int)vect.size());
	LayerPointList& points = getPoints();
	for (int x = 0; x < vect.size(); x++) {
		points[x].set(x, vect[x]);
	}
	setRegionSize(width, height);
	return;
}


void RetSegm::SegmLayer::initialize(const LayerPointList& vect, int width, int height)
{
	getPoints() = LayerPointList(vect);
	setRegionSize(width, height);
	return;
}


void RetSegm::SegmLayer::initialize(const cv::Mat & cvMat, int width, int height)
{
	clear(cvMat.cols);
	LayerPointList& points = getPoints();
	for (int c = 0; c < cvMat.cols; c++) {
		points[c].set( c, cvMat.at<int>(0, c) );
	}
	setRegionSize(width, height);
	return;
}


void RetSegm::SegmLayer::setRegionSize(int width, int height)
{
	d_ptr->regionWidth = width;
	d_ptr->regionHeight = height;
	return;
}


int RetSegm::SegmLayer::getRegionWidth(void) const
{
	return d_ptr->regionWidth;
}


int RetSegm::SegmLayer::getRegionHeight(void) const
{
	return d_ptr->regionHeight;
}


bool RetSegm::SegmLayer::resize(int targetWidth, int targetHeight)
{
	if (targetWidth <= 0 || targetHeight <= 0) {
		return false;
	}

	if (targetWidth != getRegionWidth() || targetHeight != getRegionHeight()) {
		/*
		auto dest = vector<SegmPoint>(targetWidth);
		float horzRatio = (float)getRegionWidth() / (float)targetWidth;
		float vertRatio = (float)targetHeight / (float)getRegionHeight();

		vector<SegmPoint>& points = getPoints();
		for (int i = 0; i < targetWidth; i++) {
			SegmPoint p = points[(int)(i*horzRatio)];
			if (p.valid) {
				dest[i].set(i, (int)(p.y*vertRatio));
			}
			// DebugOut() << dest[i].getX() << ", " << dest[i].getY() << ", " << dest[i].isValid();
		}
		*/
		LayerPointList& points = getPoints();
		points = getPointsResized(targetWidth, targetHeight);
		setRegionSize(targetWidth, targetHeight);
	}
	return true;
}


int RetSegm::SegmLayer::getSize(void) const
{
	return (int)getPoints().size();
}


bool RetSegm::SegmLayer::isEmpty(void) const
{
	return getPoints().empty();
}


void RetSegm::SegmLayer::clear(int size)
{
	LayerPointList& points = getPoints();
	if (size <= 0) {
		points.clear();
	}
	else {
		points = LayerPointList(size);
	}
	return;
}
