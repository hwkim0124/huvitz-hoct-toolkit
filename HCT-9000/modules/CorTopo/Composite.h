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
	class CORTOPO_DLL_API Composite
	{
	public:
		Composite();
		~Composite();

	public: 
		static bool detectTrunkLinesOnCornea(const RetSegm::SegmImage* srcImg, const std::vector<int>& antes, const std::vector<int>& posts,
			std::vector<int>& line1, std::vector<int>& line2);
		static bool detectTrunkLinesOnChamber(const RetSegm::SegmImage* srcImg, 
			std::vector<int>& line1, std::vector<int>& line2, float rangeX = 16.0f);
		static bool searchCorneaChamberOffsets(const RetSegm::SegmImage* cornea, const RetSegm::SegmImage* chamber,
			const std::vector<float>& line1, const std::vector<float>& line2, int& offsetX, int& offsetY);
		
		static bool updateCorneaChamberOffsets(const RetSegm::SegmImage* cornea, const std::vector<int>& corneaHinge1, const std::vector<int>& corneaHinge2,
			const std::vector<int>& chamberHinge1, const std::vector<int>& chamberHinge2, int& offsetX, int& offsetY);
		static bool updateCorneaChamberOffsets2(const RetSegm::SegmImage* cornea, const std::vector<int>& corenaAntes, const std::vector<int>& chamberHinge1, 
			const std::vector<int>& chamberHinge2, int& offsetX, int& offsetY);
		static bool stitchCorneaChamberImages(const RetSegm::SegmImage* cornea, const RetSegm::SegmImage* chamber,
			int offsetX, int offsetY, RetSegm::SegmImage* result);
		static bool removeReversedCorneaOnChamber(const RetSegm::SegmImage* cornea, const RetSegm::SegmImage* chamber);
		static bool removeReversedCorneaOnChamber2(const RetSegm::SegmImage* cornea, const RetSegm::SegmImage* chamber,
			const std::vector<int>& corneaAntes, const std::vector<int>& corneaPosts, const std::vector<int>& corneaSides,
			const std::vector<int>& chamberHinge1, const std::vector<int>& chamberHinge2,
			int centerX, int centerY, int offsetX, int offsetY);
		static bool removeBackgroundOnCornea(const RetSegm::SegmImage* cornea, int centerX, int centerY, const std::vector<int> antes, const std::vector<int> posts);
		static bool removeBackgroundOnAnterior(const RetSegm::SegmImage* anterior);

		static bool applyDewarpingOnAnterior(RetSegm::SegmImage* anterior);
	};
}

