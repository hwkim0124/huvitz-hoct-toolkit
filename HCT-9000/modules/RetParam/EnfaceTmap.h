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
	class RETPARAM_DLL_API EnfaceTmap : public EnfaceData
	{
	public:
		EnfaceTmap();
		virtual ~EnfaceTmap();

		EnfaceTmap(EnfaceTmap&& rhs);
		EnfaceTmap& operator=(EnfaceTmap&& rhs);
		EnfaceTmap(const EnfaceTmap& rhs);
		EnfaceTmap& operator=(const EnfaceTmap& rhs);

	public:
		bool copyToImage(CppUtil::CvImage* image, int width, int height);

	private:
		struct EnfaceTmapImpl;
		std::unique_ptr<EnfaceTmapImpl> d_ptr;
	};
}

