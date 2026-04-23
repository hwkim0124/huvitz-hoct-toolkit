#include "stdafx.h"
#include "DiscBsegm.h"
#include "OcularLayers.h"
#include "RetinaLayers.h"
#include "OcularBsegm.h"

/*
#ifdef __USE_SEMT_SEGM__
#include "SemtSegm2.h"
#endif
*/

#include "RetSegm2.h"
#include "CppUtil2.h"
#include "SemtRetina2.h"

using namespace SegmScan;

/*
#ifdef __USE_SEMT_SEGM__
using namespace SemtSegm;
#endif
*/

using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct DiscBsegm::DiscBsegmImpl
{
	bool isOpticDisc;
	int disc_x1;
	int disc_x2;
	int disc_pixels;

	bool isOpticCup;
	int cup_x1;
	int cup_x2;
	int cup_pixels;

	int reti_x1;
	int reti_x2;
	float rangeX;

	SemtRetina::OpticDiscSegmFrame segmFrame;
/*
#ifdef __USE_SEMT_SEGM__
	OpticDiscBscan bscan;
#endif
*/
	DiscBsegmImpl() : isOpticDisc(false), isOpticCup(false), reti_x1(-1), reti_x2(-1),
		disc_x1(-1), disc_x2(-1), cup_x1(-1), cup_x2(-1), disc_pixels(0), cup_pixels(0), rangeX(0.0f)
	{
	}
};


DiscBsegm::DiscBsegm() :
	d_ptr(make_unique<DiscBsegmImpl>())
{
	auto layers = make_unique<RetinaLayers>();
	setOcularLayers(std::move(layers));
}


SegmScan::DiscBsegm::~DiscBsegm() = default;
SegmScan::DiscBsegm::DiscBsegm(DiscBsegm && rhs) = default;
DiscBsegm & SegmScan::DiscBsegm::operator=(DiscBsegm && rhs) = default;


bool SegmScan::DiscBsegm::performAnalysis(bool meye)
{
	if (source()->isEmpty()) {
		return false;
	}

	float rangeX = getPatternDescript().getScanLength();
	getImpl().rangeX = rangeX;

	bool isDisc = getPatternDescript().isDiscScan();
	bool isAngio = getPatternDescript().isAngioScan();
	isDisc = (rangeX > 9.0f ? true : isDisc);

	bool useSemtVersion = false;
	useSemtVersion = SemtRetina::RetinaSegmentModel::isInitialized();

	if (useSemtVersion) {
		auto d_ptr = source()->getBitsData();
		auto src_w = source()->getWidth();
		auto src_h = source()->getHeight();
		auto index = getImageIndex();

		auto& frame = getImpl().segmFrame;
		frame.setBscanImage(d_ptr, src_w, src_h, rangeX, index);
		if (!frame.segmentLayers(isAngio)) {
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
		layerBASE()->initialize(frame.boundaryBRM(), src_w, src_h);

		auto layers = getRetinaLayers();
		layers->setRegionSize(src_w, src_h);
		
		if (frame.isONHDiscRegionDetected()) {
			getImpl().disc_x1 = frame.onhDiscMarginMinX();
			getImpl().disc_x2 = frame.onhDiscMarginMaxX();
			getImpl().disc_pixels = frame.onhDiscRimVoxelCount();
			getImpl().isOpticDisc = true;
		}

		if (frame.isONHCupRegionDetected()) {
			getImpl().cup_x1 = frame.onhCupOpeningMinX();
			getImpl().cup_x2 = frame.onhCupOpeningMaxX();
			getImpl().cup_pixels = frame.onhDiscCupVoxelCount();
			getImpl().isOpticCup = true;
		}
	}
	else
	{
		prepareSample(source(), sample());
		if (!Preprocess::checkIfValidScanImage(sample(), getImageIndex())) {
			goto failed;
		}

		createGradients(sample(), gradients(), ascends(), descends());
		if (!makeupRetinaBorders(isDisc, rangeX)) {
			goto failed;
		}

		buildLayerILM(sample(), layerInn(), layerOut(), layerILM());
		buildLayerOut(ascends(), layerILM(), layerOut());
		buildLayerOPR(ascends(), layerILM(), layerOut(), layerOPR());

		buildLayerIOS(ascends(), layerILM(), layerOut(), layerOPR(), layerIOS());
		buildLayerBRM(sample(), layerIOS(), layerOPR(), layerBRM());
		buildLayerRPE(sample(), layerIOS(), layerOPR(), layerBRM(), layerRPE());

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

			vector<int> baseLine;
			for (auto iter : layerRPE()->getYs()) {
				baseLine.push_back(iter + 45);
			}
			layerBASE()->initialize(baseLine, source()->getWidth(), source()->getHeight());

			elaborateParams(layerILM(), layerRPE());

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


bool SegmScan::DiscBsegm::elaborateParams(const OcularLayer* layerILM, const OcularLayer* layerRPE)
{
	if (isOpticNerveDisc()) {
		assignOpticDiscRegion(layerILM, layerRPE);
		assignOpticCupRegion(layerILM, layerRPE);
	}
	return true;
}


bool SegmScan::DiscBsegm::makeupRetinaBorders(bool isDisc, float rangeX)
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


bool SegmScan::DiscBsegm::isOpticNerveDisc(void) const
{
	return d_ptr->isOpticDisc;
}


bool SegmScan::DiscBsegm::getOpticNerveDiscRange(int & x1, int & x2) const
{
	if (!isOpticNerveDisc()) {
		x1 = x2 = -1;
		return false;
	}
	x1 = d_ptr->disc_x1;
	x2 = d_ptr->disc_x2;
	return true;
}


bool SegmScan::DiscBsegm::getOpticNerveDiscPixels(int & pixels) const
{
	if (!isOpticNerveDisc()) {
		pixels = 0;
		return false;
	}
	pixels = d_ptr->disc_pixels;
	return true;
}


void SegmScan::DiscBsegm::setOpticNerveDiscRange(int x1, int x2)
{
	d_ptr->disc_x1 = x1;
	d_ptr->disc_x2 = x2;
	d_ptr->isOpticDisc = (x1 >= 0 && x2 > 0 && (x2 - x1) > 0);
	return;
}


void SegmScan::DiscBsegm::setOpticNerveDiscPixels(int pixels)
{
	d_ptr->disc_pixels = pixels;
	// d_ptr->isOpticDisc = (pixels > 0);
	return;
}


int SegmScan::DiscBsegm::getOpticNerveDiscX1(void)
{
	return d_ptr->disc_x1;
}


int SegmScan::DiscBsegm::getOpticNerveDiscX2(void)
{
	return d_ptr->disc_x2;
}


bool SegmScan::DiscBsegm::isOpticNerveCup(void) const
{
	return d_ptr->isOpticCup;
}


bool SegmScan::DiscBsegm::getOpticNerveCupRange(int & x1, int & x2) const
{
	if (!isOpticNerveCup()) {
		x1 = x2 = -1;
		return false;
	}
	x1 = d_ptr->cup_x1;
	x2 = d_ptr->cup_x2;
	return true;
}


bool SegmScan::DiscBsegm::getOpticNerveCupPixels(int & pixels) const
{
	if (!isOpticNerveCup()) {
		pixels = 0;
		return false;
	}
	pixels = d_ptr->cup_pixels;
	return true;
}


void SegmScan::DiscBsegm::setOpticNerveCupRange(int x1, int x2)
{
	d_ptr->cup_x1 = x1;
	d_ptr->cup_x2 = x2;
	d_ptr->isOpticCup = (x1 >= 0 && x2 > 0);
	return;
}


void SegmScan::DiscBsegm::setOpticNerveCupPixels(int pixels)
{
	d_ptr->cup_pixels = pixels;
	d_ptr->isOpticCup = (pixels > 0);
	return;
}


void SegmScan::DiscBsegm::clearOpticNerveRange(void)
{
	d_ptr->isOpticCup = false;
	d_ptr->isOpticDisc = false;
	return;
}


bool SegmScan::DiscBsegm::buildLayerILM(const OcularImage * imgSrc, OcularLayer * layerInn, OcularLayer * layerOut, OcularLayer * layerILM)
{
	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	layerInn->resize(w, h);
	layerOut->resize(w, h);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = true;

	if (!OptimalPath::designPathConstraintsOfILM(imgSrc, layerInn, layerOut, upper, lower, delta, d_ptr->reti_x1, d_ptr->reti_x2, isDisc, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::createPathCostMapOfILM(imgSrc, imgCost, upper, lower, delta, isDisc)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothCurveILM(path, w, h, layerILM, isDisc, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::buildLayerOut(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerOut)
{
	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	layerILM->resize(w, h);
	layerOut->resize(w, h);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	bool isDisc = true;
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


bool SegmScan::DiscBsegm::buildLayerOPR(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerOut, OcularLayer * layerOPR)
{
	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	if (!OptimalPath::designPathConstraintsOfOPR(imgSrc, layerILM, layerOut, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
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
	if (!Smooth::smoothCurveOPR(path, w, h, layerILM->getYs(), layerOPR, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::buildLayerIOS(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerOut, OcularLayer * layerOPR, OcularLayer * layerIOS)
{
	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	if (!OptimalPath::designPathConstraintsOfIOS(imgSrc, layerILM, layerOut, layerOPR, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
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
	if (!Smooth::smoothCurveIOS(path, w, h, layerILM->getYs(), layerOPR->getYs(), layerIOS, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::buildLayerBRM(const OcularImage * imgSrc, OcularLayer* layerIOS, OcularLayer * layerOPR, OcularLayer * layerBRM)
{
	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	if (!OptimalPath::designPathConstraintsOfBRM(imgSrc, layerIOS, layerOPR, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
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
	if (!Smooth::smoothCurveBRM(path, w, h, layerIOS->getYs(), layerBRM, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::buildLayerRPE(const OcularImage * imgSrc, OcularLayer * layerIOS, OcularLayer * layerOPR, OcularLayer * layerBRM, OcularLayer * layerRPE)
{
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
	if (!Smooth::smoothCurveRPE(path, w, h, layerIOS->getYs(), layerBRM->getYs(), layerRPE, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::buildLayerOPL(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerIOS, OcularLayer * layerOPL)
{
	vector<int> upper, lower, delta;
	vector<int> path;

	upper = layerILM->getYs();
	lower = layerIOS->getYs();
	path = vector<int>(upper.size(), -1);
	auto imgCost = costs();
	
	if (!OptimalPath::designPathConstraintsOfOPL(imgSrc, layerILM, layerIOS, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::createPathCostMapOfOPL(imgSrc, imgCost, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	if (!Smooth::smoothCurveOPL(path, w, h, layerILM->getYs(), layerIOS->getYs(), layerOPL, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::buildLayerIPL(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerOPL, OcularLayer * layerIPL)
{
	vector<int> upper, lower, delta;
	vector<int> path;

	upper = layerIOS()->getYs();
	lower = layerBRM()->getYs();
	path = vector<int>(upper.size(), -1);
	auto imgCost = costs();

	if (!OptimalPath::designPathConstraintsOfIPL(imgSrc, layerILM, layerOPL, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::createPathCostMapOfIPL(imgSrc, imgCost, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	if (!Smooth::smoothCurveIPL(path, w, h, layerILM->getYs(), layerOPL->getYs(), layerIPL, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::buildLayerNFL(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerIPL, OcularLayer * layerNFL)
{
	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	upper = layerIOS()->getYs();
	lower = layerBRM()->getYs();
	path = vector<int>(upper.size(), -1);

	float rangeX = getPatternDescript().getScanDistance();
	if (!OptimalPath::designPathConstraintsOfNFL4(imgSrc, layerILM, layerIPL, upper, lower, delta, rangeX, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::createPathCostMapOfNFL(imgSrc, imgCost, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	if (!Smooth::smoothCurveNFL(path, w, h, layerILM->getYs(), layerIPL->getYs(), layerNFL, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::elaborateILM(const OcularImage * imgSrc, OcularLayer * layerNFL, OcularLayer * layerILM)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerILM->resize(width, height);
	layerNFL->resize(width, height);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	if (!OptimalPath::designLayerContraintsOfILM(imgSrc, layerILM, layerNFL, 
												upper, lower, delta, 
												true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfILM(imgSrc, imgCost, upper, lower, delta, false)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerILM(path, width, height, layerNFL, layerILM, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::elaborateNFL(const OcularImage * imgSrc, OcularLayer * layerILM, OcularLayer * layerIPL, OcularLayer * layerOut, OcularLayer * layerNFL)
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

	if (!OptimalPath::designLayerContraintsOfNFL(imgSrc, layerILM, layerNFL, layerIPL, layerOut,
												upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfNFL(imgSrc, imgCost, upper, lower, delta, true)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}
	
	if (!Smooth::smoothLayerNFL(path, width, height, layerILM, layerIPL, layerNFL, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::elaborateRPE(const OcularImage * imgSrc, OcularLayer * layerIOS, OcularLayer * layerBRM, OcularLayer * layerOut, OcularLayer * layerRPE)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerIOS->resize(width, height);
	layerBRM->resize(width, height);
	layerRPE->resize(width, height);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	if (!OptimalPath::designLayerContraintsOfRPE(imgSrc, layerIOS, layerBRM, layerRPE, layerOut, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfRPE(imgSrc, imgCost, upper, lower, delta, false)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerRPE(path, width, height, layerIOS, layerBRM, layerRPE, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::elaborateIPL(const OcularImage * imgSrc, OcularLayer* layerILM, OcularLayer * layerNFL, OcularLayer * layerOPL, OcularLayer * layerOut, OcularLayer * layerIPL)
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

	if (!OptimalPath::designLayerContraintsOfIPL(imgSrc, layerNFL, layerIPL, layerOPL, layerOut, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfIPL(imgSrc, imgCost, upper, lower, delta, true)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerIPL(path, width, height, layerILM, layerOPL, layerIPL, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;

}

bool SegmScan::DiscBsegm::elaborateOPL(const OcularImage * imgSrc, OcularLayer * layerIPL, OcularLayer * layerRPE, OcularLayer * layerOut, OcularLayer * layerOPL)
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

	if (!OptimalPath::designLayerContraintsOfOPL(imgSrc, layerIPL, layerOPL, layerRPE, layerOut, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfOPL(imgSrc, imgCost, upper, lower, delta, true)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerOPL(path, width, height, layerIPL, layerRPE, layerOPL, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::elaborateIOS(const OcularImage * imgSrc, OcularLayer * layerOPL, OcularLayer * layerRPE, OcularLayer * layerBRM, OcularLayer * layerIOS)
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

	if (!OptimalPath::designLayerContraintsOfIOS(imgSrc, layerOPL, layerIOS, layerRPE, layerBRM, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfIOS(imgSrc, imgCost, upper, lower, delta, false)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerIOS(path, width, height, layerOPL, layerRPE, layerIOS, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::elaborateBRM(const OcularImage * imgSrc, OcularLayer * layerIOS, OcularLayer * layerRPE, OcularLayer * layerBRM)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerRPE->resize(width, height);
	layerBRM->resize(width, height);

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	if (!OptimalPath::designLayerContraintsOfBRM(imgSrc, layerIOS, layerRPE, layerBRM, upper, lower, delta, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}

	if (!OptimalPath::createLayerCostMapOfBRM(imgSrc, imgCost, upper, lower, delta, false)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth::smoothLayerBRM(path, width, height, layerIOS, layerBRM, true, d_ptr->disc_x1, d_ptr->disc_x2)) {
		return false;
	}
	return true;
}


bool SegmScan::DiscBsegm::estimateOpticDiscRegion(const OcularImage* imgSrc, const OcularImage* imgAsc, OcularLayer * layerInn, OcularLayer * layerOut)
{
	vector<int> inner = layerInn->getYs();
	vector<int> outer = layerOut->getYs();
	vector<int> ideal;

	/*
	if (!DataFitt::buildIdealBoundrayOfIOS(outer, ideal)) {
		return false;
	}
	*/
	Coarse::interpolateBoundaryByLinearFitting(outer, ideal, true);

	float rangeX = getPatternDescript().getScanRangeX();

	int disc1, disc2;
	if (Feature::estimateOpticDiskMargin(imgSrc, imgAsc, inner, outer, ideal, disc1, disc2, rangeX)) {
		d_ptr->isOpticDisc = true;
		d_ptr->disc_x1 = disc1;
		d_ptr->disc_x2 = disc2;

		LogD() << "Detect optic disc, range: " << disc1 << ", " << disc2 << ", size: " << (disc2 - disc1 + 1) << ", image: " << getImageIndex();
	}
	else {
		d_ptr->isOpticDisc = false;
		d_ptr->disc_x1 = -1;
		d_ptr->disc_x2 = -1;
	}

	Coarse::interpolateBoundaryByLinearFitting(inner, inner, true);
	Coarse::interpolateBoundaryByLinearFitting(outer, outer, true);
	Coarse::smoothBoundaryLine(outer, outer, 0.05f);

	layerInn->initialize(inner, layerInn->getRegionWidth(), layerInn->getRegionHeight());
	layerOut->initialize(outer, layerOut->getRegionWidth(), layerOut->getRegionHeight());
	return true;
}


bool SegmScan::DiscBsegm::inflateOpticDiscRegion(void)
{
	if (isOpticNerveDisc()) {
		float ratio = 1.0f / getSampleWidthRatio();
		d_ptr->disc_x1 = (int)(d_ptr->disc_x1 * ratio);
		d_ptr->disc_x2 = (int)(d_ptr->disc_x2 * ratio);
	}
	return true;
}


bool SegmScan::DiscBsegm::assignOpticDiscRegion(const OcularLayer * layerILM, const OcularLayer * layerRPE)
{
	if (!d_ptr->isOpticDisc) {
		return true;
	}

	vector<int> inner = layerILM->getYs();
	vector<int> outer = layerRPE->getYs();

	int disc1, disc2, pixels;
	disc1 = d_ptr->disc_x1;
	disc2 = d_ptr->disc_x2;
	if (Feature::calculateDiscSidePixels(inner, outer, disc1, disc2, pixels)) {
		d_ptr->disc_x1 = disc1;
		d_ptr->disc_x2 = disc2;
		d_ptr->disc_pixels = pixels;

		LogD() << "Optic disc, range: " << disc1 << ", " << disc2 << ", size: " << (disc2 - disc1 + 1) << ", pixels: " << pixels << ", index: " << getImageIndex();
	}
	else {
		d_ptr->isOpticDisc = false;
	}
	return true;
}


bool SegmScan::DiscBsegm::assignOpticCupRegion(const OcularLayer * layerILM, const OcularLayer * layerRPE)
{
	if (!d_ptr->isOpticDisc) {
		return true;
	}

	vector<int> inner = layerILM->getYs();
	vector<int> outer = layerRPE->getYs();

	int cup1, cup2, pixels;
	if (Feature::calculateCupDepthPixels(inner, outer, d_ptr->disc_x1, d_ptr->disc_x2, cup1, cup2, pixels)) {
		d_ptr->isOpticCup = true;
		d_ptr->cup_x1 = cup1;
		d_ptr->cup_x2 = cup2;
		d_ptr->cup_pixels = pixels;

		LogD() << "Optic cup, range: " << cup1 << ", " << cup2 << ", size: " << (cup2 - cup1 + 1) << ", pixels: " << pixels << ", index: " << getImageIndex();
	}
	else {
		d_ptr->isOpticCup = false;
	}
	return true;
}


bool SegmScan::DiscBsegm::assignOpticRimRegion(const OcularLayer * layerILM, const OcularLayer * layerRPE)
{
	return false;
}


DiscBsegm::DiscBsegmImpl & SegmScan::DiscBsegm::getImpl(void) const
{
	return *d_ptr;
}


