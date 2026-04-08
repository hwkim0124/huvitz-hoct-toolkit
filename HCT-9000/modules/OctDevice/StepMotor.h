#pragma once

#include "OctDeviceDef.h"
#include "BoardDevice.h"

#include <memory>


namespace OctConfig {
	class DeviceSettings;
}


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API StepMotor : public BoardDevice
	{
	public:
		StepMotor();
		StepMotor(MainBoard *board, StepMotorType type);
		virtual ~StepMotor();

		StepMotor(StepMotor&& rhs);
		StepMotor& operator=(StepMotor&& rhs);
		StepMotor(const StepMotor& rhs) ;
		StepMotor& operator=(const StepMotor& rhs) ;

	public:
		virtual bool initialize(void);
		bool isInitiated(void) const;

		virtual void setControls(CSliderCtrl* pSlider=nullptr, CEdit* pEdit=nullptr);
		virtual bool updatePositionByEdit(void);
		virtual bool updatePosition(int pos);
		virtual bool updatePositionByOffset(int offset);

		bool controlMove(int pos, bool async = false);
		bool controlHome(void);
		bool updateStatus(void);
		void reportStatus(void);

		void setAsyncMode(bool flag);
		bool isAsyncMode(void) const;
		bool waitForUpdate(int posOffset = 0, int timeDelay = MOTOR_CONTROL_WAIT_DELAY, int countMax = MOTOR_CONTROL_WAIT_COUNT);

		void setPosition(int pos);
		int getPosition(void) const;
		int getTargetPosition(void) const;
		int getCenterPosition(void) const;

		int getRangeMax(void) const;
		int getRangeMin(void) const;
		int getRangeSize(void) const;

		void setRangeMax(int value);
		void setRangeMin(int value);

		const char* getName(void) const;
		StepMotorType getType(void) const;
		int getSliderStepSize(void) const;

		virtual bool isEndOfLowerPosition(void) const;
		virtual bool isEndOfUpperPosition(void) const;
		virtual bool isAtLowerSideOfPosition(void) const;
		virtual bool isAtUpperSideOfPosition(void) const;
		virtual bool isAtCenterOfPosition(void) const;

		virtual bool loadConfig(OctConfig::DeviceSettings* dset);
		virtual bool saveConfig(OctConfig::DeviceSettings* dset);

	protected:
		MainBoard* getMainBoard(void) const;
		std::uint8_t getMotorId(void) const;
		std::int32_t getInitPosition(void) const;

	private:
		struct StepMotorImpl;
		std::unique_ptr<StepMotorImpl> d_ptr;
		StepMotorImpl& getImpl(void) const;
	};
}

