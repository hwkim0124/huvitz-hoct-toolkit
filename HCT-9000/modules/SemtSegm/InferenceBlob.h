#pragma once

#include "SemtSegmDef.h"

#include <inference_engine.hpp>


namespace SemtSegm
{
	class InferenceBlob
	{
	public:
		InferenceBlob(const OcularImage* image);
		virtual ~InferenceBlob();

		InferenceBlob(InferenceBlob&& rhs);
		InferenceBlob& operator=(InferenceBlob&& rhs);
		InferenceBlob(const InferenceBlob& rhs) = delete;
		InferenceBlob& operator=(const InferenceBlob& rhs) = delete;

	public:
		InferenceEngine::Blob::Ptr wrapImageToBlobFp32() const;

	private:
		struct InferenceBlobImpl;
		std::unique_ptr<InferenceBlobImpl> d_ptr;
		InferenceBlobImpl& getImpl(void) const;
	};
}

