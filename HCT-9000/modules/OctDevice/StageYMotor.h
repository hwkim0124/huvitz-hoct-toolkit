#pragma once

#include "OctDeviceDef.h"
#include "StageMotor.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API StageYMotor : public StageMotor
	{
	public:
		StageYMotor();
		StageYMotor(MainBoard *board);
		virtual ~StageYMotor();

	private:
		struct StageYMotorImpl;
		std::unique_ptr<StageYMotorImpl> d_ptr;
		StageYMotorImpl& getImpl(void) const;
	};
}

