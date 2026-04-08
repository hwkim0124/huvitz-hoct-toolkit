#pragma once

#include "CorTopoDef.h"

#include <string>
#include <memory>
#include <vector>


namespace CppUtil {
	class CvImage;
}


namespace RetSegm {
	class SegmImage;
}


namespace CorTopo
{
	class CORTOPO_DLL_API AnteriorLens
	{
	public:
		AnteriorLens();
		virtual ~AnteriorLens();

		AnteriorLens(AnteriorLens&& rhs);
		AnteriorLens& operator=(AnteriorLens&& rhs);
		AnteriorLens(const AnteriorLens& rhs);
		AnteriorLens& operator=(const AnteriorLens& rhs);

	public:
		bool loadCorneaImage(const std::wstring& path = _T(""), bool vflip=false);
		bool makeupAnteriorCorneaBorder(void);
		bool makeupAnteriorLensBorder(void);
		bool makeupPosteriorLensBorder(void);
		bool isCorneaImageLoaded(void) const;

		int getAnteriorCorneaPosition(void) const;
		int getAnteriorLensPosition(void) const;
		int getPosteriorLensPosition(void) const;

		std::wstring corneaImageName(void) const;

		RetSegm::SegmImage* imageCornea(void) const;
		RetSegm::SegmImage* sampleCornea(void) const;
		RetSegm::SegmImage* ascentCornea(void) const;
		RetSegm::SegmImage* descentCornea(void) const;

		std::vector<int>& corneaAnteriorLine(void) const;
		std::vector<int>& lensAnteriorLine(void) const;
		std::vector<int>& lensPosteriorLine(void) const;

	protected:
		bool prepareSamples(bool vflip=false);
		bool segmentCorneaBorderLines(float rangeX, int& centerX, int& centerY, std::vector<int>& antes, std::vector<int>& posts);
		bool segmentLensAnteriorLine(float rangeX, int& centerX, int& centerY, std::vector<int>& line);
		bool segmentLensPosteriorLine(float rangeX, int& centerX, int& centerY, std::vector<int>& line);
		bool upscaleCorneaBorderLines(void);
		bool upscaleLensAnteriorLine(void);
		bool upscaleLensPosteriorLine(void);

	private:
		struct AnteriorLensImpl;
		std::unique_ptr<AnteriorLensImpl> d_ptr;
		AnteriorLensImpl& getImpl(void) const;
	};
}

