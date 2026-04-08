#pragma once

#include "OctConfigDef.h"

#include <memory>
#include <vector>


namespace OctConfig
{
	class OCTCONFIG_DLL_API AngioSettings
	{
	public:
		AngioSettings();
		virtual ~AngioSettings();

		AngioSettings(AngioSettings&& rhs);
		AngioSettings& operator=(AngioSettings&& rhs);
		AngioSettings(const AngioSettings& rhs);
		AngioSettings& operator=(const AngioSettings& rhs);

	public:
		void initialize(void);
		void resetToDefaultValues(void);

		float getDecorThreshold(void);
		int getAverageOffset(void);
		float getMotionThreshold(void);
		float getMotionOverPoints(void);
		float getMotionDistRatio(void);
		int getFilterOrients(void);
		float getFilterSigma(void);
		float getFilterDivider(void);
		float getFilterWeight(void);
		float getEnhanceParam(void);
		float getBiasFieldSigma(void);

		void setDecorThreshold(float value);
		void setAverageOffset(int value);
		void setMotionThreshold(float value);
		void setMotionOverPoints(float value);
		void setMotionDistRatio(float value);
		void setFilterOrients(int value);
		void setFilterSigma(float value);
		void setFilterDivider(float value);
		void setFilterWeight(float value);
		void setEnhanceParam(float value);
		void setBiasFieldSigma(float value);

	private:
		struct AngioSettingsImpl;
		std::unique_ptr<AngioSettingsImpl> d_ptr;
		AngioSettingsImpl& getImpl(void) const;
	};
}
