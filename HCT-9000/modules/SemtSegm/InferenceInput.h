#pragma once

#include "SemtSegmDef.h"


namespace InferenceEngine {
	class Blob;
}

namespace SemtSegm
{
	class InferenceResult;

	class SEMTSEGM_DLL_API InferenceInput
	{
	public:
		InferenceInput();
		virtual ~InferenceInput();

		InferenceInput(InferenceInput&& rhs);
		InferenceInput& operator=(InferenceInput&& rhs);
		InferenceInput(const InferenceInput& rhs) = delete;
		InferenceInput& operator=(const InferenceInput& rhs) = delete;

	public:
		bool setInputImage(const OcularImage* image);
		bool requestPrediction(bool maskLabel);
		const InferenceResult& getPredictionResult() const;
		bool saveImage(int index) const;

	protected:
		bool wrapImageToBlobFp32();
		std::shared_ptr<InferenceEngine::Blob> getInputBlob() const;

	protected:
		struct InferenceInputImpl;
		std::unique_ptr<InferenceInputImpl> d_ptr;
		InferenceInputImpl& getImpl(void) const;

		friend class InferenceModel;
	};
}

