#pragma once

#include "RetSegmDef.h"

#include <memory>
#include <vector>


namespace RetSegm
{
	struct RETSEGM_DLL_API EdgePoint
	{
	public:
		EdgePoint(void) {
			m_xPos = m_yPos = -1;
			m_valid = false;
		}

		EdgePoint(int x, int y) {
			m_xPos = x; m_yPos = y;
			m_valid = (m_yPos >= 0 ? true : false) ;
		}

		int getX(void) const { return m_xPos; };
		int getY(void) const { return m_yPos; };
		bool isValid(void) const { return m_valid; };
		void setValid(bool valid) { m_valid = valid; m_yPos = (valid ? m_yPos : -1); };
		void setValue(int x, int y, bool valid) { m_xPos = x; m_yPos = y; m_valid = valid; };
		void setValue(int y) { m_yPos = y; };

	private:
		int m_xPos;
		int m_yPos;
		bool m_valid;
	};


	class RETSEGM_DLL_API Boundary
	{
	public:
		Boundary();

		static const int REGION_WIDTH = 2048;
		static const int REGION_HEIGHT = 2048;

	private:
		std::vector<EdgePoint> m_points;
		std::vector<int> m_deltas;
		int m_regionWidth;
		int m_regionHeight;
		
	public:
		int getPointY(int x) const;
		std::vector<int> getPointXs(bool pickValids=false) const;
		std::vector<int> getPointYs(bool pickValids=false) const;
		std::vector<EdgePoint> getPoints(bool pickValids = false) const;

		void setInvalid(int idx);
		void setPoint(int idx, int x, int y, bool valid = true);
		void setPoint(int idx, EdgePoint edge);
		void createPoints(const std::vector<int>& vect, int width= REGION_WIDTH, int height= REGION_HEIGHT);
		void createPoints(const std::vector<EdgePoint>& vect, int width = REGION_WIDTH, int height = REGION_HEIGHT);
		void createPoints(const cv::Mat& cvMat, int width = REGION_WIDTH, int height = REGION_HEIGHT);

		void setRegionSize(int width, int height);
		int getRegionWidth(void) const;
		int getRegionHeight(void) const;

		bool resize(int targetWidth, int targetHeight);
		int getSize(void) const;
		bool isEmpty(void) const;
		void clear(int size=0);

		// This is restricted to the classes only for the purpose of data processing to Boundary. 
		friend class Coarse;
		friend class OptimalPath;
	};
}
