#pragma once

#include "SegmScanDef.h"

#include <memory>
#include <string>
#include <vector>


// Implementation class for hiding OpenCv. 
// Forward declaration of the class from another namespace should be like below, 
// since the complier can't confirm the existence of the namespace before the class. 
// Refer: http://stackoverflow.com/questions/2059665/why-cant-i-forward-declare-a-class-in-a-namespace-like-this
namespace RetSegm {
	class SegmImage;
	class SegmLayer;
}


namespace Gdiplus {
	class Bitmap;
}


namespace SegmScan
{
	class SEGMSCAN_DLL_API BscanImage
	{
	public:
		BscanImage();
		BscanImage(const std::wstring& path);
		virtual ~BscanImage();

		BscanImage(BscanImage&& rhs);
		BscanImage& operator=(BscanImage&& rhs);
		BscanImage(const BscanImage& rhs);
		BscanImage& operator=(const BscanImage& rhs);


	private:
		struct BscanImageImpl;
		std::unique_ptr<BscanImageImpl> d_ptr;

		RetSegm::SegmImage* resized(void) const;
		RetSegm::SegmImage* sample(void) const;
		RetSegm::SegmImage* smoothed(void) const;
		RetSegm::SegmImage* source(void) const;
		RetSegm::SegmImage* gradients(void) const;
		RetSegm::SegmImage* costs(void) const;

		RetSegm::SegmLayer* layerInn(void) const;
		RetSegm::SegmLayer* layerOut(void) const;
		RetSegm::SegmLayer* layerILM(void) const;
		RetSegm::SegmLayer* layerIOS(void) const;
		RetSegm::SegmLayer* layerRPE(void) const;


	public:
		bool loadFile(const std::wstring& path);
		bool loadBitsData(const unsigned char* pBits, int width, int height, int padding = 0);
		bool isEmpty(void) const;
		bool performAnalysis(void);

		Gdiplus::Bitmap* getBitmap(ImageType type = ImageType::SOURCE);
		int getWidth(ImageType type = ImageType::SOURCE) const;
		int getHeight(ImageType type = ImageType::SOURCE) const;
		const unsigned char* getBitsData(ImageType type = ImageType::SOURCE) const;

		std::vector<int> getLayerPoints(LayerType type, ImageType image = ImageType::SOURCE) const;
		std::vector<int> getLayerPoints(LayerType type, int width, int height) const;

		std::wstring getFilePath(void) const;
		std::wstring getFileName(void) const;

		RetSegm::SegmImage* getSegmImage(ImageType type = ImageType::SOURCE) const;
		RetSegm::SegmLayer* getSegmLayer(LayerType type) const;
	};

	typedef std::vector<BscanImage> BscanImageVect;
}
