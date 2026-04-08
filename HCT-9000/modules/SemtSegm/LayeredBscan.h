#pragma once

#include "SemtSegmDef.h"


namespace RetSegm {
	class SegmLayer;
	class SegmImage;
}


namespace SemtSegm
{
	class BscanSegmentator;

	class SEMTSEGM_DLL_API LayeredBscan :
		public std::enable_shared_from_this<LayeredBscan>
	{
	public:
		LayeredBscan();
		virtual ~LayeredBscan();

		LayeredBscan(LayeredBscan&& rhs);
		LayeredBscan& operator=(LayeredBscan&& rhs);
		LayeredBscan(const LayeredBscan& rhs) = delete;
		LayeredBscan& operator=(const LayeredBscan& rhs) = delete;

	public:
		virtual bool performSegmentation() = 0;

		void setImageSource(const unsigned char* bits, int width, int height, float range = 6.0f, int index = 0);
		void setImageSource(OctScanImage image);
		void setImageIndex(int index);

		bool isImageSource(void) const;
		int getImageIndex(void) const;
		float getImageRangeX(void) const;

		const OctScanImage& getImageSource(void) const ;

		const OcularImage* getImageSample(void) const;
		const OcularImage* getImageAscent(void) const;
		const OcularImage* getImageDescent(void) const;
		const OcularImage* getImageAverage(void) const;
		const OcularImage* getInputFlattened(void) const;
		const OcularImage* getInputDenoised(void) const;

		const std::vector<int>& getInnerEdges() const;
		const std::vector<int>& getOuterEdges() const;
		const std::vector<int>& getInnerBound() const;
		const std::vector<int>& getOuterBound() const;
		const std::vector<int>& getCenterBound() const;
		const std::vector<int>& getInnerBorder() const;
		const std::vector<int>& getOuterBorder() const;
		const std::vector<int>& getInnerLayer() const;
		const std::vector<int>& getOuterLayer() const;

		const std::vector<int>& getBoundaryILM() const;
		const std::vector<int>& getBoundaryNFL() const;
		const std::vector<int>& getBoundaryIPL() const;
		const std::vector<int>& getBoundaryOPL() const;
		const std::vector<int>& getBoundaryIOS() const;
		const std::vector<int>& getBoundaryRPE() const;
		const std::vector<int>& getBoundaryBRM() const;
		const std::vector<int>& getBoundaryOPR() const;

	protected:
		virtual void resetBscanSegmentator() = 0;

		void setBscanSegmentator(BscanSegmentator* segm);
		BscanSegmentator* getBscanSegmentator() const;
		
	private:
		struct LayeredBscanImpl;
		std::unique_ptr<LayeredBscanImpl> d_ptr;
		LayeredBscanImpl& getImpl(void) const;
	};
}


