#pragma once

#ifndef SYSTEM_H
#define SYSTEM_H

#include "Graphics.h"
#include "Input.h"
#include <windows.h>

class CEngine
{
private:
	// Logger which writes to a default log file. Can be accessed through CLogger->GetInstance()->Write();
	CLogger* mpLogger;
public:
	// Default constructor.
	CEngine();
	// Default destructor.
	~CEngine();

	// Attempts to instialse Direct X within the window.
	bool Initialise();
	// Handle what happens when the window is closed and engine is told to stop.
	void Shutdown();
	// Runs the engine.
	void Run();

	// Control what happens when we recieve different messages from the operating system.
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
private:
	bool Frame();
	void InitialiseWindows(int&, int&);
	void ShutdownWindows();
private:
	// The name of our application as it will appear in windows.
	LPCWSTR mApplicationName;
	// The instance of our application as reffered to by windows.
	HINSTANCE mHinstance;
	// The handle to the window of our application.
	HWND mHwnd;

	CInput* mInput;
	CGraphics* mGraphics;

	void CheckWindowsMessages(MSG &msg);
	bool IsComplete(MSG msg);
};

// Define WndProc and the application handle pointer here so that we can re-direct the windows system messaging into our message handler 
// function inside the system class.
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static CEngine* ApplicationHandle = 0;

#endif