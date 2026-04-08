#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>
#include <string>


namespace OctResult
{
	class BscanImage2;

	class OCTRESULT_DLL_API BscanProfile
	{
	public:
		BscanProfile();
		BscanProfile(OctRoute route);
		BscanProfile(OctScanSection desc);
		virtual ~BscanProfile();

		BscanProfile(BscanProfile&& rhs);
		BscanProfile& operator=(BscanProfile&& rhs);
		BscanProfile(const BscanProfile& rhs);
		BscanProfile& operator=(const BscanProfile& rhs);

	public:
		int setImage(BscanImage2& image);
		int setImage(BscanImage2&& image);
		int addImage(BscanImage2& image);
		int addImage(BscanImage2&& image);
		BscanImage2* getImage(int index);
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
		struct BscanProfileImpl;
		std::unique_ptr<BscanProfileImpl> d_ptr;
		BscanProfileImpl& getImpl(void) const;
	};

	typedef std::vector<BscanProfile> BscanProfileList;
}
