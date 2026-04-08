#pragma once

#include "OctAngioDef.h"

#include <memory>

namespace CppUtil {
	class CvImage;
}

namespace OctAngio
{
	class OCTANGIO_DLL_API AngioChart
	{
	public:
		AngioChart();
		virtual ~AngioChart();

		AngioChart(AngioChart&& rhs);
		AngioChart& operator=(AngioChart&& rhs);
		AngioChart(const AngioChart& rhs);
		AngioChart& operator=(const AngioChart& rhs);

	public:
		virtual void setupAngioChart(CppUtil::CvImage& image, float rangeX, float rangeY, int imageW, int imageH);
		virtual void setCenterPosition(float xmm, float ymm);
		virtual bool updateAngioChart(void);
		
		void setThreshold(float threshold);
		float getThreshold(void) const;

		int centerPosX(void) const;
		int centerPosY(void) const;
		float pixelsPerXmm(void) const;
		float pixelsPerYmm(void) const;

	protected:
		CppUtil::CvImage& getAngioImage(void);

	private:
		struct AngioChartImpl;
		std::unique_ptr<AngioChartImpl> d_ptr;
		AngioChartImpl& getImpl(void) const;
	};
}

