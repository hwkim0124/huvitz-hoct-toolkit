#pragma once

#include "SemtRetinaDef.h"

namespace SemtRetina
{
	class RetinaSegmenter;

	class SEMTRETINA_DLL_API ONHMorphometrics
	{
	public:
		ONHMorphometrics(RetinaSegmenter* segm);
		virtual ~ONHMorphometrics();

		ONHMorphometrics(ONHMorphometrics&& rhs);
		ONHMorphometrics& operator=(ONHMorphometrics&& rhs);
		ONHMorphometrics(const ONHMorphometrics& rhs) = delete;
		ONHMorphometrics& operator=(const ONHMorphometrics& rhs) = delete;

	public:
		bool accumulateDiscRimVoxels(void);
		bool accumulateDiscCupVoxels(void);

		int discMarginMinX(void) const;
		int discMarginMaxX(void) const;
		int cupOpeningMinX(void) const;
		int cupOpeningMaxX(void) const;
		int discRimVoxelCount(void) const;
		int discCupVoxelCount(void) const;

		bool isDiscMarginRangeValid(void) const;
		bool isCupOpeningRangeValid(void) const;
		bool hasDiscRimMeasurements(void) const;
		bool hasDiscCupMeasurements(void) const;

	protected:

	private:
		struct ONHMorphometricsImpl;
		std::unique_ptr<ONHMorphometricsImpl> d_ptr;
		ONHMorphometricsImpl& impl(void) const;
	};
}