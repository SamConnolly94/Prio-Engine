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

// The line number which will preceed the log line in the text file.
static int mLineNumber;

class CLogger
{
/* Signleton class methods. */
public:
	static CLogger& GetLogger();
private:
	// Constructor.
	CLogger();
public:
	// Delete the object prevent memory leaks.
	CLogger(CLogger const&) = delete;
	void operator = (CLogger const&) = delete;
private:
	// A boolean flag which is toggled on / off depending on if _LOGGING_ENABLED is defined by the preprocessor and successfully opening the log file.
	bool mLoggingEnabled;

	// The log file which we will write debug info to.
	std::ofstream mLogFile;

	const std::string mDebugLogName = "Prio_Engine_Debug_Log.txt";
public:
	void Write(std::string text);
	void WriteLine(std::string text);
};

#endif