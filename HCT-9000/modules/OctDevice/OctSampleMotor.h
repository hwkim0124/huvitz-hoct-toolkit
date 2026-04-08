#pragma once

#include "OctDeviceDef.h"
#include "StepMotor.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API OctSampleMotor : public StepMotor
	{
	public:
		OctSampleMotor();
		OctSampleMotor(MainBoard *board, StepMotorType type);
		virtual ~OctSampleMotor();

		OctSampleMotor(OctSampleMotor&& rhs);
		OctSampleMotor& operator=(OctSampleMotor&& rhs);
		OctSampleMotor(const OctSampleMotor& rhs);
		OctSampleMotor& operator=(const OctSampleMotor& rhs);

	public:
		virtual bool initialize(void);

		bool updatePositionToMirrorIn(void);
		bool updatePositionToMirrorOut(void);
		bool isPositionAtMirrorIn(void);
		bool isPositionAtMirrorOut(void);

		virtual int getPositionOfMirrorIn(void) const;
		virtual int getPositionOfMirrorOut(void) const;
		virtual void setPositionOfMirrorIn(int pos);
		virtual void setPositionOfMirrorOut(int pos);

		void setCurrentPositionAsMirrorIn(void);
		void setCurrentPositionAsMirrorOut(void);

		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);
		virtual bool loadConfig(OctConfig::DeviceSettings* dset);
		virtual bool saveConfig(OctConfig::DeviceSettings* dset);

	private:
		struct OctSampleMotorImpl;
		std::unique_ptr<OctSampleMotorImpl> d_ptr;
		OctSampleMotorImpl& getImpl(void) const;
	};
}
