#pragma once

#include "OctResultDef.h"

#include <memory>


namespace OctResult
{
	class ScanImage;
	class ScanProfile;

	class OCTRESULT_DLL_API ScanResult
	{
	public:
		ScanResult();
		virtual ~ScanResult();

		ScanResult(ScanResult&& rhs);
		ScanResult& operator=(ScanResult&& rhs);
		ScanResult(const ScanResult& rhs);
		ScanResult& operator=(const ScanResult& rhs);

	public:
		void clear(void);
		void addImage(ScanImage&& image);
		ScanImage* image(int index);
		int imageCount(void) const;
		void reserveImages(int count);

		void set_profile(ScanProfile&& profile);
		ScanProfile& profile(void) const;

	private:
		struct ScanResultImpl;
		std::unique_ptr<ScanResultImpl> d_ptr;
		ScanResultImpl& getImpl(void) const;
	};
}
