#pragma once

#include "OctAngioDef.h"

#include <memory>
#include <vector>

namespace CppUtil {
	class CvImage;
}


namespace OctAngio
{
	class AngioLayout;

	class OCTANGIO_DLL_API AngioData
	{
	public:
		AngioData();
		virtual ~AngioData();

		AngioData(AngioData&& rhs);
		AngioData& operator=(AngioData&& rhs);

	public:
		bool fetchAmplitudesFromBuffer(int lines, int points, int repeats);
		bool importAmplitudesFromImageFiles(int lines, int points, int repeats, const std::string dirPath);
		bool importAmplitudesFromDataFiles(int lines, int points, int repeats, const std::string dirPath, const std::string fileName);
		bool exportAmplitudesToDataFiles(int lines, int repeats, const std::string dirPath, const std::string fileName);

		bool checkIfAmplitudesLoaded(int lines, int points, int repeats);

		bool appendAmplitudesToCircular(int lines, int repeats);
		bool alignRepeatedAmplitudes(int lines, int repeats, bool axial, bool lateral, const LayerArrays& uppers, const LayerArrays& lowers);
		bool computeSequentialDisplacements(int lines, int repeats, std::vector<float>& trans_xlist, std::vector<float>& trans_ylist, std::vector<float>& ecc_list);

		Amplitudes& getAmplitudes(void) const;
		Imaginaries& getImaginaries(void) const;
		Reals& getReals(void) const;

		int dataWidth(void) const;
		int dataHeight(void) const;
		unsigned char* getGrayscaledDataBits(int lineIdx, int repeatIdx, bool vflip=true);

	protected:
		bool readDataFile(int lineIdx, int repeatIdx, char* buff, int dataSize, const std::string dirPath = ".//export");
		bool writeDataFile(int lineIdx, int repeatIdx, char* buff, int dataSize, const std::string dirPath = ".//export");
		bool readImageDataFile(int lineIdx, int repeatIdx, char* rbuff, char* ibuff, int dataSize, const std::string dirPath = ".//export");
		bool makeAmplitudeFromImageData(int lineIdx, int repeatIdx, const float* rbuff, const float* ibuff);

		void registerAmplitudes(int lineIdx, const LayerArrays& uppers, const LayerArrays& lowers);
		void registerAmplitudes2(int lineIdx, const LayerArrays& uppers, const LayerArrays& lowers);

	private:
		struct AngioDataImpl;
		std::unique_ptr<AngioDataImpl> d_ptr;
		AngioDataImpl& getImpl(void) const;
	};
}

