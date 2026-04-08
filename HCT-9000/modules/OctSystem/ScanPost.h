#pragma once

#include "OctSystemDef.h"

#include <memory>

namespace SigChain {
	class OctProcess;
}

namespace OctDevice {
	class MainBoard;
}


namespace OctSystem
{
	class OCTSYSTEM_DLL_API ScanPost
	{
	public:
		ScanPost();
		virtual ~ScanPost();

	public:
		static void initiateScanPost(OctDevice::MainBoard* board, SigChain::OctProcess* process);
		static bool postMeasure(bool result);
		static bool isPosting(void);
		static bool isProcessingCompleted(void);

	protected:
		static bool isInitiated(void);
		static bool startScanPost(void);
		static bool closeScanPost(void);

		static void threadPostFunction(void);
		static bool waitForMeasureProcessed(bool init);
		static bool checkIfMeasureProcessed(void);
		static bool completeMeasureResult(void);

		static SigChain::OctProcess* getProcess(void);

	private:
		struct ScanPostImpl;
		static std::unique_ptr<ScanPostImpl> d_ptr;
		static ScanPostImpl& getImpl(void);

		friend class Scanner;
	};
}

