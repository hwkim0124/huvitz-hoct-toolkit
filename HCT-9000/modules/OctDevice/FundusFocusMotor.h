#pragma once

#include "OctDeviceDef.h"
#include "OctFocusMotor.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API FundusFocusMotor : public OctFocusMotor
	{
	public:
		FundusFocusMotor();
		FundusFocusMotor(MainBoard *board, StepMotorType type);
		virtual ~FundusFocusMotor();

		FundusFocusMotor(FundusFocusMotor&& rhs);
		FundusFocusMotor& operator=(FundusFocusMotor&& rhs);
		FundusFocusMotor(const FundusFocusMotor& rhs) ;
		FundusFocusMotor& operator=(const FundusFocusMotor& rhs) ;

	public:
		virtual bool initialize(void);

		virtual int getPositionOfZeroDiopter(void) const;
		virtual void setPositionOfZeroDiopter(int pos);

		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);

	private:
		struct FundusFocusMotorImpl;
		std::unique_ptr<FundusFocusMotorImpl> d_ptr;
		FundusFocusMotorImpl& getImpl(void) const;
	};
}
