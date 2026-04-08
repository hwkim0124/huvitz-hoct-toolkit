#pragma once

#include "SigProcDef.h"

#include <memory>
#include <vector>


namespace SigProc
{
	class SIGPROC_DLL_API Background
	{
	public:
		Background();
		virtual ~Background();

		Background(Background&& rhs);
		Background& operator=(Background&& rhs);
		Background(const Background& rhs) = delete;
		Background& operator=(const Background& rhs) = delete;

	public:
		void makeBackgroundSpectrum(const unsigned short* data, int width = LINE_CAMERA_CCD_PIXELS, int height = 1024);
		void setProfileData(const unsigned short* data, int size = LINE_CAMERA_CCD_PIXELS);
		unsigned short* getProfileData(void) const;
		bool isEmpty(void) const;
		void setEmpty(bool flag);
		void dumpToFile(void);

	private:
		struct BackgroundImpl;
		std::unique_ptr<BackgroundImpl> d_ptr;
		BackgroundImpl& getImpl(void) const;
	};
}
