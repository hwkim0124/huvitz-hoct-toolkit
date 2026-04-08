#pragma once

#include "OctDeviceDef.h"
#include "StepMotor.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API OctDiopterMotor : public StepMotor
	{
	public:
		OctDiopterMotor();
		OctDiopterMotor(MainBoard *board, StepMotorType type);
		virtual ~OctDiopterMotor();

		OctDiopterMotor(OctDiopterMotor&& rhs);
		OctDiopterMotor& operator=(OctDiopterMotor&& rhs);
		OctDiopterMotor(const OctDiopterMotor& rhs);
		OctDiopterMotor& operator=(const OctDiopterMotor& rhs);

	public:
		virtual bool initialize(void);

		bool updatePositionToMinusLens(void);
		bool updatePositionToPlusLens(void);
		bool updatePositionToZeroLens(void);

		bool isPlusLensMode(void) const;
		bool isZeroLensMode(void) const;
		bool isMinusLensMode(void) const;

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
		struct OctDiopterMotorImpl;
		std::unique_ptr<OctDiopterMotorImpl> d_ptr;
		OctDiopterMotorImpl& getImpl(void) const;
	};
}
