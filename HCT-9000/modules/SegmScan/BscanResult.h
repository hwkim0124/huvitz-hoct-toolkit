#pragma once

#include "SegmScanDef.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class ScanRange;
	class BscanImage;

	class SEGMSCAN_DLL_API BscanResult
	{
	public:
		BscanResult();
		BscanResult(ScanRange& range);
		BscanResult(const std::wstring& path);
		BscanResult(BscanImage& image);
		BscanResult(ScanRange& range, const std::wstring& path);
		virtual ~BscanResult();

		BscanResult(BscanResult&& rhs);
		BscanResult& operator=(BscanResult&& rhs);
		BscanResult(const BscanResult& rhs);
		BscanResult& operator=(const BscanResult& rhs);

	private:
		struct BscanResultImpl;
		std::unique_ptr<BscanResultImpl> d_ptr;

	public:
		ScanRange* getRange(void) const;
		void setRange(ScanRange& range);
		
		void addImage(BscanImage& image);
		void addImage(BscanImage&& image);
		void addImage(const std::wstring& path);
		BscanImage* getImage(int index = 0) const;
		int getImageCount(void) const;
		bool isEmpty(void) const;
	};

	typedef std::vector<BscanResult> BscanResultVect;
}
