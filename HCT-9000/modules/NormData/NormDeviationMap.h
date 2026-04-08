#pragma once


#include "NormDataDef.h"

#include <string>
#include <memory>
#include <map>


namespace CppUtil {
	class CvImage;
}

namespace NormData
{
	class NORMDATA_DLL_API NormDeviationMap
	{
	public:
		NormDeviationMap();
		virtual ~NormDeviationMap();

		NormDeviationMap(NormDeviationMap&& rhs);
		NormDeviationMap& operator=(NormDeviationMap&& rhs);

		// Prevent copy construction and assignment. 
		NormDeviationMap(const NormDeviationMap& rhs) = delete;
		NormDeviationMap& operator=(const NormDeviationMap& rhs) = delete;

	public:
		static const int WIDE_MACULAR_MAP_WIDTH = 384;
		static const int WIDE_MACULAR_MAP_HEIGHT = 96;
		
		static const int WIDE_DISC_MAP_WIDTH = 256;
		static const int WIDE_DISC_MAP_HEIGHT = 64;
		
		static const int MAP_WIDTH = 512;
		static const int MAP_HEIGHT = 96;

		static const int WIDE_MACULAR_MAP_SIZE = WIDE_MACULAR_MAP_WIDTH * WIDE_MACULAR_MAP_HEIGHT;
		static const int WIDE_DISC_MAP_SIZE = WIDE_DISC_MAP_WIDTH * WIDE_DISC_MAP_HEIGHT;
		static const int MAP_SIZE = MAP_WIDTH * MAP_HEIGHT;

		virtual CppUtil::CvImage makeImage(Ethinicity race, Gender gender, int age, EyeSide side, 
			const std::vector<float>& data, 
			int lines, int points, int width, int height, bool isWide);

	private:
		struct NormDeviationMapImpl;
		std::unique_ptr<NormDeviationMapImpl> d_ptr;
		NormDeviationMapImpl& getImpl(void) const;
	};
}

