#pragma once

#include "OctDeviceDef.h"
#include "StageMotor.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API StageZMotor : public StageMotor
	{
	public:
		StageZMotor();
		StageZMotor(MainBoard *board);
		virtual ~StageZMotor();

	public:
		bool initialize(void) override;

	private:
		struct StageZMotorImpl;
		std::unique_ptr<StageZMotorImpl> d_ptr;
		StageZMotorImpl& getImpl(void) const;
	};
}
