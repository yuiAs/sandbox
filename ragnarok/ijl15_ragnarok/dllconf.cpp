#include "dllconf.hpp"


////////////////////////////////////////////////////////////////////////////////


void DLLConf::finalize()
{
	if (m_iniFile)
		delete [] m_iniFile;
}


void DLLConf::load(const char* filename)
{
	if (m_iniFile == NULL)
		m_iniFile = new char[MAX_PATH];

	const char* s = filename;
	char* d = m_iniFile;

	memset(m_iniFile, 0, MAX_PATH);
	while (*d++ = *s++);
}

////////////////////////////////////////////////////////////////////////////////


bool DLLConf::get_bool(const char* section, const char* key)
{
	return ::GetPrivateProfileInt(section, key, 0, m_iniFile)==1;
}


int DLLConf::get_int(const char* section, const char* key)
{
	return ::GetPrivateProfileInt(section, key, 0, m_iniFile);
}


double DLLConf::get_double(const char* section, const char* key)
{
	return static_cast<double>(::GetPrivateProfileInt(section, key, 0, m_iniFile));
}


size_t DLLConf::get_string(const char* section, const char* key, char* buf, size_t buflen)
{
	return ::GetPrivateProfileString(section, key, "", buf, buflen, m_iniFile);
}
