#pragma once

#include "RetFocusDef.h"

#include <memory>
#include <string>

namespace OctDevice
{
	class MainBoard;
}


namespace RetFocus
{
	class RETFOCUS_DLL_API ScanOptimizer
	{
	public:
		ScanOptimizer();
		virtual ~ScanOptimizer();

	public:

	protected:
		static bool obtainQualityIndexFromPreview(float& qidx, float& sig_ratio, bool next = true);
		static bool obtainReferencePointFromPreview(int& refPoint, bool next = true);
		static bool obtainSplitFocusFromRetina(float& offset, bool next = true);

		static bool isQualityToSignal(float qidx);
		static bool isQualityToComplete(float qidx);
		static bool isQualityToTarget(float qidx, bool isLensBack = false);
		static bool isQualityToConfirm(float qidx);

		static void onScanOptimizingStarted(void);
		static void onScanOptimizingClosed(void);

	private:
		struct ScanOptimizerImpl;
		static std::unique_ptr<ScanOptimizerImpl> d_ptr;
		static ScanOptimizerImpl& getImpl(void);
	};
}

