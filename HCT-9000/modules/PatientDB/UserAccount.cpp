#include "stdafx.h"
#include "UserAccount.h"

using namespace PatientDB;


UserAccount::UserAccount()
{
	m_index = INVALID_INDEX;
	m_userId.clear();
	m_password.clear();
	m_permission = Permission::NOUSE;
	m_createTime = COleDateTime::GetCurrentTime();
	m_loginTime;
	m_logoutTime;
	m_emailAddress;
	m_status;
}

UserAccount::~UserAccount()
{
}

void UserAccount::setIndex(long idx)
{
	m_index = idx;
	return;
}

void UserAccount::setUserId(wstring userId)
{
	m_userId = userId;
	return;
}

void UserAccount::setPassword(wstring password)
{
	m_password = password;
	return;
}

void UserAccount::setPermission(Permission permission)
{
	m_permission = permission;
	return;
}

void UserAccount::setCreateTime(COleDateTime time)
{
	m_createTime = time;
	return;
}

void UserAccount::setLoginTime(COleDateTime time)
{
	m_loginTime = time;
	return;
}

void UserAccount::setLogoutTime(COleDateTime time)
{
	m_logoutTime = time;
	return;
}

void UserAccount::setEmailAddress(wstring email)
{
	m_emailAddress = email;
	return;
}

void UserAccount::setStatus(Status status)
{
	m_status = status;
	return;
}