#pragma once

#include "OctDeviceDef.h"
#include "StepMotor.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API FundusDiopterMotor : public StepMotor
	{
	public:
		FundusDiopterMotor();
		FundusDiopterMotor(MainBoard *board, StepMotorType type);
		virtual ~FundusDiopterMotor();

		FundusDiopterMotor(FundusDiopterMotor&& rhs);
		FundusDiopterMotor& operator=(FundusDiopterMotor&& rhs);
		FundusDiopterMotor(const FundusDiopterMotor& rhs);
		FundusDiopterMotor& operator=(const FundusDiopterMotor& rhs);

	public:
		virtual bool initialize(void);

		bool updatePositionToMinusLens(void);
		bool updatePositionToPlusLens(void);
		bool updatePositionToZeroLens(void);

		int getPositionOfMinusLens(void) const;
		int getPositionOfPlusLens(void) const;
		int getPositionOfZeroLens(void) const;
		void setPositionOfMinusLens(int pos);
		void setPositionOfPlusLens(int pos);
		void setPositionOfZeroLens(int pos);

		void setCurrentPositionAsMinusLens(void);
		void setCurrentPositionAsPlusLens(void);
		void setCurrentPositionAsZeroLens(void);

		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);

	private:
		struct FundusDiopterMotorImpl;
		std::unique_ptr<FundusDiopterMotorImpl> d_ptr;
		FundusDiopterMotorImpl& getImpl(void) const;
	};
}
