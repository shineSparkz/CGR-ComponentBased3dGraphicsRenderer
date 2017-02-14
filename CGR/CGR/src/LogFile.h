#ifndef __LOG_FILE_H__
#define __LOG_FILE_H__

#include "Singleton.h"
#include <string>
#include <fstream>
#include <sstream>

#define WRITE_LOG(msg, ext)\
{\
std::stringstream s;\
s << __DATE__ << ", " << __TIME__ << " : " << (const std::string&)(msg) << ", Line: " << __LINE__ << ", File:" << __FILE__;\
Write_log(((const std::string&)(s.str())), ((const std::string&)(ext)));\
}

class LogFile
{
public:
	LogFile(){}
	virtual ~LogFile(){}
	virtual bool CreateLogFile(const std::string& path, const std::string& name ) = 0;
	virtual void WriteLog(const std::string& msg, const std::string& extra = "none") = 0;

protected:
	int m_LogCount = 0;
	std::string m_Filename;
};

// This needs to be on another thread
class DebugLogFile : public LogFile, public Singleton<DebugLogFile>
{
	friend void Write_log(const std::string& msg, const std::string& extra = "none");
public:
	DebugLogFile();
	virtual ~DebugLogFile();

	bool CreateLogFile(const std::string& path, const std::string& name) override;
	void WriteLog(const std::string& log, const std::string& extra = "none" ) override;

private:
	std::ofstream m_FileStream;
};

inline void Write_log(const std::string& msg, const std::string& extra)
{
	DebugLogFile::Instance()->WriteLog(msg, extra);
}

#endif

