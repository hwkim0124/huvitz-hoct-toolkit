#pragma once

#include "OctDeviceDef.h"
#include "OctSampleMotor.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API SplitFocusMotor : public OctSampleMotor
	{
	public:
		SplitFocusMotor();
		SplitFocusMotor(MainBoard *board, StepMotorType type);
		virtual ~SplitFocusMotor();

		SplitFocusMotor(SplitFocusMotor&& rhs);
		SplitFocusMotor& operator=(SplitFocusMotor&& rhs);
		SplitFocusMotor(const SplitFocusMotor& rhs);
		SplitFocusMotor& operator=(const SplitFocusMotor& rhs);

	public:
		virtual bool initialize(void);

		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);

	private:
		struct SplitFocusMotorImpl;
		std::unique_ptr<SplitFocusMotorImpl> d_ptr;
		SplitFocusMotorImpl& getImpl(void) const;
	};
}
