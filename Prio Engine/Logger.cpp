#include "Logger.h"

/** Constructor for singleton logger class. */
CLogger::CLogger()
{
	mLogFile.open(mDebugLogName);

	// Check to see if logging was successfully enabled or not.
#ifdef _LOGGING_ENABLED
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
#endif
}

/** Destructor to free the debug log for others to use. */
CLogger::~CLogger() 
{
	// Check if logging is enabled.
	if (mLoggingEnabled)
	{
		// Check if the log file is open.
		if (mLogFile.is_open())
		{
			// Write one final message to let the user know that the debug log has been closed.
			Write("Logger destructor has been called, closing log.");

			// Close the debug log.
			mLogFile.close();
		}
	}
}

/**  Write a piece of text to the debug log. */
void CLogger::Write(std::string text)
{
	// Check that logging is enabled via the preprocessor command.
	if (mLoggingEnabled)
	{
		// Increment our line number.
		mLineNumber++;

		// Check that the log file we want to write to is open and available.
		if (mLogFile.is_open()) 
		{
			mLogFile << std::setfill('0') << std::setw(5) << mLineNumber << "  " << text;
		}
		else
		{
			MessageBox(0, L"Log is not open or cannot be written to so the WriteLine of the logger has failed. ", L"Log not open!", MB_OK);
		}
	}
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
