#pragma once

#include "SegmProcDef.h"
#include "SegmSession.h"

namespace cv {
	class Mat;
}

namespace SegmProc
{
	class SEGMPROC_DLL_API DiscSession : public SegmSession
	{
	public:
		DiscSession();
		virtual ~DiscSession();

	public:
		virtual bool initialize(void) override;
		virtual bool rectify(void) override;

	protected:
		void determineDiscRange(void);
		bool makeBRMImage(OctScanPattern pattern);
		bool elaborateDiscRange(OctScanPattern pattern);
		bool preprocessingEnface(cv::Mat & enfaceImg);
		bool findSmoothContour(cv::Mat& enfaceImg);
		bool modifyDiscPoint(cv::Mat& resizeDisc, int rows, int cols, bool horizontal);
		bool filterOpticDiscSet(void);
		bool adjustOpticDiscSet(void);

		bool isSegmentVersion2(void);
		bool isSemanticVersion2(void);

	private:
		struct DiscSessionImpl;
		static std::unique_ptr<DiscSessionImpl> d_ptr;
		static DiscSessionImpl& getImpl(void);
	};
}
