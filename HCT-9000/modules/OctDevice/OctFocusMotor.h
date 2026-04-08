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

	class OCTDEVICE_DLL_API OctFocusMotor : public StepMotor
	{
	public:
		OctFocusMotor();
		OctFocusMotor(MainBoard *board, StepMotorType type);
		virtual ~OctFocusMotor();

		OctFocusMotor(OctFocusMotor&& rhs);
		OctFocusMotor& operator=(OctFocusMotor&& rhs);
		OctFocusMotor(const OctFocusMotor& rhs);
		OctFocusMotor& operator=(const OctFocusMotor& rhs);

	public:
		virtual bool initialize(void);
		virtual void setControls(CSliderCtrl* pSlider=nullptr, CEdit* pEdit=nullptr, CEdit* pEditDiopt = NULL);
		virtual bool updatePositionByDiopt(void);
		virtual bool updatePositionByEdit(void);
		virtual bool updatePosition(int pos);

		virtual int getPositionOfZeroDiopter(void) const;
		virtual int getPositionOfTopography(void) const;
		virtual int getPositionOfTopographyDiopter(void);

		virtual void setPositionOfZeroDiopter(int pos);
		virtual void setPositionOfTopogrphy(int pos);
		void setCurrentPositionAsZeroDiopter(void);
		void setCurrentPositionAsTopogrphyDiopter(void);

		float getCurrentDiopter(void) const;
		int getPositionsPerDiopter(void) const;
		void setPositionsPerDiopter(int pos);

		bool updateDiopter(float diopt);
		bool updateTopographyDiopter(void);
		bool updateDiopterByOffset(float offset);
		bool updatePositionToZeroDiopter(void);

		int convertDiopterToPosition(float diopt);
		float convertPositionToDiopter(int pos);
		int getDiopterDirection(void) const;

		bool isEndOfPlusDiopter(void) const;
		bool isEndOfMinusDiopter(void) const;

		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);

		virtual bool loadConfig(OctConfig::DeviceSettings* dset);
		virtual bool saveConfig(OctConfig::DeviceSettings* dset);

	private:
		struct OctFocusMotorImpl;
		std::unique_ptr<OctFocusMotorImpl> d_ptr;
		OctFocusMotorImpl& getImpl(void) const;
	};
}
