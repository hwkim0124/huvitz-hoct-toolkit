#pragma once

#include "SemtSegmDef.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API InferenceResult
	{
	public:
		InferenceResult();
		InferenceResult(int width, int height, int channels);
		virtual ~InferenceResult();

		InferenceResult(InferenceResult&& rhs);
		InferenceResult& operator=(InferenceResult&& rhs);
		InferenceResult(const InferenceResult& rhs) = delete;
		InferenceResult& operator=(const InferenceResult& rhs) = delete;

	public:
		bool postprocess(const float* outBuffer, int width, int height, int channels, bool maskLabel);
		bool saveImage(int index) const;
		bool exists(void) const;

		const float* getBuffer() const;
		int getWidth() const;
		int getHeight() const;
		int getChannels() const;

	private:
		struct InferenceResultImpl;
		std::unique_ptr<InferenceResultImpl> d_ptr;
		InferenceResultImpl& getImpl(void) const;
	};
}

