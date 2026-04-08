#pragma once

#include "OctConfigDef.h"

#include <memory>
#include <vector>


namespace OctConfig
{
	class OCTCONFIG_DLL_API RetinaSettings
	{
	public:
		RetinaSettings();
		virtual ~RetinaSettings();

		RetinaSettings(RetinaSettings&& rhs);
		RetinaSettings& operator=(RetinaSettings&& rhs);
		RetinaSettings(const RetinaSettings& rhs);
		RetinaSettings& operator=(const RetinaSettings& rhs);

	public:
		void initialize(void);
		void resetToDefaultValues(void);

		std::pair<int, int> getSplitCenter(void) const;
		std::pair<int, int> getSplitKernel(void) const;
		void setSplitCenter(std::pair<int, int> center);
		void setSplitKernel(std::pair<int, int> kernel);

        std::pair<int, int> getCenterPointOfROI(void) const;
		std::pair<int, int> getCenterPointOfMask(void) const;
		int getRadiusOfROI(void) const;
		int getCenterMaskSize(void) const;

        void setCenterPointOfROI(std::pair<int, int> center);
		void setCenterPointOfMask(std::pair<int, int> center);
		void setRadiusOfROI(int radius);
		void setCenterMaskSize(int size);

		float getEnhanceClipLimit(void) const;
		int getEnhanceClipScalar(void) const;
		void setEnhanceClipLimit(float value);
		void setEnhanceClipScalar(int value);

		bool useROI(bool isset = false, bool flag = false) const;
        bool useImageEnhance(bool isset = false, bool flag = false) const;
		bool useCenterMask(bool isset = false, bool flag = false) const;

	private:
		struct RetinaSettingsImpl;
		std::unique_ptr<RetinaSettingsImpl> d_ptr;
		RetinaSettingsImpl& getImpl(void) const;
	};
}

