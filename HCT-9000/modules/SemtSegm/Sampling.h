#pragma once

#include "SemtSegmDef.h"

namespace SemtSegm
{
	class SEMTSEGM_DLL_API Sampling
	{
	public:
		Sampling();
		virtual ~Sampling();

		Sampling(Sampling&& rhs);
		Sampling& operator=(Sampling&& rhs);
		Sampling(const Sampling& rhs) = delete;
		Sampling& operator=(const Sampling& rhs) = delete;

	public:
		bool produceSample(OctScanImage image, int width = SAMPLE_WIDTH, int height = SAMPLE_HEIGHT);
	
		virtual bool checkIfValidSample(int index = 0) const;
		virtual bool createGradientMaps(void);

		int getSampleWidth(void) const;
		int getSampleHeight(void) const;
		int getSourceWidth(void) const;
		int getSourceHeight(void) const;
		float getSampleWidthRatio(void) const;
		float getSampleHeightRatio(void) const;

		OcularImage* source(void) const;
		OcularImage* sample(void) const;
		OcularImage* ascent(void) const;
		OcularImage* descent(void) const;
		OcularImage* average(void) const;

	protected:
		virtual bool createAscentGradients(void);
		virtual bool createDescentGraidents(void);
		virtual bool createSampleAveraging(void);

	private:
		struct SamplingImpl;
		std::unique_ptr<SamplingImpl> d_ptr;
		SamplingImpl& getImpl(void) const;
	};
}

