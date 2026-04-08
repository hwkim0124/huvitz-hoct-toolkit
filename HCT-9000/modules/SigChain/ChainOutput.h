#pragma once

#include "SigChainDef.h"

#include <memory>


namespace SigChain
{
	class SIGCHAIN_DLL_API ChainOutput
	{
	public:
		ChainOutput();
		virtual ~ChainOutput();

	public:
		static void clearResultOfLastImage(void);
		static void setPreviewImageResult(float qidx, float sig_ratio, int refPoint);
		static bool getPreviewImageResult(float* qidx=nullptr, float* sig_ratio=nullptr, int* refPoint=nullptr, unsigned long* count=nullptr);

		static void makeAxialProjectionOfRetina(uint8_t* image, uint8_t* lateral, int width, int height);
		static void setLateralLineOfEnface(uint8_t* buffer, int lineIdx, int lineSize, int numLines, bool vertical = false, bool reverse = false);
		static void setEnfaceImageResult(int width, int height);
		static std::uint8_t* getEnfaceImageBuffer(void);
		static void clearEnfaceImageBuffer(void);
		static int getEnfaceImageWidth(void);
		static int getEnfaceImageHeight(void);

	protected:
		struct ChainOutputImpl;
		static std::unique_ptr<ChainOutputImpl> d_ptr;
		static ChainOutputImpl& getImpl(void);
	};
}
