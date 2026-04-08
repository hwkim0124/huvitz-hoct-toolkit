#pragma once


#include "SigChainDef.h"

#include <memory>


namespace SigChain
{
	class OclContext;

	class SIGCHAIN_DLL_API OclSystem2
	{
	public:
		OclSystem2();
		virtual ~OclSystem2();

	public:
		static bool initializeOclSystem(void);
		static bool isInitiated(void);
		static void releaseOclSystem(void);

		static bool initializeTransform(void);
		static bool prepareDefaultTransforms(void);

		static bool prepareTransformForPreview(int dataWidth, int dataHeight);
		static bool prepareTransformForEnface(int dataWidth, int dataHeight);
		static bool prepareTransformForMeasure(int dataWidth, int dataHeight);

		static bool executeTransformForPreview(const std::uint16_t* input, int dataWidth, int dataHeight,
			std::uint8_t* output, int* imageWidth, int* imageHeight,
			float* imageQindex, float* snrRatio, int* referPoint, float* amplitudes = nullptr);
		static bool executeTransformForEnface(const std::uint16_t* input, int dataWidth, int dataHeight,
			std::uint8_t* output, int* imageWidth, int* imageHeight,
			float* imageQindex, float* snrRatio, int* referPoint, std::uint8_t* laterals);
		static bool executeTransformForMeasure(const std::uint16_t* input, int dataWidth, int dataHeight,
			std::uint8_t* output, int* imageWidth, int* imageHeight,
			float* imageQindex, float* snrRatio, int* referPoint, std::uint8_t* laterals = nullptr, float* amplitudes = nullptr);

	protected:
		static bool executeTransform(const std::uint16_t* input, int dataWidth, int dataHeight,
			std::uint8_t* output, int* imageWidth, int* imageHeight,
			float* imageQindex, float* snrRatio, int* referPoint, std::uint8_t* laterals = nullptr, float* amplitudes = nullptr);
		static OclContext& getContext(void);

		static bool prepareDefaultParameters(void);
		static void checkBackgroundSpectrum(const unsigned short * data, int width, int height, float quality);

	private:
		struct OclSystem2Impl;
		static std::unique_ptr<OclSystem2Impl> d_ptr;
		static OclSystem2Impl& getImpl(void);
	};
}

