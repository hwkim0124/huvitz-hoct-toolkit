#pragma once

#include "OctDataDef.h"

#include <string>
#include <memory>
#include <vector>


namespace OctResult {
	class ScannerResult;
	class PatternOutput2;
	class PreviewOutput;
}


namespace SegmScan {
	class OcularBsegm;
}


namespace OctData
{
	class BscanData;

	class OCTDATA_DLL_API ProtocolData
	{
	public:
		ProtocolData();
		ProtocolData(const OctScanPattern& desc);
		ProtocolData(std::unique_ptr<OctResult::ScannerResult>&& result);
		ProtocolData(std::shared_ptr<OctResult::ScannerResult> result);
		virtual ~ProtocolData();

		ProtocolData(ProtocolData&& rhs);
		ProtocolData& operator=(ProtocolData&& rhs);
		ProtocolData(const ProtocolData& rhs) = delete;
		ProtocolData& operator=(const ProtocolData& rhs) = delete;

	public:
		float getPixelWidth(void) const;
		float getPixelHeight(void) const;
		float getLinesDistance(void) const;
		int numberOfBscanPoints(void) const;

		BscanData* getBscanData(int index) const;
		BscanData* getBscanDataOfSection(int sectIdx, int imageIdx) const;
		int countBscanData(void) const;
		int countBscanSections(void) const;
		int updateBscanDataList(bool vflip=false);

		BscanData* getPreviewData(int index) const;
		int countPreviewData(void) const;
		int updatePreviewDataList(bool vflip = false);

		std::vector<SegmScan::OcularBsegm*> getBscanSegmList(void) const;

		OctResult::ScannerResult* getResult(void) const;
		OctResult::PreviewOutput* getPreview(void) const;
		OctResult::PatternOutput2* getPattern(void) const;
		OctScanPattern& getDescript(void) const;
		void setDescript(const OctScanPattern& desc);

		const OctScanImage* getPreviewImageDescript(int sectIdx, int imageIdx = 0);
		const OctScanImage* getScanImageDescript(int sectIdx, int imageIdx = 0);
		const OctRetinaImage* getRetinaImageDescript(void) const;
		const OctCorneaImage* getCorneaImageDescript(void) const;

		void importScannerResult(std::shared_ptr<OctResult::ScannerResult> result);

		virtual int importPatternImages(const std::wstring& dirPath, int numImages = 0);
		virtual int importPatternImages(const std::vector<std::wstring>& fileList);
		virtual int importPreviewImages(const std::wstring& dirPath, int numImages = 0);

		virtual int updatePatternImages(void);
		virtual int updatePreviewImages(void);

		bool exportBsegmResults(const std::wstring dirPath = L"./export");
		bool importBsegmResults(const std::wstring dirPath = L"./export");		
		void clear(void);

	protected:
		

	private:
		struct ProtocolDataImpl;
		std::unique_ptr<ProtocolDataImpl> d_ptr;
		ProtocolDataImpl& getImpl(void) const;
	};
}

