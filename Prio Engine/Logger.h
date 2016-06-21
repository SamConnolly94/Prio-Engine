#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <iomanip>

#ifdef _DEBUG
#define _LOGGING_ENABLED
#endif

class CLogger
{
private:
	// The log file which we will write debug info to.
	std::ofstream mLogFile;

	// The name of the log file as it will appear in windows.
	const std::string mDebugLogName = "Prio_Engine_Debug_Log.txt";

	// A boolean flag which is toggled on / off depending on if _LOGGING_ENABLED is defined by the preprocessor and successfully opening the log file.
	bool mLoggingEnabled;

	// The line number which will preceed the log line in the text file.
	int mLineNumber;
public:
	CLogger();
	~CLogger();
	void Write(std::string text);
	void WriteLine(std::string text);
};

#endif