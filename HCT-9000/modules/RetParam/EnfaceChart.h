#pragma once

#include "RetParamDef.h"
#include "EnfaceData.h"

#include <memory>
#include <vector>


namespace CppUtil {
	class CvImage;
}


namespace RetParam
{
	class RETPARAM_DLL_API EnfaceChart
	{
	public:
		EnfaceChart();
		virtual ~EnfaceChart();

		EnfaceChart(EnfaceChart&& rhs);
		EnfaceChart& operator=(EnfaceChart&& rhs);
		EnfaceChart(const EnfaceChart& rhs);
		EnfaceChart& operator=(const EnfaceChart& rhs);

	public:
		void setChartPosition(float xmm, float ymm);
		void setEyeSide(EyeSide side);
		void setEnfaceData(const EnfaceData * data);

		EyeSide eyeSide(void) const;
		float positionX(void) const;
		float positionY(void) const;
		const EnfaceData* enfaceData(void);

		bool isEmpty(void) const;
		
		virtual bool updateContent(void);
		virtual bool updateContent(const std::vector<float>& thicks);

	private:
		struct EnfaceChartImpl;
		std::unique_ptr<EnfaceChartImpl> d_ptr;
	};
}

