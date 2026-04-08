#include "stdafx.h"
#include "Device.h"

using namespace PatientDB;


Device::Device()
{
	m_index = INVALID_INDEX;
	m_name.clear();
	m_description.clear();
	m_ipAddress.clear();
	m_port = 0;
	m_status = Status::USE;
}

Device::~Device()
{

}

void Device::setName(wstring name)
{
	m_name = name;
	return;
}

void Device::setIndex(long idx)
{
	m_index = idx;
	return;
}

void Device::setDescription(wstring desc)
{
	m_description = desc;
	return;
}

void Device::setIpAddress(wstring ipAddress)
{
	m_ipAddress = ipAddress;
	return;
}

void Device::setPort(int port)
{
	m_port = port;
	return;
}

void Device::setStatus(Status status)
{
	m_status = status;
	return;
}
