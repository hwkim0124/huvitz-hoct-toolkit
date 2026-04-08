#pragma once

#include "OctGlobalDef.h"
#include "GlobalPatternDef.h"
#include "GlobalMeasureDef.h"

#include <memory>
#include <vector>


namespace OctGlobal
{
	class OCTGLOBAL_DLL_API PatternHelper
	{
	public:
		PatternHelper();
		virtual ~PatternHelper();

	public:
		static void initializePatternHelper(void);
		static int getNumberOfPatternTypes(void);
		static int getPatternCodeFromIndex(int index);
		static int getPatternIndexFromCode(int code);

		static const char* getPatternNameString(PatternName name);
		static const char* getPatternNameStringFromIndex(int index);
		static const char* getPatternNameStringFromCode(int code);

		static PatternDomain getPatternDomainFromCode(int code);
		static PatternName getPatternNameFromCode(int code);
		static PatternType getPatternTypeFromCode(int code, int direction = PATTERN_DIRECTION_X_Y);
		static int getPatternCodeFromName(PatternName name);

		static std::vector<float> getPatternRangesFromCode(int code);
		static std::vector<int> getPatternPointsFromCode(int code);
		static std::vector<int> getPatternDirectionFromCode(int code);
		static std::vector<int> getPatternOverlapsFromCode(int code, int points = 0);
		static std::vector<int> getPatternLinesFromCode(int code, int points = 0);

		static EyeRegion getEyeRegionFromDomain(PatternDomain domain);
		static bool isAngioPatternCode(int code);

	protected:


	private:
		struct PatternHelperImpl;
		static std::unique_ptr<PatternHelperImpl> d_ptr;
		static PatternHelperImpl& getImpl(void);
	};
}

