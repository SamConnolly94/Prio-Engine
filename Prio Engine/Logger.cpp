#include "Logger.h"

CLogger::CLogger()
{
	mLogFile.open(mDebugLogName);

	mMemoryLogFile.open(mMemoryLogName);

	// Check to see if logging was successfully enabled or not.
#ifdef _LOGGING_ENABLED
	mLoggingEnabled = true;

	if (mLogFile.is_open())
	{
		// Set our private boolean flag which toggles logging to on.
		mLoggingEnabled = true;

		// Initialise our line number to be 0.
		mLineNumber = 0;

		// Output a message to the log.
		WriteLine("Successfully opened " + mDebugLogName);
	}
	else
	{
		// Set our private boolean flag which toggles logging to off.
		mLoggingEnabled = false;

		// Output error message to user.
		MessageBox(NULL, L"Could not open the debug log to write to it, make sure you haven't left it open. The program will continue to run but without logging.", L"Could not open debug log!", MB_OK);

	}

	if (mMemoryLogFile.is_open() && mLoggingEnabled)
	{
		mMemoryLogLineNumber = 0;

		WriteLine("Successfully opened " + mMemoryLogName);
	}
	else
	{
		MessageBox(NULL, L"Could not open the memory log to write to it, make sure you haven't left it open. The program will continue to run but without logging.", L"Could not open memory log!", MB_OK);

	}

#endif

}

CLogger & CLogger::GetLogger()
{
	// A logging class which can be used to write files to a text document.
	static CLogger instance;

	return instance;
}

/**  Write a piece of text to the debug log and add a new line.. */
void CLogger::WriteLine(std::string text)
{
	if (mLoggingEnabled)
	{
		// Increment our line number.
		mLineNumber++;

		// Check that the log file we want to write to is open and available.
		if (mLogFile.is_open()) 
		{
			mLogFile << std::setfill('0') << std::setw(5) << mLineNumber << "  " << text << std::endl;
		}
		else 
		{
			MessageBox(0, L"Log is not open or cannot be written to so the WriteLine of the logger has failed. ", L"Log not open!", MB_OK);
		}
	}
}


/**  Write a piece of text to the debug log and add a new line.. */
void CLogger::MemoryAllocWriteLine(std::string name)
{
	if (mLoggingEnabled)
	{
		// Increment our line number.
		mMemoryLogLineNumber++;

		// Check that the log file we want to write to is open and available.
		if (mMemoryLogFile.is_open())
		{
			mMemoryLogFile << std::setfill('0') << std::setw(5) << mMemoryLogLineNumber << " Variable of type " << name << " was allocated memory." << std::endl;
		}
		else
		{
			MessageBox(0, L"Memory log is not open or cannot be written to so the WriteLine of the logger has failed. ", L"Memory log not opened!", MB_OK);
		}
	}
}

/**  Write a piece of text to the debug log and add a new line.. */
void CLogger::MemoryDeallocWriteLine(std::string name)
{
	if (mLoggingEnabled)
	{
		// Increment our line number.
		mMemoryLogLineNumber++;

		// Check that the log file we want to write to is open and available.
		if (mMemoryLogFile.is_open())
		{
			mMemoryLogFile << std::setfill('0') << std::setw(5) << mMemoryLogLineNumber << " Variable of type " << name << " memory was deallocated." << std::endl;
		}
		else
		{
			MessageBox(0, L"Memory log is not open or cannot be written to so the WriteLine of the logger has failed. ", L"Memory log not opened!", MB_OK);
		}
	}
}

void CLogger::Shutdown()
{
	if (mMemoryLogFile.is_open())
	{
		mMemoryLogFile.close();
	}

	if (mLogFile.is_open())
	{
		mLogFile.close();
	}

	mLineNumber = NULL;
	mMemoryLogLineNumber = NULL;

}