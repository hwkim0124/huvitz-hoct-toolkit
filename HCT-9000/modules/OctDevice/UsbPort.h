#pragma once

#include "OctDeviceDef.h"

#include <memory>
#include <string>


namespace OctDevice
{
	class OCTDEVICE_DLL_API UsbPort
	{
	public:
		UsbPort();
		virtual ~UsbPort();

		UsbPort(UsbPort&& rhs);
		UsbPort& operator=(UsbPort&& rhs);

		// Prevent copy construction and assignment. 
		UsbPort(const UsbPort& rhs) = delete;
		UsbPort& operator=(const UsbPort& rhs) = delete;

	public:
		bool open(const std::string &desc = USB_BOARD_DESC, unsigned long timeout = USB_TIMEOUT,
					unsigned char readPid = USB_READ_PID, unsigned char writePid = USB_WRITE_PID, 
					unsigned char readIr1 = USB_IR1_PID, unsigned char readIr2 = USB_IR2_PID);
		void close(void);
		void clear(void);

		bool isOpened(void) const;
		bool isAvailable(unsigned char pid) const;
		void setPipeError(unsigned char pid, bool flag);
		bool isPipeError(unsigned char pid) const;

		PVOID getHandle(void) const;
		void setTimeout(unsigned long timeout);
		void setReadPipeID(unsigned char pid);
		void setWritePipeID(unsigned char pid);

		void abortPipe(unsigned char pid);

		bool receive(unsigned char* buff, unsigned long size, bool check = true);
		bool receiveFromIr1(unsigned char* buff, unsigned long size, bool check = true);
		bool receiveFromIr2(unsigned char* buff, unsigned long size, bool check = true);
		bool receive(unsigned char epid, unsigned char* buff, unsigned long size, bool check = true);
		bool receive2(unsigned char epid, unsigned char* buff, unsigned long size, bool check = true);

		bool send(unsigned char* buff, unsigned long size, bool check=true);

	private:
		struct UsbPortImpl;
		std::unique_ptr<UsbPortImpl> d_ptr;
		UsbPortImpl& getImpl(void) const;

		int getIndexOfPipe(unsigned char pid) const;

	};
}
