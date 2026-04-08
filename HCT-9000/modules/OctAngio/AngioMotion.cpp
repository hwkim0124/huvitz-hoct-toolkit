#include "stdafx.h"
#include "AngioMotion.h"
#include "AngioLayout.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace OctAngio;
using namespace std;
using namespace cv;

#include <iterator>
#include <algorithm>
#include <numeric>



struct AngioMotion::AngioMotionImpl
{
	AngioLayout layout;

	vector<float> bscanMotionTransX;
	vector<float> bscanMotionTransY;
	vector<float> bscanMotionEccVal;
	vector<int> motionShifts;

	int motionVersion = MOTION_CORRECTION_VERSION;

	float lineDecorrMovingAvgSize = 0.08f;
	float lineDecorrThreshold = 1.25f; // 1.25f; //  1.25f; // 1.5;
	float overPointsThreshold1 = 0.60f; // 45f; // 0.45f;
	float overPointsThreshold2 = 0.55f;
	float overPointsThreshold3 = 0.50f; // 45f; // 0.45f;
	float overPointsThresholdBase = 0.35f; // 45f; // 0.45f;

	float vesselDecorrThreshold = 0.0f; // 0.15f; // 15f; // 0.15f;
	float vesselPointsThreshold = 0.0f; // 0.05f; // 0.05f;
	float vesselShiftDifferMin = 0.85f; // 1.15f; // 1.09f;

	int motionFreeStripeSizeMin = 24; // 32; // 12; // 3;
	int motionFreeStripesNumberMax = 15;
	float motionFreeStripesRatioMin = 0.90f;

	float vesselOverThreshold = 1.8f;
	float vesselShiftPointsMin = 1; //  5; // 3;
	float vesselShiftDistanceMax = -1.0f; // -3.5f;

	vector<vector<float>> stripesDecorr;
	vector<vector<float>> stripesDiffer;
	vector<vector<float>> stripesMarker;
	vector<int> stripesIndice;
	vector<int> stripesShifts;

	vector<int> stripeStarts;
	vector<int> stripeCloses;
	vector<bool> motionLines;

	vector<float> freeDecorrs;
	vector<float> freeDiffers;
	vector<float> freeMarkers;

	float averageMarker = 0.0f;

	AngioMotionImpl() {

	};
};


AngioMotion::AngioMotion()
	: d_ptr(make_unique<AngioMotionImpl>())
{
}


AngioMotion::~AngioMotion()
{
	// Destructor should be defined for unique_ptr to delete AngioMotionImpl as an incomplete type.
}



OctAngio::AngioMotion::AngioMotion(AngioMotion && rhs) = default;
AngioMotion & OctAngio::AngioMotion::operator=(AngioMotion && rhs) = default;


OctAngio::AngioMotion::AngioMotion(const AngioMotion & rhs)
	: d_ptr(make_unique<AngioMotionImpl>(*rhs.d_ptr))
{
}


AngioMotion & OctAngio::AngioMotion::operator=(const AngioMotion & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}

void OctAngio::AngioMotion::clearAngioMotion(void)
{
	getImpl().bscanMotionTransX.clear();
	getImpl().bscanMotionTransY.clear();
	getImpl().bscanMotionEccVal.clear();
	return;
}

int OctAngio::AngioMotion::getMotionVersion(void) const
{
	return getImpl().motionVersion;
}

void OctAngio::AngioMotion::setMotionVersion(int version)
{
	getImpl().motionVersion = version;
	return;
}

bool OctAngio::AngioMotion::isMotionShiftSupported(void) const
{
	auto version = getMotionVersion();
	return version >= MOTION_CORRECTION_VERSION;
}

void OctAngio::AngioMotion::setBscanTranslations(const std::vector<float>& transX, const std::vector<float>& transY, const std::vector<float>& eccs)
{
	getImpl().bscanMotionTransX = transX;
	getImpl().bscanMotionTransY = transY;
	getImpl().bscanMotionEccVal = eccs;
	return;
}

bool OctAngio::AngioMotion::computeMotionShifts(const AngioLayout& layout, const std::vector<float>& angio)
{
	auto& xlist = getImpl().bscanMotionTransX;
	auto& ylist = getImpl().bscanMotionTransY;
	auto& clist = getImpl().bscanMotionEccVal;

	if (xlist.size() != ylist.size() || xlist.size() != clist.size() || xlist.size() != layout.getHeight()) {
		LogD() << "Motion translation vectors not matched!, size: " << xlist.size();
		return false;
	}
	if (angio.empty() || angio.size() != layout.getSize()) {
		return false;
	}

	int w = layout.getWidth();
	int h = layout.getHeight();
	
	auto shifts = vector<int>(xlist.size(), 0);
	auto vmask = vector<float>(angio);
	int count = 0;

	const int VERT_MARGIN = 12;
	const int DIST_OFFSET = 1;
	const int DISTANCE_MIN = 2;
	const int DISTANCE_MAX = 9;
	const float ECC_MIN = 0.60f;
	const float NCC1_MIN = 0.05f;
	const float NCC2_MIN = 0.35f;
	const float NCC_DIFF = 0.01f;

	for (int y = 1; y < (h-1); y++) {
		int dist = (int)std::round(xlist[y]);
		int dist_p = (int)std::round(xlist[y - 1]);
		int dist_n = (int)std::round(xlist[y + 1]);
		float ecc = clist[y];

		if (y < VERT_MARGIN || y >= (h - VERT_MARGIN)) {
			continue;
		}
		if (abs(dist) < DISTANCE_MIN || abs(dist) > DISTANCE_MAX) {
			continue;
		}
		if (ecc < ECC_MIN) {
			continue;
		}

		LogD() << "Bscan translation at line: " << y << ", trans_x: " << xlist[y] << ", dist: " << dist << ", ecc : " << ecc;

		if (fabs(dist_p) >= DISTANCE_MIN && (dist_p * dist) < 0) {
			dist = 0; //  dist + dist_p;
			LogD() << "\tTranslation prev: " << dist_p << " => dist: " << dist;
			continue;
		}
		else if (fabs(dist_n) >= DISTANCE_MIN && (dist_n * dist) < 0) {
			dist = 0; //  dist + dist_n;
			LogD() << "\tTranslation next: " << dist_n << " => dist: " << dist;
			continue;
		}

		auto prev = vector<float>(vmask.cbegin() + (y - 1) * w, vmask.cbegin() + y * w);
		auto curr = vector<float>(vmask.cbegin() + y * w, vmask.cbegin() + (y + 1) * w);
		/*
		auto pre1 = vector<float>(vmask.cbegin() + (y - 1) * w, vmask.cbegin() + y * w);
		auto pre2 = vector<float>(vmask.cbegin() + (y - 2) * w, vmask.cbegin() + (y - 1) * w);
		auto pre3 = vector<float>(vmask.cbegin() + (y - 3) * w, vmask.cbegin() + (y - 2) * w);
		
		auto cur1 = vector<float>(vmask.cbegin() + y * w, vmask.cbegin() + (y + 1) * w);
		auto cur2 = vector<float>(vmask.cbegin() + (y + 1) * w, vmask.cbegin() + (y + 2) * w);
		auto cur3 = vector<float>(vmask.cbegin() + (y + 2) * w, vmask.cbegin() + (y + 3) * w);

		for (int s = 0; s < w; s++) {
			prev[s] = (pre1[s] + pre2[s] + pre3[s]) / 3;
			curr[s] = (cur1[s] + cur2[s] + cur3[s]) / 3;
		}
		*/

		float ncc1_max = 0.0f;
		float ncc2_max = 0.0f;
		int dofs_max = 0;

		for (int dofs = dist - DIST_OFFSET; dofs <= dist + DIST_OFFSET; dofs++)
		{
			auto comp = vector<float>(w, 0.0f);
			if (dofs > 0) {
				std::copy(curr.cbegin() + dofs, curr.cend(), comp.begin());
			}
			else {
				std::copy(curr.cbegin(), curr.cend() + dofs, comp.begin() - dofs);
			}

			auto sidx = 0;
			auto eidx = w - 1;
			for (int k = 0; k < w; k++) {
				if (prev[k] > 0.0f && comp[k] > 0.0f) {
					sidx = k;
					break;
				}
			}
			for (int k = w - 1; k >= 0; k--) {
				if (prev[k] > 0.0f && comp[k] > 0.0f) {
					eidx = k;
					break;
				}
			}

			auto prev2 = vector<float>(prev.cbegin() + sidx, prev.cbegin() + eidx + 1);
			auto curr2 = vector<float>(curr.cbegin() + sidx, curr.cbegin() + eidx + 1);
			auto comp2 = vector<float>(comp.cbegin() + sidx, comp.cbegin() + eidx + 1);

			Mat mat1 = Mat(1, (int)prev2.size(), CV_32F, prev2.data());
			Mat mat2 = Mat(1, (int)curr2.size(), CV_32F, curr2.data());
			Mat mat3;
			cv::matchTemplate(mat1, mat2, mat3, CV_TM_CCOEFF_NORMED);
			float ncc1 = mat3.at<float>(0, 0);

			mat2 = Mat(1, (int)comp2.size(), CV_32F, comp2.data());
			cv::matchTemplate(mat1, mat2, mat3, CV_TM_CCOEFF_NORMED);
			float ncc2 = mat3.at<float>(0, 0);

			/*
			for (int s = 0; s < prev2.size(); s++) {
				LogD() << s << " : " << prev2[s] << ", " << curr2[s] << ", " << comp2[s];
			}
			*/
			LogD() << "\tMatch template at line: " << y << ", dist: " << dist << ", ecc: " << ecc << ", dofs: " << dofs << ", ncc1: " << ncc1 << ", ncc2 : " << ncc2;

			if (ncc2_max < ncc2) {
				ncc2_max = ncc2;
				ncc1_max = ncc1;
				dofs_max = dofs;
			}
		}

		if ((ncc2_max - ncc1_max) >= NCC_DIFF && abs(dofs_max) > 0 && abs(dofs_max) <= DISTANCE_MAX && ncc2_max >= NCC2_MIN && ncc1_max >= NCC1_MIN) {
			shifts[y] = dofs_max;
			dist = dofs_max;
			count++;

			LogD() << "\t=> Motion shift at line: " << y << ", dist: " << dofs_max << ", ncc1: " << ncc1_max << ", ncc2 : " << ncc2_max << ", count: " << count;
			
			for (int t = y; t < h; t++) {
				auto curr = vector<float>(vmask.cbegin() + t * w, vmask.cbegin() + (t + 1) * w);
				auto comp = vector<float>(w, 0.0f);
				if (dist > 0) {
					std::copy(curr.cbegin() + dist, curr.cend(), comp.begin());
				}
				else {
					std::copy(curr.cbegin(), curr.cend() + dist, comp.begin() - dist);
				}
				std::copy(comp.cbegin(), comp.cend(), vmask.begin() + t * w);
			}
		}
	}

	getImpl().motionShifts = shifts;
	LogD() << "Motion shifts detected: " << count;
	return true;
}


bool OctAngio::AngioMotion::applyMotionCorrection(const AngioLayout& layout, std::vector<float>& angio)
{
	auto motion = getImpl().motionShifts;
	if (motion.empty() || motion.size() != layout.getHeight()) {
		return false;
	}

	int w = layout.getWidth();
	int h = layout.getHeight();

	auto image = vector<float>(angio);
	auto count = 0;

	for (int y = h / 2; y >= 1; y--) {
		if (motion[y] == 0) {
			continue;
		}

		auto shift = motion[y];
		LogD() << "Motion correction at line: " << y << ", shift: " << shift;
		count++;

		for (int t = y; t >= 0; t--) {
			auto curr = vector<float>(image.cbegin() + t * w, image.cbegin() + (t + 1) * w);
			auto comp = vector<float>(w, 0.0f);

			if (shift > 0) {
				std::copy(curr.cbegin(), curr.cend() - shift, comp.begin() + shift);
			}
			else {
				std::copy(curr.cbegin() - shift, curr.cend(), comp.begin());
			}
			std::copy(comp.cbegin(), comp.cend(), image.begin() + t * w);
		}
	}

	for (int y = h / 2; y < (h - 1); y++) {
		if (motion[y] == 0) {
			continue;
		}

		auto shift = motion[y];
		LogD() << "Motion correction at line: " << y << ", shift: " << shift;
		count++;

		for (int t = y; t < h; t++) {
			auto curr = vector<float>(image.cbegin() + t * w, image.cbegin() + (t + 1) * w);
			auto comp = vector<float>(w, 0.0f);

			if (shift > 0) {
				std::copy(curr.cbegin() + shift, curr.cend(), comp.begin());
			}
			else {
				std::copy(curr.cbegin(), curr.cend() + shift, comp.begin() - shift);
			}
			std::copy(comp.cbegin(), comp.cend(), image.begin() + t * w);
		}
	}
	
	std::copy(image.cbegin(), image.cend(), angio.begin());
	LogD() << "Motion correction applied: " << count;
	return true;
}

std::vector<int> OctAngio::AngioMotion::getMotionShifts(void) const
{
	return getImpl().motionShifts;
}

void OctAngio::AngioMotion::setMotionShifts(std::vector<int> shifts)
{
	getImpl().motionShifts = shifts;
	return;
}

void OctAngio::AngioMotion::correctMotionArtifacts(const AngioLayout& layout, std::vector<float>& decorrs, std::vector<float>& differs, std::vector<float>& markers)
{
	getImpl().layout = layout;
	auto dms = markers;
	auto dcs = decorrs;
	auto dfs = differs;

	if (Layout().isVerticalScan()) {
		CvImage image1, image2, image3;
		image1.fromFloat32((const unsigned char*)&dcs[0], Layout().getWidth(), Layout().getHeight());
		image1.rotate90();
		image2.fromFloat32((const unsigned char*)&dfs[0], Layout().getWidth(), Layout().getHeight());
		image2.rotate90();
		image3.fromFloat32((const unsigned char*)&dms[0], Layout().getWidth(), Layout().getHeight());
		image3.rotate90();
		dcs = image1.copyDataInFloats();
		dfs = image2.copyDataInFloats();
		dms = image3.copyDataInFloats();
	}

	if (makeMotionFreeStripes(dcs, dfs, dms)) {
		measureStripesDisplacement();
		if (registMotionFreeStripes(true)) {
			if (Layout().isVerticalScan()) {
				CvImage image1, image2;
				image1.fromFloat32((const unsigned char*)&getImpl().freeDecorrs[0], Layout().getWidth(), Layout().getHeight());
				image1.rotate90(false);
				image2.fromFloat32((const unsigned char*)&getImpl().freeDiffers[0], Layout().getWidth(), Layout().getHeight());
				image2.rotate90(false);
				decorrs = image1.copyDataInFloats();
				differs = image2.copyDataInFloats();
			}
			else {
				decorrs = getImpl().freeDecorrs;
				differs = getImpl().freeDiffers;
			}
		}
	}
	return;
}


void OctAngio::AngioMotion::correctMotionArtifacts2(const AngioLayout & layout, std::vector<float>& decorrs, std::vector<float>& differs, std::vector<float>& markers, bool regist)
{
	getImpl().layout = layout;
	auto dms = markers;
	auto dcs = decorrs;
	auto dfs = differs;

	if (regist) {
		if (splitMotionFreeStripes(dcs, dfs, dms) > 1) {
			measureStripesDisplacement();
			if (registMotionFreeStripes2()) {
				interpolateEmptyLines(dms);
				resizeMotionFreeImages();
				decorrs = getImpl().freeDecorrs;
				differs = getImpl().freeDiffers;
				markers = getImpl().freeMarkers;
			}
		}
	}
	else {
		getImpl().freeDecorrs = decorrs;
		getImpl().freeDiffers = differs;
		interpolateEmptyLines(dms);
		decorrs = getImpl().freeDecorrs;
		differs = getImpl().freeDiffers;
	}
	return;
}


bool OctAngio::AngioMotion::correctBiasField(const AngioLayout & layout, std::vector<float>& projects, float sigma, bool isFovea)
{
	if (projects.empty() || projects.size() != layout.getSize() || sigma <= 0.0f) {
		return false;
	}
	if (all_of(projects.begin(), projects.end(), [](float i) { return i <= 0; })) {
		return true;
	}

	int w = layout.getWidth();
	int h = layout.getHeight();

	CvImage image;
	image.fromFloat32((const unsigned char*)&projects[0], w, h);
	image.applyGaussianBlur(sigma);

	float mean, stdev;
	image.getMeanStddev(&mean, &stdev);
	auto biases = image.copyDataInFloats();

	auto weights = vector<float>(biases.size(), 1.0f);

	if (isFovea) {
		CvImage filter;
		filter.fromFloat32((const unsigned char*)&weights[0], w, h);

		int cx, cy, cx2, cy2;
		int size1, size2;
		if (!layout.getFoveaCenterInPixel(cx, cy)) {
			LogD() << "Bias field correction, fovea center not found!";
			return false;
		}
		if (!layout.getFoveaRadiusInPixel(size1, size2)) {
			LogD() << "Bias field correction, fovea radius not found!";
			return false;
		}
	
		int x1 = max(cx - size1, 0);
		int x2 = min(cx + size1, w-1);
		int y1 = max(cy - size2, 0);
		int y2 = min(cy + size2, h-1);

		auto data = biases;
		if (mean < 1.0f) {
			// Decorrelation image.
			std::transform(begin(data), end(data), begin(data), [=](float e) {
				return (mean - e) * 100.0f + 0.1f;
			});
		}
		else {
			std::transform(begin(data), end(data), begin(data), [=](float e) {
				return (mean - e) + 1.0f;
			});
		}
		
		float xsum = 0.0f;
		float ysum = 0.0f;
		float wsum = 0.0f;
		float val;
		for (int y = y1; y <= y2; y++) {
			for (int x = x1; x <= x2; x++) {
				val = data[y*w + x];
				xsum += x * val;
				ysum += y * val;
				wsum += val;
			}
		}

		if (wsum > 0.0f) {
			cx2 = (int)(xsum / wsum);
			cy2 = (int)(ysum / wsum);
		}
		else {
			cx2 = cx;
			cy2 = cy;
		}

		LogD() << "Bias field correction, center: " << layout.foveaCenterX() << ", " << layout.foveaCenterY() << ", radius: " << size1 << ", rangeX: " << layout.scanRangeX() << ", rangeY: " << layout.scanRangeY();
		LogD() << "Pupil center position: " << cx << ", " << cy << ", adjusted: " << cx2 << ", " << cy2;

		cx = cx2;
		cy = cy2;
		cv::ellipse(filter.getCvMat(), cvPoint(cx, cy), cvSize(size1, size2), 0.0, 0.0, 360.0, Scalar(0.0f), -1);
		filter.applyGaussianBlur(3.0f);

		weights = filter.copyDataInFloats();
	}

	// Field correction by negative weights incurs the shadowed nerve fiber regions.
	transform(begin(weights), end(weights), begin(biases), begin(weights), [&](float e1, float e2) {
		return max((e1 * log(mean / (e2 + 0.00001f)) * 1.0f), 0.0f);
		// return e1 * log(mean / (e2 + 0.00001f)) * 1.0f;
	});

	/*
	auto means = vector<float>(h);
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			means[i] += biases[j];
		}
		means[i] /= w;
	}

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			weights[i*w + j] = log(means[i] / (biases[i*w + j] + 0.000001f));
		}
	}
	*/

	transform(begin(projects), end(projects), begin(weights), begin(projects), [&](float e1, float e2) {
		return e1 * max((1.0f + e2), 0.25f);
		// return e1 * max(log(mean / (e2 + 0.00001f)), 0.0f);
		// return max(e2, 0.0f);
	});

	// projects = biases;
	return true;
}


bool OctAngio::AngioMotion::rotateVerticalScan(const AngioLayout & layout, bool direction, std::vector<float>& decorrs, std::vector<float>& differs, std::vector<float>& markers)
{
	getImpl().layout = layout;
	auto& dms = markers;
	auto& dcs = decorrs;
	auto& dfs = differs;

	if (direction) {
		if (Layout().isVerticalScan()) {
			CvImage image1, image2, image3;
			image1.fromFloat32((const unsigned char*)&dcs[0], Layout().getWidth(), Layout().getHeight());
			image1.rotate90();
			image2.fromFloat32((const unsigned char*)&dfs[0], Layout().getWidth(), Layout().getHeight());
			image2.rotate90();
			image3.fromFloat32((const unsigned char*)&dms[0], Layout().getWidth(), Layout().getHeight());
			image3.rotate90();
			dcs = image1.copyDataInFloats();
			dfs = image2.copyDataInFloats();
			dms = image3.copyDataInFloats();
		}
	}
	else {
		if (Layout().isVerticalScan()) {
			CvImage image1, image2, image3;
			image1.fromFloat32((const unsigned char*)&dcs[0], Layout().getWidth(), Layout().getHeight());
			image1.rotate90(false);
			image2.fromFloat32((const unsigned char*)&dfs[0], Layout().getWidth(), Layout().getHeight());
			image2.rotate90(false);
			image3.fromFloat32((const unsigned char*)&dms[0], Layout().getWidth(), Layout().getHeight());
			image3.rotate90(false);
			dcs = image1.copyDataInFloats();
			dfs = image2.copyDataInFloats();
			dms = image3.copyDataInFloats();
		}
	}
	return true;
}


bool OctAngio::AngioMotion::rotateVerticalScan(const AngioLayout & layout, bool direction, std::vector<float>& project)
{
	getImpl().layout = layout;

	if (direction) {
		if (Layout().isVerticalScan()) {
			CvImage image1;
			image1.fromFloat32((const unsigned char*)&project[0], Layout().getWidth(), Layout().getHeight());
			image1.rotate90();
			project = image1.copyDataInFloats();
		}
	}
	else {
		if (Layout().isVerticalScan()) {
			CvImage image1;
			image1.fromFloat32((const unsigned char*)&project[0], Layout().getHeight(), Layout().getWidth());
			image1.rotate90(false);
			project = image1.copyDataInFloats();
		}
	}
	return true;
}


int OctAngio::AngioMotion::maskMotionAffectedLines(const AngioLayout & layout, std::vector<float>& vmask, std::vector<float>& dmask)
{
	if (vmask.empty() || dmask.empty()) {
		return 0;
	}

	int h = layout.getHeight();
	int w = layout.getWidth();

	auto avgs = vector<float>(h, 0.0f);
	auto fpos = dmask.begin();
	auto epos = fpos + w;

	for (int i = 0; i < h; i++) {
		avgs[i] = accumulate(fpos, epos, 0.0f) / w;
		fpos += w;
		epos = (i < h - 1 ? epos + w : dmask.end());
	}

	getImpl().averageMarker = accumulate(avgs.begin(), avgs.end(), 0.0f) / avgs.size();

	auto overs = vector<float>(h, 0.0f);
	auto mavgs = vector<float>(h, 0.0f);
	auto fpos2 = dmask.begin();
	auto epos2 = fpos2 + w;
	auto count = 0;
	auto dmean = 0.0f;
	int sects = (int)(getImpl().lineDecorrMovingAvgSize * h);
	int halfs = sects / 2;

	for (int i = 0, k = 0; i < h; i++) {
		if (i < halfs) {
			k = 0;
		}
		else if (i >= (h - halfs - 1)) {
			k = h - sects - 1;
		}
		else {
			k = i - halfs;
		}

		for (count = 0, dmean = 0.0f; count < sects; count++) {
			dmean += avgs[k + count];
		}
		mavgs[i] = dmean / sects;
		float threshold = mavgs[i] * getImpl().lineDecorrThreshold;

		count = (int)count_if(fpos2, epos2, [threshold](float e) { return e > threshold; });
		overs[i] = (float)count / w;
		fpos2 += w;
		epos2 = (i < h - 1 ? epos2 + w : dmask.end());
		// LogD() << "Line: " << i << ", decorr: " << mavgs[i] << ", overs: " << count << ", " << overs[i] << ", thresh: " << threshold;
	}

	auto motionLines = vector<bool>(h, false);
	float rangeX = getImpl().layout.scanRangeX();
	float threshold = (rangeX >= 9.0f ? getImpl().overPointsThreshold3 : (rangeX >= 6.0f ? getImpl().overPointsThreshold2 : getImpl().overPointsThreshold1));

	for (int i = 0; i < h; i++) {
		if (overs[i] > threshold) {
			for (int j = i; j >= max((i - 2), 0); j--) {
				if (overs[j] > getImpl().overPointsThresholdBase) {
					for_each(vmask.begin() + j * w, vmask.begin() + (j + 1)*w, [&](float &e) { e = 0.0f; });
					motionLines[j] = true;
				}
				else {
					break;
				}
			}

			for (int j = i + 1; j < min(i + 3, h); j++) {
				if (overs[j] > getImpl().overPointsThresholdBase) {
					for_each(vmask.begin() + j * w, vmask.begin() + (j + 1)*w, [&](float &e) { e = 0.0f; });
					motionLines[j] = true;
					i = j;
				}
				else {
					break;
				}
			}
		}
		else {
		}
	}

	getImpl().motionLines = motionLines;
	int affected = (int)(count_if(motionLines.begin(), motionLines.end(), [&](bool e) { return e; }));

	for (int i = 0; i < h; i++) {
		if (motionLines[i]) {
			LogD() << "Motion affected, line: " << i << ", decorr: " << avgs[i] << ", overs: " << overs[i] << ", motion: " << motionLines[i];
		}
	}
	LogD() << "Motion affected lines: " << affected;
	return affected;
}


int OctAngio::AngioMotion::splitMotionFreeStripes(const std::vector<float>& decorrs, const std::vector<float>& differs, const std::vector<float>& markers)
{
	if (markers.empty() || decorrs.empty() || differs.empty()) {
		return 0;
	}

	int h = Layout().getHeight();
	int w = Layout().getWidth();

	auto stripes1 = vector<vector<float>>();
	auto stripes2 = vector<vector<float>>();
	auto stripes3 = vector<vector<float>>();

	getImpl().stripeStarts.clear();
	getImpl().stripeCloses.clear();

	int sidx = -1, eidx = -1;
	for (int i = 0; i < h; i++) {
		if (getImpl().motionLines[i] || (i == h-1)) {
			if (eidx >= 0) {
				auto pos1 = decorrs.begin() + sidx * w;
				auto pos2 = decorrs.begin() + (eidx + 1) * w;
				auto part = vector<float>(pos1, pos2);
				stripes1.push_back(move(part));

				auto pos3 = differs.begin() + sidx * w;
				auto pos4 = differs.begin() + (eidx + 1) * w;
				auto part2 = vector<float>(pos3, pos4);
				stripes2.push_back(move(part2));

				auto pos5 = markers.begin() + sidx * w;
				auto pos6 = markers.begin() + (eidx + 1) * w;
				auto part3 = vector<float>(pos5, pos6);
				stripes3.push_back(move(part3));

				getImpl().stripeStarts.push_back(sidx);
				getImpl().stripeCloses.push_back(eidx);

				LogD() << "Motion free stripe: " << (stripes1.size() - 1) << ", sidx: " << sidx << ", eidx: " << eidx;
			}
			sidx = eidx = -1;
		}
		else {
			sidx = (sidx >= 0 ? sidx : i);
			eidx = i;
		}
	}

	int count = 0;
	for (const auto& s : stripes1) {
		count += (int)(s.size() / w);
	}
	float ratio = ((float)count / h);
	LogD() << "Motion free lines: " << count << " / " << h << ", ratio: " << ratio;

	getImpl().stripesDecorr = stripes1;
	getImpl().stripesDiffer = stripes2;
	getImpl().stripesMarker = stripes3;

	float mean = accumulate(markers.begin(), markers.end(), 0.0f) / markers.size();
	getImpl().averageMarker = mean;

	return count;
}



bool OctAngio::AngioMotion::measureStripesDisplacement(void)
{
	auto& stripes = getImpl().stripesMarker;
	auto shifts = vector<int>();

	if (stripes.size() <= 1) {
		return false;
	}

	int h = Layout().getHeight();
	int w = Layout().getWidth();
	int margin = max((int)(w * 0.01f), 10);
	int startPos = margin;
	int closePos = (w - margin - 1);

	float range = (Layout().isVerticalScan() ? Layout().scanRangeY() : Layout().scanRangeX());
	float limit = (21.0f / max(range, 3.0f)) * 3.0f;

	int fastHalf = (int)(limit);
	int fastSize = (int)(fastHalf * 2 + 1);
	int slowSize = 1;
	int meanSize = 1;

	int shiftAcc = 0;
	shifts.push_back(shiftAcc);

	float thresh1 = getImpl().averageMarker * getImpl().vesselOverThreshold; // max_v1 / 2.0f;
	float thresh2 = getImpl().averageMarker * getImpl().vesselOverThreshold; // max_v2 / 2.0f;

	for (int i = 1; i < stripes.size(); i++) {
		auto& s1 = stripes[i - 1];
		auto& s2 = stripes[i];

		int size1 = (int)(s1.size() / w);
		int size2 = (int)(s2.size() / w);

		LogD() << "Free Stipes start: " << (i - 1) << ", " << i;
		LogD() << "stripe start: " << getImpl().stripeStarts[i - 1] << ", size: " << size1 << ", thresh: " << thresh1;
		LogD() << "stripe start: " << getImpl().stripeStarts[i] << ", size: " << size2 << ", thresh: " << thresh2;

		if ((size1 < getImpl().motionFreeStripeSizeMin || size2 < getImpl().motionFreeStripeSizeMin) || 
			(getImpl().stripeStarts[i] < (h / 9))) {
			shifts.push_back(shiftAcc);
			LogD() << "shift acc: " << shiftAcc << ", size: " << shifts.size();
			continue;
		}

		auto dists = vector<float>(fastSize * slowSize, 0.0f);
		auto moves = vector<int>(fastSize * slowSize, 0);
		auto votes = vector<int>(fastSize * slowSize, 0);

		for (int j = 0; j < slowSize; j++) {
			auto t1 = s1;
			auto t2 = s2;

			// for_each(t1.begin(), t1.end(), [&](float &e) { e = max(e - thresh1, 0.0f); });
			// for_each(t2.begin(), t2.end(), [&](float &e) { e = max(e - thresh2, 0.0f); });

			/*
			if (i == 1) {
				for (int m = startPos; m < closePos; m++) {
					int n = m;
					LogD() << m << ", " << t1[m + (size1 - 5)*w] << " " << t1[m + (size1 - 4)*w] << " " << t1[m + (size1 - 3)*w] << " " << t1[m + (size1 - 2)*w] << " " << t1[m + (size1 - 1)*w] << "   :  " << t2[n] << " " << t2[n + w] << " " << t2[n + w * 2] << " " << t2[n + w * 3] << " " << t2[n + w * 4];
				}
			}
			*/
			
			/*
			if (i == 1) {
				for (int m = startPos; m < closePos; m++) {
					LogD() << m << " : " << t1[m + (size1 - 1)*w] << ", " << t2[m];
				}
			}
			*/

			for (int k = -fastHalf; k <= fastHalf; k++) {
				float dsum = 0.0f;
				float dist = 0.0f;
				int dcnt = 0;

				for (int m = startPos; m < closePos; m++) {
					int sp = k + m;
					if (sp >= startPos && sp < closePos) {
						float c = connectivityOfStripes2(t1, t2, size1, size2, m, sp, w, 5);
						/*
						if (i == 1 && (m >= 85 && m <= 105)) {
							LogD() << "m: " << m << ", k: " << k << ", c: " << c;
						}
						*/
						if (c > 10.0f) {
							// Choose the maximum response from the vessel. 
							dist = max(c, dist);
						}
						else {
							// Accumulate the distance from the vessel as the kernel response has been faded away. 
							if (dist > 0.0f) {
								// c /= thresh1;
								dsum += (dist * -1.0f);
								dcnt++;
							}
							dist = 0.0f;
						}
					}
				}

				if (dcnt > 0) {
					int idx = k + fastHalf;
					dists[j * fastSize + idx] = dsum;
					moves[j * fastSize + idx] = k;
					votes[j * fastSize + idx] = dcnt;
					// LogD() << "row: " << j << ", col: " << k << ", dsum: " << dsum << ", dcnt: " << dcnt;
				}
			}
		}

		auto min_pos = min_element(dists.cbegin(), dists.cend());
		auto min_idx = distance(dists.cbegin(), min_pos);
		auto min_dist = dists[min_idx];
		auto min_move = moves[min_idx];
		auto min_vote = votes[min_idx];
		auto org_dist = dists[slowSize / 2 + fastSize / 2];

		LogD() << "min dist: " << min_dist << ", move: " << min_move << ", vote: " << min_vote << ", org_dist: " << org_dist;

		if (min_vote < getImpl().vesselShiftPointsMin || min_dist >= getImpl().vesselShiftDistanceMax) {
			min_move = 0;
		}
		if (min_dist > (org_dist * 1.1f)) {
			min_move = 0;
		}

		shiftAcc += min_move;
		shifts.push_back(shiftAcc);

		LogD() << "shift acc: " << shiftAcc << ", size: " << shifts.size();
		// break;
	}

	getImpl().stripesShifts = shifts;
	return true;
}



bool OctAngio::AngioMotion::makeMotionFreeStripes(const std::vector<float>& decorrs, const std::vector<float>& differs, const std::vector<float>& markers)
{
	if (markers.empty() || decorrs.empty() || differs.empty()) {
		return 0;
	}

	int h = Layout().getHeight();
	int w = Layout().getWidth();

	auto avgs = vector<float>(h, 0.0f);
	auto fpos = markers.begin();
	auto epos = fpos + w;

	for (int i = 0; i < h; i++) {
		avgs[i] = accumulate(fpos, epos, 0.0f) / w;
		fpos += w;
		epos = (i < h - 1 ? epos + w : markers.end());
	}

	getImpl().averageMarker = accumulate(avgs.begin(), avgs.end(), 0.0f) / avgs.size();

	auto overs = vector<float>(h, 0.0f);
	auto mavgs = vector<float>(h, 0.0f);
	auto fpos2 = markers.begin();
	auto epos2 = fpos2 + w;
	auto count = 0;
	auto dmean = 0.0f;
	int sects = (int)(getImpl().lineDecorrMovingAvgSize * h);
	int halfs = sects / 2;

	for (int i = 0, k = 0; i < h; i++) {
		if (i < halfs) {
			k = 0;
		}
		else if (i >= (h - halfs - 1)) {
			k = h - sects - 1;
		}
		else {
			k = i - halfs;
		}

		for (count = 0, dmean = 0.0f; count < sects; count++) {
			dmean += avgs[k + count];
		}
		mavgs[i] = dmean / sects;
		float threshold = mavgs[i] * getImpl().lineDecorrThreshold;

		count = (int)count_if(fpos2, epos2, [threshold](float e) { return e > threshold; });
		overs[i] = (float)count / w;
		fpos2 += w;
		epos2 = (i < h - 1 ? epos2 + w : markers.end());
		LogD() << "Line: " << i << ", decorr: " << mavgs[i] << ", overs: " << count << ", " << overs[i] << ", thresh: " << threshold;
	}

	auto stripes1 = vector<vector<float>>();
	auto stripes2 = vector<vector<float>>();
	auto stripes3 = vector<vector<float>>();
	auto indice = vector<int>();

	int sidx = -1, eidx = -1;
	for (int i = 0; i < h; i++) {
		if (/*avgs[i] > threshold ||*/ overs[i] > getImpl().overPointsThreshold1) {
			LogD() << "Motion affected, line: " << i << ", decorr: " << avgs[i] << ", overs: " << overs[i];
			eidx -= 1;

			if ((eidx - sidx) >= getImpl().motionFreeStripeSizeMin) {
				auto pos1 = decorrs.begin() + sidx * w;
				auto pos2 = decorrs.begin() + (eidx + 1) * w;
				auto part = vector<float>(pos1, pos2);
				stripes1.push_back(move(part));
				LogD() << "Motion free stripe: " << (stripes1.size() - 1) << ", sidx: " << sidx << ", eidx: " << eidx;

				auto pos3 = differs.begin() + sidx * w;
				auto pos4 = differs.begin() + (eidx + 1) * w;
				auto part2 = vector<float>(pos3, pos4);
				stripes2.push_back(move(part2));

				auto pos5 = markers.begin() + sidx * w;
				auto pos6 = markers.begin() + (eidx + 1) * w;
				auto part3 = vector<float>(pos5, pos6);
				stripes3.push_back(move(part3));

				indice.push_back(sidx);
			}
			sidx = eidx = -1;
		}
		else {
			sidx = (sidx < 0 ? (i == 0 ? i : i+1) : sidx);
			eidx = i;
		}
	}

	if ((eidx - sidx) >= getImpl().motionFreeStripeSizeMin) {
		auto pos1 = decorrs.begin() + sidx * w;
		auto pos2 = decorrs.begin() + (eidx + 1) * w;
		auto part = vector<float>(pos1, pos2);
		stripes1.push_back(move(part));
		LogD() << "Motion free stripe: " << (stripes1.size() - 1) << ", sidx: " << sidx << ", eidx: " << eidx;

		auto pos3 = differs.begin() + sidx * w;
		auto pos4 = differs.begin() + (eidx + 1) * w;
		auto part2 = vector<float>(pos3, pos4);
		stripes2.push_back(move(part2));

		auto pos5 = markers.begin() + sidx * w;
		auto pos6 = markers.begin() + (eidx + 1) * w;
		auto part3 = vector<float>(pos5, pos6);
		stripes3.push_back(move(part3));

		indice.push_back(sidx);
	}

	count = 0;
	for (const auto& s : stripes1) {
		count += (int)(s.size() / w);
	}
	float ratio = ((float)count / h);

	LogD() << "Motion free lines: " << count << " / " << h << ", ratio: " << ratio;
	LogD() << "Motion free stripes: " << stripes1.size() << ", " << stripes2.size() << ", " << stripes3.size();
	getImpl().stripesDecorr = stripes1;
	getImpl().stripesDiffer = stripes2;
	getImpl().stripesMarker = stripes3;
	getImpl().stripesIndice = indice;

	if ((int)stripes1.size() <= getImpl().motionFreeStripesNumberMax &&
		ratio >= getImpl().motionFreeStripesRatioMin) {
		return true;
	}
	return false;
}


float OctAngio::AngioMotion::connectivityOfStripes(const std::vector<float>& stripe1, const std::vector<float>& stripe2, 
													int size1, int size2, int pos1, int pos2, int width, int ksize)
{
	vector<float> cs;
	float w1, w2, wc;
	auto& s1 = stripe1;
	auto& s2 = stripe2;
	int h1 = size1;
	int h2 = size2;
	int hspan = ksize / 2;

	w1 = w2 = 0.0f;
	for (int k = 1; k <= hspan; k++) {
		w1 += s1[pos1 + (h1 - k)*width];
		w2 += s2[pos2 + (k - 1) * width];
	}
	wc = (w1 + w2) / (ksize - 1);
	wc = ((w1 <= 1.0f || w2 <= 1.0f) ? 0.0f : wc);
	cs.push_back(wc);
	/*
	if (pos1 == pos2) {
		LogD() << pos1 << ", " << w1 << " : " << w2 << " = " << wc;
	}
	*/

	w1 = w2 = 0.0f;
	for (int k = 1; k <= hspan; k++) {
		w1 += s1[pos1 + k + (h1 - k)*width];
		w2 += s2[pos2 - k + (k - 1) * width];
	}
	wc = (w1 + w2) / (ksize - 1);
	wc = ((w1 <= 1.0f || w2 <= 1.0f) ? 0.0f : wc);
	cs.push_back(wc);

	w1 = w2 = 0.0f;
	for (int k = 1; k <= hspan; k++) {
		w1 += s1[pos1 - k + (h1 - k)*width];
		w2 += s2[pos2 + k + (k - 1) * width];
	}
	wc = (w1 + w2) / (ksize - 1);
	wc = ((w1 <= 1.0f || w2 <= 1.0f) ? 0.0f : wc);
	cs.push_back(wc);

	w1 = w2 = 0.0f;
	for (int k = 1; k <= hspan; k++) {
		w1 += s1[pos1 + k / 2 + (h1 - k)*width];
		w2 += s2[pos2 - k / 2 + (k - 1) * width];
	}
	wc = (w1 + w2) / (ksize - 1);
	wc = ((w1 <= 1.0f || w2 <= 1.0f) ? 0.0f : wc);
	cs.push_back(wc);

	w1 = w2 = 0.0f;
	for (int k = 1; k <= hspan; k++) {
		w1 += s1[pos1 - k / 2 + (h1 - k)*width];
		w2 += s2[pos2 + k / 2 + (k - 1) * width];
	}
	wc = (w1 + w2) / (ksize - 1);
	wc = ((w1 <= 1.0f || w2 <= 1.0f) ? 0.0f : wc);
	cs.push_back(wc);

	/*
	w1 = w2 = 0.0f;
	for (int k = 1; k <= hspan; k++) {
		w1 += s1[pos1 + k + (h1 - (k+1)/2)*width];
		w2 += s2[pos2 - k + ((k+1)/2 - 1) * width];
	}
	wc = (w1 + w2) / (ksize - 1);
	wc = (w1 * w2 <= 0.0f ? 0.0f : wc);
	cs.push_back(wc);

	w1 = w2 = 0.0f;
	for (int k = 1; k <= hspan; k++) {
		w1 += s1[pos1 - k + (h1 - (k+1)/2)*width];
		w2 += s2[pos2 + k + ((k+1)/2 - 1) * width];
	}
	wc = (w1 + w2) / (ksize - 1);
	wc = (w1 * w2 <= 0.0f ? 0.0f : wc);
	cs.push_back(wc);
	*/

	return *max_element(cs.begin(), cs.end());
}


float OctAngio::AngioMotion::connectivityOfStripes2(const std::vector<float>& stripe1, const std::vector<float>& stripe2, int size1, int size2, int pos1, int pos2, int width, int ksize)
{
	vector<float> cs;
	float w1, w2, wc;
	auto& s1 = stripe1;
	auto& s2 = stripe2;
	const int WIDE = 3;
	const int t1 = -(WIDE / 2);
	const int t2 = +(WIDE / 2);
	const int DENOM = WIDE * ksize * 2;

	w1 = w2 = 0.0f;
	for (int t = t1; t <= t2; t++) {
		for (int k = 1; k <= ksize; k++) {
			w1 += s1[pos1 + (size1 - k)*width + t];
			w2 += s2[pos2 + (k - 1)*width + t];
		}
	}
	wc = ((w1 <= 1.0f || w2 <= 1.0f) ? 0.0f : (w1 + w2) / DENOM);
	cs.push_back(wc);

	w1 = w2 = 0.0f;
	for (int t = t1; t <= t2; t++) {
		for (int k = 1; k <= ksize; k++) {
			w1 += s1[pos1 + (size1 - k)*width - (k/2) + t];
			w2 += s2[pos2 + (k - 1)*width + (k/2) + t];
		}
	}
	wc = ((w1 <= 1.0f || w2 <= 1.0f) ? 0.0f : (w1 + w2) / DENOM);
	cs.push_back(wc);

	w1 = w2 = 0.0f;
	for (int t = t1; t <= t2; t++) {
		for (int k = 1; k <= ksize; k++) {
			w1 += s1[pos1 + (size1 - k)*width + (k / 2) + t];
			w2 += s2[pos2 + (k - 1)*width - (k / 2) + t];
		}
	}
	wc = ((w1 <= 1.0f || w2 <= 1.0f) ? 0.0f : (w1 + w2) / DENOM);
	cs.push_back(wc);

	w1 = w2 = 0.0f;
	for (int t = t1; t <= t2; t++) {
		for (int k = 1; k <= ksize; k++) {
			w1 += s1[pos1 + (size1 - k)*width - (k - 1) + t - 1];
			w2 += s2[pos2 + (k - 1)*width + (k - 1) + t];
		}
	}
	wc = ((w1 <= 1.0f || w2 <= 1.0f) ? 0.0f : (w1 + w2) / DENOM);
	cs.push_back(wc);

	w1 = w2 = 0.0f;
	for (int t = t1; t <= t2; t++) {
		for (int k = 1; k <= ksize; k++) {
			w1 += s1[pos1 + (size1 - k)*width + (k + 1) + t + 1];
			w2 += s2[pos2 + (k - 1)*width - (k - 1) + t];
		}
	}
	wc = ((w1 <= 1.0f || w2 <= 1.0f) ? 0.0f : (w1 + w2) / DENOM);
	cs.push_back(wc);

	return *max_element(cs.begin(), cs.end());
}


std::vector<float> OctAngio::AngioMotion::computeFeaturesOfSplits(const std::vector<float>& image, int sidx, int eidx, 
									int pos, int width, int height, const std::vector<float>& flows)
{
	auto feats = std::vector<float>(2, 0.0f);
	int pos1 = sidx * width + pos;
	int pos2 = eidx * width + pos;
	int ksize = 4;

	vector<float> dat1;
	vector<float> dat2;
	float avg; 

	const float FLOW = 1.0f;
	int flow1 = 0;
	int flow2 = 0;

	if (sidx >= ksize) {
		vector<float> v;
		for (int k = 0; k < ksize; k++) {
			int n = pos1 - width * k;
			v.push_back(image[n]);
			if (flows[n] >= FLOW) {
				flow1++;
			}
		}
		avg = accumulate(v.cbegin(), v.cend(), 0.0f) / v.size();
		dat1.push_back(avg);

		if (pos >= ksize) {
			v.clear();
			for (int k = 0; k < ksize; k++) {
				int n = pos1 - width * k - k;
				v.push_back(image[n]);
				if (flows[n] >= FLOW) {
					flow1++;
				}
			}
			avg = accumulate(v.cbegin(), v.cend(), 0.0f) / v.size();
			dat1.push_back(avg);
		}

		if (pos >= ksize) {
			v.clear();
			for (int k = 0; k < ksize; k++) {
				int n = pos1 - width * (k / 2) - k;
				v.push_back(image[n]);
				if (flows[n] >= FLOW) {
					flow1++;
				}
			}
			avg = accumulate(v.cbegin(), v.cend(), 0.0f) / v.size();
			dat1.push_back(avg);
		}

		if (pos < (width - ksize)) {
			v.clear();
			for (int k = 0; k < ksize; k++) {
				int n = pos1 - width * k + k;
				v.push_back(image[n]);
				if (flows[n] >= FLOW) {
					flow1++;
				}
			}
			avg = accumulate(v.cbegin(), v.cend(), 0.0f) / v.size();
			dat1.push_back(avg);
		}

		if (pos < (width - ksize)) {
			v.clear();
			for (int k = 0; k < ksize; k++) {
				int n = pos1 - width * (k / 2) + k;
				v.push_back(image[n]);
				if (flows[n] >= FLOW) {
					flow1++;
				}
			}
			avg = accumulate(v.cbegin(), v.cend(), 0.0f) / v.size();
			dat1.push_back(avg);
		}
	}
	else {
		vector<float> v;
		for (int i = sidx, n = 0; i >= 0; i--, n++) {
			v.push_back(image[pos1 - width*n]);
		}
		avg = accumulate(v.cbegin(), v.cend(), 0.0f) / v.size();
		dat1.push_back(avg);
	}


	if (eidx < (height - ksize)) {
		vector<float> v;
		for (int k = 0; k < ksize; k++) {
			int n = pos2 + width * k;
			v.push_back(image[n]);
			if (flows[n] >= FLOW) {
				flow2++;
			}
		}
		avg = accumulate(v.cbegin(), v.cend(), 0.0f) / v.size();
		dat2.push_back(avg);

		if (pos >= ksize) {
			v.clear();
			for (int k = 0; k < ksize; k++) {
				int n = pos2 + width * k - k;
				v.push_back(image[n]);
				if (flows[n] >= FLOW) {
					flow2++;
				}
			}
			avg = accumulate(v.cbegin(), v.cend(), 0.0f) / v.size();
			dat2.push_back(avg);
		}

		if (pos >= ksize) {
			v.clear();
			for (int k = 0; k < ksize; k++) {
				int n = pos2 + width * (k / 2) - k;
				v.push_back(image[n]);
				if (flows[n] >= FLOW) {
					flow2++;
				}
			}
			avg = accumulate(v.cbegin(), v.cend(), 0.0f) / v.size();
			dat2.push_back(avg);
		}

		if (pos < (height - ksize)) {
			v.clear();
			for (int k = 0; k < ksize; k++) {
				int n = pos2 + width * k + k;
				v.push_back(image[n]);
				if (flows[n] >= FLOW) {
					flow2++;
				}
			}
			avg = accumulate(v.cbegin(), v.cend(), 0.0f) / v.size();
			dat2.push_back(avg);
		}

		if (pos < (height - ksize)) {
			v.clear();
			for (int k = 0; k < ksize; k++) {
				int n = pos2 + width * (k / 2) + k;
				v.push_back(image[n]);
				if (flows[n] >= FLOW) {
					flow2++;
				}
			}
			avg = accumulate(v.cbegin(), v.cend(), 0.0f) / v.size();
			dat2.push_back(avg);
		}
	}
	else {
		vector<float> v;
		for (int i = eidx, n = 0; i < height; i++, n++) {
			v.push_back(image[pos2 + width*n]);
		}
		avg = accumulate(v.cbegin(), v.cend(), 0.0f) / v.size();
		dat2.push_back(avg);
	}

	// LogD() << "x: " << pos << ", val1: " << flow1 << ", val2: " << flow2;

	if (!dat1.empty()) {
		if (flow1 > ksize && flow2 > ksize) {
			feats[0] = *max_element(dat1.cbegin(), dat1.cend());
		}
		else {
			feats[0] = *min_element(dat1.cbegin(), dat1.cend());
		}
	}
	if (!dat2.empty()) {
		if (flow1 > ksize && flow2 > ksize) {
			feats[1] = *max_element(dat2.cbegin(), dat2.cend());
		}
		else {
			feats[1] = *min_element(dat2.cbegin(), dat2.cend());
		}
	}
	return feats;
}



std::vector<float> OctAngio::AngioMotion::featuresOfSplit(const std::vector<float>& image, int sidx, int eidx, int pos, int width, int height, int ksize)
{
	auto feats = std::vector<float>(2, 0.0f);
	int hspan = ksize / 2 ;
	int w = width;
	int h = height;

	vector<float> cs;
	vector<float> w1s;
	vector<float> w2s;
	float w1, w2, wc;

	if (pos < hspan || pos >= (width - hspan)) {
		feats[0] = image[pos + sidx * w];
		feats[1] = image[pos + eidx * w];
		return feats;
	}

	w1 = w2 = 0.0f;
	for (int k = 1; k <= hspan; k++) {
		w1 += image[pos + (sidx - k + 1) * width];
		w2 += image[pos + (eidx + k - 1) * width];
	}
	wc = (w1 + w2) / (ksize - 1);
	w1 /= hspan;
	w2 /= hspan;
	cs.push_back(wc);
	w1s.push_back(w1);
	w2s.push_back(w2);

	w1 = w2 = 0.0f;
	for (int k = 1; k <= hspan; k++) {
		w1 += image[pos + k + (sidx - k + 1) * width];
		w2 += image[pos - k + (eidx + k - 1) * width];
	}
	wc = (w1 + w2) / (ksize - 1);
	w1 /= hspan;
	w2 /= hspan;
	cs.push_back(wc);
	w1s.push_back(w1);
	w2s.push_back(w2);

	w1 = w2 = 0.0f;
	for (int k = 1; k <= hspan; k++) {
		w1 += image[pos - k + (sidx - k + 1) * width];
		w2 += image[pos + k + (eidx + k - 1) * width];
	}
	wc = (w1 + w2) / (ksize - 1);
	w1 /= hspan;
	w2 /= hspan;
	cs.push_back(wc);
	w1s.push_back(w1);
	w2s.push_back(w2);

	if (ksize >= 5) {
		w1 = w2 = 0.0f;
		for (int k = 1; k <= hspan; k++) {
			w1 += image[pos + k / 2 + (sidx - k + 1) * width];
			w2 += image[pos - k / 2 + (eidx + k - 1) * width];
		}
		wc = (w1 + w2) / (ksize - 1);
		w1 /= hspan;
		w2 /= hspan;
		cs.push_back(wc);
		w1s.push_back(w1);
		w2s.push_back(w2);

		w1 = w2 = 0.0f;
		for (int k = 1; k <= hspan; k++) {
			w1 += image[pos - k / 2 + (sidx - k + 1) * width];
			w2 += image[pos + k / 2 + (eidx + k - 1) * width];
		}
		wc = (w1 + w2) / (ksize - 1);
		w1 /= hspan;
		w2 /= hspan;
		cs.push_back(wc);
		w1s.push_back(w1);
		w2s.push_back(w2);

		w1 = w2 = 0.0f;
		for (int k = 1; k <= hspan; k++) {
			w1 += image[pos + k + (sidx - ((k + 1) / 2) + 1) * width];
			w2 += image[pos - k + (eidx + ((k + 1) / 2) - 1) * width];
		}
		w1 /= hspan;
		w2 /= hspan;
		wc = (w1 + w2) / (ksize - 1);
		cs.push_back(wc);
		w1s.push_back(w1);
		w2s.push_back(w2);

		w1 = w2 = 0.0f;
		for (int k = 1; k <= hspan; k++) {
			w1 += image[pos - k + (sidx - ((k + 1) / 2) + 1) * width];
			w2 += image[pos + k + (eidx + ((k + 1) / 2) - 1) * width];
		}
		wc = (w1 + w2) / (ksize - 1);
		w1 /= hspan;
		w2 /= hspan;
		cs.push_back(wc);
		w1s.push_back(w1);
		w2s.push_back(w2);
	}

	int index = (int)distance(cs.begin(), max_element(cs.begin(), cs.end()));

	feats[0] = w1s[index];
	feats[1] = w2s[index];
	return feats;
}


bool OctAngio::AngioMotion::registMotionFreeStripes2(void)
{
	auto& shifts = getImpl().stripesShifts;
	if (shifts.size() <= 0) {
		return false;
	}

	auto& ss1 = getImpl().stripesDecorr;
	auto& ss2 = getImpl().stripesDiffer;
	auto& ss3 = getImpl().stripesMarker;

	if (ss1.size() <= 0 || ss2.size() <= 0 || ss3.size() <= 0) {
		return false;
	}

	int h = Layout().getHeight();
	int w = Layout().getWidth();

	auto image1 = vector<float>(h*w, 0.0f);
	auto image2 = vector<float>(h*w, 0.0f);
	auto image3 = vector<float>(h*w, 0.0f);

	for (int i = 0; i < ss1.size(); i++) {
		auto shift = shifts[i];
		auto smove = abs(shift);

		if (smove != 0)
		{
			{
				auto& patch = ss1[i];
				auto rows = patch.size() / w;
				for (int k = 0; k < rows; k++) {
					auto pos1 = patch.begin() + w*k;
					auto pos2 = patch.begin() + w*(k + 1);
					auto line = vector<float>(pos1, pos2);

					if (shift > 0) {
						rotate(line.begin(), line.begin() + smove, line.end());
						fill_n(line.rbegin(), smove, 0.0f);
					}
					else {
						rotate(line.rbegin(), line.rbegin() + smove, line.rend());
						fill_n(line.begin(), smove, 0.0f);
					}
					std::copy(line.begin(), line.end(), pos1);
				}
			}

			{
				auto& patch = ss2[i];
				auto rows = patch.size() / w;
				for (int k = 0; k < rows; k++) {
					auto pos1 = patch.begin() + w*k;
					auto pos2 = patch.begin() + w*(k + 1);
					auto line = vector<float>(pos1, pos2);

					if (shift > 0) {
						rotate(line.begin(), line.begin() + smove, line.end());
						fill_n(line.rbegin(), smove, 0.0f);
					}
					else {
						rotate(line.rbegin(), line.rbegin() + smove, line.rend());
						fill_n(line.begin(), smove, 0.0f);
					}
					std::copy(line.begin(), line.end(), pos1);
				}
			}

			{
				auto& patch = ss3[i];
				auto rows = patch.size() / w;
				for (int k = 0; k < rows; k++) {
					auto pos1 = patch.begin() + w*k;
					auto pos2 = patch.begin() + w*(k + 1);
					auto line = vector<float>(pos1, pos2);

					if (shift > 0) {
						rotate(line.begin(), line.begin() + smove, line.end());
						fill_n(line.rbegin(), smove, 0.0f);
					}
					else {
						rotate(line.rbegin(), line.rbegin() + smove, line.rend());
						fill_n(line.begin(), smove, 0.0f);
					}
					std::copy(line.begin(), line.end(), pos1);
				}
			}
		}

		int sidx = getImpl().stripeStarts[i];
		int eidx = getImpl().stripeCloses[i];
		int size = (eidx - sidx + 1);

		std::copy(ss1[i].cbegin(), ss1[i].cend(), image1.begin() + sidx*w);
		std::copy(ss2[i].cbegin(), ss2[i].cend(), image2.begin() + sidx*w);
		std::copy(ss3[i].cbegin(), ss3[i].cend(), image3.begin() + sidx*w);
	}

	getImpl().freeDecorrs = image1;
	getImpl().freeDiffers = image2;
	getImpl().freeMarkers = image3;
	return true;
}


void OctAngio::AngioMotion::interpolateEmptyLines(const std::vector<float>& markers)
{
	auto& dcs = getImpl().freeDecorrs;
	auto& dfs = getImpl().freeDiffers;

	int h = Layout().getHeight();
	int w = Layout().getWidth();
	int size = h * w;

	if (dcs.size() != size || dfs.size() != size) {
		return;
	}
	if (getImpl().motionLines.size() != h) {
		return;
	}

	for (int i = 0; i < h; i++) {
		if (!getImpl().motionLines[i]) {
			continue;
		}

		int sidx = -1;
		int eidx = -1;
		for (int j = i; j >= 0; j--) {
			if (!getImpl().motionLines[j]) {
				sidx = j;
				break;
			}
		}
		for (int j = i; j < h; j++) {
			if (!getImpl().motionLines[j]) {
				eidx = j;
				break;
			}
		}
		if (sidx <= 0 || eidx <= 0 || sidx >= h-1 || eidx >= h-1) {
			continue;
		}

		LogD() << "Line interpolated, line: " << i << ", sidx: " << sidx << ", eidx: " << eidx << ", size: " << (eidx - sidx + 1);

		for (int k = 0; k < w; k++) {
			// auto feats = featuresOfSplit(dcs, sidx, eidx, k, w, h, 5);
			auto feats = computeFeaturesOfSplits(dcs, sidx, eidx, k, w, h, markers);
			int d1 = i - sidx;
			int d2 = eidx - i;
			float val = (d2 * feats[0] + d1 * feats[1]) / (d1 + d2);
			dcs[k + i * w] = val;
		}

		for (int k = 0; k < w; k++) {
			// auto feats = featuresOfSplit(dfs, sidx, eidx, k, w, h, 5);
			auto feats = computeFeaturesOfSplits(dfs, sidx, eidx, k, w, h, markers);
			int d1 = i - sidx;
			int d2 = eidx - i;
			float val = (d2 * feats[0] + d1 * feats[1]) / (d1 + d2);
			dfs[k + i * w] = val;
		}
	}
	return;
}


void OctAngio::AngioMotion::resizeMotionFreeImages(void)
{
	auto& shifts = getImpl().stripesShifts;
	if (shifts.size() <= 0) {
		return ;
	}

	int h = Layout().getHeight();
	int w = Layout().getWidth();

	int shiftL = 0;
	int shiftR = 0;

	for (int i = 1; i < shifts.size(); i++) {
		if (shifts[i] < 0) {
			shiftR = max(abs(shifts[i]), shiftR);
		}
		if (shifts[i] > 0) {
			shiftL = max(abs(shifts[i]), shiftL);
		}
	} 

	int leftPoints = w - (shiftR + shiftL);
	int leftLines = h; // (int)(composed1.size() / w);

	if (w != leftPoints || h != leftLines) {
		int size = min(leftLines, leftPoints);
		int spos = shiftR;

		CvImage image1, image2;
		image1.fromFloat32((const unsigned char*)&getImpl().freeDecorrs[0], w, leftLines);
		image2.fromFloat32((const unsigned char*)&getImpl().freeDiffers[0], w, leftLines);

		image1.crop(spos, 0, size, size);
		image2.crop(spos, 0, size, size);
		image1.resize(w, h);
		image2.resize(w, h);

		getImpl().freeDecorrs = image1.copyDataInFloats();
		getImpl().freeDiffers = image2.copyDataInFloats();
	}
	return;
}


bool OctAngio::AngioMotion::registMotionFreeStripes(bool resize)
{
	auto& shifts = getImpl().stripesShifts;
	if (shifts.size() <= 0) {
		return false;
	}

	auto& stripes1 = getImpl().stripesDecorr;
	auto& stripes2 = getImpl().stripesDiffer;
	auto& stripes3 = getImpl().stripesMarker;

	if (stripes1.size() <= 0 || stripes2.size() <= 0 || stripes3.size() <= 0) {
		return false;
	}

	auto composed1 = stripes1[0];
	auto composed2 = stripes2[0];
	auto composed3 = stripes3[0];

	int h = Layout().getHeight();
	int w = Layout().getWidth();
	int shiftL = 0;
	int shiftR = 0;

	for (int i = 1; i < stripes1.size(); i++) {
		auto& s1 = stripes1[i - 1];
		auto& s2 = stripes1[i];
		auto& s3 = stripes2[i - 1];
		auto& s4 = stripes2[i];
		auto& s5 = stripes3[i - 1];
		auto& s6 = stripes3[i];

		{
			auto& patch = s2;
			auto rows = patch.size() / w;
			auto shift = shifts[i];
			auto smove = abs(shift);

			for (int k = 0; k < rows; k++) {
				auto pos1 = patch.begin() + w*k;
				auto pos2 = patch.begin() + w*(k + 1);
				auto line = vector<float>(pos1, pos2);

				if (shift > 0) {
					rotate(line.begin(), line.begin() + smove, line.end());
					fill_n(line.rbegin(), smove, 0.0f);
				}
				else {
					rotate(line.rbegin(), line.rbegin() + smove, line.rend());
					fill_n(line.begin(), smove, 0.0f);
				}
				std::copy(line.begin(), line.end(), pos1);
			}
		}

		{
			auto& patch = s4;
			auto rows = patch.size() / w;
			auto shift = shifts[i];
			auto smove = abs(shift);

			for (int k = 0; k < rows; k++) {
				auto pos1 = patch.begin() + w*k;
				auto pos2 = patch.begin() + w*(k + 1);
				auto line = vector<float>(pos1, pos2);

				if (shift > 0) {
					rotate(line.begin(), line.begin() + smove, line.end());
					fill_n(line.rbegin(), smove, 0.0f);
				}
				else {
					rotate(line.rbegin(), line.rbegin() + smove, line.rend());
					fill_n(line.begin(), smove, 0.0f);
				}
				std::copy(line.begin(), line.end(), pos1);
			}
		}

		std::copy(s2.begin(), s2.end(), back_inserter(composed1));
		std::copy(s4.begin(), s4.end(), back_inserter(composed2));

		if (shifts[i] < 0) {
			shiftR = max(abs(shifts[i]), shiftR);
		}
		if (shifts[i] > 0) {
			shiftL = max(abs(shifts[i]), shiftL);
		}
	}

	int leftPoints = w - (shiftR + shiftL);
	int leftLines = (int)(composed1.size() / w);
	if (h != leftLines) {
		auto empty = vector<float>((h - leftLines) * w, 0.0f);
		std::copy(empty.begin(), empty.end(), back_inserter(composed1));
		std::copy(empty.begin(), empty.end(), back_inserter(composed2));
		// leftLines = h;
	}

	LogD() << "Composite angiogram, effective lines: " << leftLines << ", points: " << leftPoints;

	if (resize) {
		if (w != leftPoints || h != leftLines) {
			int size = min(leftLines, leftPoints);
			int spos = shiftR;

			CvImage image1, image2;
			image1.fromFloat32((const unsigned char*)&composed1[0], w, leftLines);
			image2.fromFloat32((const unsigned char*)&composed2[0], w, leftLines);

			image1.crop(spos, 0, size, size);
			image2.crop(spos, 0, size, size);
			image1.resize(w, h);
			image2.resize(w, h);

			getImpl().freeDecorrs = image1.copyDataInFloats();
			getImpl().freeDiffers = image2.copyDataInFloats();
			return true;
		}
	}

	getImpl().freeDecorrs = composed1;
	getImpl().freeDiffers = composed2;
	return true;
}


bool OctAngio::AngioMotion::composeMotionFreeProfiles(bool regist)
{
	auto& stripes1 = getImpl().stripesDecorr;
	auto& stripes2 = getImpl().stripesDiffer;
	auto& stripes3 = getImpl().stripesMarker;
	
	if (stripes1.size() <= 0 || stripes2.size() <= 0 || stripes3.size() <= 0) {
		return false;
	}

	int h = Layout().getHeight();
	int w = Layout().getWidth();
	int shiftAccR = 0, shiftCurR = 0;
	int shiftAccL = 0, shiftCurL = 0;

	auto composed1 = stripes1[0];
	auto composed2 = stripes2[0];
	auto composed3 = stripes3[0];

	for (int i = 1; i < stripes1.size(); i++) {
		auto& s1 = stripes1[i - 1];
		auto& s2 = stripes1[i];
		auto& s3 = stripes2[i - 1];
		auto& s4 = stripes2[i];
		auto& s5 = stripes3[i - 1];
		auto& s6 = stripes3[i];

		if (regist)
		{
			int margin = (int)(w * 0.0f);
			int shiftOffset1 = (int)(w * -0.05f);
			int shiftOffset2 = (int)(w * +0.05f);
			float threshDecorr = getImpl().vesselDecorrThreshold;
			int threshPoints = (int)(w * getImpl().vesselPointsThreshold);

			auto line1 = vector<float>(w, 0.0f);
			auto line2 = vector<float>(w, 0.0f);
			int avgSize = 1;

			for (int k = 0; k < avgSize; k++) {
				transform(line1.begin(), line1.end(), s5.end() - w*(k+1), line1.begin(), plus<float>());
				transform(line2.begin(), line2.end(), s6.begin() + w*k, line2.begin(), plus<float>());
			}
			for_each(line1.begin(), line1.end(), [&](float &e) { e /= avgSize; });
			for_each(line2.begin(), line2.end(), [&](float &e) { e /= avgSize; });

			auto max_v1 = *max_element(line1.cbegin() + margin, line1.cend() - margin);
			auto max_v2 = *max_element(line2.cbegin() + margin, line2.cend() - margin);
			// LogD() << "Stripe index: " << (i - 1) << ", " << i << ", max_v: " << max_v1 << ", " << max_v2 << ", threshold: " << threshDecorr << ", count: " << cnt1 << ", " << cnt2;

			float thresh1 = getImpl().averageMarker * 1.5f; // max_v1 / 2.0f;
			float thresh2 = getImpl().averageMarker * 1.5f; // max_v2 / 2.0f;
			for_each(line1.begin(), line1.end(), [&](float &e) { e = max(e - thresh1, 0.0f); });
			for_each(line2.begin(), line2.end(), [&](float &e) { e = max(e - thresh2, 0.0f); });

			auto cnt1 = count_if(line1.begin(), line1.end(), [threshDecorr](float e) { return e > threshDecorr; });
			auto cnt2 = count_if(line2.begin(), line2.end(), [threshDecorr](float e) { return e > threshDecorr; });

			LogD() << "stipes start: " << getImpl().stripesIndice[i - 1] << ", " << getImpl().stripesIndice[i];
			LogD() << "free stripe index: " << (i - 1) << ", size: " << (s5.size() / w) << ", max_v: " << max_v1 << ", count: " << cnt1 << ", thresh: " << thresh1;
			LogD() << "free stripe index: " << i << ", size: " << (s6.size() / w) << ", max_v: " << max_v2 << ", count: " << cnt2;

			if (max_v1 >= threshDecorr && max_v2 >= threshDecorr && cnt1 >= threshPoints && cnt2 >= threshPoints) {
				auto sft_span = shiftOffset2 - shiftOffset1 + 1;
				auto ret_span = 1;
				auto sft_dist = vector<float>(sft_span * ret_span);
				auto sft_move = vector<int>(sft_span * ret_span);
				auto sft_dcnt = vector<int>(sft_span * ret_span, 0);

				float base_dist = 0.1f;
				float min_dist = 999999.0f;
				int min_move = 0;
				int min_pull = 0;
				int min_dcnt = 0;

				for (int m = 0; m < ret_span; m++) {
					auto line1 = vector<float>(w, 0.0f);
					auto line2 = vector<float>(w, 0.0f);

					for (int k = 0; k < avgSize; k++) {
						transform(line1.begin(), line1.end(), s5.end() - w*(k + 1 + m), line1.begin(), plus<float>());
						transform(line2.begin(), line2.end(), s6.begin() + w*k, line2.begin(), plus<float>());
					}
					for_each(line1.begin(), line1.end(), [&](float &e) { e /= avgSize; });
					for_each(line2.begin(), line2.end(), [&](float &e) { e /= avgSize; });

					auto max_v1 = *max_element(line1.cbegin() + margin, line1.cend() - margin);
					auto max_v2 = *max_element(line2.cbegin() + margin, line2.cend() - margin);
					// LogD() << "Stripe index: " << (i - 1) << ", " << i << ", max_v: " << max_v1 << ", " << max_v2 << ", threshold: " << threshDecorr << ", count: " << cnt1 << ", " << cnt2;

					float thresh1 = getImpl().averageMarker * 1.5f; // max_v1 / 2.0f;
					float thresh2 = getImpl().averageMarker * 1.5f; // max_v2 / 2.0f;
					for_each(line1.begin(), line1.end(), [&](float &e) { e = max(e - thresh1, 0.0f); });
					for_each(line2.begin(), line2.end(), [&](float &e) { e = max(e - thresh2, 0.0f); });

					auto cnt1 = count_if(line1.begin(), line1.end(), [threshDecorr](float e) { return e > threshDecorr; });
					auto cnt2 = count_if(line2.begin(), line2.end(), [threshDecorr](float e) { return e > threshDecorr; });

					// LogD() << "free stripe index: " << (i - 1) << ", size: " << (s5.size() / w) << ", max_v: " << max_v1 << ", count: " << cnt1 << ", thresh: " << thresh1;
					// LogD() << "free stripe index: " << i << ", size: " << (s6.size() / w) << ", max_v: " << max_v2 << ", count: " << cnt2;

					for (int sofs = shiftOffset1, n = 0; sofs < shiftOffset2; sofs++, n++) {
						float dsum = 0.0f;
						int dcnt = 0;

						for (int k = 0; k < w; k++) {
							int p1 = k;
							int p2 = k + sofs;
							if (p2 >= margin && p2 <= (w - margin - 1)) {
								int range = 1;
								float w1 = 0.0f;
								float w2 = 0.0f;
								for (int t = p1 - range / 2; t <= p1 + range / 2; t++) {
									if (t >= 0 && t < w) {
										if (line1[t] > 0.0f) {
											w1 += (line1[t] / thresh1);
										}
									}
								}
								for (int t = p2 - range / 2; t <= p2 + range / 2; t++) {
									if (t >= 0 && t < w) {
										if (line2[t] > 0.0f) {
											w2 += (line2[t] / thresh2);
										}
									}
								}

								if (w1 > 0.0f && w2 > 0.0f) {
									dsum += (w1 + w2) * -1.0f;
									dcnt++;
								}
							}
						}

						/*
						for (int m = margin; m < (w / 2 - margin); m++) {
							auto a1 = line1[m];
							auto a2 = line2[m + sofs];
							dsum += fabs(a1 - a2); //  *-1.0f;
							dcnt += 1;
							*/
							/*
							if (a1 >= threshDecorr || a2 >= threshDecorr) {
								dsum += (std::pow((a1 - a2), 2.0f) / (std::pow(a1, 2.0f) + std::pow(a2, 2.0f) + 0.0000001f));
								dcnt += 1;
								// dsum = log(a1 * a2 + 1.0f);
								// dcnt += 1;
							}
							*/
						/*
						}
						*/

						if (dcnt > 0) {
							float dist = dsum; // / dcnt;
							sft_dist[m * ret_span + n] = dist;
							sft_move[m * ret_span + n] = sofs;
							sft_dcnt[m * ret_span + n] = dcnt;

							// LogD() << "m: " << m << ", n: " << n << ", dist: " << dist << ", dcnt: " << dcnt;
						
							if (sofs == 0 && m == 0) {
								base_dist = dist;
							}
							if (dist < min_dist) {
								min_dist = dist;
								min_pull = m;
								min_move = sofs;
								min_dcnt = dcnt;
							}
						}
					}
				}

				if (true || min_pull != 0 || min_move != 0) {
					/*
					auto max_pos = max_element(sft_dist.cbegin(), sft_dist.cend());
					auto min_pos = min_element(sft_dist.cbegin(), sft_dist.cend());
					auto ratio = *max_pos / base_dist; // *min_pos;
					auto shift = sft_move[max_pos - sft_dist.cbegin()];
					LogD() << "Stripe offset max: " << *max_pos << ", min: " << *min_pos << ", ratio: " << ratio << ", shift: " << shift;
					*/

					auto ratio = min_dist / base_dist; 
					auto shift = min_move;
					LogD() << "stripe offset, min_pull: " << min_pull << ", shift: " << shift << ", base_dist: " << base_dist << ", min_dist: " << min_dist << ", ratio: " << ratio << ", dcnt: " << min_dcnt;

					if (true && shift != 0 && min_dcnt >= 5) { // ratio <= getImpl().vesselShiftDifferMin) {
						auto smove = abs(shift);
						for (int j = i; j < stripes1.size(); j++) {
							auto& patch = stripes1[j];
							auto rows = patch.size() / w;
							for (int k = 0; k < rows; k++) {
								auto pos1 = patch.begin() + w*k;
								auto pos2 = patch.begin() + w*(k + 1);
								auto line = vector<float>(pos1, pos2);

								if (shift > 0) {
									rotate(line.begin(), line.begin() + smove, line.end());
									fill_n(line.rbegin(), abs(shift), 0.0f);
								}
								else {
									rotate(line.rbegin(), line.rbegin() + smove, line.rend());
									fill_n(line.begin(), abs(shift), 0.0f);
								}
								std::copy(line.begin(), line.end(), pos1);
							}

							auto& patch2 = stripes2[j];
							auto rows2 = patch2.size() / w;
							for (int k = 0; k < rows2; k++) {
								auto pos1 = patch2.begin() + w*k;
								auto pos2 = patch2.begin() + w*(k + 1);
								auto line = vector<float>(pos1, pos2);

								if (shift > 0) {
									rotate(line.begin(), line.begin() + smove, line.end());
									fill_n(line.rbegin(), abs(shift), 0.0f);
								}
								else {
									rotate(line.rbegin(), line.rbegin() + smove, line.rend());
									fill_n(line.begin(), abs(shift), 0.0f);
								}
								std::copy(line.begin(), line.end(), pos1);
							}

							auto& patch3 = stripes3[j];
							auto rows3 = patch3.size() / w;
							for (int k = 0; k < rows3; k++) {
								auto pos1 = patch3.begin() + w*k;
								auto pos2 = patch3.begin() + w*(k + 1);
								auto line = vector<float>(pos1, pos2);

								if (shift > 0) {
									rotate(line.begin(), line.begin() + smove, line.end());
									fill_n(line.rbegin(), abs(shift), 0.0f);
								}
								else {
									rotate(line.rbegin(), line.rbegin() + smove, line.rend());
									fill_n(line.begin(), abs(shift), 0.0f);
								}
								std::copy(line.begin(), line.end(), pos1);
							}
						}

						if (shift > 0) {
							shiftCurR += smove;
							shiftCurL -= smove;
							shiftAccR = max(shiftAccR, shiftCurR);
						}
						else {
							shiftCurL += smove;
							shiftCurR -= smove;
							shiftAccL = max(shiftAccL, shiftCurL);
						}
					}
				}
			}
		}
		std::copy(s2.begin(), s2.end(), back_inserter(composed1));
		std::copy(s4.begin(), s4.end(), back_inserter(composed2));
		std::copy(s6.begin(), s6.end(), back_inserter(composed3));
	}

	int leftLines = (int)(composed1.size() / w);
	int leftPoints = (w - shiftAccL - shiftAccR);;

	CvImage image1, image2;
	LogD() << "Composite angiogram, effective lines: " << leftLines << ", points: " << leftPoints << ", shift: " << shiftAccL << ", " << shiftAccR;

	if (h != leftLines) {
		auto empty = vector<float>((h - leftLines) * w, 0.0f);
		std::copy(empty.begin(), empty.end(), back_inserter(composed1));
		std::copy(empty.begin(), empty.end(), back_inserter(composed2));
		leftLines = h;
	}

	if (h != leftLines || w != leftPoints) {
		image1.fromFloat32((const unsigned char*)&composed1[0], w, leftLines);
		image1.resize(w, h);

		if (false) { //regist) {
			if (shiftAccL > 0 || shiftAccR > 0) {
				image1.crop((shiftAccL > 0 ? shiftAccL : 0), 0, leftPoints, h);
				image1.resize(w, h);
			}
		}

		image2.fromFloat32((const unsigned char*)&composed2[0], w, leftLines);
		image2.resize(w, h);

		if (false) { //regist) {
			if (shiftAccL > 0 || shiftAccR > 0) {
				image2.crop((shiftAccL > 0 ? shiftAccL : 0), 0, leftPoints, h);
				image2.resize(w, h);
			}
		}

		getImpl().freeDecorrs = image1.copyDataInFloats();
		getImpl().freeDiffers = image2.copyDataInFloats();
	}
	else {
		getImpl().freeDecorrs = composed1;
		getImpl().freeDiffers = composed2;
	}
	return true;
}


AngioLayout & OctAngio::AngioMotion::Layout(void) const
{
	return getImpl().layout;
}



AngioMotion::AngioMotionImpl & OctAngio::AngioMotion::getImpl(void) const
{
	return *d_ptr;
}
