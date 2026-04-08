#pragma once

#include "OctDeviceDef.h"
#include "StepMotor.h"

#include <memory>

namespace OctConfig {
	class DeviceSettings;
}


namespace OctDevice
{
	class OCTDEVICE_DLL_API PolarizationMotor : public StepMotor
	{
	public:
		PolarizationMotor();
		PolarizationMotor(MainBoard *board, StepMotorType type);
		virtual ~PolarizationMotor();

		PolarizationMotor(PolarizationMotor&& rhs);
		PolarizationMotor& operator=(PolarizationMotor&& rhs);
		PolarizationMotor(const PolarizationMotor& rhs) ;
		PolarizationMotor& operator=(const PolarizationMotor& rhs) ;

	public:
		virtual bool initialize(void);
		virtual void setControls(CSliderCtrl* pSlider=NULL, CEdit* pEdit=NULL, CEdit* pEditDegree = NULL);
		virtual bool updatePositionByDegree(void);
		virtual bool updatePositionByEdit(void);
		virtual bool updatePosition(int pos);

		int getPositionOfZeroDegree(void) const;
		void setPositionOfZeroDegree(int pos);
		void setCurrentPositionAsZeroDegree(void);

		float getCurrentDegree(void) const;
		int getPositionsPerDegree(void) const;
		void setPositionsPerDegree(int pos);

		bool updateDegree(float diopt);
		bool updateDegreeByOffset(float degree);
		bool updatePositionToZeroDegree(void);

		int convertDegreeToPosition(float diopt);
		float convertPositionToDegree(int pos);
		int getDegreeDirection(void) const;

		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);

		virtual bool loadConfig(OctConfig::DeviceSettings* dset);
		virtual bool saveConfig(OctConfig::DeviceSettings* dset);

	private:
		struct PolarizationMotorImpl;
		std::unique_ptr<PolarizationMotorImpl> d_ptr;
		PolarizationMotorImpl& getImpl(void) const;
	};
}
