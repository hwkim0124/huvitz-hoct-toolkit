#pragma once

#include "OctDeviceDef.h"
#include "StepMotor.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API DiopterSelMotor : public StepMotor
	{
	public:
		DiopterSelMotor();
		DiopterSelMotor(MainBoard *board, StepMotorType type);
		virtual ~DiopterSelMotor();

		DiopterSelMotor(DiopterSelMotor&& rhs);
		DiopterSelMotor& operator=(DiopterSelMotor&& rhs);
		DiopterSelMotor(const DiopterSelMotor& rhs);
		DiopterSelMotor& operator=(const DiopterSelMotor& rhs);

	public:
		virtual bool initialize(void);

		bool updatePositionToMinusLens(bool isScan);
		bool updatePositionToPlusLens(bool isScan);
		bool updatePositionToZeroLens(bool isScan);

		int getPositionOfMinusLens(bool isScan) const;
		int getPositionOfPlusLens(bool isScan) const;
		int getPositionOfZeroLens(bool isScan) const;
		void setPositionOfMinusLens(bool isScan, int pos);
		void setPositionOfPlusLens(bool isScan, int pos);
		void setPositionOfZeroLens(bool isScan, int pos);

		void setCurrentPositionAsMinusLens(bool isScan);
		void setCurrentPositionAsPlusLens(bool isScan);
		void setCurrentPositionAsZeroLens(bool isScan);

		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);
		virtual bool loadConfig(OctConfig::DeviceSettings* dset);
		virtual bool saveConfig(OctConfig::DeviceSettings* dset);

	private:
		struct DiopterSelMotorImpl;
		std::unique_ptr<DiopterSelMotorImpl> d_ptr;
		DiopterSelMotorImpl& getImpl(void) const;
	};
}
