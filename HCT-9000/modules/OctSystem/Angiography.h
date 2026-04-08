#pragma once

#include "OctSystemDef.h"

#include <memory>

namespace OctData {
	class ProtocolData;
}

namespace CppUtil {
	class CvImage;
}

namespace OctAngio {
	class Angiogram2;
	class AngioLayers;
	class AngioEtdrsChart;
	class AngioDonutChart;
	class AngioGridChart;
}

namespace OctSystem
{
	class OCTSYSTEM_DLL_API Angiography
	{
	public:
		Angiography();
		virtual ~Angiography();

	public:
		static void clearAngiography(void);
		static void setCurrentAngiogram(int index = 0);

		static bool initiateAngiogram(OctData::ProtocolData* data, bool prepare=true);
		static bool initiateAngiogram(const OctScanPattern& desc, bool prepare=true);
		static bool initiateAngiogramByDataFile(OctData::ProtocolData* data, const std::string dirPath = ".//export", const std::string fileName = "angio.dat");
		static bool initiateAngiogramByDataFile(const OctScanPattern& desc, const std::string dirPath = ".//export", const std::string fileName="angio.dat");
		static bool initiateAngiogramByDataFile(int lines, int points, int repeats, bool vertical, const std::string dirPath = ".//export", const std::string fileName = "angio.dat");
		static bool initiateAngiogramByImageFiles(int lines, int points, int repeats, bool vertical, const std::string dirPath = ".//export");
		static bool initiateAngiogramByDataFile2(const std::string dirPath = ".//export", const std::string fileName = "angio2.dat");

		static bool prepareAngiogram(bool alignAxial = true, bool alignLateral = true, bool realign = true);
		static bool generateMotionData(void);

		static bool updateAngiogram(OcularLayerType upper, OcularLayerType lower, float upperOffset=0.0f, float lowerOffset=0.0f, 
									float rangeX=3.0f, float rangeY=3.0f, float centerX=0.0f, float centerY=0.0f, 
									bool isDisc = false);
		static bool updateAngiogram2(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f,
									float rangeX = 3.0f, float rangeY = 3.0f, float centerX = 0.0f, float centerY = 0.0f,
									bool isDisc = false);
		
		static bool importDataFile(const std::string dirPath = ".//export", const std::string fileName = "angio.dat");
		static bool exportDataFile(const std::string dirPath = ".//export", const std::string fileName = "angio.dat");
		static bool importDataFile2(const std::string dirPath = ".//export", const std::string fileName = "angio2.dat");
		static bool exportDataFile2(const std::string dirPath = ".//export", const std::string fileName = "angio2.dat");

		static std::string getImportDirPath(void);

		static bool loadLayerSegmentsToAngiogram(void);

		static OctAngio::Angiogram2& getAngiogram(void);
		static CppUtil::CvImage getAngioImage(int width = 0, int height = 0, float clipLimit = 0.0f);
		static CppUtil::CvImage getOffsetImage(int width = 0, int height = 0);

		static CppUtil::CvImage getScanImage(int lineIdx, int width = 0, int height = 0, float clipLimit = 1.0f);
		static CppUtil::CvImage getDecorrImage(int lineIdx, int width = 0, int height = 0, float clipLimit = 1.0f);

		static OctAngio::AngioEtdrsChart createEtdrsChart(float threshold=0.15f, float centerX=0.0f, float centerY=0.0f, float innerDiam=1.5f, float outerDiam=2.5f);
		static OctAngio::AngioDonutChart createDonutChart(float threshold = 0.15f, float centerX = 0.0f, float centerY = 0.0f, float innerDiam = 1.0f, float outerDiam = 3.0f);
		static OctAngio::AngioGridChart createGridChart(float threshold = 0.15f, float centerX = 0.0f, float centerY = 0.0f, float gridSize = 1.0f);

		static bool isValidProtocol(void);
		static bool isAllDataLoaded(void);
		static void useVascularLayers(bool flag = false);
		static void useMotionCorrection(bool flag = false);
		static void useBiasFieldCorrection(bool flag = false);
		static void setNoiseReductionRate(float rate);

	protected:
		static bool importLayerSegmentsFromProtocolData(OctAngio::AngioLayers& layers);
		static bool importLayerSegments(OcularLayerType upper, OcularLayerType lower, float upperOffset=0.0f, float lowerOffset=0.0f);


	private:
		struct AngiographyImpl;
		static std::unique_ptr<AngiographyImpl> d_ptr;
		static AngiographyImpl& getImpl(void);
	};
}

