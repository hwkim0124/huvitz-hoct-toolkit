#include "stdafx.h"
#include "SegmSession.h"

#include "CppUtil2.h"
#include "OctData2.h"
#include "SemtSegm2.h"
#include "SegmScan2.h"

using namespace SegmProc;
using namespace SemtSegm;
using namespace SegmScan;
using namespace OctData;
using namespace CppUtil;
using namespace std;


struct SegmSession::SegmSessionImpl
{
	ProtocolData* data;
	vector<unique_ptr<OcularBsegm>> bsegms;
	vector<unique_ptr<OcularBsegm>> previews;

	SegmSessionImpl() : data(nullptr) {

	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<SegmSession::SegmSessionImpl> SegmSession::d_ptr(new SegmSessionImpl());


SegmSession::SegmSession()
{
}


SegmSession::~SegmSession()
{
}


void SegmProc::SegmSession::loadScanData(OctData::ProtocolData * data)
{
	getImpl().data = data;
	return;
}


bool SegmProc::SegmSession::processScanData(void)
{
	if (!initialize()) {
		return false;
	}

	if (!execute()) {
		return false;
	}

	if (!rectify()) {
		return false;
	}

	if (!finalize()) {
		return false;
	}

	return true;
}


bool SegmProc::SegmSession::initialize(void)
{
	int overlaps = getScanData()->getDescript().getScanOverlaps();

	clearOcularBsegms();
	for (int i = 0; i < getScanData()->countBscanData(); i++) {
		auto bscan = getScanData()->getBscanData(i);
		if (bscan == nullptr || bscan->isEmpty()) {
			LogW() << i << "th bscan data is empty!";
			return false;
		}

		auto bsegm = std::make_unique<OcularBsegm>();
		// if (!(i % overlaps)) {
			bsegm->loadSource(bscan->getImageBuffer(), bscan->getImageWidth(), bscan->getImageHeight());
			bsegm->setPatternDescript(getScanData()->getDescript());
		// }
		addOcularBsegm(std::move(bsegm));
	}
	LogD() << "SegmSession measure bscans: " << getScanData()->countBscanData();

	clearPreviewBsegms();
	for (int i = 0; i < getScanData()->countPreviewData(); i++) {
		auto bscan = getScanData()->getPreviewData(i);
		if (bscan == nullptr || bscan->isEmpty()) {
			LogW() << i << "th preview data is empty!";
			return false;
		}

		auto bsegm = std::make_unique<OcularBsegm>();
		bsegm->loadSource(bscan->getImageBuffer(), bscan->getImageWidth(), bscan->getImageHeight());
		bsegm->setPatternDescript(getScanData()->getDescript());
		addPreviewBsegm(std::move(bsegm));
	}
	LogD() << "SegmSession preview bscans: " << getScanData()->countPreviewData();
	return true;
}


bool SegmProc::SegmSession::execute(void)
{
	auto n_workers = std::thread::hardware_concurrency();
	// n_workers = 1;

	std::vector<std::vector<OcularBsegm*>> tasks(n_workers);
	int size = countOcularBsegms();

	for (int i = 0; i < size; i++) {
		auto bsegm = d_ptr->bsegms[i].get();

		LogD() << "Perform analysis, pattern image: " << i << " / " << size;
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
					bool result = p->performAnalysis();
					// LogD() << "Result: " << result;
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
		auto bsegm = d_ptr->previews[i].get();

		LogD() << "Perform analysis, preview image: " << i << " / " << size;
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
					bool result = p->performAnalysis();
					// LogD() << "Result: " << result;
				}
			}));
		}
	}

	std::for_each(workers2.begin(), workers2.end(), [](std::thread& t)
	{
		t.join();
	});


	/*
	int size = countOcularBsegms();
	for (int i = 0; i < size; i++) {
		auto bsegm = d_ptr->bsegms[i].get();

		LogD() << "Perform analysis, image: " << i << " / " << size;
		LogD() << "name: " << wtoa(getScanData()->getBscanData(i)->getImageName());
		bool result = bsegm->performAnalysis();
		LogD() << "Result: " << result;
	}
	*/
	return true;
}


bool SegmProc::SegmSession::rectify(void)
{
	return true;
}


bool SegmProc::SegmSession::finalize(void)
{
	for (int i = 0; i < getScanData()->countBscanData(); i++) {
		auto bscan = getScanData()->getBscanData(i);
		bscan->setOcularBsegm(std::move(d_ptr->bsegms[i]));
	}

	for (int i = 0; i < getScanData()->countPreviewData(); i++) {
		auto bscan = getScanData()->getPreviewData(i);
		bscan->setOcularBsegm(std::move(d_ptr->previews[i]));
	}

	// auto layer = getScanData()->getPreviewData(0)->getLayerPoints(OcularLayerType::ILM, 512, 512);

	return true;
}


ProtocolData * SegmProc::SegmSession::getScanData(void) const
{
	return d_ptr->data;
}


int SegmProc::SegmSession::countOcularBsegms(void) const
{
	return (int)d_ptr->bsegms.size();
}


void SegmProc::SegmSession::clearOcularBsegms(void)
{
	d_ptr->bsegms.clear();
	return;
}


void SegmProc::SegmSession::addOcularBsegm(std::unique_ptr<OcularBsegm> bsegm)
{
	d_ptr->bsegms.push_back(std::move(bsegm));
	return;
}


SegmScan::OcularBsegm * SegmProc::SegmSession::getOcularBsegm(int index) const
{
	if (index >= 0 && index < countOcularBsegms()) {
		return d_ptr->bsegms[index].get();
	}
	return nullptr;
}

int SegmProc::SegmSession::countPreviewBsegms(void) const
{
	return (int)d_ptr->previews.size();
}

void SegmProc::SegmSession::clearPreviewBsegms(void)
{
	d_ptr->previews.clear();
	return;
}

void SegmProc::SegmSession::addPreviewBsegm(std::unique_ptr<SegmScan::OcularBsegm> bsegm)
{
	d_ptr->previews.push_back(std::move(bsegm));
	return;
}

SegmScan::OcularBsegm * SegmProc::SegmSession::getPreviewBsegm(int index) const
{
	if (index >= 0 && index < countPreviewBsegms()) {
		return d_ptr->previews[index].get();
	}
	return nullptr;
}


SegmSession::SegmSessionImpl & SegmProc::SegmSession::getImpl(void)
{
	return *d_ptr;
}
