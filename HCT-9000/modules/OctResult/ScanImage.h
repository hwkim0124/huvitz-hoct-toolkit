#pragma once

#include "OctResultDef.h"

#include <memory>


namespace OctResult
{
	class OCTRESULT_DLL_API ScanImage
	{
	public:
		ScanImage();
		virtual ~ScanImage();

		ScanImage(ScanImage&& rhs);
		ScanImage& operator=(ScanImage&& rhs);
		ScanImage(const ScanImage& rhs);
		ScanImage& operator=(const ScanImage& rhs);

	public:
		void copyData(unsigned char* data, int width, int height);
		void set_quality(float quality);
		void set_referPoint(int refPoint);
		void set_path(float x1, float y1, float x2, float y2);
		void set_path(float x, float y, float radius);
		void clear(void);

		unsigned char* data(void) const;
		int width(void) const;
		int height(void) const;
		float quality(void) const;
		int referPoint(void) const;
		bool empty(void) const;

	private:
		struct ScanImageImpl;
		std::unique_ptr<ScanImageImpl> d_ptr;
		ScanImageImpl& getImpl(void) const;
	};
}
