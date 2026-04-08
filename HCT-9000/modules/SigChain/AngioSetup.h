#pragma once


#include "SigChainDef.h"

#include <memory>
#include <string>
#include <vector>


namespace SigProc {
	class Angiogram;
}

namespace CppUtil {
	class CvImage;
}


namespace SigChain
{
	class SIGCHAIN_DLL_API AngioSetup
	{
	public:
		AngioSetup();
		virtual ~AngioSetup();

	public:
		static void setAngiogramLayout(int lines, int points, int repeats=2, bool vertial=false);

		static void resetImageBuffers(int width, int height = 1025, int linePadds = 0);
		static float* getImageRealBuffer(void);
		static float* getImageImagBuffer(void);
		static void copyImageRealsToBuffer(float* data, int width, int height, int linePadds, bool vflip = false);
		static void copyImageImagsToBuffer(float* data, int width, int height, int linePadds, bool vflip = false);

		static int getBufferWidth(void);
		static int getBufferHeight(void);
		static int getImageBufferSize(void);
		static int getDataSize(void);
		static int getDataWidth(void);
		static int getDataHeight(void);
		static int getPreviewWidth(void);
		static int getPreviewSize(void);

		static bool isSavingBuffersToFiles(void);
		static bool isSavingFramesToFiles(void);
		static bool isEdiScanningMode(void);

		static void setEdiScanningMode(bool flag);
		static void setSavingBuffersToFiles(bool flag);
		static void setSavingFramesToFiles(bool flag);
		static void setPreviewDecorrelations(bool flag);

		static void dumpImageBuffersToFile(int index);
		static void saveAmplitudesAsBinaryFiles(const std::string dirPath = ".//export");

		static bool storeAmplitudesToPatternBuffer(int index, float* buff);
		static bool storeAmplitudesToPreviewBuffer(float* buff);
		static bool updateDecorrelationsOfPreview(void);
		static float* getDecorrelation(void);

		static int numberOfAngioImages(void);
		static int numberOfAngioRepeats(void);
		static int numberOfAngioLines(void);
		static int numberOfAngioPoints(void);
		static int getIndexOfLineFromBufferIndex(int index);
		static int getIndexOfRepeatFromBufferIndex(int index);
		static bool isVerticalScan(void);

		static std::vector<std::vector<CppUtil::CvImage>>& getAmplitudes(void);

		static void setImageRepeats(int repeats);
		static bool makeAmplitudesOfLineRepeats(int idxLine);
		static bool makeDecorrelationOfAmplitudes(void);

		static const int getNumberOfRepeatedData(void);
		//static const std::vector<float> getAmplitudes(int frameIdx);
		static const std::vector<float> getDecorrelations(void);

		static const float getDecorrMean(int row1, int row2, int axialIdx = -1);
		static const float getDecorrStdev(int row1, int row2, int axialIdx = -1);
		static const float getDecorrMax(int row1, int row2, int axialIdx = -1);

	private:
		struct AngioSetupImpl;
		static std::unique_ptr<AngioSetupImpl> d_ptr;
		static AngioSetupImpl& getImpl(void);
	};
}

