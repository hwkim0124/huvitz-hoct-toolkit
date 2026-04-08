#pragma once

#include "OctSystemDef.h"

#include <memory>


namespace OctSystem
{
	class OCTSYSTEM_DLL_API CorneaTrack
	{
	public:
		CorneaTrack();
		virtual ~CorneaTrack();

	public:
		static bool getCorneaCenterOffsetX(float& offsetX, int idxOfImage);

	private:
		struct CorneaTrackImpl;
		static std::unique_ptr<CorneaTrackImpl> d_ptr;
		static CorneaTrackImpl& getImpl(void);
	};
}

