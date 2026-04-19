#include "stdafx.h"
#include "MacularBsegm.h"
#include "OcularLayers.h"
#include "RetinaLayers.h"
#include "OcularBsegm.h"

#include "RetSegm2.h"
#include "CppUtil2.h"
#include "SemtRetina2.h"


using namespace SegmScan;
using namespace RetSegm;
using namespace SemtRetina;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct MacularBsegm::MacularBsegmImpl
{
	bool isOpticDisc;
	int disc_x1;
	int disc_x2;

	int reti_x1;
	int reti_x2;
	float rangeX;

	SemtRetina::MacularSegmFrame segmFrame;

	MacularBsegmImpl() : reti_x1(-1), reti_x2(-1), isOpticDisc(false), disc_x1(-1), disc_x2(-1), rangeX(0.0f)
	{
	}
};


MacularBsegm::MacularBsegm() :
	d_ptr(make_unique<MacularBsegmImpl>())
{
	auto layers = make_unique<RetinaLayers>();
	setOcularLayers(std::move(layers));
}


SegmScan::MacularBsegm::~MacularBsegm() = default;
SegmScan::MacularBsegm::MacularBsegm(MacularBsegm && rhs) = default;
MacularBsegm & SegmScan::MacularBsegm::operator=(MacularBsegm && rhs) = default;

/*
SegmScan::MacularBsegm::MacularBsegm(const MacularBsegm & rhs)
	: d_ptr(make_unique<MacularBsegmImpl>(*rhs.d_ptr))
{
}


MacularBsegm & SegmScan::MacularBsegm::operator=(const MacularBsegm & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}
*/


bool SegmScan::MacularBsegm::performAnalysis(bool meye)
{
	if (source()->isEmpty()) {
		return false;
	}

	float rangeX = getPatternDescript().getScanLength();
	getImpl().rangeX = rangeX;

	bool isDisc = getPatternDescript().isDiscScan();
	bool isAngio = getPatternDescript().isAngioScan();
	if (isAngio) {
		isDisc = (rangeX >= 9.0f ? true : isDisc);
	}
	else {
		isDisc = (rangeX > 9.0f ? true : isDisc);
	}

	bool useSemtVersion = false;
	useSemtVersion = SemtRetina::RetinaSegmentModel::isInitialized();
	/*
	bool useSemtVersion = false;
	if (true || isAngio) {
		useSemtVersion = SemtRetina::RetinaSegmentModel::isInitialized();
	}
	
	if (rangeX > 9.0f) {
		useSemtVersion = false;
	}
	*/

	if (useSemtVersion) {
		auto d_ptr = source()->getBitsData();
		auto src_w = source()->getWidth();
		auto src_h = source()->getHeight();
		auto index = getImageIndex();

		auto& frame = getImpl().segmFrame;
		frame.setBscanImage(d_ptr, src_w, src_h, rangeX, index);
		if (!frame.segmentMacularLayers(true)) {
			goto failed;
		}

		layerILM()->initialize(frame.boundaryILM(), src_w, src_h);
		layerNFL()->initialize(frame.boundaryNFL(), src_w, src_h);
		layerIPL()->initialize(frame.boundaryIPL(), src_w, src_h);
		layerOPL()->initialize(frame.boundaryOPL(), src_w, src_h);
		layerIOS()->initialize(frame.boundaryIOS(), src_w, src_h);
		layerRPE()->initialize(frame.boundaryRPE(), src_w, src_h);
		layerBRM()->initialize(frame.boundaryBRM(), src_w, src_h);

		layerInn()->initialize(frame.boundaryINN(), src_w, src_h);
		layerOut()->initialize(frame.boundaryONL(), src_w, src_h);
		layerOPR()->initialize(frame.boundaryONL(), src_w, src_h);

		auto layers = getRetinaLayers();
		layers->setRegionSize(src_w, src_h);
	}
	else {
		prepareSample(source(), sample());
		if (!Preprocess::checkIfValidScanImage(sample(), getImageIndex())) {
			goto failed;
			// return false;
		}

		createGradients(sample(), gradients(), ascends(), descends());
		if (!makeupRetinaBorders(isDisc, rangeX)) {
			goto failed;
		}
		// return true;

		buildLayerILM(sample(), layerInn(), layerOut(), layerILM());
		buildLayerOut(ascends(), layerILM(), layerOut());
		buildLayerOPR(ascends(), layerILM(), layerOut(), layerOPR());

		buildLayerIOS(ascends(), layerILM(), layerOut(), layerOPR(), layerIOS());
		buildLayerBRM(sample(), layerIOS(), layerOPR(), layerBRM());
		buildLayerRPE(sample(), layerIOS(), layerOPR(), layerBRM(), layerRPE());
		// return true;

		buildLayerOPL(sample(), layerILM(), layerIOS(), layerOPL());
		buildLayerIPL(sample(), layerILM(), layerOPL(), layerIPL());
		buildLayerNFL(sample(), layerILM(), layerIPL(), layerNFL());

		if (sample()->getWidth() < source()->getWidth() ||
			sample()->getHeight() < source()->getHeight())
		{
			Preprocess::collectImageStats(source());
			Preprocess::collectColumnStats(source());
			inflateOpticDiscRegion();

			elaborateILM(source(), layerNFL(), layerILM());
			elaborateOPL(source(), layerIPL(), layerIOS(), layerOut(), layerOPL());
			elaborateIPL(source(), layerILM(), layerNFL(), layerOPL(), layerOut(), layerIPL());
			elaborateNFL(source(), layerILM(), layerIPL(), layerOut(), layerNFL());

			elaborateIOS(source(), layerOut(), layerRPE(), layerBRM(), layerIOS());
			elaborateBRM(source(), layerIOS(), layerRPE(), layerBRM());
			elaborateRPE(source(), layerIOS(), layerBRM(), layerOut(), layerRPE());

			reinstateBRM(source(), layerOPR(), layerBRM(), isDisc);

			/*
			auto outer = layerBRM()->getYs();
			auto ideal = outer;
			Feature2::makeupIdealOuterBoundary2(outer, 4.5f, ideal);
			layerOut()->initialize(ideal, ideal.size(), 768);
			*/
			auto layers = getRetinaLayers();
			layers->setRegionSize(source()->getWidth(), source()->getHeight());
		}
	}

	setResult(true);
	return true;

failed:
	getRetinaLayers()->initialize(source()->getWidth(), source()->getHeight());
	return false;
}


void SegmScan::MacularBsegm::initializeLayers(int width, int height)
{
	if (width == 0) {
		width = getSourceWidth();
	}
	if (height == 0) {
		height = getSourceHeight();
	}

	getRetinaLayers()->initialize(width, height);
	return;
}


bool SegmScan::MacularBsegm::prepareSample(const OcularImage * imgSrc, OcularImage * imgSample)
{
	Preprocess::createSampleImage(source(), sample());
	Preprocess::collectImageStats(sample());
	Preprocess::collectColumnStats(sample());
	Preprocess::collectImageStats(source());
	Preprocess::collectColumnStats(source());

	auto layers = getRetinaLayers();
	layers->initialize(sample()->getWidth(), sample()->getHeight());
	return true;
}


bool SegmScan::MacularBsegm::createGradients(const OcularImage * imgSrc, OcularImage * imgGradients, OcularImage * imgAscends, OcularImage * imgDescends)
{
	Coarse::createGradientMapOfRetina(imgSrc, imgAscends, imgDescends, imgGradients);
	Preprocess::collectImageStats(imgAscends);
	Preprocess::collectColumnStats(imgAscends);
	Preprocess::collectImageStats(imgDescends);
	Preprocess::collectColumnStats(imgDescends);
	return true;
}


bool SegmScan::MacularBsegm::makeupRetinaBorders(bool isDisc, float rangeX)
{
	int discX1 = -1, discX2 = -1;
	int retiX1 = -1, retiX2 = -1;
	if (!Coarse2::makeBoundaryLinesOfRetina(sample(), ascends(), descends(), layerInn(), layerOut(), rangeX, isDisc, discX1, discX2, retiX1, retiX2)) {
		return false;
	}
	else {
		d_ptr->reti_x1 = retiX1;
		d_ptr->reti_x2 = retiX2;
		if (isDisc) {
			if (discX2 > discX1) {
				d_ptr->isOpticDisc = true;
				d_ptr->disc_x1 = discX1;
				d_ptr->disc_x2 = discX2;
			}
		}
	}
	return true;
}

bool SegmScan::MacularBsegm::inflateOpticDiscRegion(void)
{
	if (isOpticNerveDisc()) {
		float ratio = 1.0f / getSampleWidthRatio();
		d_ptr->disc_x1 = (int)(d_ptr->disc_x1 * ratio);
		d_ptr->disc_x2 = (int)(d_ptr->disc_x2 * ratio);
	}
	return true;
}


bool SegmScan::MacularBsegm::elaborateILM(const OcularImage * imgSrc, OcularLayer * layerNFL, OcularLayer * layerILM)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerILM->resize(width, height);
	layerNFL->resize(width, height);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designLayerContraintsOfILM(imgSrc, layerILM, layerNFL, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfILM(imgSrc, imgCost, upper, lower, delta, false)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerILM(path, width, height, layerNFL, layerILM, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::elaborateNFL(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer* layerIPL, OcularLayer * layerOut, OcularLayer * layerNFL)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	layerILM->resize(width, height);
	layerNFL->resize(width, height);
	layerIPL->resize(width, height);
	layerOut->resize(width, height);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designLayerContraintsOfNFL(imgSrc, layerILM, layerNFL, layerIPL, layerOut, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfNFL(imgSrc, imgCost, upper, lower, delta, isDisc)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerNFL(path, width, height, layerILM, layerIPL, layerNFL, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::elaborateRPE(const OcularImage * imgSrc, OcularLayer * layerIOS, OcularLayer * layerBRM, OcularLayer * layerOut, OcularLayer * layerRPE)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerIOS->resize(width, height);
	layerBRM->resize(width, height);
	layerRPE->resize(width, height);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designLayerContraintsOfRPE(imgSrc, layerIOS, layerBRM, layerRPE, layerOut, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfRPE(imgSrc, imgCost, upper, lower, delta, false)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerRPE(path, width, height, layerIOS, layerBRM, layerRPE, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::elaborateIPL(const OcularImage * imgSrc, OcularLayer* layerILM, OcularLayer * layerNFL, OcularLayer * layerOPL, OcularLayer * layerOut, OcularLayer * layerIPL)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerIPL->resize(width, height);
	layerOPL->resize(width, height);
	layerOut->resize(width, height);
	layerNFL->resize(width, height);
	layerILM->resize(width, height);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designLayerContraintsOfIPL(imgSrc, layerNFL, layerIPL, layerOPL, layerOut, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfIPL(imgSrc, imgCost, upper, lower, delta, false)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerIPL(path, width, height, layerILM, layerOPL, layerIPL, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::elaborateOPL(const OcularImage * imgSrc, OcularLayer * layerIPL, OcularLayer * layerRPE, OcularLayer * layerOut, OcularLayer * layerOPL)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerIPL->resize(width, height);
	layerRPE->resize(width, height);
	layerOPL->resize(width, height);
	layerOut->resize(width, height);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designLayerContraintsOfOPL(imgSrc, layerIPL, layerOPL, layerRPE, layerOut, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfOPL(imgSrc, imgCost, upper, lower, delta, false)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerOPL(path, width, height, layerIPL, layerRPE, layerOPL, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::elaborateIOS(const OcularImage * imgSrc, OcularLayer * layerOPL, OcularLayer * layerRPE, OcularLayer * layerBRM, OcularLayer * layerIOS)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerIOS->resize(width, height);
	layerRPE->resize(width, height);
	layerOPL->resize(width, height);
	layerBRM->resize(width, height);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designLayerContraintsOfIOS(imgSrc, layerOPL, layerIOS, layerRPE, layerBRM, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfIOS(imgSrc, imgCost, upper, lower, delta, false)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerIOS(path, width, height, layerOPL, layerRPE, layerIOS, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::elaborateBRM(const OcularImage * imgSrc, OcularLayer * layerIOS, OcularLayer * layerRPE, OcularLayer * layerBRM)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerRPE->resize(width, height);
	layerBRM->resize(width, height);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designLayerContraintsOfBRM(imgSrc, layerIOS, layerRPE, layerBRM, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfBRM(imgSrc, imgCost, upper, lower, delta, false)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerBRM(path, width, height, layerIOS, layerBRM, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


void SegmScan::MacularBsegm::reinstateBRM(const OcularImage* imgSrc, OcularLayer* layerOPR, OcularLayer * layerBRM, bool isDisc)
{
	auto oprs = layerBRM->getYs();
	auto path = layerBRM->getYs();
	int width = (int)path.size();
	int height = 768;

	auto flags = std::vector<bool>(width, false);
	
	layerOPR->initialize(path, width, height);
	if (isDisc) {
		return;
	}

	// path = CppUtil::SgFilter::smoothInts(path, 5, 1);
	// transform(begin(path), end(path), begin(path), [=](int elm) { return min(max(elm, 0), height - 1); });

	for (int retry = 0; retry < 9; retry++) {
		auto dirs = vector<int>(width, 0);
		auto ests = vector<int>(width, 0);
		int count = 0;

		for (int i = 1; i < width - 1; i++) {
			int diff = path[i] - path[i - 1];
			dirs[i] = (diff > 0 ? -1 : (diff < 0));
		}

		for (int i = 1; i < width - 1; i++) {
			int sidx = -1;
			int eidx = -1;
			for (int j = i - 1; j >= 1; j--) {
				if (dirs[j] != 0) {
					sidx = j;
					break;
				}
			}
			for (int k = i + 1; k < width-1; k++) {
				if (dirs[k] != 0) {
					eidx = k;
					break;
				}
			}

			if (sidx < 0 || eidx < 0 || dirs[sidx] <= 0 || dirs[eidx] >= 0) {
				continue;
			}

			int x1 = sidx; 
			int x2 = eidx;
			for (int j = sidx; j >= 0; j--) {
				if (dirs[j] < 0) {
					for (; j < i; j++) {
						if (dirs[j] > 0) {
							break;
						}
						x1 = j;
					}
					break;
				}
			}

			for (int k = eidx; k < width; k++) {
				if (dirs[k] > 0) {
					for (; k > i; k--) {
						if (dirs[k] < 0) {
							break;
						}
						x2 = k;
					}
					break;
				}
			}

			x1 = (x1 < 0 ? 0 : x1);
			x2 = (x2 >= width ? width - 1 : x2);

			float slope = (float)(path[x2] - path[x1]) / (float)(x2 - x1);

			for (int n = (x1 + 1), dist = 1; n < x2; n++, dist++) {
				int y = (int)(path[x1] + dist * slope);
				if (y < path[n]) {
					for (int p = x1; p < i; p++) {
						if (ests[p]) {
							x1 = p;
						}
					}
					slope = (float)(path[x2] - path[x1]) / (float)(x2 - x1);
					break;
				}
			}

			for (int n = (x1 + 1), dist = 1; n < x2; n++, dist++) {
				int y = (int)(path[x1] + dist * slope);
				path[n] = min(max(path[n], y), height - 1);
				dirs[n] = (slope > 0 ? -1 : (slope < 0));
				ests[n] = 1;
				count++;

				flags[n] = true;
			}

			i = eidx;
		}

		if (count <= 0) {
			break;
		}
	}



	path = CppUtil::SgFilter::smoothInts(path, 15, 1);
	auto outs = path;
	transform(begin(path), end(path), begin(oprs), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	std::transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });

	/*
	auto upper = vector<int>(width, -1);
	auto lower = vector<int>(width, -1);
	auto delta = vector<int>(width, -1);
	auto imgCost = costs();

	int maxv;
	for (int i = 0; i < width; i++) {
		maxv = max(maxv, path[i]);
	}

	for (int i = 0; i < width; i++) {
		if (flags[i]) {
			upper[i] = path[i];
			lower[i] = maxv;
			delta[i] = 7;
		}
		else {
			upper[i] = path[i];
			lower[i] = path[i];
			delta[i] = 3;
		}
	}

	if (!OptimalPath::createLayerCostMapOfRPE(imgSrc, imgCost, upper, lower, delta, false)) {
		return;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return;
	}
	*/
	layerBRM->initialize(outs, width, height);
	return;
}


void SegmScan::MacularBsegm::initializeBASE(const OcularImage * imgSrc, OcularLayer * layerRPE, OcularLayer * layerBASE)
{
	vector<int> baseLine;
	for (auto iter : layerRPE->getYs()) {
		//			baseLine.push_back(iter - 18);
		baseLine.push_back(iter + 45);
	}
	layerBASE->initialize(baseLine, source()->getWidth(), source()->getHeight());
	return;
}


bool SegmScan::MacularBsegm::buildLayerILM(const OcularImage * imgSrc, OcularLayer * layerInn, OcularLayer * layerOut, OcularLayer * layerILM)
{
	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	layerInn->resize(w, h);
	layerOut->resize(w, h);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designPathConstraintsOfILM(imgSrc, layerInn, layerOut, upper, lower, delta, d_ptr->reti_x1, d_ptr->reti_x2, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createPathCostMapOfILM(imgSrc, imgCost, upper, lower, delta, isDisc)) {
		return false;
	}
	
	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothCurveILM(path, w, h, layerILM, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::buildLayerOut(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerOut)
{
	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	layerILM->resize(w, h);
	layerOut->resize(w, h);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designPathConstraintsOfOut(imgSrc, layerILM, layerOut, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createPathCostMapOfOut(imgSrc, imgCost, upper, lower, delta)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothCurveOPR(path, w, h, layerILM->getYs(), layerOut, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::buildLayerOPR(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerOut, OcularLayer * layerOPR)
{
	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designPathConstraintsOfOPR(imgSrc, layerILM, layerOut, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createPathCostMapOfOPR(imgSrc, imgCost, upper, lower, delta)) {
		return false;
	}
	
	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	if (!Smooth::smoothCurveOPR(path, w, h, layerILM->getYs(), layerOPR, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::buildLayerIOS(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerOut, OcularLayer * layerOPR, OcularLayer * layerIOS)
{
	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designPathConstraintsOfIOS(imgSrc, layerILM, layerOut, layerOPR, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}
	
	if (!OptimalPath::createPathCostMapOfIOS(imgSrc, imgCost, upper, lower, delta)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	if (!Smooth::smoothCurveIOS(path, w, h, layerILM->getYs(), layerOPR->getYs(), layerIOS, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::buildLayerBRM(const OcularImage * imgSrc, OcularLayer * layerIOS, OcularLayer* layerOPR, OcularLayer * layerBRM)
{
	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designPathConstraintsOfBRM(imgSrc, layerIOS, layerOPR, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createPathCostMapOfBRM(imgSrc, imgCost, upper, lower, delta)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	if (!Smooth::smoothCurveBRM(path, w, h, layerIOS->getYs(), layerBRM, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::buildLayerRPE(const OcularImage * imgSrc, OcularLayer * layerIOS, OcularLayer * layerOPR, OcularLayer * layerBRM, OcularLayer * layerRPE)
{
	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	if (!OptimalPath::designPathConstraintsOfRPE(imgSrc, layerIOS, layerOPR, layerBRM, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::createPathCostMapOfRPE(imgSrc, imgCost, upper, lower, delta)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	if (!Smooth::smoothCurveRPE(path, w, h, layerIOS->getYs(), layerBRM->getYs(), layerRPE, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::buildLayerOPL(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerIOS, OcularLayer * layerOPL)
{
	vector<int> upper, lower, delta;
	vector<int> path;

	upper = layerILM->getYs();
	lower = layerIOS->getYs();
	path = vector<int>(upper.size(), -1);
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designPathConstraintsOfOPL(imgSrc, layerILM, layerIOS, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createPathCostMapOfOPL(imgSrc, imgCost, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	if (!Smooth::smoothCurveOPL(path, w, h, layerILM->getYs(), layerIOS->getYs(), layerOPL, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::buildLayerIPL(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerOPL, OcularLayer * layerIPL)
{
	vector<int> upper, lower, delta;
	vector<int> path;

	upper = layerIOS()->getYs();
	lower = layerBRM()->getYs();
	path = vector<int>(upper.size(), -1);
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!OptimalPath::designPathConstraintsOfIPL(imgSrc, layerILM, layerOPL, upper, lower, delta, isDisc, discX1, discX2)) {
		return false;
	}

	if (!OptimalPath::createPathCostMapOfIPL(imgSrc, imgCost, upper, lower, delta)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	if (!Smooth::smoothCurveIPL(path, w, h, layerILM->getYs(), layerOPL->getYs(), layerIPL, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}


bool SegmScan::MacularBsegm::buildLayerNFL(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerIPL, OcularLayer * layerNFL)
{
	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = d_ptr->isOpticDisc;
	int discX1 = d_ptr->disc_x1;
	int discX2 = d_ptr->disc_x2;

	if (!isDisc) {
		upper = layerOPL()->getYs();
		lower = layerRPE()->getYs();
		path = vector<int>(upper.size(), -1);

		if (!OptimalPath::designPathConstraintsOfNFL(imgSrc, layerILM, layerIPL, upper, lower, delta)) {
			return false;
		}
	}
	else {
		upper = layerIOS()->getYs();
		lower = layerBRM()->getYs();
		path = vector<int>(upper.size(), -1);

		float rangeX = getPatternDescript().getScanDistance();
		if (!OptimalPath::designPathConstraintsOfNFL4(imgSrc, layerILM, layerIPL, upper, lower, delta, rangeX, isDisc, discX1, discX2)) {
			return false;
		}
	}

	if (!OptimalPath::createPathCostMapOfNFL(imgSrc, imgCost, upper, lower, delta)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	if (!Smooth::smoothCurveNFL(path, w, h, layerILM->getYs(), layerIPL->getYs(), layerNFL, isDisc, discX1, discX2)) {
		return false;
	}
	return true;
}



OcularLayer * MacularBsegm::layerInn(void) const
{
	return getRetinaLayers()->getInner();
}


OcularLayer * MacularBsegm::layerOut(void) const
{
	return getRetinaLayers()->getOuter();
}


OcularLayer * MacularBsegm::layerILM(void) const
{
	return getRetinaLayers()->getILM();
}


OcularLayer * MacularBsegm::layerNFL(void) const
{
	return getRetinaLayers()->getNFL();
}


OcularLayer * MacularBsegm::layerIPL(void) const
{
	return getRetinaLayers()->getIPL();
}


OcularLayer * MacularBsegm::layerOPL(void) const
{
	return getRetinaLayers()->getOPL();
}


OcularLayer * MacularBsegm::layerOPR(void) const
{
	return getRetinaLayers()->getOPR();
}


OcularLayer * MacularBsegm::layerIOS(void) const
{
	return getRetinaLayers()->getIOS();
}


OcularLayer * MacularBsegm::layerRPE(void) const
{
	return getRetinaLayers()->getRPE();
}


OcularLayer * MacularBsegm::layerBRM(void) const
{
	return getRetinaLayers()->getBRM();
}

OcularLayer * MacularBsegm::layerBASE(void) const
{
	return getRetinaLayers()->getBASE();
}


MacularBsegm::MacularBsegmImpl & SegmScan::MacularBsegm::getImpl(void) const
{
	return *d_ptr;
}
