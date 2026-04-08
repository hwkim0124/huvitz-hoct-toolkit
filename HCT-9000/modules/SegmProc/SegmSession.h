#pragma once

#include "SegmProcDef.h"

#include <memory>
#include <vector>


namespace OctData {
	class ProtocolData;
	class ProtocolDataset;

	class LineScanData;
	class CubeScanData;
	class CircleScanData;
	class CrossScanData;
	class RadialScanData;
	class RasterScanData;
}

namespace SegmScan {
	class OcularBsegm;
}


namespace SegmProc
{
	class SEGMPROC_DLL_API SegmSession
	{
	public:
		SegmSession();
		virtual ~SegmSession();

	public:
		void loadScanData(OctData::ProtocolData* data);
		bool processScanData(void);

	protected:
		virtual bool initialize(void);
		virtual bool execute(void);
		virtual bool rectify(void);
		virtual bool finalize(void);

		OctData::ProtocolData* getScanData(void) const;

		int countOcularBsegms(void) const;
		void clearOcularBsegms(void);
		void addOcularBsegm(std::unique_ptr<SegmScan::OcularBsegm> bsegm);
		SegmScan::OcularBsegm* getOcularBsegm(int index) const;

		int countPreviewBsegms(void) const;
		void clearPreviewBsegms(void);
		void addPreviewBsegm(std::unique_ptr<SegmScan::OcularBsegm> bsegm);
		SegmScan::OcularBsegm* getPreviewBsegm(int index) const;

	private:
		struct SegmSessionImpl;
		static std::unique_ptr<SegmSessionImpl> d_ptr;
		static SegmSessionImpl& getImpl(void);
	};
}

