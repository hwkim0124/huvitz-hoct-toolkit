#pragma once

#include "RetParamDef.h"
#include "EnfaceDraw.h"

#include <memory>
#include <vector>


namespace RetParam
{
	class RETPARAM_DLL_API EnfacePlot : public EnfaceDraw
	{
	public:
		EnfacePlot();
		virtual ~EnfacePlot();

		EnfacePlot(EnfacePlot&& rhs);
		EnfacePlot& operator=(EnfacePlot&& rhs);
		EnfacePlot(const EnfacePlot& rhs);
		EnfacePlot& operator=(const EnfacePlot& rhs);

	protected:
		struct EnfacePlotImpl;
		std::unique_ptr<EnfacePlotImpl> d_ptr;

		virtual void buildPlate(void);
		virtual void copyToImage(CppUtil::CvImage* image, int width, int height);

		void updateSectionThickness(void);

	public:
		float getMeanCenter(void) const;
		float getMeanInner(int index) const;
		float getMeanOuter(int index) const;
		float getMeanTotal(void) const;
		float getMeanSuperior(void) const;
		float getMeanInferior(void) const;
	};

	typedef std::vector<EnfacePlot> EnfacePlotVect;
}
