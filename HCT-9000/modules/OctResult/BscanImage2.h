#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>
#include <string>


namespace CppUtil {
	class CvImage;
}


namespace OctResult
{
	class OCTRESULT_DLL_API BscanImage2
	{
	public:
		BscanImage2();
		BscanImage2(const OctScanImage& desc);
		BscanImage2(std::uint8_t* data, int width, int height, float quality = 0.0f, float sigRatio = 0.0f, int refPoint = 0);
		virtual ~BscanImage2();

		BscanImage2(BscanImage2&& rhs);
		BscanImage2& operator=(BscanImage2&& rhs);
		BscanImage2(const BscanImage2& rhs);
		BscanImage2& operator=(const BscanImage2& rhs);

	public:
		void setData(const std::uint8_t* data, int width, int height, float quality = 0.0f, float sigRatio = 0.0f, int refPoint = 0, bool reverse=false);
		void setData(const OctScanImage& desc);
		bool setImage(CppUtil::CvImage* image);
		
		std::uint8_t* getBuffer(void) const;
		const OctScanImage& getDescript(void) const;
		CppUtil::CvImage& getImage(void) const;
		std::wstring getFileName(bool path = false) const;

		bool isEmpty(void) const;
		int getWidth(void) const;
		int getHeight(void) const;

		float getQualityIndex(void) const;
		float getSignalRatio(void) const;
		int getReferencePoint(void) const;

		void setQualityIndex(float value);
		void setSignalRatio(float value);
		void setReferencePoint(int value);

		bool updateFile(void);
		bool exportFile(const std::wstring& path);
		bool importFile(const std::wstring& path);
		void flipVert(void);

	private:
		struct BscanImage2Impl;
		std::unique_ptr<BscanImage2Impl> d_ptr;
		BscanImage2Impl& getImpl(void) const;
	};

	typedef std::vector<BscanImage2> BscanImage2List;
}

