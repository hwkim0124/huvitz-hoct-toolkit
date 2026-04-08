#pragma once

// It could pollute the modules's namespace including this header file, 
// instead use forward declaration for cv::Mat.
// using namespace cv;

namespace cv {
	class Mat;
}

namespace Gdiplus {
	class Bitmap;
	class BitmapData;
}

#include "CppUtilDef.h"

class CPPUTIL_DLL_API CGdiPlus
{
public:
	static void  Init();
	static cv::Mat  ImgRead(const WCHAR* u16_File);
	static void ImgWrite(cv::Mat i_Mat, const WCHAR* u16_File);
	static cv::Mat  CopyBmpToMat(Gdiplus::Bitmap* pi_Bmp);
	static cv::Mat  CopyBmpDataToMat(Gdiplus::BitmapData* pi_Data);
	static Gdiplus::Bitmap* CopyMatToBmp(cv::Mat& i_Mat);

private:
	static CLSID GetEncoderClsid(const WCHAR* u16_File);

	static BOOL mb_InitDone;
};


