#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>
#include <string>


namespace OctResult
{
	class SectionImage;

	class OCTRESULT_DLL_API SectionData
	{
	public:
		SectionData();
		SectionData(OctRoute route);
		SectionData(OctScanSection desc);
		virtual ~SectionData();

		SectionData(SectionData&& rhs);
		SectionData& operator=(SectionData&& rhs);
		SectionData(const SectionData& rhs);
		SectionData& operator=(const SectionData& rhs);

	public:
		int setImage(SectionImage& image);
		int setImage(SectionImage&& image);
		int addImage(SectionImage& image);
		int addImage(SectionImage&& image);
		SectionImage* getImage(int index);
		int getImageCount(void) const;
		void clearAllImages(void);

		OctScanSection& getDescript(void) const;
		void setRouteOfScan(OctRoute route);
		OctRoute getRouteOfScan(void) const;

		OctPoint& getStartPoint(void);
		OctPoint& getEndPoint(void);
		bool isCircle(void);

		bool exportFiles(std::wstring path, std::wstring prefix, std::wstring imageExt = _T("png"));

	private:
		struct SectionDataImpl;
		std::unique_ptr<SectionDataImpl> d_ptr;
		SectionDataImpl& getImpl(void) const;
	};

	typedef std::vector<SectionData> SectionDataList;
}
