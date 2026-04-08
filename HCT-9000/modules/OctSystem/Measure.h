#pragma once

#include "OctSystemDef.h"

#include <memory>
#include <string>
#include <vector>


namespace OctResult 
{
	class ScannerResult;
	class FundusResult;

	class FundusImage;
	class FundusFrame;
}


namespace CppUtil
{
	class CvImage;
}


namespace OctSystem
{

	class OCTSYSTEM_DLL_API Measure
	{
	public:
		Measure();
		virtual ~Measure();

	public:
		static void setPatient(const OctPatient& patient);
		static OctPatient& getPatient(void);

		static bool isScannerResult(EyeSide side = EyeSide::BOTH);
		static bool isFundusResult(EyeSide side = EyeSide::BOTH);
		static void clearScannerResults(void);
		static void clearFundusResults(void);
		static std::shared_ptr<OctResult::ScannerResult> fetchScannerResult(int index=0);
		static std::shared_ptr<OctResult::FundusResult> fetchFundusResult(int index=0);

		static int getCountOfPreviewSections(void);
		static int getCountOfPatternSections(void);
		static float getPatternQualityIndex(void);
		static void getPatternQualityStat(std::vector<float>& stat, float sectSize=5.0f);
	
		static const OctScanImage* getPreviewImageDescript(int sectIdx = 0, int imageIdx = 0);
		static const OctScanImage* getPatternImageDescript(int sectIdx = 0, int imageIdx = 0);
		static const OctEnfaceImage* getEnfaceImageDescript(void);
		static const OctRetinaImage* getRetinaImageDescriptOfScannerResult(void);
		static const OctCorneaImage* getCorneaImageDescriptOfScannerResult(void);

		static int getCountOfFundusSections(void);
		static const OctFundusImage* getFundusImageDescript(int sectIdx = 0);
		static const OctRetinaImage* getRetinaImageDescriptOfFundusResult(void);
		static const OctCorneaImage* getCorneaImageDescriptOfFundusResult(void);

		static OctResult::FundusImage* getFundusImageObject(int sectIdx = 0);
		static OctResult::FundusFrame* getFundusFrameObject(int sectIdx = 0);

		static int exportScannerResults(std::wstring dirPath = L"", bool subPath = true,
										std::wstring imagePrefix = L"", 
										std::wstring enfaceName = L"enface",
										std::wstring previewName = L"preview", 
										std::wstring retinaName = L"retina", 
										std::wstring corneaName = L"cornea");
		static int exportFundusResults(unsigned int imgQuality,
										std::wstring dirPath = L"", bool subPath = true,
										bool removeReflectionLight = false,
										std::wstring imageName = L"fundus",
										std::wstring thumbName = L"thumbnail",
										std::wstring retinaName = L"retina",
										std::wstring corneaName = L"cornea");

		static bool createDefaultExportDirectory(bool bufferDirs=false);

	protected:
		static void initialize(void);

		static bool initiateScannerResult(const OctScanMeasure& desc, bool clear = true);
		static bool initiateFundusResult(const OctFundusMeasure& desc, bool clear = true);
		static bool completeScannerResult(void);
		static bool completeFundusResult(void);

		static int getCountOfScannerResults(EyeSide side = EyeSide::BOTH);
		static int getCountOfFundusResults(EyeSide side = EyeSide::BOTH);

		static OctResult::ScannerResult* getScannerResultByIndex(int index = 0);
		static OctResult::ScannerResult* getScannerResultLast(void);
		static OctResult::ScannerResult* getScannerResultFirst(void);

		static OctResult::FundusResult* getFundusResultByIndex(int index = 0);
		static OctResult::FundusResult* getFundusResultLast(void);

		static bool assignPreviewImage(const OctScanSection& section, const OctScanImage& image);
		static bool assignPatternImage(const OctScanSection& section, const OctScanImage& image, int idxOverlap, bool segment = false);
		static bool assignEnfaceImage(const OctEnfaceImage& enface);
		static bool assignRetinaImageToScannerResult(const OctRetinaImage& retina);
		static bool assignCorneaImageToScannerResult(const OctCorneaImage& cornea);
		static bool assignRetinaImageToScannerResult(const CppUtil::CvImage& retina);
		static bool assignCorneaImageToScannerResult(const CppUtil::CvImage& cornea);

		static bool assignFundusImage(const OctFundusSection& section, const OctFundusImage& image, const OctFundusFrame& frame);
		static bool assignRetinaImageToFundusResult(const OctRetinaImage& retina);
		static bool assignCorneaImageToFundusResult(const OctCorneaImage& cornea);
		static bool assignRetinaImageToFundusResult(const CppUtil::CvImage& retina);
		static bool assignCorneaImageToFundusResult(const CppUtil::CvImage& cornea);

	private:
		struct MeasureImpl;
		static std::unique_ptr<MeasureImpl> d_ptr;
		static MeasureImpl& getImpl(void);

		friend class Controller;
		friend class Camera;
		friend class Scanner;
		friend class ScanPost;
		friend class Loader;
	};
}

