#pragma once

#include "OctAngioDef.h"
#include "AngioChart.h"

#include <memory>

namespace CppUtil {
	class CvImage;
}

namespace OctAngio
{
	class OCTANGIO_DLL_API AngioFAZ : public AngioChart
	{
	public:
		AngioFAZ();
		~AngioFAZ();

	public:
		const std::vector<std::pair<int, int>> getFazPoint(void);
		const std::pair<int, int> getFazCenter(void);
		const std::pair<int, int> getCicleCenter(void);
		const double getCicleRadius(void);
		const double getFazArea(void);
		const double getFazPerimeter(void);
		const float getThreshold(void);
		void setupAngioFAZ(CppUtil::CvImage & image, float fazThreshold = 0.1);
		void setFazPoint(std::vector<std::pair<int, int>> point);
		void setFazCenter(std::pair<int, int> center);
		void setApproxCicle(std::pair<int, int> center, double radius);
		void setFazArea(double area);
		void setFazPerimeter(double perimeter);
		void setThreshold(float Threshold);
		bool preprocessingImage(void);
		bool calculateFAZ(void);

	protected:
		CppUtil::CvImage & getFazImage(void);

	private:
		struct AngioFAZImpl;
		std::unique_ptr<AngioFAZImpl> d_ptr;
		AngioFAZImpl& getImpl(void) const;
	};

}


