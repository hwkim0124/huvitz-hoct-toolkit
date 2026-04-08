#pragma once

#include "SemtSegmDef.h"
#include "Sampling.h"
#include "Bordering.h"
#include "Classifier.h"
#include "Measuring.h"

#include "LayerInner.h"
#include "LayerOuter.h"
#include "LayerILM.h"
#include "LayerNFL.h"
#include "LayerIPL.h"
#include "LayerOPL.h"
#include "LayerONL.h"
#include "LayerIOS.h"
#include "LayerRPE.h"
#include "LayerBRM.h"
#include "LayerOPR.h"

#include "InferenceModel.h"
#include "InferenceInput.h"
#include "InferenceResult.h"


namespace RetSegm {
	class SegmLayer;
	class SegmImage;
}

namespace SemtSegm
{
	class LayeredBscan;
	class Sampling;
	class Bordering;

	class SEMTSEGM_DLL_API BscanSegmentator
	{
	public:
		BscanSegmentator(LayeredBscan* bscan);
		virtual ~BscanSegmentator();

		BscanSegmentator(BscanSegmentator&& rhs);
		BscanSegmentator& operator=(BscanSegmentator&& rhs);
		BscanSegmentator(const BscanSegmentator& rhs) = delete;
		BscanSegmentator& operator=(const BscanSegmentator& rhs) = delete;

	public:
		virtual bool doSegmentation() = 0;

		const LayeredBscan* getLayeredBscan() const;
		const OctScanImage& getImageSource() const ;

		Sampling* getSampling() const;
		Bordering* getBodering() const;
		Classifier* getClassifier() const;
		Measuring* getMeasuring() const;

		LayerInner* getLayerInner() const;
		LayerOuter* getLayerOuter() const;

		LayerILM* getLayerILM() const;
		LayerNFL* getLayerNFL() const;
		LayerIPL* getLayerIPL() const;
		LayerOPL* getLayerOPL() const;
		LayerONL* getLayerONL() const;
		LayerIOS* getLayerIOS() const;
		LayerRPE* getLayerRPE() const;
		LayerBRM* getLayerBRM() const;
		LayerOPR* getLayerOPR() const;

		int getImageIndex() const;
		float getImageRangeX() const;

	protected:
		void resetAlgorithms();
		
		virtual Sampling* createSampling() ;
		virtual Bordering* createBordering() ;
		virtual Classifier* createClassifier();
		virtual Measuring* createMeasuring();

		virtual LayerInner* createLayerInner();
		virtual LayerOuter* createLayerOuter() ;

		virtual LayerILM* createLayerILM();
		virtual LayerNFL* createLayerNFL();
		virtual LayerIPL* createLayerIPL();
		virtual LayerOPL* createLayerOPL();
		virtual LayerONL* createLayerONL();
		virtual LayerIOS* createLayerIOS();
		virtual LayerRPE* createLayerRPE();
		virtual LayerBRM* createLayerBRM();
		virtual LayerOPR* createLayerOPR();

	private:
		struct BscanSegmentatorImpl;
		std::unique_ptr<BscanSegmentatorImpl> d_ptr;
		BscanSegmentatorImpl& getImpl(void) const;
	};
}


