#pragma once

#include "OctConfigDef.h"

#include <memory>
#include <vector>


namespace OctConfig
{
	class OCTCONFIG_DLL_API PatternSettings
	{
	public:
		PatternSettings();
		virtual ~PatternSettings();

		PatternSettings(PatternSettings&& rhs);
		PatternSettings& operator=(PatternSettings&& rhs);
		PatternSettings(const PatternSettings& rhs);
		PatternSettings& operator=(const PatternSettings& rhs);

	public:
		void initialize(void);
		void resetToDefaultValues(void);

		std::pair<float, float>& retinaPatternScale(int index);
		std::pair<float, float>& corneaPatternScale(int index);
		std::pair<float, float>& topographyPatternScale(int index);

		std::pair<float, float>& retinaPatternOffset(int index);
		std::pair<float, float>& corneaPatternOffset(int index);
		std::pair<float, float>& topographyPatternOffset(int index);

		void setRetinaPatternScale(std::pair<float, float> value, int index = 0);
		void setCorneaPatternScale(std::pair<float, float> value, int index = 0);
		void setTopographyPatternScale(std::pair<float, float> value, int index = 0);

		void setRetinaPatternOffset(std::pair<float, float> value, int index = 0);
		void setCorneaPatternOffset(std::pair<float, float> value, int index = 0);
		void setTopographyPatternOffset(std::pair<float, float> value, int index = 0);

		int getReferenceRangeLowerSize(void);
		int getReferenceRangeUpperSize(void);
		void setReferenceRangeLowerSize(int size);
		void setReferenceRangeUpperSize(int size);

	private:
		struct PatternSettingsImpl;
		std::unique_ptr<PatternSettingsImpl> d_ptr;
		PatternSettingsImpl& getImpl(void) const;
	};
}

