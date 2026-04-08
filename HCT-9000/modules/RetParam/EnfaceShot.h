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

	class RETPARAM_DLL_API EnfaceShot : public EnfaceData
	{
	public:
		EnfaceShot();
		virtual ~EnfaceShot();

		EnfaceShot(EnfaceShot&& rhs);
		EnfaceShot& operator=(EnfaceShot&& rhs);
		EnfaceShot(const EnfaceShot& rhs);
		EnfaceShot& operator=(const EnfaceShot& rhs);

	public:
		bool copyToImage(CppUtil::CvImage* image, int width, int height);

	private:
		struct EnfaceShotImpl;
		std::unique_ptr<EnfaceShotImpl> d_ptr;
	};
}
