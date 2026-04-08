#pragma once

#include "SigChainDef.h"

#include <memory>


namespace SigChain
{
	class SIGCHAIN_DLL_API ImageForm
	{
	public:
		ImageForm();
		virtual ~ImageForm();

	private:
		struct ImageFormImpl;
		static std::unique_ptr<ImageFormImpl> d_ptr;
		static ImageFormImpl& getImpl(void);

	public:
		static bool performFFT(unsigned short* lineBuffer, int ccdPixels, int numOfLines);
		static bool performFFT(float* lineBuffer, int linePixels, int numOfLines);

	private:
		static bool initializeOclSystem(void);
		static bool transform_forward(unsigned short* inputBuffer, int dataLength, int batchSize, 
						float* outputReal, float* outputImag, int* outputLength);

	};
}
