#pragma once

#include "SemtSegmDef.h"


namespace SemtSegm
{
	class InferenceInput;
	class InferenceResult;

	class SEMTSEGM_DLL_API InferenceModel
	{
	public:
		InferenceModel();
		virtual ~InferenceModel();

	public:
		static bool initializeNetwork();
		static bool isInitialized();
		static bool requestPrediction(const OcularImage* image = nullptr);
		static bool requestPrediction(const InferenceInput& input, InferenceResult& result, bool maskLabel);

	protected:
		
	protected:
		struct InferenceModelImpl;
		static std::unique_ptr<InferenceModelImpl> d_ptr;
		static InferenceModelImpl& getImpl(void);
	};
}

