#pragma once

#include "OctGlobalDef.h"

#include <memory>

namespace OctGlobal
{
	class OCTGLOBAL_DLL_API GlobalMemory
	{
	public:
		GlobalMemory();
		virtual ~GlobalMemory();

	public:
		static unsigned short* getOctBackgroundSpectrum(void);
		static void setOctBackgroundSpectrum(unsigned short* data);

		static void setOctManualExposureTime(float expTime);
		static float getOctManualExposureTime(bool clear);
		static bool isOctManualExposureTime(void);
		static void clearOctManualExposureTime(void);


	private:
		struct GlobalMemoryImpl;
		static std::unique_ptr<GlobalMemoryImpl> d_ptr;
		static GlobalMemoryImpl& getImpl(void);
	};
}
