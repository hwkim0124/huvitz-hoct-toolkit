#pragma once

#include "OctDeviceDef.h"
#include "OctConfigDef.h"

#include <memory>


namespace OctDevice
{
	class UsbPort;

	class OCTDEVICE_DLL_API UsbComm
	{
	public:
		UsbComm();
		virtual ~UsbComm();

		UsbComm(UsbComm&& rhs);
		UsbComm& operator=(UsbComm&& rhs);

		// Prevent copy construction and assignment. 
		UsbComm(const UsbComm& rhs) = delete;
		UsbComm& operator=(const UsbComm& rhs) = delete;

	public:
		bool openChannel(void);
		void release(void);
		bool isInitiated(void) const;
		bool isRestoring(void) const;
		void setCommandAsyncMode(bool flag);
		bool isCommandAsyncMode(void) const;

		bool readAddress(std::uint32_t addr, std::uint8_t* buff, std::uint32_t size);
		bool writeAddress(std::uint32_t addr, std::uint8_t* buff, std::uint32_t size, bool reply = true);
		
		bool readStream(CameraType type, std::uint8_t* buff, std::uint32_t size);
		bool sendMsgCmd(MsgCommand* msgCmd, bool reply = true);
		bool recvMsgCmdRes(void);
		bool restoreFromUsbError(void);

		bool PI_ReadStatus(std::uint32_t baseAddr, std::uint32_t* status);
		bool SLD_ReadStatus(std::uint32_t baseAddr, SldSensorData* status);
		bool Interrupt_ReadStatus(std::uint32_t baseAddr, std::uint32_t* status);

		bool SysCalRead(std::uint32_t baseAddr, SysCal_st* sysCal);
		bool SysCalWrite(std::uint32_t baseAddr, SysCal_st* sysCal);
		bool SysCalLoad(std::uint16_t offset, std::uint16_t size);
		bool SysCalSave(std::uint16_t offset, std::uint16_t size);
		bool SysCal2Read(std::uint32_t baseAddr, SysCal2_st* sysCal);
		bool SysCal2Write(std::uint32_t baseAddr, SysCal2_st* sysCal);
		bool SysCal2Load(std::uint16_t offset, std::uint16_t size);
		bool SysCal2Save(std::uint16_t offset, std::uint16_t size);

		bool UartDebugSet(std::uint32_t baseAddr, std::uint16_t flag);
		bool UartDebugGet(std::uint32_t baseAddr, std::uint16_t* flag);

		bool StageJog(std::uint8_t mid, int delta);
		bool MotorJog(std::uint8_t mid, int delta);
		bool MotorMove(std::uint8_t mid, int pos);
		bool MotorHome(std::uint8_t mid);
		bool MotorStop(std::uint8_t mid);
		bool MotorStatus(std::uint8_t mid, std::uint32_t baseAddr, StepMotorInfo* info);
		bool StageStatus(StageMotorType type, std::uint32_t baseAddr, StageDcMotorInfo* info);
		bool StageStatus(StageMotorType type, std::uint32_t baseAddr, StageStepMotorInfo* info);
		
		bool StageMove(StageMotorType type, std::uint8_t dir, std::uint8_t duty);
		bool StageStop(StageMotorType type);
		bool StageLimitOverride(std::uint32_t baseAddr, std::uint16_t flag);
		bool SldWariningEnabled(std::uint32_t baseAddr, std::uint16_t flag);
		bool ScannerFaultParameters(std::uint16_t timer, std::uint16_t thresh);

		bool ChinrestMove(std::uint8_t dir);
		bool ChinrestStop(void);

		bool IrCameraControl(std::uint8_t cid, std::uint8_t onoff);
		bool IrCameraDigitalGain(std::uint8_t cid, std::uint8_t gain);
		bool IrCameraAnalogGain(std::uint8_t cid, std::uint8_t gain);
		bool IrCameraExposureTime(std::uint8_t cid, std::uint16_t ints);

		bool TestCommand(std::uint8_t cmd);

		bool LedSetIntensity(LightLedType type, std::uint8_t value);
		bool OctSldUpdateParameters(std::uint8_t channel=4);
		bool OctSldGetParameters(std::uint8_t channel = 4);
		bool OctSldPotentiometer(std::uint8_t channel, std::uint16_t data);
		bool FlashLedContinous(std::uint32_t baseAddr, std::uint16_t flag);

		bool QuickReturnMirror(std::uint8_t inout);
		bool SplitFocusMirror(std::uint8_t inout);
		bool DiopterCompensationLens(std::uint8_t focus_sel, std::uint8_t lens_id);
		bool PupilMaskSelect(std::uint8_t mask_id);
		bool FundusCameraCapture(void);
		
		bool InternalFixationControl(std::uint8_t row, std::uint8_t col);
		bool LcdFixationControl(std::uint8_t row, std::uint8_t col);
		bool LcdFixationParameters(std::uint8_t bright, std::uint8_t blink, std::uint16_t period, std::uint16_t ontime, std::uint8_t type);

		bool UpdateDeviceStatus(std::uint8_t dev_id);

		bool GalvanoMoveX(short x);
		bool GalvanoMoveY(short y);
		bool GalvanoMoveXY(short x, short y);
		bool GalvanoSlewX(short x);
		bool GalvanoSlewY(short y);
		bool GalvanoSlewXY(short x, short y);
		bool GalvanoDeltaMoveX(short x);
		bool GalvanoDeltaMoveY(short y);
		bool GalvanoDeltaMoveXY(short x, short y);

		bool GalvanoDynamicOffsetWrite(std::uint32_t baseAddr, std::int16_t offsetX, std::int16_t offsetY);
		bool GalvanoDynamicOffsetRead(std::uint32_t baseAddr, std::int16_t& offsetX, std::int16_t& offsetY);
		bool GalvanoSlewRate(std::uint32_t baseAddr, std::uint32_t rate);
		bool GalvanoAutoSlew(std::uint32_t baseAddr, std::uint16_t flag);

		bool GalvanoScanXY(std::uint16_t sid, std::uint16_t eid);
		bool GalvanoRasterX(std::uint16_t tid, std::uint16_t lines, std::int16_t xoffs, std::int16_t yoffs);
		bool GalvanoRasterY(std::uint16_t tid, std::uint16_t lines, std::int16_t xoffs, std::int16_t yoffs);
		bool GalvanoRasterXY(std::uint16_t tid, std::uint16_t lines, std::int16_t xoffs, std::int16_t yoffs);
		bool GalvanoRasterFastX(std::uint16_t tid, std::uint16_t lines, std::int16_t xoffs, std::int16_t yoffs);
		bool GalvanoRasterFastY(std::uint16_t tid, std::uint16_t lines, std::int16_t xoffs, std::int16_t yoffs);

		bool TrajectoryParams(std::uint8_t tid, std::uint32_t baseAddr, TrajectoryProfileParams* params);
		bool TrajectoryPositionsX(std::uint8_t tid, std::uint32_t baseAddr, std::int16_t* positions, std::uint16_t count);
		bool TrajectoryPositionsY(std::uint8_t tid, std::uint32_t baseAddr, std::int16_t* positions, std::uint16_t count);

		UsbPort* getUsbPort(void) const;

	private:
		struct UsbCommImpl;
		std::unique_ptr<UsbCommImpl> d_ptr;
		UsbCommImpl& getImpl(void) const;

		CommandType getLightLedCommandType(LightLedType type);

		unsigned char getNextPacketID(void);
		void attachCRC(std::uint8_t* addr, std::uint32_t offset);
		void attachCRC(std::uint8_t* addr);

		ReadRequest* getReadRequest(std::uint32_t addr, std::uint16_t rd_size);
		ReadResponse* getReadResponse(std::uint16_t rd_size);
		WriteRequest* getWriteRequest(std::uint32_t addr, std::uint8_t *data, std::uint16_t size, bool reply = true);
		WriteResponse* getWriteResponse(void);
		MsgCommand* getMsgCommand(CommandType ctrl, std::uint8_t msg_len);

	};
}
