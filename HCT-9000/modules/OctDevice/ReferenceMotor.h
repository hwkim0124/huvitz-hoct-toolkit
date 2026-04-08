#pragma once


#include "OctDeviceDef.h"
#include "StepMotor.h"

#include <memory>


namespace OctConfig {
	class DeviceSettings;
}


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API ReferenceMotor : public StepMotor
	{
	public:
		ReferenceMotor();
		ReferenceMotor(MainBoard *board, StepMotorType type);
		virtual ~ReferenceMotor();

		ReferenceMotor(ReferenceMotor&& rhs);
		ReferenceMotor& operator=(ReferenceMotor&& rhs);
		ReferenceMotor(const ReferenceMotor& rhs) ;
		ReferenceMotor& operator=(const ReferenceMotor& rhs) ;

	public:
		virtual bool initialize(void);

		int getPositionOfRetinaOrigin(void) const;
		int getPositionOfCorneaOrigin(void) const;
		void setPositionOfRetinaOrigin(int pos);
		void setPositionOfCorneaOrigin(int pos);

		void setCurrentPositionAsRetinaOrigin(void);
		void setCurrentPositionAsCorneaOrigin(void);

		bool updatePositionToRetinaOrigin(void);
		bool updatePositionToCorneaOrigin(void);
		bool updatePositionToUpperEnd(void);
		bool updatePositionToLowerEnd(void);

		bool isEndOfLowerPosition(bool inRange = true) const;
		bool isEndOfUpperPosition(bool inRange = true) const;
		bool isAtLowerSide(bool isCornea = false) const;
		bool isAtUpperSide(bool isCornea = false) const;

		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);

		virtual bool loadConfig(OctConfig::DeviceSettings* dset);
		virtual bool saveConfig(OctConfig::DeviceSettings* dset);

	private:
		struct ReferenceMotorImpl;
		std::unique_ptr<ReferenceMotorImpl> d_ptr;
		ReferenceMotorImpl& getImpl(void) const;
	};
}
