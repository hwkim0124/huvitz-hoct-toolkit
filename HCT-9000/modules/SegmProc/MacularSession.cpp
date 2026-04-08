#include "stdafx.h"
#include "MacularSession.h"

#include "CppUtil2.h"
#include "OctData2.h"
#include "SegmScan2.h"
#include "SemtRetina2.h"

using namespace SegmProc;
using namespace SegmScan;
using namespace OctData;
using namespace CppUtil;
using namespace std;


struct MacularSession::MacularSessionImpl
{
	MacularSessionImpl() {

	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<MacularSession::MacularSessionImpl> MacularSession::d_ptr(new MacularSessionImpl());


MacularSession::MacularSession()
{
}


MacularSession::~MacularSession()
{
}


bool SegmProc::MacularSession::initialize(void)
{
	int overlaps = getScanData()->getDescript().getScanOverlaps();

	clearOcularBsegms();
	for (int i = 0; i < SegmSession::getScanData()->countBscanData(); i++) {
		auto bscan = getScanData()->getBscanData(i);
		if (bscan == nullptr || bscan->isEmpty()) {
			LogW() << i << "th bscan data is empty!";
			return false;
		}

		// LogD() << i << " : " << bscan->getImageBuffer() << ", " << bscan->getImageWidth() << ", " << bscan->getImageHeight();
		auto bsegm = make_unique<MacularBsegm>();
		// if (!(i % overlaps)) {
			bsegm->loadSource(bscan->getImageBuffer(), bscan->getImageWidth(), bscan->getImageHeight());
		// }
		bsegm->setPatternDescript(getScanData()->getDescript());
		addOcularBsegm(std::move(bsegm));
	}

	LogD() << "MacularSession measure bscans: " << getScanData()->countBscanData();

	clearPreviewBsegms();
	for (int i = 0; i < SegmSession::getScanData()->countPreviewData(); i++) {
		auto bscan = getScanData()->getPreviewData(i);
		if (bscan == nullptr || bscan->isEmpty()) {
			LogW() << i << "th preview data is empty!";
			return false;
		}

		auto bsegm = make_unique<MacularBsegm>();
		bsegm->loadSource(bscan->getImageBuffer(), bscan->getImageWidth(), bscan->getImageHeight());
		bsegm->setPatternDescript(getScanData()->getDescript());
		addPreviewBsegm(std::move(bsegm));
	}
	LogD() << "MacularSession preview bscans: " << getScanData()->countPreviewData();


	try {
		if (!SemtRetina::RetinaSegmentModel::isInitialized()) {
			SemtRetina::RetinaSegmentModel::initializeNetwork();
		}
	} 
	catch (exception e) {
	}

	return true;
}


MacularSession::MacularSessionImpl & SegmProc::MacularSession::getImpl(void)
{
	return *d_ptr;
}
