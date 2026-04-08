#pragma once

#include "OctDeviceDef.h"
#include "StepMotor.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API PupilMaskMotor : public StepMotor
	{
	public:
		PupilMaskMotor();
		PupilMaskMotor(MainBoard *board, StepMotorType type);
		virtual ~PupilMaskMotor();

		PupilMaskMotor(PupilMaskMotor&& rhs);
		PupilMaskMotor& operator=(PupilMaskMotor&& rhs);
		PupilMaskMotor(const PupilMaskMotor& rhs);
		PupilMaskMotor& operator=(const PupilMaskMotor& rhs);

	public:
		virtual bool initialize(void);

		bool updatePositionToNormal(void);
		bool updatePositionToSmall(void);

		int getPositionOfNormal(void) const;
		int getPositionOfSmall(void) const;
		void setPositionOfNormal(int pos);
		void setPositionOfSmall(int pos);
	
		void setCurrentPositionAsNormal(void);
		void setCurrentPositionAsSmall(void);

		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);
		virtual bool loadConfig(OctConfig::DeviceSettings* dset);
		virtual bool saveConfig(OctConfig::DeviceSettings* dset);

	private:
		struct PupilMaskMotorImpl;
		std::unique_ptr<PupilMaskMotorImpl> d_ptr;
		PupilMaskMotorImpl& getImpl(void) const;
	};
}
