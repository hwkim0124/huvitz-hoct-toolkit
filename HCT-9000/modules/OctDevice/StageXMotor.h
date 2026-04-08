#pragma once

#include "OctDeviceDef.h"
#include "StageMotor.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API StageXMotor : public StageMotor
	{
	public:
		StageXMotor();
		StageXMotor(MainBoard *board);
		virtual ~StageXMotor();

	public:
		bool initialize(void) override;

	private:
		struct StageXMotorImpl;
		std::unique_ptr<StageXMotorImpl> d_ptr;
		StageXMotorImpl& getImpl(void) const;
	};
}
