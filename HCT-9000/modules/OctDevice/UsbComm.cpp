#include "stdafx.h"
#include "UsbComm.h"
#include "UsbPort.h"


#include <string>
#include <mutex>

#include "CppUtil2.h"

using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct UsbComm::UsbCommImpl
{
	UsbPort usbPort;
	ReadRequest readReq;
	ReadResponse readResp;
	WriteRequest writeReq;
	WriteResponse writeResp;
	MsgCommand msgCmd;

	bool initiated;
	bool restoring;
	bool cmdAsync;
	mutex mutexRestore;
	mutex mutexControl;
	mutex mutexStream;

	UsbCommImpl() : initiated(false), restoring(false), cmdAsync(false) {
	}
};


UsbComm::UsbComm() :
	d_ptr(make_unique<UsbCommImpl>())
{
}


UsbComm::~UsbComm()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctDevice::UsbComm::UsbComm(UsbComm && rhs) = default;
UsbComm & OctDevice::UsbComm::operator=(UsbComm && rhs) = default;


bool OctDevice::UsbComm::openChannel(void)
{
	bool ret = false;
	int retry = 0;
	
	try {
		ret = getImpl().usbPort.open();
	}
	catch (const std::exception&) {
	}

	if (ret) {
		getImpl().restoring = false;
		getImpl().initiated = true;
	}
	return ret;
}


void OctDevice::UsbComm::release(void)
{
	getImpl().usbPort.close();
	return;
}


bool OctDevice::UsbComm::isInitiated(void) const
{
	return getImpl().initiated;
}


bool OctDevice::UsbComm::isRestoring(void) const
{
	return getImpl().restoring;
}


void OctDevice::UsbComm::setCommandAsyncMode(bool flag)
{
	getImpl().cmdAsync = flag;
	return;
}


bool OctDevice::UsbComm::isCommandAsyncMode(void) const
{
	return getImpl().cmdAsync;
}


bool OctDevice::UsbComm::readAddress(std::uint32_t addr, std::uint8_t* buff, std::uint32_t size)
{
	lock_guard<mutex> lock(getImpl().mutexControl);

	std::uint8_t* p_buff = buff;
	std::uint32_t n_read = 0;
	std::uint32_t n_data = size;
	std::uint32_t n_addr = addr;

	while (n_data > 0)
	{
		n_read = (n_data > 256 ? 256 : n_data);

		ReadRequest* pReq = getReadRequest(n_addr, n_read);
		ReadResponse* pRes = getReadResponse(n_read);
		UsbPort* usbPort = getUsbPort();

		// The request size to read should be in the range of unsigned char. 
		// It doesn't cosider incomplete send or receive for the requested size. 
		if (usbPort->send((uint8_t*)&pReq->packet, pReq->size, true)) {
			if (usbPort->receive((uint8_t*)&pRes->packet, pRes->size, true)) {
				if (pRes->packet.type == static_cast<uint8_t>(PacketType::NREAD_RESP_OK)) {
					//if (n_read > 0) {
						memcpy(p_buff, pRes->packet.data, n_read);
						n_data -= n_read;
						if (n_data <= 0) {
							return true;
						}
						else {
							p_buff += n_read;
							n_addr += n_read;
							continue;
						}
					//}
				}
				else {
					DebugOut2() << "Read response invalid type=" << int(pRes->packet.type);
				}
			}
		}
		break;
	}
	DebugOut2() << "readAddress() failed!";
	return false;
}


bool OctDevice::UsbComm::writeAddress(std::uint32_t addr, std::uint8_t* buff, std::uint32_t size, bool reply)
{
	lock_guard<mutex> lock(getImpl().mutexControl);

	std::uint8_t* p_buff = buff;
	std::uint32_t n_sent = 0;
	std::uint32_t n_data = size;
	std::uint32_t n_addr = addr;

	while (n_data > 0)
	{
		n_sent = (n_data > 256 ? 256 : n_data);

		WriteRequest* pReq = getWriteRequest(n_addr, p_buff, n_sent);
		WriteResponse* pRes = getWriteResponse();
		UsbPort* usbPort = getUsbPort();

		if (usbPort->send((uint8_t*)&pReq->packet, pReq->size, true)) {
			bool resp = false;
			if (reply) {
				if (usbPort->receive((uint8_t*)&pRes->packet, pRes->size)) {
					if (pRes->packet.type == static_cast<uint8_t>(PacketType::NWRITE_RESP) ||
						pRes->packet.type == static_cast<uint8_t>(PacketType::MSG_RESP)) {
						if (pRes->packet.status == static_cast<uint8_t>(PacketType::STATUS_OK)) {
							resp = true;
						}
						else {
							DebugOut2() << "Write response invalid status=" << pRes->packet.status;
						}
					}
					else {
						DebugOut2() << "Write response invalid type=" << int(pRes->packet.type);
					}
				}
			}
			else {
				resp = true;
			}

			if (resp) {
				n_data -= n_sent;
				if (n_data <= 0) {
					return true;
				}
				else {
					p_buff += n_sent;
					n_addr += n_sent;
					continue;
				}
			}
			break;
		}
	}
	DebugOut2() << "writeAddress() failed!";
	return false;
}


bool OctDevice::UsbComm::readStream(CameraType type, std::uint8_t * buff, std::uint32_t size)
{
	// lock_guard<mutex> lock(getImpl().mutexStream);
	// lock_guard<mutex> lock(getImpl().mutexControl);

	bool ret = false;
	if (type == CameraType::CORNEA) {
		ret = getUsbPort()->receiveFromIr1(buff, size, true);
	}
	else {
		ret = getUsbPort()->receiveFromIr2(buff, size, true);
	}

	if (!ret) {
		DebugOut2() << "readStream() failed!, camera type = " << static_cast<int>(type);
		// restoreFromUsbError();
	}
	return ret;
}


bool OctDevice::UsbComm::sendMsgCmd(MsgCommand* msgCmd, bool reply)
{
	UsbPort* usbPort = getUsbPort();

	if (usbPort->send((uint8_t*)&msgCmd->packet, msgCmd->size)) {
		if (reply == false /*|| isCommandAsyncMode()*/) {
			return true;
		}
		else {
			MSG_CMD_RES msgRes;
			if (usbPort->receive((uint8_t*)&msgRes, sizeof(MSG_CMD_RES))) {
				// ??
				return true;
			}
		}
	}
	DebugOut2() << "sendMsgCmd() failed!";
	// restoreFromUsbError();
	return false;
}


bool OctDevice::UsbComm::recvMsgCmdRes(void)
{
	UsbPort* usbPort = getUsbPort();

	MSG_CMD_RES msgRes;
	if (usbPort->receive((uint8_t*)&msgRes, sizeof(MSG_CMD_RES))) {
		// ??
		return true;
	}
	DebugOut2() << "recvMsgCmdRes() failed!";
	return false;
}


bool OctDevice::UsbComm::restoreFromUsbError(void)
{
	lock_guard<mutex> lock(getImpl().mutexRestore);
	/*
	if (!isRestoring()) 
	{
		getImpl().restoring = true;
		serial::Serial sport(UART_PORT_NAME, UART_PORT_BAUDRATE);
		if (sport.isOpen()) {
			string req_msg = UART_CMD_USB_RESET;
			string res_msg;
			if (sport.write(req_msg) > 0) {
				if (sport.readline(res_msg) > 0) {
					if (!res_msg.find(UART_CMD_ACK_PREFIX)) {
						getUsbPort()->close();
						std::this_thread::sleep_for(std::chrono::milliseconds(USB_RESET_WAIT_TIME));

						for (int i = 0; i < USB_RESET_RETRY_MAX; i++) {
							if (getUsbPort()->open()) {
								getImpl().restoring = false;
								return true;
							}
							std::this_thread::sleep_for(std::chrono::milliseconds(USB_RESET_RETRY_DELAY));
						}
					}
				}
			}
		}
		getImpl().restoring = false;
	}
	*/

	CppUtil::SerialPort serial;
	if (serial.open(1)) {
		std::string msg1 = UART_CMD_USB_RESET; // "res\r";
		std::string res1 = "";
		if (serial.write(msg1)) {
			/*
			if (serial.read(res1, 5)) {

			}
			*/
		}
	}
	return false;
}


bool OctDevice::UsbComm::PI_ReadStatus(std::uint32_t baseAddr, std::uint32_t * status)
{
	uint32_t addr = baseAddr;
	GP_Status_st gpStatus;
	if (readAddress(addr, (uint8_t*)&gpStatus, sizeof(GP_Status_st))) {
		*status = gpStatus.pi_status;
		return true;
	}
	return false;
}


bool OctDevice::UsbComm::SLD_ReadStatus(std::uint32_t baseAddr, SldSensorData * status)
{
	uint32_t addr = baseAddr;
	GP_Status_st gpStatus;
	if (readAddress(addr, (uint8_t*)&gpStatus, sizeof(GP_Status_st))) {
		*status = gpStatus.sldSensorData;
		return true;
	}
	return false;
}


bool OctDevice::UsbComm::Interrupt_ReadStatus(std::uint32_t baseAddr, std::uint32_t * status)
{
	uint32_t addr = baseAddr;
	GP_Status_st gpStatus;
	if (readAddress(addr, (uint8_t*)&gpStatus, sizeof(GP_Status_st))) {
		*status = gpStatus.intr_src;
		return true;
	}
	return false;
}


bool OctDevice::UsbComm::SysCalRead(std::uint32_t baseAddr, SysCal_st * sysCal)
{
	uint32_t addr = baseAddr;
	return readAddress(addr, (uint8_t*)sysCal, sizeof(SysCal_st));
}


bool OctDevice::UsbComm::SysCalWrite(std::uint32_t baseAddr, SysCal_st * sysCal)
{
	uint32_t addr = baseAddr;
	return writeAddress(addr, (uint8_t*)sysCal, sizeof(SysCal_st));
}


bool OctDevice::UsbComm::SysCalLoad(std::uint16_t offset, std::uint16_t size)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::LOAD_CALIB, 4);
	msg->packet.ctrl6.s1 = offset;
	msg->packet.ctrl6.s2 = size;
	attachCRC(msg->packet.ctrl6.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::SysCalSave(std::uint16_t offset, std::uint16_t size)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::SAVE_CALIB, 4);
	msg->packet.ctrl6.s1 = offset;
	msg->packet.ctrl6.s2 = size;
	attachCRC(msg->packet.ctrl6.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::SysCal2Read(std::uint32_t baseAddr, SysCal2_st * sysCal)
{
	uint32_t addr = baseAddr;
	return readAddress(addr, (uint8_t*)sysCal, sizeof(SysCal2_st));
}


bool OctDevice::UsbComm::SysCal2Write(std::uint32_t baseAddr, SysCal2_st * sysCal)
{
	uint32_t addr = baseAddr;
	return writeAddress(addr, (uint8_t*)sysCal, sizeof(SysCal2_st));
}


bool OctDevice::UsbComm::SysCal2Load(std::uint16_t offset, std::uint16_t size)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::LOAD_CALIB2, 4);
	msg->packet.ctrl6.s1 = offset;
	msg->packet.ctrl6.s2 = size;
	attachCRC(msg->packet.ctrl6.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::SysCal2Save(std::uint16_t offset, std::uint16_t size)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::SAVE_CALIB2, 4);
	msg->packet.ctrl6.s1 = offset;
	msg->packet.ctrl6.s2 = size;
	attachCRC(msg->packet.ctrl6.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::UartDebugSet(std::uint32_t baseAddr, std::uint16_t flag)
{
	uint32_t addr = baseAddr + 8;
	return writeAddress(addr, (uint8_t*)&flag, sizeof(uint16_t));
}


bool OctDevice::UsbComm::UartDebugGet(std::uint32_t baseAddr, std::uint16_t * flag)
{
	uint32_t addr = baseAddr + 8;
	return readAddress(addr, (uint8_t*)flag, sizeof(uint16_t));
}


bool OctDevice::UsbComm::StageJog(std::uint8_t mid, int delta)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::AF_XZPOS, 5);
	msg->packet.motor.c1 = mid;
	msg->packet.motor.n1 = delta;
	attachCRC(msg->packet.motor.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::MotorJog(std::uint8_t mid, int delta)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::MOTJOG, 5);
	msg->packet.motor.c1 = mid;
	msg->packet.motor.n1 = delta;
	attachCRC(msg->packet.motor.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::MotorMove(std::uint8_t mid, int pos)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::MOTMOVE, 5);
	msg->packet.motor.c1 = mid;
	msg->packet.motor.n1 = pos;
	attachCRC(msg->packet.motor.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::MotorHome(std::uint8_t mid)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::MOTHOME, 1);
	msg->packet.ctrl1.c1 = mid;
	attachCRC(msg->packet.ctrl1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::MotorStop(std::uint8_t mid)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::XZ_STOP, 1);
	msg->packet.ctrl1.c1 = mid;
	attachCRC(msg->packet.ctrl1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::MotorStatus(std::uint8_t mid, std::uint32_t baseAddr, StepMotorInfo * info)
{
	uint32_t addr = baseAddr + mid * sizeof(StepMotorInfo);
	return readAddress(addr, (uint8_t*)info, sizeof(StepMotorInfo));
}


bool OctDevice::UsbComm::StageStatus(StageMotorType type, std::uint32_t baseAddr, StageDcMotorInfo * info)
{
	uint32_t addr = baseAddr;
	if (type == StageMotorType::STAGE_Z) {
		addr += sizeof(StageDcMotorInfo);
	}
	bool ret = readAddress(addr, (uint8_t*)info, sizeof(StageDcMotorInfo));
	return ret;
}


bool OctDevice::UsbComm::StageStatus(StageMotorType type, std::uint32_t baseAddr, StageStepMotorInfo * info)
{
	uint32_t addr = baseAddr + 2 * sizeof(StageDcMotorInfo);
	bool ret = readAddress(addr, (uint8_t*)info, sizeof(StageStepMotorInfo));
	return ret;
}


bool OctDevice::UsbComm::StageMove(StageMotorType type, std::uint8_t dir, std::uint8_t duty)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::AF_XZMOVE, 3);
	msg->packet.ctrl9.c1 = static_cast<uint8_t>(type);
	msg->packet.ctrl9.c2 = dir;
	msg->packet.ctrl9.c3 = duty;
	attachCRC(msg->packet.ctrl9.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::StageStop(StageMotorType type)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::XZ_STOP, 1);
	msg->packet.ctrl1.c1 = static_cast<uint8_t>(type);
	attachCRC(msg->packet.ctrl1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::StageLimitOverride(std::uint32_t baseAddr, std::uint16_t flag)
{
	uint32_t addr = baseAddr + 42;
	return writeAddress(addr, (uint8_t*)&flag, sizeof(uint16_t));
}

bool OctDevice::UsbComm::SldWariningEnabled(std::uint32_t baseAddr, std::uint16_t flag)
{
	uint32_t addr = baseAddr + 46;
	return writeAddress(addr, (uint8_t*)&flag, sizeof(uint16_t));
}

bool OctDevice::UsbComm::ScannerFaultParameters(std::uint16_t timer, std::uint16_t thresh)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::SCAN_FAULT_PARAM, 4);
	msg->packet.ctrl6.s1 = timer;
	msg->packet.ctrl6.s2 = thresh;
	attachCRC(msg->packet.ctrl6.crc);
	return sendMsgCmd(msg);
}

bool OctDevice::UsbComm::ChinrestMove(std::uint8_t dir)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::CR_MOVE, 2);
	msg->packet.ctrl2.c1 = dir;
	msg->packet.ctrl2.c2 = 0;
	attachCRC(msg->packet.ctrl2.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::ChinrestStop(void)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::CR_STOP, 1);
	msg->packet.ctrl1.c1 = 0;
	attachCRC(msg->packet.ctrl1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::IrCameraControl(std::uint8_t cid, std::uint8_t onoff)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::IRCAM, 2);
	msg->packet.ctrl2.c1 = cid;
	msg->packet.ctrl2.c2 = onoff;
	attachCRC(msg->packet.ctrl2.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::IrCameraDigitalGain(std::uint8_t cid, std::uint8_t gain)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::IRCAM_DGAIN, 2);
	msg->packet.ctrl2.c1 = cid;
	msg->packet.ctrl2.c2 = gain;
	attachCRC(msg->packet.ctrl2.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::IrCameraAnalogGain(std::uint8_t cid, std::uint8_t gain)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::IRCAM_AGAIN, 2);
	msg->packet.ctrl2.c1 = cid;
	msg->packet.ctrl2.c2 = gain;
	attachCRC(msg->packet.ctrl2.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::IrCameraExposureTime(std::uint8_t cid, std::uint16_t ints)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::IRCAM_INT, 3);
	msg->packet.ctrl7.c1 = cid;
	msg->packet.ctrl7.s1 = ints;
	attachCRC(msg->packet.ctrl7.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::TestCommand(std::uint8_t cmd)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::TEST_CMD, 1);
	msg->packet.ctrl1.c1 = cmd;
	attachCRC(msg->packet.ctrl1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::LedSetIntensity(LightLedType type, std::uint8_t value)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(getLightLedCommandType(type), 1);
	msg->packet.ctrl1.c1 = value;
	attachCRC(msg->packet.ctrl1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::OctSldUpdateParameters(std::uint8_t channel)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::FLASH_SLD_POT, 1);
	msg->packet.ctrl1.c1 = channel;
	attachCRC(msg->packet.ctrl1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::OctSldGetParameters(std::uint8_t channel)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::GET_SLD_POT, 1);
	msg->packet.ctrl1.c1 = channel;
	attachCRC(msg->packet.ctrl1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::OctSldPotentiometer(std::uint8_t channel, std::uint16_t data)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::SLD_POTENTIO, 3);
	msg->packet.ctrl7.c1 = channel;
	msg->packet.ctrl7.s1 = data;
	attachCRC(msg->packet.ctrl7.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::FlashLedContinous(std::uint32_t baseAddr, std::uint16_t flag)
{
	uint32_t addr = baseAddr + 34;
	return writeAddress(addr, (uint8_t*)&flag, sizeof(uint16_t));
}


bool OctDevice::UsbComm::QuickReturnMirror(std::uint8_t inout)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::QRTNM, 1);
	msg->packet.ctrl1.c1 = inout;
	attachCRC(msg->packet.ctrl1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::SplitFocusMirror(std::uint8_t inout)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::SPLIT_FM, 1);
	msg->packet.ctrl1.c1 = inout;
	attachCRC(msg->packet.ctrl1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::DiopterCompensationLens(std::uint8_t focus_sel, std::uint8_t lens_id)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::FDCS_LENS, 2);
	msg->packet.ctrl2.c1 = focus_sel;
	msg->packet.ctrl2.c2 = lens_id;
	attachCRC(msg->packet.ctrl2.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::PupilMaskSelect(std::uint8_t mask_id)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::PUPIL_MASK, 1);
	msg->packet.ctrl1.c1 = mask_id;
	attachCRC(msg->packet.ctrl1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::FundusCameraCapture(void)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::FC_CAPTURE, 0);
	attachCRC(msg->packet.ctrl0.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::InternalFixationControl(std::uint8_t row, std::uint8_t col)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::INTFIXA, 2);
	msg->packet.ctrl2.c1 = row;
	msg->packet.ctrl2.c2 = col;
	attachCRC(msg->packet.ctrl2.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::LcdFixationControl(std::uint8_t row, std::uint8_t col)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::LCDFIXA, 2);
	msg->packet.ctrl2.c1 = row;
	msg->packet.ctrl2.c2 = col;
	attachCRC(msg->packet.ctrl2.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::LcdFixationParameters(std::uint8_t bright, std::uint8_t blink, std::uint16_t period, std::uint16_t ontime, std::uint8_t type)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::LCDFIXPARAM, 7);
	msg->packet.ctrl8.c1 = bright;
	msg->packet.ctrl8.c2 = blink;
	msg->packet.ctrl8.s1 = period;
	msg->packet.ctrl8.s2 = ontime;
	msg->packet.ctrl8.c3 = type;
	attachCRC(msg->packet.ctrl8.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::UpdateDeviceStatus(std::uint8_t dev_id)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::UPDATE_DEV_STATUS, 1);
	msg->packet.ctrl1.c1 = dev_id;
	attachCRC(msg->packet.ctrl1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::GalvanoMoveX(short x)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::GALVO_MOVEX, 2);
	msg->packet.galv1.s1 = x;
	attachCRC(msg->packet.galv1.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::GalvanoSlewXY(short x, short y)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::GALVO_SLEWXY, 4);
	msg->packet.galv2.s1 = x;
	msg->packet.galv2.s2 = y;
	attachCRC(msg->packet.galv2.crc);
	return sendMsgCmd(msg);
}


bool OctDevice::UsbComm::GalvanoDynamicOffsetWrite(std::uint32_t baseAddr, std::int16_t offsetX, std::int16_t offsetY)
{
	uint32_t addr = baseAddr;
	Galvano_Dynamic_st galv;
	galv.offsetX = offsetX;
	galv.offsetY = offsetY;
	return writeAddress(addr, (uint8_t*)&galv, sizeof(Galvano_Dynamic_st));
}


bool OctDevice::UsbComm::GalvanoDynamicOffsetRead(std::uint32_t baseAddr, std::int16_t & offsetX, std::int16_t & offsetY)
{
	uint32_t addr = baseAddr;
	Galvano_Dynamic_st galv;
	if (readAddress(addr, (uint8_t*)&galv, sizeof(Galvano_Dynamic_st))) {
		offsetX = galv.offsetX;
		offsetY = galv.offsetY;
		return true;
	}
	return false;
}

bool OctDevice::UsbComm::GalvanoSlewRate(std::uint32_t baseAddr, std::uint32_t rate)
{
	uint32_t addr = baseAddr + 4;
	return writeAddress(addr, (uint8_t*)&rate, sizeof(uint32_t));
}

bool OctDevice::UsbComm::GalvanoAutoSlew(std::uint32_t baseAddr, std::uint16_t flag)
{
	uint32_t addr = baseAddr + 2;
	return writeAddress(addr, (uint8_t*)&flag, sizeof(uint16_t));
}


bool OctDevice::UsbComm::GalvanoScanXY(std::uint16_t sid, std::uint16_t eid)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::START_ACQ, 6);
	msg->packet.scan1.s1 = sid;
	msg->packet.scan1.s2 = eid;
	msg->packet.scan1.s3 = static_cast<uint16_t>(ScanModeType::SCAN_MODE_XY);
	attachCRC(msg->packet.scan1.crc);

	bool reply = (isCommandAsyncMode() ? false : true);
	return sendMsgCmd(msg, reply);
}


bool OctDevice::UsbComm::GalvanoRasterX(std::uint16_t tid, std::uint16_t lines, std::int16_t xoffs, std::int16_t yoffs)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::START_ACQR, 10);
	msg->packet.scan2.s1 = tid;
	msg->packet.scan2.s2 = lines;
	msg->packet.scan2.s3 = static_cast<uint16_t>(ScanModeType::SCAN_MODE_X);
	msg->packet.scan2.s4 = xoffs;
	msg->packet.scan2.s5 = yoffs;
	attachCRC(msg->packet.scan2.crc);

	bool reply = (isCommandAsyncMode() ? false : true);
	return sendMsgCmd(msg, reply);
}


bool OctDevice::UsbComm::GalvanoRasterY(std::uint16_t tid, std::uint16_t lines, std::int16_t xoffs, std::int16_t yoffs)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::START_ACQR, 10);
	msg->packet.scan2.s1 = tid;
	msg->packet.scan2.s2 = lines;
	msg->packet.scan2.s3 = static_cast<uint16_t>(ScanModeType::SCAN_MODE_Y);
	msg->packet.scan2.s4 = xoffs;
	msg->packet.scan2.s5 = yoffs;
	attachCRC(msg->packet.scan2.crc);

	bool reply = (isCommandAsyncMode() ? false : true);
	return sendMsgCmd(msg, reply);
}


bool OctDevice::UsbComm::GalvanoRasterXY(std::uint16_t tid, std::uint16_t lines, std::int16_t xoffs, std::int16_t yoffs)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::START_ACQR, 10);
	msg->packet.scan2.s1 = tid;
	msg->packet.scan2.s2 = lines;
	msg->packet.scan2.s3 = static_cast<uint16_t>(ScanModeType::SCAN_MODE_XY);
	msg->packet.scan2.s4 = xoffs;
	msg->packet.scan2.s5 = yoffs;
	attachCRC(msg->packet.scan2.crc);

	bool reply = (isCommandAsyncMode() ? false : true);
	return sendMsgCmd(msg, reply);
}


bool OctDevice::UsbComm::GalvanoRasterFastX(std::uint16_t tid, std::uint16_t lines, std::int16_t xoffs, std::int16_t yoffs)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::START_ACQRF, 10);
	msg->packet.scan2.s1 = tid;
	msg->packet.scan2.s2 = lines;
	msg->packet.scan2.s3 = static_cast<uint16_t>(ScanModeType::SCAN_MODE_X);
	msg->packet.scan2.s4 = xoffs;
	msg->packet.scan2.s5 = yoffs;
	attachCRC(msg->packet.scan2.crc);

	bool reply = (isCommandAsyncMode() ? false : true);
	return sendMsgCmd(msg, reply);
}


bool OctDevice::UsbComm::GalvanoRasterFastY(std::uint16_t tid, std::uint16_t lines, std::int16_t xoffs, std::int16_t yoffs)
{
	lock_guard<mutex> lock(getImpl().mutexControl);
	MsgCommand* msg = getMsgCommand(CommandType::START_ACQRF, 10);
	msg->packet.scan2.s1 = tid;
	msg->packet.scan2.s2 = lines;
	msg->packet.scan2.s3 = static_cast<uint16_t>(ScanModeType::SCAN_MODE_Y);
	msg->packet.scan2.s4 = xoffs;
	msg->packet.scan2.s5 = yoffs;
	attachCRC(msg->packet.scan2.crc);

	bool reply = (isCommandAsyncMode() ? false : true);
	return sendMsgCmd(msg, reply);
}


bool OctDevice::UsbComm::TrajectoryParams(std::uint8_t tid, std::uint32_t baseAddr, TrajectoryProfileParams* params)
{
	uint32_t addr = baseAddr + tid * sizeof(TrajectoryProfile);
	return writeAddress(addr, (uint8_t*)params, sizeof(TrajectoryProfileParams));
}


bool OctDevice::UsbComm::TrajectoryPositionsX(std::uint8_t tid, std::uint32_t baseAddr, std::int16_t* positions, std::uint16_t count)
{
	uint32_t addr = baseAddr + tid * sizeof(TrajectoryProfile) + sizeof(TrajectoryProfileParams);
	return writeAddress(addr, (uint8_t*)positions, count * sizeof(int16_t));
}


bool OctDevice::UsbComm::TrajectoryPositionsY(std::uint8_t tid, std::uint32_t baseAddr, std::int16_t* positions, std::uint16_t count)
{
	uint32_t addr = baseAddr + tid * sizeof(TrajectoryProfile) + sizeof(TrajectoryProfileParams) + sizeof(int16_t) * TRAJECT_SAMPLE_SIZE_MAX;
	return writeAddress(addr, (uint8_t*)positions, count * sizeof(int16_t));
}


UsbComm::UsbCommImpl & OctDevice::UsbComm::getImpl(void) const
{
	return *d_ptr;
}


CommandType OctDevice::UsbComm::getLightLedCommandType(LightLedType type)
{
	switch (type) {
	case LightLedType::RETINA_IR:
		return CommandType::IRLED;
	case LightLedType::CORNEA_IR:
		return CommandType::COR740_LED;
	case LightLedType::KER_RING:
		return CommandType::RNGELED;
	case LightLedType::KER_FOCUS:
		return CommandType::RNGFLED;
	case LightLedType::FUNDUS_FLASH:
		return CommandType::FLASH_LED;
	case LightLedType::SPLIT_FOCUS:
		return CommandType::SPFOCUS_LED;
	case LightLedType::EXT_FIXATION:
		return CommandType::EXTFIXA;
	case LightLedType::WORKING_DOTS:
		return CommandType::WDOT_LED;
	case LightLedType::WORKING_DOT2:
		return CommandType::WDOT_LED2;
	case LightLedType::OCT_SLD:
		return CommandType::SLD_CTL;
	case LightLedType::PANNEL:
		return CommandType::PNL_LED;
	}
	return CommandType::UNKNOWN;
}


unsigned char OctDevice::UsbComm::getNextPacketID(void)
{
	static unsigned char pktId = PACKET_ID_START;
	unsigned char curId = pktId;

	pktId++;
	if (pktId > PACKET_ID_END) {
		pktId = PACKET_ID_START;
	}
	return curId;
}


void OctDevice::UsbComm::attachCRC(std::uint8_t * addr, std::uint32_t offset)
{
	memset(addr + offset, PACKET_CRC_CODE, PACKET_CRC_SIZE);
	return;
}


void OctDevice::UsbComm::attachCRC(std::uint8_t * addr)
{
	memset(addr, PACKET_CRC_CODE, PACKET_CRC_SIZE);
	return;
}


UsbPort * OctDevice::UsbComm::getUsbPort(void) const
{
	return &getImpl().usbPort;
}


ReadRequest * OctDevice::UsbComm::getReadRequest(std::uint32_t addr, std::uint16_t rd_size)
{
	ReadRequest* req = &getImpl().readReq;
	req->packet.nread = static_cast<std::uint8_t>(PacketType::NREAD);
	req->packet.pkt_id = getNextPacketID();
	req->packet.rd_size = (rd_size == 256 ? 0 : rd_size);
	memcpy(req->packet.addr, &addr, sizeof(uint32_t));

	attachCRC(req->packet.crc);
	req->size = sizeof(NREAD_REQ);
	return req;
}


ReadResponse * OctDevice::UsbComm::getReadResponse(std::uint16_t rd_size)
{
	ReadResponse* res = &getImpl().readResp;
	res->packet.type = 0;
	res->packet.pkt_id = 0;
	res->size = sizeof(NREAD_RES) - (PACKET_DATA_SIZE_MAX - rd_size);
	return res;
}


WriteRequest * OctDevice::UsbComm::getWriteRequest(std::uint32_t addr, std::uint8_t * data, std::uint16_t size, bool reply)
{
	WriteRequest* req = &getImpl().writeReq;
	req->packet.pkt_id = getNextPacketID();
	req->packet.wr_size = (size == 256 ? 0 : size);
	memcpy(req->packet.addr, &addr, sizeof(uint32_t));
	memcpy(req->packet.data, data, size);

	if (reply) {
		req->packet.nwrite = static_cast<uint8_t>(PacketType::NWRITE_R);
	}
	else {
		req->packet.nwrite = static_cast<uint8_t>(PacketType::NWRITE);
	}

	attachCRC(req->packet.data, size);
	req->size = (11 + size);
	return req;
}


WriteResponse * OctDevice::UsbComm::getWriteResponse(void)
{
	WriteResponse* res = &getImpl().writeResp;
	res->packet.type = 0;
	res->packet.pkt_id = 0;
	res->packet.status = 0;
	res->size = sizeof(NWRITE_RES);
	return res;
}


MsgCommand * OctDevice::UsbComm::getMsgCommand(CommandType ctrl, std::uint8_t msg_len)
{
	MsgCommand* msg = &getImpl().msgCmd;
	msg->packet.hdr.type = static_cast<std::uint8_t>(PacketType::MSG_CMD);
	msg->packet.hdr.pkt_id = getNextPacketID();
	msg->packet.hdr.msg_len = msg_len;
	msg->packet.hdr.ctrl = static_cast<std::uint16_t>(ctrl);
	msg->size = sizeof(MSG_CMD_HEAD) + msg_len + PACKET_CRC_SIZE;
	return msg;
}

