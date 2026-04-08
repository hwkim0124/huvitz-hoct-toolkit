#include "stdafx.h"
#include "Measure.h"
#include "RetinaAlign.h"

#include "OctResult2.h"
#include "SegmScan2.h"
#include "CppUtil2.h"

using namespace OctSystem;
using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct Measure::MeasureImpl
{
	PatientData patientData;
	vector<shared_ptr<ScannerResult>> scanners; 
	vector<shared_ptr<FundusResult>> cameras;

	std::wstring exportPath;
	std::wstring cameraPath;
	std::wstring bufferPath;

	MeasureImpl()
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<Measure::MeasureImpl> Measure::d_ptr(new MeasureImpl());


Measure::Measure()
{
}


Measure::~Measure()
{
}


bool OctSystem::Measure::isScannerResult(EyeSide side)
{
	if (getCountOfScannerResults(side) > 0) {
		return true;
	}
	return false;
}


bool OctSystem::Measure::isFundusResult(EyeSide side)
{
	if (getCountOfFundusResults(side) > 0) {
		return true;
	}
	return false;
}


int OctSystem::Measure::getCountOfScannerResults(EyeSide side)
{
	int count = 0;
	
	if (side == EyeSide::OD || side == EyeSide::OS) {
		for (auto iter = cbegin(d_ptr->scanners); iter != cend(d_ptr->scanners); ++iter) {
			auto result = iter->get();
			if (!result && result->getEyeSide() == side) {
				count++;
			}
		}
	}
	else {
		count = (int)d_ptr->scanners.size();
	}
	return count;
}


int OctSystem::Measure::getCountOfFundusResults(EyeSide side)
{
	int count = 0;

	if (side == EyeSide::OD || side == EyeSide::OS) {
		for (auto iter = cbegin(d_ptr->cameras); iter != cend(d_ptr->cameras); ++iter) {
			auto result = iter->get();
			if (!result && result->getEyeSide() == side) {
				count++;
			}
		}
	}
	else {
		count = (int)d_ptr->cameras.size();
	}
	return count;
}


int OctSystem::Measure::getCountOfPreviewSections(void)
{
	int count = 0;
	ScannerResult* result = getScannerResultLast();
	if (result) {
		count = result->getPreviewOutput().getSectionCount();
	}
	return count;
}


int OctSystem::Measure::getCountOfPatternSections(void)
{
	int count = 0;
	ScannerResult* result = getScannerResultLast();
	if (result) {
		count = result->getPatternOutput().getSectionCount();
	}
	return count;
}


float OctSystem::Measure::getPatternQualityIndex(void)
{
	ScannerResult* result = getScannerResultLast();
	if (result) {
		return result->getPatternOutput().getAverageOfQualityIndex();
	}
	return 0.0f;
}


void OctSystem::Measure::getPatternQualityStat(std::vector<float>& stat, float sectSize)
{
	stat.empty();

	ScannerResult* result = getScannerResultLast();
	if (!result) {
		return;
	}

	auto& pattern = result->getPatternOutput();
	auto& desc = pattern.getDescript();
	if (!desc.isCubeScan()) {
		return;
	}
	
	stat.resize(3);

	int lines = desc.getNumberOfScanLines();
	int dsize = min(max((int)(lines * sectSize), 3), lines / 3);

	float tacc = getPatternQualityIndex();
	float mean[3] = { 0.0f };
	float sum = 0.0f;
	int count = 0;

	for (int i = 0; i < dsize; i++) {
		sum += pattern.getSectionImage(i)->getQualityIndex();
		count++;
	}
	stat[0] = sum / count;

	sum = 0.0f; count = 0;
	for (int i = ((lines - dsize)/2); i < ((lines + dsize)/2); i++) {
		sum += pattern.getSectionImage(i)->getQualityIndex();
		count++;
	}
	stat[1] = sum / count;

	sum = 0.0f; count = 0;
	for (int i = (lines - dsize); i < lines; i++) {
		sum += pattern.getSectionImage(i)->getQualityIndex();
		count++;
	}
	stat[2] = sum / count;
	return;
}


const OctScanImage* OctSystem::Measure::getPreviewImageDescript(int sectIdx, int imageIdx)
{
	ScannerResult* result = getScannerResultLast();
	if (result == nullptr) {
		return nullptr;
	}

	return result->getPreviewOutput().getSectionImageDescript(sectIdx, imageIdx);
}


const OctScanImage* OctSystem::Measure::getPatternImageDescript(int sectIdx, int imageIdx)
{
	ScannerResult* result = getScannerResultLast();
	if (result == nullptr) {
		return false;
	}

	return result->getPatternOutput().getSectionImageDescript(sectIdx, imageIdx);
}


const OctEnfaceImage * OctSystem::Measure::getEnfaceImageDescript(void)
{
	ScannerResult* result = getScannerResultLast();
	if (result == nullptr) {
		return nullptr;
	}

	return result->getEnfaceOutput().getEnfaceImageDescript();
}


const OctRetinaImage * OctSystem::Measure::getRetinaImageDescriptOfScannerResult(void)
{
	ScannerResult* result = getScannerResultLast();
	if (result == nullptr) {
		return nullptr;
	}

	return result->getRetinaImageDescript();
}


const OctCorneaImage * OctSystem::Measure::getCorneaImageDescriptOfScannerResult(void)
{
	ScannerResult* result = getScannerResultLast();
	if (result == nullptr) {
		return nullptr;
	}

	return result->getCorneaImageDescript();
}


int OctSystem::Measure::getCountOfFundusSections(void)
{
	int count = 0;
	FundusResult* result = getFundusResultLast();
	if (result) {
		count = result->getFundusOutput().getSectionCount();
	}
	return count;
}


const OctFundusImage * OctSystem::Measure::getFundusImageDescript(int sectIdx)
{
	FundusResult* result = getFundusResultLast();
	if (result == nullptr) {
		return nullptr;
	}

	return result->getFundusOutput().getSectionImageDescript(sectIdx, 0);
}


const OctRetinaImage * OctSystem::Measure::getRetinaImageDescriptOfFundusResult(void)
{
	FundusResult* result = getFundusResultLast();
	if (result == nullptr) {
		return nullptr;
	}

	return result->getRetinaImageDescript();
}


const OctCorneaImage * OctSystem::Measure::getCorneaImageDescriptOfFundusResult(void)
{
	FundusResult* result = getFundusResultLast();
	if (result == nullptr) {
		return nullptr;
	}

	return result->getCorneaImageDescript();
}


OctResult::FundusImage * OctSystem::Measure::getFundusImageObject(int sectIdx)
{
	FundusResult* result = getFundusResultLast();
	if (result == nullptr) {
		return nullptr;
	}

	return result->getFundusOutput().getSectionImage(sectIdx, 0);
}


OctResult::FundusFrame * OctSystem::Measure::getFundusFrameObject(int sectIdx)
{
	FundusResult* result = getFundusResultLast();
	if (result == nullptr) {
		return nullptr;
	}

	return result->getFundusOutput().getSectionFrame(sectIdx, 0);
}


void OctSystem::Measure::setPatient(const OctPatient& patient)
{
	getImpl().patientData.setPatient(patient);
	return;
}


OctPatient & OctSystem::Measure::getPatient(void)
{
	return getImpl().patientData.getPatient();
}


int OctSystem::Measure::exportScannerResults(std::wstring dirPath, bool subPath, 
						std::wstring imagePrefix, std::wstring enfaceName,
						std::wstring previewName, std::wstring retinaName, std::wstring corneaName)
{
	if (dirPath.empty()) {
		if (!createDefaultExportDirectory()) {
			return 0;
		}
	}
	else {
		d_ptr->exportPath = dirPath;
	}

	int count = 0;
	for (auto iter = begin(d_ptr->scanners); iter != end(d_ptr->scanners); ++iter) {
		auto result = iter->get();
		if (!result) {
			continue;
		}
		if (result->exportFiles(d_ptr->exportPath, subPath, imagePrefix, enfaceName, previewName, retinaName, corneaName)) {
			count++;
		}
	}
	return count;
}


int OctSystem::Measure::exportFundusResults(unsigned int imgQuality,
							std::wstring dirPath, bool subPath, bool removeReflectionLight,
							std::wstring imageName, std::wstring thumbName,
							std::wstring retinaName, std::wstring corneaName)
{
	if (dirPath.empty()) {
		if (!createDefaultExportDirectory()) {
			return 0;
		}
	}
	else {
		d_ptr->exportPath = dirPath;
	}

	int count = 0;
	for (auto iter = begin(d_ptr->cameras); iter != end(d_ptr->cameras); ++iter) {
		auto result = iter->get();
		if (!result) {
			continue;
		}
		if (result->exportFiles(imgQuality, d_ptr->exportPath, subPath, removeReflectionLight, imageName, thumbName,
			retinaName, corneaName)) {
			count++;
		}
	}
	return count;
}


void OctSystem::Measure::initialize(void)
{
	return;
}


bool OctSystem::Measure::initiateScannerResult(const OctScanMeasure& desc, bool clear)
{
	if (clear) {
		clearScannerResults();
	}

	auto result = make_unique<ScannerResult>(desc);
	d_ptr->scanners.push_back(std::move(result));
	// auto result2 = make_unique<ScannerResult>(desc);
	// d_ptr->scanners.push_back(std::move(result2));
	return true;
}


bool OctSystem::Measure::initiateFundusResult(const OctFundusMeasure & desc, bool clear)
{
	if (clear) {
		clearFundusResults();
	}

	auto result = make_unique<FundusResult>(desc);
	d_ptr->cameras.push_back(std::move(result));
	return true;
}


bool OctSystem::Measure::completeScannerResult(void)
{
	ScannerResult* result = getScannerResultLast();
	if (result == nullptr) {
		return false;
	}

	double msec;
	CppUtil::ClockTimer::start();
	// LogD() << "Preview images averaging";
	result->getPreviewOutput().applyAveraging();
	msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Preview images averaged, elapsed: " << msec;

	if (result->getDescript().getPattern().getScanOverlaps() > 1) {
		if (true) { // !result->getDescript().getPattern().isAngioScan()) {
			CppUtil::ClockTimer::start();
			result->getPatternOutput().applyAveraging();
			msec = CppUtil::ClockTimer::elapsedMsec();
			LogD() << "Pattern images averaged, elapsed: " << msec;
		}
	}
	return true;
}


bool OctSystem::Measure::completeFundusResult(void)
{
	return true;
}


void OctSystem::Measure::clearScannerResults(void)
{
	d_ptr->scanners.clear();
	return;
}


void OctSystem::Measure::clearFundusResults(void)
{
	d_ptr->cameras.clear();
	return;
}


std::shared_ptr<OctResult::ScannerResult> OctSystem::Measure::fetchScannerResult(int index)
{
	/*
	if (getScannerResultLast() != nullptr) {
		auto result = std::move(d_ptr->scanners.back());
		d_ptr->scanners.pop_back();
		return std::move(result);
	}
	*/

	if (index >= 0 && index < d_ptr->scanners.size()) {
		return d_ptr->scanners[index];
	}
	return nullptr;
}


std::shared_ptr<OctResult::FundusResult> OctSystem::Measure::fetchFundusResult(int index)
{
	/*
	if (getFundusResultLast() != nullptr) {
		auto result = std::move(d_ptr->cameras.back());
		d_ptr->cameras.pop_back();
		return std::move(result);
	}
	*/
	
	if (index >= 0 && index < d_ptr->cameras.size()) {
		return d_ptr->cameras[index];
	}
	return nullptr;
}


OctResult::ScannerResult * OctSystem::Measure::getScannerResultLast(void)
{
	ScannerResult* p = nullptr;
	if (!d_ptr->scanners.empty()) {
		p = d_ptr->scanners.back().get();
	}
	return p;
}


OctResult::ScannerResult * OctSystem::Measure::getScannerResultFirst(void)
{
	ScannerResult* p = nullptr;
	if (!d_ptr->scanners.empty()) {
		p = d_ptr->scanners.front().get();
	}
	return p;
}


OctResult::FundusResult * OctSystem::Measure::getFundusResultByIndex(int index)
{
	FundusResult* p = nullptr;
	if (index >= 0 && index < d_ptr->cameras.size()) {
		p = d_ptr->cameras[index].get();
	}
	return p;
}


OctResult::FundusResult * OctSystem::Measure::getFundusResultLast(void)
{
	FundusResult* p = nullptr;
	if (!d_ptr->cameras.empty()) {
		p = d_ptr->cameras.back().get();
	}
	return p;
}


ScannerResult * OctSystem::Measure::getScannerResultByIndex(int index)
{
	ScannerResult* p = nullptr;
	if (index >= 0 && index < d_ptr->scanners.size()) {
		p = d_ptr->scanners[index].get();
	}
	return p;
}


bool OctSystem::Measure::assignPreviewImage(const OctScanSection& section, const OctScanImage& image)
{
	ScannerResult* result = getScannerResultLast();
	if (result == nullptr) {
		return false;
	}

	// return result->getPreviewOutput().addSectionImage(section, image, PATTERN_PREVIEW_HISTORY_SIZE);
	return result->getPreviewOutput().addSectionImage(section, image);
}


bool OctSystem::Measure::assignPatternImage(const OctScanSection& section, const OctScanImage& image, int idxOverlap, bool segment)
{
	ScannerResult* result = getScannerResultLast();
	if (result == nullptr) {
		return false;
	}

	/*
	try {
		CvImage img;
		img.fromBitsDataNoCopy(image.getData(), image.getWidth(), image.getHeight());
		img.equalizeHistogram(2.0);
	}
	catch (...) {
	}
	*/

	if (segment) {
		SegmScan::MacularBsegm segm;
		segm.loadSource(image.getData(), image.getWidth(), image.getHeight());
		segm.setPatternDescript(result->getDescript().getPattern());
		segm.performAnalysis();
		LogD() << "Segment image index: " << section.getIndex() << ", result: " << segm.isResult();
	}

	auto p = result->getPatternOutput().getSection(section.getIndex());
	if (p) {
		if (!idxOverlap) {
			p->clearAllImages();
		}
	}

	return result->getPatternOutput().addSectionImage(section, image);
}


bool OctSystem::Measure::assignEnfaceImage(const OctEnfaceImage& enface)
{
	ScannerResult* result = getScannerResultLast();
	if (result == nullptr) {
		return false;
	}

	bool process = GlobalSettings::useEnfaceImageCorrection();
	LogD() << "Measure enface image, process: " << process;
	return result->getEnfaceOutput().setEnfaceImage(enface, process);
}


bool OctSystem::Measure::assignRetinaImageToScannerResult(const OctRetinaImage & retina)
{
	ScannerResult* result = getScannerResultLast();
	if (result == nullptr) {
		return false;
	}

	return result->setRetinaImage(retina);
}


bool OctSystem::Measure::assignCorneaImageToScannerResult(const OctCorneaImage & cornea)
{
	ScannerResult* result = getScannerResultLast();
	if (result == nullptr) {
		return false;
	}

	return result->setCorneaImage(cornea);
}


bool OctSystem::Measure::assignRetinaImageToScannerResult(const CppUtil::CvImage & retina)
{
	OctRetinaImage image;
	image.setup(retina.getBitsData(), retina.getWidth(), retina.getHeight());
	return assignRetinaImageToScannerResult(image);
}


bool OctSystem::Measure::assignCorneaImageToScannerResult(const CppUtil::CvImage & cornea)
{
	OctCorneaImage image;
	image.setup(cornea.getBitsData(), cornea.getWidth(), cornea.getHeight());
	return assignCorneaImageToScannerResult(image);
}


bool OctSystem::Measure::assignFundusImage(const OctFundusSection & section, const OctFundusImage & image, const OctFundusFrame& frame)
{
	FundusResult* result = getFundusResultLast();
	if (result == nullptr) {
		return false;
	}

	return result->getFundusOutput().addSectionImage(section, image, frame);
}


bool OctSystem::Measure::assignRetinaImageToFundusResult(const OctRetinaImage & retina)
{
	FundusResult* result = getFundusResultLast();
	if (result == nullptr) {
		return false;
	}

	return result->setRetinaImage(retina);
}


bool OctSystem::Measure::assignCorneaImageToFundusResult(const OctCorneaImage & cornea)
{
	FundusResult* result = getFundusResultLast();
	if (result == nullptr) {
		return false;
	}

	return result->setCorneaImage(cornea);
}


bool OctSystem::Measure::assignRetinaImageToFundusResult(const CppUtil::CvImage & retina)
{
	OctRetinaImage image;
	image.setup(retina.getBitsData(), retina.getWidth(), retina.getHeight());
	return assignRetinaImageToFundusResult(image);
}


bool OctSystem::Measure::assignCorneaImageToFundusResult(const CppUtil::CvImage & cornea)
{
	OctCorneaImage image;
	image.setup(cornea.getBitsData(), cornea.getWidth(), cornea.getHeight());
	return assignCorneaImageToFundusResult(image);
}


bool OctSystem::Measure::createDefaultExportDirectory(bool bufferDirs)
{
	auto path = _T(".//export");
	getImpl().exportPath = path;
	if (CreateDirectory(getImpl().exportPath.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
		auto camera = path + wstring(_T("//camera"));
		getImpl().cameraPath = camera;
		if (CreateDirectory(camera.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
			auto buffer = path + wstring(_T("//buffer"));
			getImpl().bufferPath = buffer;
			if (CreateDirectory(buffer.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
			}
		}
		return true;
	}
	return false;
}


Measure::MeasureImpl & OctSystem::Measure::getImpl(void)
{
	return *d_ptr;
}
