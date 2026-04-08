#include "stdafx.h"
#include "SerialPort.h"

#include "serial\serial.h"

using namespace CppUtil;
using namespace std;


struct SerialPort::SerialPortImpl
{
	serial::Serial sport;

};


SerialPort::SerialPort() :
	d_ptr(make_unique<SerialPortImpl>())
{
}


CppUtil::SerialPort::~SerialPort() = default;
CppUtil::SerialPort::SerialPort(SerialPort && rhs) = default;
SerialPort & CppUtil::SerialPort::operator=(SerialPort && rhs) = default;



bool CppUtil::SerialPort::open(std::uint32_t portNum, std::uint32_t baudRate, std::uint32_t timeout)
{
	string name = "COM" + to_string(portNum);

	d_ptr->sport.setPort(name);
	d_ptr->sport.setBaudrate(baudRate);
	d_ptr->sport.setTimeout(0, timeout, 0, timeout, 0);

	bool res = false;
	try {
		d_ptr->sport.open();
		res = d_ptr->sport.isOpen();
	}
	catch (...) {
	}
	return res;
}


bool CppUtil::SerialPort::isOpen(void) const
{
	return d_ptr->sport.isOpen();
}


void CppUtil::SerialPort::close(void)
{
	d_ptr->sport.close();
	return;
}


bool CppUtil::SerialPort::read(std::string & data, std::uint32_t size)
{
	size_t bytes = 0;
	try {
		bytes = d_ptr->sport.read(data, size);
	}
	catch (...) {
	}
	return (bytes == size);
}


bool CppUtil::SerialPort::read(std::uint8_t * data, std::uint32_t size)
{
	size_t bytes = 0;
	try {
		bytes = d_ptr->sport.read(data, size);
	}
	catch (...) {
	}
	return (bytes == size);
}


bool CppUtil::SerialPort::write(std::string data)
{
	size_t bytes = 0;
	try {
		bytes = d_ptr->sport.write(data);
	}
	catch (...) {
	}
	return (bytes == data.length());
}


bool CppUtil::SerialPort::write(const std::uint8_t * data, std::uint32_t size)
{
	size_t bytes = 0;
	try {
		bytes = d_ptr->sport.write(data, size);
	}
	catch (...) {
	}
	return (bytes == size);
}


