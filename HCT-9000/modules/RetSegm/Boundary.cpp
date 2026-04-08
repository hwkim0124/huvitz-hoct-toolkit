#include "stdafx.h"
#include "Boundary.h"
#include "CppUtil2.h"

using namespace RetSegm;
using namespace cv;

#include <iterator>
using namespace std;


Boundary::Boundary()
{
	m_regionWidth = 0;
	m_regionHeight = 0;
}


int RetSegm::Boundary::getPointY(int x) const
{
	if (m_points.size() > x) {
		if (m_points[x].isValid()) {
			return m_points[x].getY();
		}
	}
	return -1;
}


std::vector<int> RetSegm::Boundary::getPointXs(bool pickValids) const
{
	if (pickValids == false) {
		std::vector<int> vect(m_points.size());
		for (int k = 0; k < m_points.size(); k++) {
			vect[k] = m_points[k].getX();
		}
		return vect;
	}
	else {
		std::vector<int> vect;
		for (int k = 0; k < m_points.size(); k++) {
			if (m_points[k].isValid()) {
				vect.push_back(m_points[k].getX());
			}
		}
		return vect;
	}
}


std::vector<int> RetSegm::Boundary::getPointYs(bool pickValids) const
{
	if (pickValids == false) {
		std::vector<int> vect(m_points.size());
		for (int k = 0; k < m_points.size(); k++) {
			vect[k] = m_points[k].getY();
		}
		return vect;
	}
	else {
		std::vector<int> vect;
		for (int k = 0; k < m_points.size(); k++) {
			if (m_points[k].isValid()) {
				vect.push_back(m_points[k].getY());
			}
		}
		return vect;
	}
}


std::vector<EdgePoint> RetSegm::Boundary::getPoints(bool pickValids) const
{
	if (pickValids == false) {
		vector<EdgePoint> vect(m_points);
		return vect;
	}
	else {
		vector<EdgePoint> vect;
		for (int k = 0; k < m_points.size(); k++) {
			if (m_points[k].isValid()) {
				vect.push_back(m_points[k]);
			}
		}
		return vect;
	}
}


void RetSegm::Boundary::setInvalid(int idx)
{
	if (idx >= 0 && idx < m_points.size()) {
		m_points[idx].setValid(false);
	}
	return;
}


void RetSegm::Boundary::setPoint(int idx, int x, int y, bool valid)
{
	if (idx >= 0 && idx < m_points.size()) {
		m_points[idx].setValue(x, y, valid);
	}
	return;
}


void RetSegm::Boundary::setPoint(int idx, EdgePoint edge)
{
	if (idx >= 0 && idx < m_points.size()) {
		m_points[idx] = edge;
	}
	return;
}


void RetSegm::Boundary::createPoints(const std::vector<int>& vect, int width, int height)
{
	m_points = vector<EdgePoint>(vect.size());
	for (int x = 0; x < vect.size(); x++) {
		if (vect[x] >= 0) {
			m_points[x].setValue(x, vect[x], true);
		}
		else {
			m_points[x].setValue(x, -1, false);
		}
	}
	setRegionSize(width, height);
	return;
}


void RetSegm::Boundary::createPoints(const std::vector<EdgePoint>& vect, int width, int height)
{
	m_points = vector<EdgePoint>(vect);
	setRegionSize(width, height);
	return;
}


void RetSegm::Boundary::createPoints(const cv::Mat & cvMat, int width, int height)
{
	/*
	// Pointer to the 0th row. 
	const int* p = vals.ptr<int>(0);

	// Pass the range of plain pointer to vector. 
	// http://stackoverflow.com/questions/9790124/converting-a-row-of-cvmat-to-stdvector
	m_pointYs = std::vector<int>(p, p + vals.cols);

	m_pointXs = std::vector<int>();
	m_pointXs.reserve(m_pointYs.size());

	int n(0);
	std::generate_n(std::back_inserter(m_pointXs), m_pointYs.size(), [n]()mutable { return n++; });
	*/

	m_points.clear();
	for (int c = 0; c < cvMat.cols; c++) {
		m_points.push_back({ c, cvMat.at<int>(0, c) });
	}
	setRegionSize(width, height);
	return;
}


void RetSegm::Boundary::setRegionSize(int width, int height)
{
	m_regionWidth = width;
	m_regionHeight = height;
	return;
}


int RetSegm::Boundary::getRegionWidth(void) const
{
	return m_regionWidth;
}


int RetSegm::Boundary::getRegionHeight(void) const
{
	return m_regionHeight;
}


bool RetSegm::Boundary::resize(int targetWidth, int targetHeight)
{
	if (targetWidth <= 0 || targetHeight <= 0) {
		return false;
	}

	auto dest = vector<EdgePoint>(targetWidth);
	float horzRatio = (float)m_regionWidth / (float)targetWidth;
	float vertRatio = (float)targetHeight / (float)m_regionHeight;

	for (int i = 0; i < targetWidth; i++) {
		EdgePoint p = m_points[(int)(i*horzRatio)];
		if (p.isValid()) {
			dest[i].setValue(i, (int)(p.getY()*vertRatio), true);
		}
		// DebugOut() << dest[i].getX() << ", " << dest[i].getY() << ", " << dest[i].isValid();
	}

	m_points = dest;
	setRegionSize(targetWidth, targetHeight);
	return true;
}


int RetSegm::Boundary::getSize(void) const
{
	return (int)m_points.size();
}


bool RetSegm::Boundary::isEmpty(void) const
{
	return m_points.empty();
}


void RetSegm::Boundary::clear(int size)
{
	if (size <= 0) {
		m_points.clear();
	}
	else {
		m_points = vector<EdgePoint>(size);
	}
	return;
}

