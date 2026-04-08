#pragma once

#include "CppUtilDef.h"

#include <string>
#include <memory>


namespace CppUtil
{
	class CPPUTIL_DLL_API SerialPort
	{
	public:
		SerialPort();
		virtual ~SerialPort();

		SerialPort(SerialPort&& rhs);
		SerialPort& operator=(SerialPort&& rhs);
		// SerialPort(const SerialPort& rhs);
		//SerialPort& operator=(const SerialPort& rhs);

	public:
		bool open(std::uint32_t portNum = 0, std::uint32_t baudRate = 115200, std::uint32_t timeout = 500);
		bool isOpen(void) const;
		void close(void);

		bool read(std::string& data, std::uint32_t size);
		bool read(std::uint8_t* data, std::uint32_t size);
		bool write(std::string data);
		bool write(const std::uint8_t* data, std::uint32_t size);

	private:
		struct SerialPortImpl;
		std::unique_ptr<SerialPortImpl> d_ptr;
	};
}
