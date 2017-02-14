#include "LogFile.h"
#include <iostream>

DebugLogFile::DebugLogFile() :
	LogFile()
{
}

DebugLogFile::~DebugLogFile()
{
	if (m_FileStream.is_open())
	{
		WRITE_LOG("Closing log file", "good");
		m_FileStream.close();
	}
}

bool DebugLogFile::CreateLogFile(const std::string& path, const std::string& name)
{
	this->m_Filename = name;
	m_FileStream.open(path + this->m_Filename, std::ios::out);

	if ( !m_FileStream.fail() )
	{
		m_FileStream << "<head><style>.error{color:red;}.warning{color:orange;}.good{color:green;}.none{color:black;}</style></head><h1>Log File</h1><hr>";
		WRITE_LOG("Successfully opened debug log file", "good");
	}
	else
	{
		WRITE_LOG("Unable to open debug log file", "error");
		return false;
	}

	return true;
}

void DebugLogFile::WriteLog(const std::string& log, const std::string& extra)
{
	std::cout << log << "\n";

	std::string log_out = "  " + log;
	std::string html_wrap = "<div class=\"" + extra + "\">" + std::to_string(++m_LogCount) + " : " + log_out + "</div>";

	if (m_FileStream.is_open())
	{
		m_FileStream << html_wrap;
	}
	else
	{
		m_FileStream.open("res/logs/" + this->m_Filename, std::ios::app);
		if ( !m_FileStream.fail() )
		{
			m_FileStream << html_wrap;
		}
	}

}