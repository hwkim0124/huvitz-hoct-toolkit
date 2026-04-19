#pragma once

#include "SemtRetinaDef.h"
#include "BscanResampler.h"
#include "RetinaBandExtractor.h"
#include "RetinaInferPipeline.h"
#include "RetinaSegmCriteria.h"
#include "ONHMorphometrics.h"


#include "BoundaryILM.h"
#include "BoundaryNFL.h"
#include "BoundaryIPL.h"
#include "BoundaryOPL.h"
#include "BoundaryONL.h"
#include "BoundaryIOS.h"
#include "BoundaryRPE.h"
#include "BoundaryBRM.h"
#include "BoundaryINN.h"
#include "BoundaryOUT.h"


namespace SemtRetina
{
	class RetinaSegmFrame;
	class RetinaBandExtractor;
	class RetinaInferPipeline;
	class RetinaSegmCriteria;
	class BscanResampler;
	class ONHMorphometrics;

	class SEMTRETINA_DLL_API RetinaSegmenter
	{
	public:
		RetinaSegmenter(RetinaSegmFrame* frame);
		virtual ~RetinaSegmenter();

		RetinaSegmenter(RetinaSegmenter&& rhs);
		RetinaSegmenter& operator=(RetinaSegmenter&& rhs);
		RetinaSegmenter(const RetinaSegmenter& rhs) = delete;
		RetinaSegmenter& operator=(const RetinaSegmenter& rhs) = delete;

	public:
		virtual bool segment(bool angio) = 0;

		const RetinaSegmFrame* retinaSegmFrame(void) const;
		BscanResampler* bscanResampler(void) const;
		RetinaBandExtractor* retinaBandExtractor(void) const;
		RetinaInferPipeline* retinaInferPipeline(void) const;
		RetinaSegmCriteria* retinaSegmCriteria(void) const;
		ONHMorphometrics* onhMorphometrics(void) const;

		BoundaryILM* boundaryILM(void) const;
		BoundaryNFL* boundaryNFL(void) const;
		BoundaryIPL* boundaryIPL(void) const;
		BoundaryOPL* boundaryOPL(void) const;
		BoundaryONL* boundaryONL(void) const;
		BoundaryIOS* boundaryIOS(void) const;
		BoundaryRPE* boundaryRPE(void) const;
		BoundaryBRM* boundaryBRM(void) const;

		BoundaryINN* boundaryINN(void) const;
		BoundaryOUT* boundaryOUT(void) const;

	protected:
		virtual void resetAlgorithms() = 0;

		void setBscanResampler(BscanResampler* p);
		void setRetinaBandExtractor(RetinaBandExtractor* p);
		void setRetinaInferPipeline(RetinaInferPipeline* p);
		void setRetinaSegmCriteria(RetinaSegmCriteria* p);
		void setONHMorphometrics(ONHMorphometrics* p);

		void setBoundaryILM(BoundaryILM* p);
		void setBoundaryNFL(BoundaryNFL* p);
		void setBoundaryIPL(BoundaryIPL* p);
		void setBoundaryOPL(BoundaryOPL* p);
		void setBoundaryONL(BoundaryONL* p);
		void setBoundaryIOS(BoundaryIOS* p);
		void setBoundaryRPE(BoundaryRPE* p);
		void setBoundaryBRM(BoundaryBRM* p);
		void setBoundaryINN(BoundaryINN* p);
		void setBoundaryOUT(BoundaryOUT* p);

	private:
		struct RetinaSegmenterImpl;
		std::unique_ptr<RetinaSegmenterImpl> d_ptr;
		RetinaSegmenterImpl& impl(void) const;
	};
}
