#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>
#include <string>
#include <map>


namespace OctResult
{
	class BscanImage2;

	class OCTRESULT_DLL_API BscanSection
	{
	public:
		BscanSection();
		//BscanSection(OctRoute route);
		BscanSection(const OctScanSection& desc);
		virtual ~BscanSection();

		BscanSection(BscanSection&& rhs);
		BscanSection& operator=(BscanSection&& rhs);
		BscanSection(const BscanSection& rhs) = delete;
		BscanSection& operator=(const BscanSection& rhs) = delete;

	public:
		bool setImage(const OctScanImage& desc);
		bool addImage(const OctScanImage& desc);
		bool removeImage(int index);

		BscanImage2* getImage(int index = 0);
		int getImageCount(void) const;
		void clearAllImages(void);

		void setDescript(const OctScanSection& desc);
		const OctScanSection& getDescript(void) const;
		void setRouteOfScan(OctRoute route);
		const OctRoute& getRouteOfScan(void) const;

		const OctPoint& getStartPoint(void) const;
		const OctPoint& getEndPoint(void) const;
		bool isCircle(void);

		bool updateImages(void);
		bool exportImages(const std::wstring& dirPath, const std::wstring& prefix, bool single = false);
		bool importImages(const std::wstring& dirPath, const std::wstring& prefix, int size = 1);

		bool updateImage(int imageIdx = 0);
		bool exportImage(const std::wstring& path, int imageIdx = 0);
		bool importImage(const std::wstring& path, bool append = false);

		bool importImageWithExt(const std::wstring& dirPath, const std::wstring& name,
			const std::wstring& imageExt, int size, int idx);

		bool applyAveraging(bool forward = true);

	private:
		struct BscanSectionImpl;
		std::unique_ptr<BscanSectionImpl> d_ptr;
		BscanSectionImpl& getImpl(void) const;
	};

	// typedef std::vector<BscanSection> BscanSectionList;
	typedef std::map<int, BscanSection> BscanSectionList;
}
