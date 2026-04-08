#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class EnfaceImage;

	class OCTRESULT_DLL_API EnfaceData
	{
	public:
		EnfaceData();
		virtual ~EnfaceData();

		EnfaceData(EnfaceData&& rhs);
		EnfaceData& operator=(EnfaceData&& rhs);
		EnfaceData(const EnfaceData& rhs);
		EnfaceData& operator=(const EnfaceData& rhs);

	public:
		int addImage(EnfaceImage& image);
		int addImage(EnfaceImage&& image);
		EnfaceImage* getImage(int index);
		int getImageCount(void) const;
		void clearAllImages(void);

		void setRangeOfEnface(OctRange range);
		OctRange& getRangeOfEnface(void);

	private:
		struct EnfaceDataImpl;
		std::unique_ptr<EnfaceDataImpl> d_ptr;
		EnfaceDataImpl& getImpl(void) const;
	};


	typedef std::vector<EnfaceData> EnfaceDataList;
}