#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API UserAccount
	{
	public:
		UserAccount();
		virtual ~UserAccount();

	private:
		long			m_index;
		wstring			m_userId;
		wstring			m_password;
		Permission		m_permission;
		COleDateTime	m_createTime;
		COleDateTime	m_loginTime;
		COleDateTime	m_logoutTime;
		wstring			m_emailAddress;
		Status			m_status;

	public:
		long			getIndex(void) { return m_index; }
		wstring			getUserId(void) { return m_userId; }
		wstring			getPassword(void) { return m_password; }
		Permission		getPermission(void) { return m_permission; }
		COleDateTime	getCreateTime(void) { return m_createTime; }
		COleDateTime	getLoginTime(void) { return m_loginTime; }
		COleDateTime	getLogoutTime(void) { return m_logoutTime; }
		wstring			getEmailAddress(void) { return m_emailAddress; }
		Status			getStatus(void) { return m_status; }

		void			setIndex(long idx);
		void			setUserId(wstring userId);
		void			setPassword(wstring password);
		void			setPermission(Permission permission);
		void			setCreateTime(COleDateTime time);
		void			setLoginTime(COleDateTime time);
		void			setLogoutTime(COleDateTime time);
		void			setEmailAddress(wstring email);
		void			setStatus(Status status);
	};
}
