#pragma once

#include "SigProcDef.h"

#include <memory>
#include <vector>


namespace SigProc
{
	class SIGPROC_DLL_API Magnitude
	{
	public:
		Magnitude();
		virtual ~Magnitude();

		Magnitude(Magnitude&& rhs);
		Magnitude& operator=(Magnitude&& rhs);
		Magnitude(const Magnitude& rhs) = delete;
		Magnitude& operator=(const Magnitude& rhs) = delete;

	public:
		void setIntensityData(float* data, int width, int height, int wPadd, int hPadd, int multi=1);
		void updateDataProfile(void);
		void chooseAxialDataIndex(int index);
		int getAxialDataIndex(void);

		void setAxialDataAverageSize(int size);
		void setAxialData(int index, float* data);
		float* getAxialData(int index);

		void setFixedNoiseValue(float level);

		void setAxialPeakRangeOffset(int offset1, int offset2);
		void updateAxialDataPeak(int index);
		void updateAxialDataSignal(int index);
		void clearAxialDataHistory(void);

		bool buildHistogram(int rowStart, int colStart, int rowSize, int colSize, int binSize, int binUnit);
		bool updateSignalToNoiseRatios(float sigRatio, float bgdRatio);

		int frameCols(void) const;
		int frameRows(void) const;
		int dataCols(void) const;
		int dataRows(void) const;
		int imageCols(void) const;
		int imageRows(void) const;

		int peakTopIndex(void) const;
		float peakTopValue(void) const;
		int peakHalfIndex(int index) const;
		int peakTermIndex(int index) const;
		float peakFWHM(void) const;
		float peakSNR(void) const;
		float maxPeakSNR(void) const;
		float meanPeakSNR(void) const;
		float meanPeakFWHM(void) const;
		float signalLevel(void) const;
		float noiseLevel(void) const;
	
		float* getHistogram(int col) const;
		float* getSnrRatios(void) const;

	private:
		struct MagnitudeImpl;
		std::unique_ptr<MagnitudeImpl> d_ptr;
		MagnitudeImpl& getImpl(void) const;
	};
}
