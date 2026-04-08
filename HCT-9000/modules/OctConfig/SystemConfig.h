#pragma once

#include "OctConfigDef.h"

#include <memory>


namespace OctConfig
{
	class OCTCONFIG_DLL_API SystemConfig
	{
	public:
		SystemConfig();
		virtual ~SystemConfig();

	public:
		void initialize(void);
		static void resetToDefaultValues(void);
		static SysCal_st* getSysCalibData(bool checksum = true);
		static bool isModified(void);
		static void setModified(bool flag);
		static bool isValidChecksum(void);

		static KerSetupDiopter_st* keratoSetupDiopter(int index);
		static KerSetupData_st* keratoSetupData(int diopt, int index);

		static std::int32_t referencePosToRetina(bool isSet = false, std::int32_t value = 0);
		static std::int32_t referencePosToCornea(bool isSet = false, std::int32_t value = 0);
		static std::int32_t polarizationPos(bool isSet = false, std::int32_t value = 0);

		static std::int32_t zeroDioptPosToIrFocus(bool isSet = false, std::int32_t value = 0);
		static std::int32_t zeroDioptPosToScanFocus(bool isSet = false, std::int32_t value = 0);
		static std::int32_t zeroDioptPosToTopographyScanFocus(bool isSet = false, std::int32_t value = 0);

		static std::double_t dispersionParameterToRetina(int index, bool isSet = false, std::double_t value = 0);
		static std::double_t dispersionParameterToCornea(int index, bool isSet = false, std::double_t value = 0);
		static std::double_t spectrometerParameter(int index, bool isSet = false, std::double_t value = 0);

		static std::uint16_t sldParameterHighCode(bool isSet = false, std::uint16_t value = 0);
		static std::uint16_t sldParameterLowCode1(bool isSet = false, std::uint16_t value = 0);
		static std::uint16_t sldParameterLowCode2(bool isSet = false, std::uint16_t value = 0);
		static std::uint16_t sldParameterRsiCode(bool isSet = false, std::uint16_t value = 0);

		static std::int16_t quickReturnMirrorPos(bool isOut = false, bool isSet = false, std::int16_t value = 0);
		static std::int16_t splitFocusMirrorPos(bool isOut = false, bool isSet = false, std::int16_t value = 0);
		static std::int16_t pupilMaskPos(bool isSmall = false, bool isSet = false, std::int16_t value = 0);

		static std::int16_t minusLensPosToFundus(bool isSet = false, std::int16_t value = 0);
		static std::int16_t plusLensPosToFundus(bool isSet = false, std::int16_t value = 0);
		static std::int16_t zeroLensPosToFundus(bool isSet = false, std::int16_t value = 0);
		static std::int16_t minusLensPosToScan(bool isSet = false, std::int16_t value = 0);
		static std::int16_t plusLensPosToScan(bool isSet = false, std::int16_t value = 0);
		static std::int16_t zeroLensPosToScan(bool isSet = false, std::int16_t value = 0);

		static std::uint8_t retinaCameraAgain(bool isSet = false, std::uint8_t value = 0);
		static std::uint8_t retinaCameraDgain(bool isSet = false, std::uint8_t value = 0);
		static std::uint8_t corneaCameraAgain(bool isSet = false, std::uint8_t value = 0);
		static std::uint8_t corneaCameraDgain(bool isSet = false, std::uint8_t value = 0);

		static std::int16_t intFixationOffsetX(bool isSet = false, std::int16_t value = 0);
		static std::int16_t intFixationOffsetY(bool isSet = false, std::int16_t value = 0);

		static std::uint64_t calculateCheckSum(void);
		static std::uint64_t getCheckSum(void);

	protected:
		static void updateCheckSum(void);

	private:
		struct SystemConfigImpl;
		static std::unique_ptr<SystemConfigImpl> d_ptr;
		static SystemConfigImpl& getImpl(void);
	};
}
