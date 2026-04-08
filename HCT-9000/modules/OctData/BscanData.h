#pragma once

#include "OctDataDef.h"

#include <string>
#include <vector>
#include <memory>


namespace OctResult {
	class BscanImage2;
}


namespace SegmScan {
	class OcularBsegm;
}


namespace RetSegm {
	class SegmImage;
}


namespace OctData
{
	class OCTDATA_DLL_API BscanData 
	{
	public:
		BscanData();
		BscanData(OctResult::BscanImage2* image);
		virtual ~BscanData();

		BscanData(BscanData&& rhs);
		BscanData& operator=(BscanData&& rhs);
		BscanData(const BscanData& rhs) = delete;
		BscanData& operator=(const BscanData& rhs) = delete;

	public:
		const OctScanImage* getImageDescript(void) const;
		OctResult::BscanImage2* getImageObject(void) const;
		std::uint8_t* getImageBuffer(void) const;
		std::wstring getImageName(bool path = false) const;

		int getImageWidth(void) const;
		int getImageHeight(void) const;
		int getSectionIndex(void) const;
		int getOverlapIndex(void) const;

		void setSectionIndex(int index);
		void setOverlapIndex(int index);
		void setOcularBsegm(std::unique_ptr<SegmScan::OcularBsegm> bsegm);
		bool employBsegmImage(void);

		SegmScan::OcularBsegm* getOcularBsegm(void) const;
		bool isBsegmResult(void) const;
		bool isBsegmEmpty(void) const;

		RetSegm::SegmImage* getBsegmSource(void);
		RetSegm::SegmImage* getBsegmSample(void);
		RetSegm::SegmImage* getBsegmDenoised(void);
		RetSegm::SegmImage* getBsegmGradients(void);
		RetSegm::SegmImage* getBsegmAscends(void);
		RetSegm::SegmImage* getBsegmDescends(void);

		std::vector<int> getLayerPoints(OcularLayerType layer, int width = 0, int height = 0) const;
		std::vector<float> getLayerThickness(OcularLayerType upper, OcularLayerType lower, int width = 0, int height = 0) const;
		std::vector<float> getLayerDistance(OcularLayerType upper, OcularLayerType lower, int width = 0, int height = 0) const;
		std::vector<float> getCurvatureRadius(void) const;
		std::vector<float> getCorneaCurvature(int mapType) const;
		bool isLayerPoints(OcularLayerType layer) const;

		void setLayerPoints(OcularLayerType layer, std::vector<int> points) const;
		void setCurvatureRadius(const std::vector<float>& rads);

		bool isOpticNerveDisc(void) const;
		bool getOpticNerveDiscRange(int& x1, int& x2) const;
		bool getOpticNerveDiscPixels(int& pixels) const;
		bool isOpticNerveCup(void) const;
		bool getOpticNerveCupRange(int& x1, int& x2) const;
		bool getOpticNerveCupPixels(int& pixels) const;

		void setOpticNerveDiscRange(int x1, int x2);
		void setOpticNerveDiscPixels(int pixels);
		void setOpticNerveCupRange(int x1, int x2);
		void setOpticNerveCupPixels(int pixels);

		bool isEmpty(void) const;

		bool exportBsegmResult(std::wstring path, const OctScanPattern desc);
		bool importBsegmResult(std::wstring path, const OctScanPattern desc);

	private:
		struct BscanDataImpl;
		std::unique_ptr<BscanDataImpl> d_ptr;
		BscanDataImpl& getImpl(void) const;
	};
}
