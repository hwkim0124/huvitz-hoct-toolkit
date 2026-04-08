#pragma once

#include "OctDeviceDef.h"
#include "ReferenceMotor.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API OctCompensationMotor : public ReferenceMotor
	{
	public:
		OctCompensationMotor();
		OctCompensationMotor(MainBoard *board, StepMotorType type);
		virtual ~OctCompensationMotor();

		OctCompensationMotor(OctCompensationMotor&& rhs);
		OctCompensationMotor& operator=(OctCompensationMotor&& rhs);
		OctCompensationMotor(const OctCompensationMotor& rhs);
		OctCompensationMotor& operator=(const OctCompensationMotor& rhs);

	public:
		virtual bool initialize(void);

	private:
		struct OctCompensationMotorImpl;
		std::unique_ptr<OctCompensationMotorImpl> d_ptr;
		OctCompensationMotorImpl& getImpl(void) const;
	};
}
