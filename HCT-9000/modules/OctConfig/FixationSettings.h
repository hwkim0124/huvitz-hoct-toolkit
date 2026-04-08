#pragma once

#include "OctConfigDef.h"

#include <memory>
#include <vector>


namespace OctConfig
{
	class OCTCONFIG_DLL_API FixationSettings
	{
	public:
		FixationSettings();
		virtual ~FixationSettings();

		FixationSettings(FixationSettings&& rhs);
		FixationSettings& operator=(FixationSettings&& rhs);
		FixationSettings(const FixationSettings& rhs);
		FixationSettings& operator=(const FixationSettings& rhs);

	public:
		void initialize(void);
		void resetToDefaultValues(void);

		std::pair<int, int> getCenterOD(void) const;
		std::pair<int, int> getFundusOD(void) const;
		std::pair<int, int> getScanDiskOD(void) const;
		std::pair<int, int> getLeftSideOD(int index) const;
		std::pair<int, int> getRightSideOD(int index) const;

		std::pair<int, int> getCenterOS(void) const;
		std::pair<int, int> getFundusOS(void) const;
		std::pair<int, int> getScanDiskOS(void) const;
		std::pair<int, int> getLeftSideOS(int index) const;
		std::pair<int, int> getRightSideOS(int index) const;

		int getBrightness(void) const;
		int getBlinkPeriod(void) const;
		int getBlinkOnTime(void) const;
		int getFixationType(void) const;

		void setCenterOD(std::pair<int, int> pos);
		void setFundusOD(std::pair<int, int> pos);
		void setScanDiskOD(std::pair<int, int> pos);
		void setLeftSideOD(int index, std::pair<int, int> pos);
		void setRightSideOD(int index, std::pair<int, int> pos);

		void setCenterOS(std::pair<int, int> pos);
		void setFundusOS(std::pair<int, int> pos);
		void setScanDiskOS(std::pair<int, int> pos);
		void setLeftSideOS(int index, std::pair<int, int> pos);
		void setRightSideOS(int index, std::pair<int, int> pos);

		void setBrightness(int bright);
		void setBlinkPeriod(int period);
		void setBlinkOnTime(int onTime);
		void setFixationType(int type);

		bool useLcdFixation(bool isset = false, bool flag = false) const;
		bool useLcdBlinkOn(bool isset = false, bool flag = false) const;

	private:
		struct FixationSettingsImpl;
		std::unique_ptr<FixationSettingsImpl> d_ptr;
		FixationSettingsImpl& getImpl(void) const;
	};
}
