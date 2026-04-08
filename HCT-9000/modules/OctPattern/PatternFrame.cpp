#include "stdafx.h"
#include "PatternFrame.h"
#include "LineTrace.h"

#include "CppUtil2.h"

using namespace OctPattern;
using namespace CppUtil;
using namespace std;


struct PatternFrame::PatternFrameImpl
{
	LineTraceVect lines;

	PatternFrameImpl()
	{
	}
};


PatternFrame::PatternFrame() :
	d_ptr(make_unique<PatternFrameImpl>())
{
}


OctPattern::PatternFrame::~PatternFrame() = default;
OctPattern::PatternFrame::PatternFrame(PatternFrame && rhs) = default;
PatternFrame & OctPattern::PatternFrame::operator=(PatternFrame && rhs) = default;


OctPattern::PatternFrame::PatternFrame(const PatternFrame & rhs)
	: d_ptr(make_unique<PatternFrameImpl>(*rhs.d_ptr))
{
}


PatternFrame & OctPattern::PatternFrame::operator=(const PatternFrame & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctPattern::PatternFrame::addLineTrace(LineTrace & trace)
{
	getImpl().lines.push_back(move(trace));
	return;
}


std::vector<LineTrace> & OctPattern::PatternFrame::getLineTraceList(void) const
{
	return getImpl().lines;
}


int OctPattern::PatternFrame::getLineTraceListSize(void) const
{
	return (int)getImpl().lines.size();
}


void OctPattern::PatternFrame::clearLineTraceList(void)
{
	getImpl().lines.clear();
	return;
}


LineTrace * OctPattern::PatternFrame::getLineTraceFirst(void) const
{
	return getLineTrace(0);
}


LineTrace * OctPattern::PatternFrame::getLineTrace(int index) const
{
	if (index >= 0 && index < getLineTraceListSize()) {
		return &getImpl().lines[index];
	}
	return nullptr;
}


int OctPattern::PatternFrame::getLateralSizeFirst(void) const
{
	return getLateralSizeList(false)[0];
}


std::vector<int> OctPattern::PatternFrame::getLateralSizeList(bool repeat) const
{
	auto sizes = std::vector<int>();
	for (const auto& line : d_ptr->lines) {
		int points = line.getNumberOfScanPoints(false);
		if (repeat) {
			for (int k = 0; k < line.getNumberOfRepeats(); k++) {
				sizes.push_back(points);
			}
			// Each frame has only line repeated multiple times. 
			// break;
		}
		else {
			sizes.push_back(points);
		}
	}
	return sizes;
}


int OctPattern::PatternFrame::getNumberOfLineTraces(bool repeat) const
{
	int count = 0;
	for (const auto& line : d_ptr->lines) {
		if (repeat) {
			count += line.getNumberOfRepeats();
		}
		else {
			count++;
		}
	}
	return count;
}


int OctPattern::PatternFrame::getPositionOffsetX(int index1, int index2, int xPos)
{
	LineTrace* line1 = getLineTrace(index1);
	LineTrace* line2 = getLineTrace(index2);
	if (!line1 || !line2) {
		return 0;
	}

	int count1 = line1->getCountOfPositionsX();
	int count2 = line2->getCountOfPositionsX();
	if (count1 <= xPos || count2 <= xPos) {
		return 0;
	}

	short* pts1 = line1->getGalvanoPositionsX();
	short* pts2 = line2->getGalvanoPositionsX();

	int p1 = pts1[xPos];
	int p2 = pts2[xPos];
	return (p2 - p1);
}


int OctPattern::PatternFrame::getPositionOffsetY(int index1, int index2, int yPos)
{
	LineTrace* line1 = getLineTrace(index1);
	LineTrace* line2 = getLineTrace(index2);
	if (!line1 || !line2) {
		return 0;
	}

	int count1 = line1->getCountOfPositionsX();
	int count2 = line2->getCountOfPositionsX();
	if (count1 <= yPos || count2 <= yPos) {
		return 0;
	}

	short* pts1 = line1->getGalvanoPositionsY();
	short* pts2 = line2->getGalvanoPositionsY();

	int p1 = pts1[yPos];
	int p2 = pts2[yPos];
	return (p2 - p1);
}


PatternFrame::PatternFrameImpl & OctPattern::PatternFrame::getImpl(void) const
{
	return *d_ptr;
}
