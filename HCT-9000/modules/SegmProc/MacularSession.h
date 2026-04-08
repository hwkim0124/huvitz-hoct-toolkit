#pragma once

#include "SegmProcDef.h"
#include "SegmSession.h"


namespace SegmProc
{
	class SEGMPROC_DLL_API MacularSession : public SegmSession
	{
	public:
		MacularSession();
		virtual ~MacularSession();

	public:
		virtual bool initialize(void);

	private:
		struct MacularSessionImpl;
		static std::unique_ptr<MacularSessionImpl> d_ptr;
		static MacularSessionImpl& getImpl(void);
	};
}

