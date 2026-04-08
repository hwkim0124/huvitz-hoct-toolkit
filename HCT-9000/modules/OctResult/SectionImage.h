#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>
#include <string>

namespace OctResult
{
	class OCTRESULT_DLL_API SectionImage
	{
	public:
		SectionImage();
		SectionImage(OctScanImage desc);
		SectionImage(std::uint8_t* data, int width, int height, float quality = 0.0f, float sigRatio = 0.0f, int refPoint = 0);
		virtual ~SectionImage();

		SectionImage(SectionImage&& rhs);
		SectionImage& operator=(SectionImage&& rhs);
		SectionImage(const SectionImage& rhs);
		SectionImage& operator=(const SectionImage& rhs);

	public:
		void setData(const std::uint8_t* data, int width, int height, float quality = 0.0f, float sigRatio = 0.0f, int refPoint = 0);
		const std::uint8_t* getData(void) const;
		OctScanImage& getDescript(void) const;

		bool isEmpty(void) const;
		int getWidth(void) const;
		int getHeight(void) const;

		float getQualityIndex(void) const;
		float getSignalRatio(void) const;
		int getReferencePoint(void) const;

		void setQualityIndex(float value);
		void setSignalRatio(float value);
		void setReferencePoint(int value);

		bool exportImage(std::wstring path);

	private:
		struct SectionImageImpl;
		std::unique_ptr<SectionImageImpl> d_ptr;
		SectionImageImpl& getImpl(void) const;
	};

	typedef std::vector<SectionImage> SectionImageList;
}

