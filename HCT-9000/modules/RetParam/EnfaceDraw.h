#pragma once

#include "RetParamDef.h"

#include <memory>
#include <vector>


namespace RetSegm {
	class SegmImage;
	class SegmLayer;
}


namespace CppUtil {
	class CvImage;
}


namespace Gdiplus {
	class Bitmap;
}


namespace RetParam 
{
	class RETPARAM_DLL_API EnfaceDraw
	{
	public:
		EnfaceDraw();
		virtual ~EnfaceDraw();

		EnfaceDraw(EnfaceDraw&& rhs);
		EnfaceDraw& operator=(EnfaceDraw&& rhs);
		EnfaceDraw(const EnfaceDraw& rhs);
		EnfaceDraw& operator=(const EnfaceDraw& rhs);

	private:
		struct EnfaceDrawImpl;
		std::unique_ptr<EnfaceDrawImpl> d_ptr;

	protected:
		bool checkIfValidAllBscans(void) ;
		bool checkIfValidAllLayers(void) ;

		virtual void buildPlate(void);
		virtual void copyToImage(CppUtil::CvImage* image, int width, int height);

		void equalizeHistogram(CppUtil::CvImage* image);

		int getPlateWidth(void) const;
		int getPlateHeight(void) const;
		int getBscanWidth(void) const;
		int getBscanHeight(void) const;
		float getScanWidth(void) const;
		float getScanHeight(void) const;
		float getHorzPixelPerMM(void) const;
		float getVertPixelPerMM(void) const;

		RetSegm::SegmLayer* getUpperLayer(int index) const;
		RetSegm::SegmLayer* getLowerLayer(int index) const;
		CppUtil::CvImage* getPlate(void) const;

	public:
		bool initialize(const std::vector<RetSegm::SegmImage*> bscans, const std::vector<RetSegm::SegmLayer*> layers1, 
						const std::vector<RetSegm::SegmLayer*> layers2, float scanWidth, float scanHeight);
		bool drawImage(CppUtil::CvImage* image, int width, int height);
	};

	typedef std::vector<EnfaceDraw> EnfaceDrawVect;

}

