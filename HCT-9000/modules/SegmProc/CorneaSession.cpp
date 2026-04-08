#include "stdafx.h"
#include "CorneaSession.h"

#include "CppUtil2.h"
#include "OctData2.h"
#include "SegmScan2.h"

using namespace SegmProc;
using namespace SegmScan;
using namespace OctData;
using namespace CppUtil;
using namespace std;


struct CorneaSession::CorneaSessionImpl
{
	bool isDewarping = false;

	CorneaSessionImpl() {

	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<CorneaSession::CorneaSessionImpl> CorneaSession::d_ptr(new CorneaSessionImpl());


CorneaSession::CorneaSession()
{
}


CorneaSession::~CorneaSession()
{
}


bool SegmProc::CorneaSession::processDewarping(void)
{
	getImpl().isDewarping = true;

	if (!initialize()) {
		return false;
	}

	if (!executeDewarping()) {
		return false;
	}

	if (!finalize()) {
		return false;
	}

	if (!employDewarping()) {
		return false;
	}
	return true;
}


bool SegmProc::CorneaSession::employDewarping(void)
{
	int size = countOcularBsegms();
	for (int i = 0; i < size; i++) {
		auto data = getScanData()->getBscanData(i);
		data->employBsegmImage();
	}

	size = countPreviewBsegms();
	for (int i = 0; i < size; i++) {
		auto data = getScanData()->getPreviewData(i);
		data->employBsegmImage();
	}

	return true;
}



bool SegmProc::CorneaSession::initialize(void)
{
	clearOcularBsegms();
	for (int i = 0; i < SegmSession::getScanData()->countBscanData(); i++) {
		auto bscan = getScanData()->getBscanData(i);
		if (bscan == nullptr || bscan->isEmpty()) {
			LogW() << i << "th bscan data is empty!";
			return false;
		}

		auto bsegm = make_unique<CorneaBsegm>();
		bsegm->loadSource(bscan->getImageBuffer(), bscan->getImageWidth(), bscan->getImageHeight());
		bsegm->setPatternDescript(getScanData()->getDescript());
		addOcularBsegm(std::move(bsegm));
	}
	LogD() << "CorneaSession measure bscans: " << getScanData()->countBscanData();

	clearPreviewBsegms();
	for (int i = 0; i < SegmSession::getScanData()->countPreviewData(); i++) {
		auto bscan = getScanData()->getPreviewData(i);
		if (bscan == nullptr || bscan->isEmpty()) {
			LogW() << i << "th preview data is empty!";
			return false;
		}

		auto bsegm = make_unique<CorneaBsegm>();
		bsegm->loadSource(bscan->getImageBuffer(), bscan->getImageWidth(), bscan->getImageHeight());
		bsegm->setPatternDescript(getScanData()->getDescript());
		addPreviewBsegm(std::move(bsegm));
	}
	LogD() << "CorneaSession preview bscans: " << getScanData()->countPreviewData();
	return true;
}


bool SegmProc::CorneaSession::executeDewarping(void)
{
	auto n_workers = std::thread::hardware_concurrency();

	std::vector<std::vector<OcularBsegm*>> tasks(n_workers);
	int size = countOcularBsegms();

	for (int i = 0; i < size; i++) {
		auto bsegm = getOcularBsegm(i);
		LogD() << "Perform dewarping, pattern image: " << i << " / " << size;
		LogD() << "name: " << wtoa(getScanData()->getBscanData(i)->getImageName());

		if (bsegm != nullptr) {
			bsegm->setImageIndex(i);
			int k = i % n_workers;
			tasks[k].push_back(bsegm);
		}
	}

	std::vector<std::thread> workers;

	for (unsigned int k = 0; k < n_workers; k++) {
		if (tasks[k].size() > 0) {
			workers.push_back(std::thread([tasks, k]() {
				for (auto p : tasks[k]) {
					bool result = ((CorneaBsegm*)p)->performDewarping();
				}
			}));
		}
	}

	std::for_each(workers.begin(), workers.end(), [](std::thread& t)
	{
		t.join();
	});


	std::vector<std::vector<OcularBsegm*>> tasks2(n_workers);
	size = countPreviewBsegms();

	for (int i = 0; i < size; i++) {
		auto bsegm = getPreviewBsegm(i);

		LogD() << "Perform dewarping, preview image: " << i << " / " << size;
		LogD() << "name: " << wtoa(getScanData()->getPreviewData(i)->getImageName());

		if (bsegm != nullptr) {
			int k = i % n_workers;
			tasks2[k].push_back(bsegm);
		}
	}

	std::vector<std::thread> workers2;

	for (unsigned int k = 0; k < n_workers; k++) {
		if (tasks2[k].size() > 0) {
			workers2.push_back(std::thread([tasks2, k]() {
				for (auto p : tasks2[k]) {
					bool result = ((CorneaBsegm*)p)->performDewarping();
				}
			}));
		}
	}

	std::for_each(workers2.begin(), workers2.end(), [](std::thread& t)
	{
		t.join();
	});

	return true;
}


CorneaSession::CorneaSessionImpl & SegmProc::CorneaSession::getImpl(void)
{
	return *d_ptr;
}
