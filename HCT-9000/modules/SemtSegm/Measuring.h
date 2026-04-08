#pragma once

#include "SemtSegmDef.h"

namespace SemtSegm
{
	class BscanSegmentator;

	class SEMTSEGM_DLL_API Measuring
	{
	public:
		Measuring(BscanSegmentator* pSegm);
		virtual ~Measuring();

		Measuring(Measuring&& rhs);
		Measuring& operator=(Measuring&& rhs);
		Measuring(const Measuring& rhs) = delete;
		Measuring& operator=(const Measuring& rhs) = delete;

	public:
		bool findOpticDiscSideParams() const;
		bool findOpticDiscCupParams() const;

		int getOpticDiscX1() const;
		int getOpticDiscX2() const;
		int getOpticDiscPixels() const;
		int getOpticCupX1() const;
		int getOpticCupX2() const;
		int getOpticCupPixels() const;

		bool isOpticDiscRegion() const;
		bool isOpticCupRegion() const;

	private:
		struct MeasuringImpl;
		std::unique_ptr<MeasuringImpl> d_ptr;
		MeasuringImpl& getImpl(void) const;
	};
}

