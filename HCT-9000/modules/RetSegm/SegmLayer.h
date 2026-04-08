#pragma once

#include "RetSegmDef.h"
#include "SegmPoint.h"

#include <memory>
#include <vector>


namespace cv {
	class Mat;
}


namespace RetSegm
{
	class RETSEGM_DLL_API SegmLayer
	{
	public:
		SegmLayer();
		SegmLayer(int width, int height);
		virtual ~SegmLayer();

		static const int REGION_INIT_WIDTH = 1024;
		static const int REGION_INIT_HEIGHT = 1024;

		SegmLayer(SegmLayer&& rhs);
		SegmLayer& operator=(SegmLayer&& rhs);
		SegmLayer(const SegmLayer& rhs);
		SegmLayer& operator=(const SegmLayer& rhs);

	private:
		struct SegmLayerImpl;
		std::unique_ptr<SegmLayerImpl> d_ptr;

	public:
		std::vector<int> getXs(bool pickValids = false) const;
		std::vector<int> getYs(bool pickValids = false) const;
		std::vector<int> getYsResized(int width, int height) const;

		LayerPointList getPointsResized(int width, int height) const;
		LayerPointList getPointsValid(void) const;
		LayerPointList& getPoints(void) const;
		std::vector<int>& getDeltas(void) const;

		void setValid(int index, bool flag);
		bool isValid(int index);
		int getY(int index) const;
		void setY(int index, int y);

		void setPoint(int index, int x, int y);
		void setPoint(int index, int x, int y, bool valid);
		void setPoint(int index, const LayerPoint& point);
		void setPoints(const std::vector<int>& ys);

		void initialize(const std::vector<int>& vect, int width = REGION_INIT_WIDTH, int height = REGION_INIT_HEIGHT);
		void initialize(const LayerPointList& vect, int width = REGION_INIT_WIDTH, int height = REGION_INIT_HEIGHT);
		void initialize(const cv::Mat& cvMat, int width = REGION_INIT_WIDTH, int height = REGION_INIT_HEIGHT);

		void setRegionSize(int width, int height);
		int getRegionWidth(void) const;
		int getRegionHeight(void) const;

		bool resize(int targetWidth, int targetHeight);
		int getSize(void) const;
		bool isEmpty(void) const;
		void clear(int size = 0);

		// This is restricted to the classes only for the purpose of data processing to Boundary. 
		friend class Coarse;
		friend class OptimalPath;
	};

}