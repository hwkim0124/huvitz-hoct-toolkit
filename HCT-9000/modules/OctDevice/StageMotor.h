#pragma once

#include "OctDeviceDef.h"

#include <memory>

namespace OctConfig {
	class DeviceSettings;
}


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API StageMotor
	{
	public:
		StageMotor();
		StageMotor(MainBoard *board, StageMotorType type);
		virtual ~StageMotor();

		StageMotor(StageMotor&& rhs);
		StageMotor& operator=(StageMotor&& rhs);
		StageMotor(const StageMotor& rhs);
		StageMotor& operator=(const StageMotor& rhs);

	public:
		virtual bool initialize(void);
		bool isInitiated(void) const;
		bool isStepMotor(void) const;

		virtual void setControls(CSliderCtrl* pSlider=nullptr, CEdit* pEdit=nullptr);
		virtual bool updatePositionByEdit(void);
		virtual bool updatePosition(int pos);
		virtual bool updatePositionByOffset(int offset);
		virtual bool updatePositionToCenter(void);
		int getSliderStepSize(void) const;

		bool updatePositionByDelta(int delta);

		bool controlMove(int pos);
		bool controlJogg(int delta);
		bool controlDelta(int delta);
		bool controlHome(void);
		bool controlStop(void);

		bool updateStatus(void);
		void reportStatus(void);

		void setPosition(int pos);
		int getPosition(void) const;
		int getTargetPosition(void) const;
		int getCenterPosition(void) const;

		void setLimitRange(int low, int high);
		int getRangeMax(void) const;
		int getRangeMin(void) const;
		int getRangeSize(void) const;

		void setAsyncMode(bool flag);
		bool isAsyncMode(void) const;
		bool waitForUpdate(int posOffset = 3, int timeDelay = 30, int countMax = 50);

		const char* getName(void) const;
		StageMotorType getType(void) const;

		virtual bool isEndOfLowerPosition(void) const;
		virtual bool isEndOfUpperPosition(void) const;
		virtual bool isAtLowerSideOfPosition(void) const;
		virtual bool isAtUpperSideOfPosition(void) const;
		virtual bool isAtCenterOfPosition(void) const;

		virtual bool loadConfig(OctConfig::DeviceSettings* dset);
		virtual bool saveConfig(OctConfig::DeviceSettings* dset);


	private:
		struct StageMotorImpl;
		std::unique_ptr<StageMotorImpl> d_ptr;
		StageMotorImpl& getImpl(void) const;

		MainBoard* getMainBoard(void) const;
		std::uint8_t getMotorId(void) const;
		std::int32_t getInitPosition(void) const;
	};
}
