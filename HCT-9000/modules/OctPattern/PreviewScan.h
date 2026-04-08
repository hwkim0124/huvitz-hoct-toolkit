#pragma once

#include "OctPatternDef.h"
#include "PatternScan.h"

#include <memory>


namespace OctPattern
{
	class OCTPATTERN_DLL_API PreviewScan : public PatternScan
	{
	public:
		PreviewScan();
		virtual ~PreviewScan();

		PreviewScan(PreviewScan&& rhs);
		PreviewScan& operator=(PreviewScan&& rhs);
		PreviewScan(const PreviewScan& rhs);
		PreviewScan& operator=(const PreviewScan& rhs);

	public:
		virtual bool buildPattern(bool hidden = false);

	protected:
		virtual void buildPatternPoint(void);
		virtual void buildPatternLine(bool isVert);
		virtual void buildPatternCircle(void);
		virtual void buildPatternCross(void);
		virtual void buildPatternRadial(void);
		virtual void buildPatternRaster(bool isVert);
		virtual void buildPatternCube(bool isVert);

		void buildPreviewPoint(void);
		void buildPreviewHidden(void);
		void buildPreviewLine(bool isVert, int lineCount = 0);
		void buildPreviewCircle(void);
		void buildPreviewCross(void);
		void buildPreviewRadial(void);
		void buildPreviewRaster(bool isVert);
		void buildPreviewCube(bool isVert);

	private:
		struct PreviewScanImpl;
		std::unique_ptr<PreviewScanImpl> d_ptr;
		PreviewScanImpl& getImpl(void) const;
	};
}
