#include "stdafx.h"
#include "KeratoSetup.h"
#include "KeratoImage.h"
#include "KeratoImage2.h"

#include <fstream>

#include <cereal\types\array.hpp>
#include <cereal\archives\binary.hpp>

#include "CppUtil2.h"

using namespace KerFocus; 
using namespace std;
using namespace cv;


struct KeratoSetup::KeratoSetupImpl
{
	KerSetupTable table;
};


KeratoSetup::KeratoSetup()
	: d_ptr(make_unique<KeratoSetupImpl>())
{
	initialize();
}


KerFocus::KeratoSetup::~KeratoSetup() = default;
KerFocus::KeratoSetup::KeratoSetup(KeratoSetup && rhs) = default;
KeratoSetup & KerFocus::KeratoSetup::operator=(KeratoSetup && rhs) = default;


void KerFocus::KeratoSetup::initialize(void)
{
	for (int i = 0; i < KER_SETUP_DIOPTER_NUM; i++) {
		d_ptr->table.diopts[i].mm = getInitialDiameter(i);
		d_ptr->table.diopts[i].diopt = getInitialDiopter(i);

		for (int j = 0; j < KER_SETUP_FOCUS_STEP_NUM; j++) {
			KerSetupData* data = getKerSetupData(i, j);
			data->valid = false;
		}
	}
	return;
}


bool KerFocus::KeratoSetup::isSetupDataIndex(int diopt) const
{
	if (diopt < 0 || diopt >= KER_SETUP_DIOPTER_NUM) {
		return false;
	}
	return true;
}


bool KerFocus::KeratoSetup::isSetupDataIndex(int diopt, int step) const
{
	if (isSetupDataIndex(diopt) == false || step < 0 || step >= KER_SETUP_FOCUS_STEP_NUM) {
		return false;
	}
	return true;
}


bool KerFocus::KeratoSetup::checkIfValidFocusStepData(int diopt, int istep, float flen, float mlen) const
{
	if (isSetupDataIndex(diopt, istep) == false) {
		return false;
	}

	// Focus and Mire length should be decreased as focus steps (further away from eye).
	for (int i = 0; i < istep; i++) {
		KerSetupData* data = getKerSetupData(diopt, i);
		if (data != nullptr && data->valid) {
			if (data->flen <= flen || data->mlen <= mlen) {
				return false;
			}
		}
	}
	return true;
}


KerSetupData * KerFocus::KeratoSetup::getKerSetupData(int diopt, int istep) const
{
	if (isSetupDataIndex(diopt, istep)) {
		return &(d_ptr->table.diopts[diopt].steps[istep]);
	}
	return nullptr;
}


KerSetupDiopter * KerFocus::KeratoSetup::getKerSetupDiopter(int diopt) const
{
	if (isSetupDataIndex(diopt)) {
		return &(d_ptr->table.diopts[diopt]);
	}
	return nullptr;
}


bool KerFocus::KeratoSetup::isValidSetupData(int diopt, int istep) const
{
	KerSetupData* data = getKerSetupData(diopt, istep);
	if (data != nullptr && data->valid) {
		return true;
	}
	return false;
}


bool KerFocus::KeratoSetup::updateFocusStepData(int diopt, int istep, const KeratoImage & image)
{
	if (isSetupDataIndex(diopt, istep) == false) {
		return false;
	}

	KerSetupData& data = d_ptr->table.diopts[diopt].steps[istep];

	float fdif, flen, mlen, cx, cy, m, n, a;

	if (!image.getFocusLength(flen)) {
		DebugOut2() << "Focus length not found!";
		return false;
	}

	if (!image.getMireLength(mlen)) {
		DebugOut2() << "Mire length not found!";
		return false;
	}

	if (!image.getMireEllipse(cx, cy, m, n, a)) {
		DebugOut2() << "Mire ellipse not found!";
		return false;
	}


	if (!image.getFocusDifference(fdif)) {
		DebugOut2() << "Focus difference not found!";
		return false;
	}


	if (true) {
		// Check validity of setup data at the current focus step of diopter.
		if (checkIfValidFocusStepData(diopt, istep, flen, mlen) == false) {
			DebugOut2() << "Invalid focus setup data: " << diopt << ", " << istep << ", flen: " << flen << ", mlen: " << mlen;
			return false;
		}

		data.m = m;
		data.n = n;
		data.a = a;
		data.fdif = fdif;
		data.flen = flen;
		data.mlen = mlen;
		data.valid = true;
		DebugOut2() << "Focus setup data: " << diopt << ", " << istep << ", flen: " << flen << ", mlen: " << mlen << ", fdif: " << fdif;
		return true;
	}
	return false;
}


bool KerFocus::KeratoSetup::getFocusStepData(int diopt, int istep, float& mlen, float& flen, float& fdif, float& m, float& n, float& a) const
{
	KerSetupData* data = getKerSetupData(diopt, istep);
	if (data == nullptr || !data->valid) {
		return false;
	}

	m = data->m;
	n = data->n;
	a = data->a;
	flen = data->flen;
	mlen = data->mlen;
	fdif = data->fdif;
	return true;
}


bool KerFocus::KeratoSetup::getDiopterSetupData(int diopt, float& mlen, float& flen, float& fdif, float& m, float& n, float& a, float & k) const
{
	if (getFocusStepData(diopt, KER_SETUP_DIOPTER_INDEX, mlen, flen, fdif, m, n, a) == false) {
		return false;
	}

	float la = 0.0f, lb;
	getFocusStepLine(diopt, la, lb);
	k = la;
	return true;
}


bool KerFocus::KeratoSetup::getFocusStepLine(int diopt, float & a, float & b) const
{
	std::vector<cv::Point2f> points;
	KerSetupData* p;
	for (int i = 0; i < KER_SETUP_FOCUS_STEP_NUM; i++) {
		p = getKerSetupData(diopt, i);
		if (p != nullptr && p->valid) {
			points.push_back({ p->mlen, p->fdif });
		}
	}

	if (points.size() < 2) {
		return false;
	}

	std::vector<float> line(4);
	cv::fitLine(points, line, CV_DIST_L2, 0, 0.01, 0.01);

	a = line[1] / line[0];
	b = line[3] - a * line[2];

	if (DEBUG_OUT) {
		DebugOut2() << "Focus line, diopt: " << diopt << ", a: " << a << ", b: " << b;
	}
	return true;
}


bool KerFocus::KeratoSetup::getDiopterSetupLine(float & a, float & b) const
{
	std::vector<cv::Point2f> points;
	KerSetupData* p;
	for (int i = 0; i < KER_SETUP_DIOPTER_NUM; i++) {
		p = getKerSetupData(i, KER_SETUP_DIOPTER_INDEX);
		if (p != nullptr && p->valid) {
			points.push_back({ p->mlen, p->fdif });
		}
		else {
			return false;
		}
	}

	if (points.size() < 2) {
		return false;
	}

	std::vector<float> line(4);
	cv::fitLine(points, line, CV_DIST_L2, 0, 0.01, 0.01);

	a = line[1] / line[0];
	b = line[3] - a * line[2];

	if (DEBUG_OUT) {
		DebugOut2() << "Setup line, a: " << a << ", b: " << b;
	}
	return true;
}


bool KerFocus::KeratoSetup::getFocusDistance(float mlen, float flen, float & dist) const
{
	int idx1, idx2;
	if (!getDiopterSection(mlen, flen, idx1, idx2)) {
		return false;
	}

	float a1, b1, a2, b2;
	if (!getFocusStepLine(idx1, a1, b1) || !getFocusStepLine(idx2, a2, b2)) {
		return false;
	}

	float f1 = (a1 + a2) * 0.5f;
	float f2 = (b1 + b2) * 0.5f;
	float diff = mlen - flen;
	f2 = diff - mlen * f1;

	float k1, k2;
	if (!getDiopterSetupLine(k1, k2)) {
		return false;
	}

	if (k1 == f1) {
		return false;
	}

	float x = -(k2 - f2) / (k1 - f1);
	float y = k1 * x + k2;

	float xd = (mlen - x);
	float yd = (diff - y);
	dist = sqrt(xd*xd + yd*yd);
	//dist = log(dist);

	if (dist > 1.0f) {
		dist = log2(dist) + 1.0f;
	}
	dist *= (xd < 0.0f ? -1.0f : +1.0f);

	if (DEBUG_OUT) {
		DebugOut2() << "Intersect at x: " << x << ", y: " << y << ", dist: " << dist;
	}
	return true;
}


bool KerFocus::KeratoSetup::getDiopterSection(float mlen, float flen, int & idx1, int & idx2) const
{
	for (int i = 0; i < KER_SETUP_DIOPTER_NUM; i++) {
		KerSetupData* p = getKerSetupData(i, KER_SETUP_DIOPTER_INDEX);
		if (p == nullptr || !p->valid) {
			return false;
		}
		else {
			if (p->mlen >= mlen) {
				idx1 = (i == 0 ? 0 : i-1);
				idx2 = (i == 0 ? 1 : i);
				return true;
			}
		}
	}
	idx1 = KER_SETUP_DIOPTER_NUM - 2;
	idx2 = KER_SETUP_DIOPTER_NUM - 1;
	return true;
}


void KerFocus::KeratoSetup::clearFocusStepData(int diopt, int istep)
{
	if (diopt < 0) {
		initialize();
	}
	else {
		if (istep < 0) {
			for (int i = 0; i < KER_SETUP_FOCUS_STEP_NUM; i++) {
				getKerSetupData(diopt, i)->valid = false;
			}
		}
		else {
			getKerSetupData(diopt, istep)->valid = false;
		}
	}
	return ;
}


bool KerFocus::KeratoSetup::loadDataFile(const char* path)
{
	ifstream ifs(path, std::ios::in|std::ios::binary);

	if (ifs.is_open() == false) {
		DebugOut2() << "Error in opening setup file: " << path;
	}
	else {
		try {
			cereal::BinaryInputArchive ar(ifs);
			ar(d_ptr->table);
			ifs.close();
			return true;
		}
		catch (ifstream::failure e) {
			DebugOut2() << "Exception openning setup file: " << path;
		}
		catch (cereal::Exception e) {
			DebugOut2() << "Exception loading setup file: " << e.what();
		}
	}	
	initialize();
	return false;
}


bool KerFocus::KeratoSetup::saveDataFile(const char* path)
{
	ofstream ofs(path, std::ios::out|std::ios::binary|std::ios::trunc);

	if (ofs.is_open() == false) {
		DebugOut2() << "Error in opening setup file: " << path;
	}
	else {
		try {
			cereal::BinaryOutputArchive ar(ofs);
			ar(d_ptr->table);
			ofs.close();
			return true;
		}
		catch (ifstream::failure e) {
			DebugOut2() << "Exception openning setup file: " << path;
		}
		catch (cereal::Exception e) {
			DebugOut2() << "Exception saving setup file: " << e.what();
		}
	}
	return false;
}


float KerFocus::KeratoSetup::getSetupDiopter(int diopt) const
{
	KerSetupDiopter *p = getKerSetupDiopter(diopt);
	if (p != nullptr) {
		return p->diopt;
	}
	return 0.0f;
}


float KerFocus::KeratoSetup::getSetupDiameter(int diopt) const
{
	KerSetupDiopter *p = getKerSetupDiopter(diopt);
	if (p != nullptr) {
		return p->mm;
	}
	return 0.0f;
}


float KerFocus::KeratoSetup::getInitialDiopter(int diopt) const
{
	switch (diopt) {
	case 0:
		return KER_SETUP_DIOPTER_1;
	case 1:
		return KER_SETUP_DIOPTER_2;
	case 2:
		return KER_SETUP_DIOPTER_3;
	case 3:
		return KER_SETUP_DIOPTER_4;
	case 4:
		return KER_SETUP_DIOPTER_5;
	default:
		return KER_SETUP_DIOPTER_3;
	}
}


float KerFocus::KeratoSetup::getInitialDiameter(int diopt) const
{
	switch (diopt) {
	case 0:
		return KER_SETUP_DIAMETER_1;
	case 1:
		return KER_SETUP_DIAMETER_2;
	case 2:
		return KER_SETUP_DIAMETER_3;
	case 3:
		return KER_SETUP_DIAMETER_4;
	case 4:
		return KER_SETUP_DIAMETER_5;
	default:
		return KER_SETUP_DIAMETER_3;
	}
}
