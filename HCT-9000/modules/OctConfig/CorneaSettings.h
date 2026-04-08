#pragma once

#include "OctConfigDef.h"

#include <memory>
#include <vector>


namespace OctConfig
{
	class OCTCONFIG_DLL_API CorneaSettings
	{
	public:
		CorneaSettings();
		virtual ~CorneaSettings();

		CorneaSettings(CorneaSettings&& rhs);
		CorneaSettings& operator=(CorneaSettings&& rhs);
		CorneaSettings(const CorneaSettings& rhs);
		CorneaSettings& operator=(const CorneaSettings& rhs);

	public:
		void initialize(void);
		void resetToDefaultValues(void);

		int getPixelsPerMM(void) const;
		float getSmallPupilSize(void) const;

		void setPixelsPerMM(int pixels);
		void setSmallPupilSize(float size);

	private:
		struct CorneaSettingsImpl;
		std::unique_ptr<CorneaSettingsImpl> d_ptr;
		CorneaSettingsImpl& getImpl(void) const;
	};
}

